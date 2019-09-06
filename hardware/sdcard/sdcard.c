
// Marcel Timm, RhinoDevel, 2019sep04

// *****************************************************************************
// Originally taken from:
//
// https://raw.githubusercontent.com/moizumi99/RPiHaribote/master/haribote/sdcard.c
// *****************************************************************************

// Taken from here
// https://www.raspberrypi.org/forums/viewtopic.php?f=72&t=94133
// Update: Control2 is removed according the the advice from LdB. Thanks a lot.
//
// Notes:
// VOLTAGE_SWITCH: Haven't yet got this to work.  Command appears to work but dat0-3 go low and stay there.
// Data transfer notes:
// The EMMC module restricts the maximum block size to the size of the internal data FIFO which is 1k bytes
// 0x80  Extension FIFO config - what's that?
// This register allows fine tuning the dma_req generation for paced DMA transfers when reading from the card.

#include "../../lib/mem/mem.h"
#include "../mailbox/mailbox.h"
#include "../armtimer/armtimer.h"
#include "../baregpio/baregpio.h"
#ifndef NDEBUG
    #include "../../lib/console/console.h"
#endif //NDEBUG

#include "sdcard.h"

// GPIO pins used:
//
#define GPIO_DAT3  53
#define GPIO_DAT2  52
#define GPIO_DAT1  51
#define GPIO_DAT0  50
#define GPIO_CMD   49
#define GPIO_CLK   48

// EMMC command flags used:
//
#define CMD_IS_DATA      0x00200000
#define CMD_RSPNS_NO     0x00000000
#define CMD_RSPNS_136    0x00010000
#define CMD_RSPNS_48     0x00020000
#define CMD_RSPNS_48B    0x00030000
#define TM_MULTI_BLOCK   0x00000020
#define TM_DAT_DIR_HC    0x00000000
#define TM_DAT_DIR_CH    0x00000010
#define TM_AUTO_CMD12    0x00000004
#define TM_BLKCNT_EN     0x00000002
#define TM_MULTI_DATA    (CMD_IS_DATA | TM_MULTI_BLOCK | TM_BLKCNT_EN)

// Interrupt register settings used:
//
#define INT_AUTO_ERROR   0x01000000
#define INT_DATA_END_ERR 0x00400000
#define INT_DATA_CRC_ERR 0x00200000
#define INT_DATA_TIMEOUT 0x00100000
#define INT_INDEX_ERROR  0x00080000
#define INT_END_ERROR    0x00040000
#define INT_CRC_ERROR    0x00020000
#define INT_CMD_TIMEOUT  0x00010000
#define INT_ERR          0x00008000
#define INT_READ_RDY     0x00000020
#define INT_WRITE_RDY    0x00000010
#define INT_DATA_DONE    0x00000002
#define INT_CMD_DONE     0x00000001
#define INT_ERROR_MASK   (INT_CRC_ERROR|INT_END_ERROR|INT_INDEX_ERROR| \
                          INT_DATA_TIMEOUT|INT_DATA_CRC_ERR|INT_DATA_END_ERR| \
                          INT_ERR|INT_AUTO_ERROR)
#define INT_ALL_MASK     (INT_CMD_DONE|INT_DATA_DONE|INT_READ_RDY|INT_WRITE_RDY|INT_ERROR_MASK)

// CONTROL register settings:

#define C0_SPI_MODE_EN   0x00100000
#define C0_HCTL_HS_EN    0x00000004
#define C0_HCTL_DWITDH   0x00000002

#define C1_SRST_DATA     0x04000000
#define C1_SRST_CMD      0x02000000
#define C1_SRST_HC       0x01000000
#define C1_TOUNIT_DIS    0x000f0000
#define C1_TOUNIT_MAX    0x000e0000
#define C1_CLK_GENSEL    0x00000020
#define C1_CLK_EN        0x00000004
#define C1_CLK_STABLE    0x00000002
#define C1_CLK_INTLEN    0x00000001

#define FREQ_SETUP           400000  // 400 Khz
#define FREQ_NORMAL        25000000  // 25 Mhz

// CONTROL2 values
#define C2_VDD_18        0x00080000
#define C2_UHSMODE       0x00070000
#define C2_UHS_SDR12     0x00000000
#define C2_UHS_SDR25     0x00010000
#define C2_UHS_SDR50     0x00020000
#define C2_UHS_SDR104    0x00030000
#define C2_UHS_DDR50     0x00040000

// SLOTISR_VER values
#define HOST_SPEC_NUM              0x00ff0000
#define HOST_SPEC_NUM_SHIFT        16
#define HOST_SPEC_V3               2
#define HOST_SPEC_V2               1
#define HOST_SPEC_V1               0

// STATUS register settings
#define SR_DAT_LEVEL1        0x1e000000
#define SR_CMD_LEVEL         0x01000000
#define SR_DAT_LEVEL0        0x00f00000
#define SR_DAT3              0x00800000
#define SR_DAT2              0x00400000
#define SR_DAT1              0x00200000
#define SR_DAT0              0x00100000
#define SR_WRITE_PROT        0x00080000  // From SDHC spec v2, BCM says reserved
#define SR_READ_AVAILABLE    0x00000800  // ???? undocumented
#define SR_WRITE_AVAILABLE   0x00000400  // ???? undocumented
#define SR_READ_TRANSFER     0x00000200
#define SR_WRITE_TRANSFER    0x00000100
#define SR_DAT_ACTIVE        0x00000004
#define SR_DAT_INHIBIT       0x00000002
#define SR_CMD_INHIBIT       0x00000001

// Arguments for specific commands.
// TODO: What's the correct voltage window for the RPi SD interface?
// 2.7v-3.6v (given by 0x00ff8000) or something narrower?
// TODO: For now, don't offer to switch voltage.
#define ACMD41_HCS           0x40000000
#define ACMD41_SDXC_POWER    0x10000000
#define ACMD41_S18R          0x01000000
#define ACMD41_VOLTAGE       0x00ff8000
#define ACMD41_ARG_HC        (ACMD41_HCS|ACMD41_SDXC_POWER|ACMD41_VOLTAGE|ACMD41_S18R)
#define ACMD41_ARG_SC        (ACMD41_VOLTAGE|ACMD41_S18R)

// R1 (Status) values
#define ST_OUT_OF_RANGE      0x80000000  // 31   E
#define ST_ADDRESS_ERROR     0x40000000  // 30   E
#define ST_BLOCK_LEN_ERROR   0x20000000  // 29   E
#define ST_ERASE_SEQ_ERROR   0x10000000  // 28   E
#define ST_ERASE_PARAM_ERROR 0x08000000  // 27   E
#define ST_WP_VIOLATION      0x04000000  // 26   E
#define ST_CARD_IS_LOCKED    0x02000000  // 25   E
#define ST_LOCK_UNLOCK_FAIL  0x01000000  // 24   E
#define ST_COM_CRC_ERROR     0x00800000  // 23   E
#define ST_ILLEGAL_COMMAND   0x00400000  // 22   E
#define ST_CARD_ECC_FAILED   0x00200000  // 21   E
#define ST_CC_ERROR          0x00100000  // 20   E
#define ST_ERROR             0x00080000  // 19   E
#define ST_CSD_OVERWRITE     0x00010000  // 16   E
#define ST_WP_ERASE_SKIP     0x00008000  // 15   E
#define ST_CARD_ECC_DISABLED 0x00004000  // 14   E
#define ST_ERASE_RESET       0x00002000  // 13   E
#define ST_CARD_STATE        0x00001e00  // 12:9
#define ST_READY_FOR_DATA    0x00000100  // 8
#define ST_APP_CMD           0x00000020  // 5
#define ST_AKE_SEQ_ERROR     0x00000004  // 3    E

#define R1_CARD_STATE_SHIFT  9
#define R1_ERRORS_MASK       0xfff9c004  // All above bits which indicate errors.

// R3 (ACMD41 APP_SEND_OP_COND)
#define R3_COMPLETE    0x80000000
#define R3_CCS         0x40000000
#define R3_S18A        0x01000000

// R6 (CMD3 SEND_REL_ADDR)
#define R6_RCA_MASK    0xffff0000
#define R6_ERR_MASK    0x0000e000
#define R6_STATE_MASK  0x00001e00

// Card state values as they appear in the status register.
#define CS_IDLE    0 // 0x00000000
#define CS_READY   1 // 0x00000200
#define CS_IDENT   2 // 0x00000400
#define CS_STBY    3 // 0x00000600
#define CS_TRAN    4 // 0x00000800
#define CS_DATA    5 // 0x00000a00
#define CS_RCV     6 // 0x00000c00
#define CS_PRG     7 // 0x00000e00
#define CS_DIS     8 // 0x00001000

// Response types.
// Note that on the PI, the index and CRC are dropped, leaving 32 bits in RESP0.
#define RESP_NO    0     // No response
#define RESP_R1    1     // 48  RESP0    contains card status
#define RESP_R1b  11     // 48  RESP0    contains card status, data line indicates busy
#define RESP_R2I   2     // 136 RESP0..3 contains 128 bit CID shifted down by 8 bits as no CRC
#define RESP_R2S  12     // 136 RESP0..3 contains 128 bit CSD shifted down by 8 bits as no CRC
#define RESP_R3    3     // 48  RESP0    contains OCR register
#define RESP_R6    6     // 48  RESP0    contains RCA and status bits 23,22,19,12:0
#define RESP_R7    7     // 48  RESP0    contains voltage acceptance and check pattern

#define RCA_NO     1
#define RCA_YES    2

// Command indexes in the command table
#define IX_GO_IDLE_STATE    0
#define IX_ALL_SEND_CID     1
#define IX_SEND_REL_ADDR    2
#define IX_SET_DSR          3
#define IX_SWITCH_FUNC      4
#define IX_CARD_SELECT      5
#define IX_SEND_IF_COND     6
#define IX_SEND_CSD         7
#define IX_SEND_CID         8
#define IX_VOLTAGE_SWITCH   9
#define IX_STOP_TRANS       10
#define IX_SEND_STATUS      11
#define IX_GO_INACTIVE      12
#define IX_SET_BLOCKLEN     13
#define IX_READ_SINGLE      14
#define IX_READ_MULTI       15
#define IX_SEND_TUNING      16
#define IX_SPEED_CLASS      17
#define IX_SET_BLOCKCNT     18
#define IX_WRITE_SINGLE     19
#define IX_WRITE_MULTI      20
#define IX_PROGRAM_CSD      21
#define IX_SET_WRITE_PR     22
#define IX_CLR_WRITE_PR     23
#define IX_SND_WRITE_PR     24
#define IX_ERASE_WR_ST      25
#define IX_ERASE_WR_END     26
#define IX_ERASE            27
#define IX_LOCK_UNLOCK      28
#define IX_APP_CMD          29
#define IX_APP_CMD_RCA      30  // APP_CMD used once we have the RCA.
#define IX_GEN_CMD          31

// Commands hereafter require APP_CMD.
#define IX_APP_CMD_START    32
#define IX_SET_BUS_WIDTH    32
#define IX_SD_STATUS        33
#define IX_SEND_NUM_WRBL    34
#define IX_SEND_NUM_ERS     35
#define IX_APP_SEND_OP_COND 36
#define IX_SET_CLR_DET      37
#define IX_SEND_SCR         38

// CSD flags
// Note: all flags are shifted down by 8 bits as the CRC is not included.
// Most flags are common:
// in V1 the size is 12 bits with a 3 bit multiplier.
// in V1 currents for read and write are specified.
// in V2 the size is 22 bits, no multiplier, no currents.
#define CSD0_VERSION               0x00c00000
#define CSD0_V1                    0x00000000
#define CSD0_V2                    0x00400000

// CSD Version 1 and 2 flags
#define CSD1VN_TRAN_SPEED          0xff000000

#define CSD1VN_CCC                 0x00fff000
#define CSD1VN_READ_BL_LEN         0x00000f00
#define CSD1VN_READ_BL_LEN_SHIFT   8
#define CSD1VN_READ_BL_PARTIAL     0x00000080
#define CSD1VN_WRITE_BLK_MISALIGN  0x00000040
#define CSD1VN_READ_BLK_MISALIGN   0x00000020
#define CSD1VN_DSR_IMP             0x00000010

#define CSD2VN_ERASE_BLK_EN        0x00000040
#define CSD2VN_ERASE_SECTOR_SIZEH  0x0000003f
#define CSD3VN_ERASE_SECTOR_SIZEL  0x80000000

#define CSD3VN_WP_GRP_SIZE         0x7f000000

#define CSD3VN_WP_GRP_ENABLE       0x00800000
#define CSD3VN_R2W_FACTOR          0x001c0000
#define CSD3VN_WRITE_BL_LEN        0x0003c000
#define CSD3VN_WRITE_BL_LEN_SHIFT  14
#define CSD3VN_WRITE_BL_PARTIAL    0x00002000
#define CSD3VN_FILE_FORMAT_GROUP   0x00000080
#define CSD3VN_COPY                0x00000040
#define CSD3VN_PERM_WRITE_PROT     0x00000020
#define CSD3VN_TEMP_WRITE_PROT     0x00000010
#define CSD3VN_FILE_FORMAT         0x0000000c
#define CSD3VN_FILE_FORMAT_HDD     0x00000000
#define CSD3VN_FILE_FORMAT_DOSFAT  0x00000004
#define CSD3VN_FILE_FORMAT_UFF     0x00000008
#define CSD3VN_FILE_FORMAT_UNKNOWN 0x0000000c

// CSD Version 1 flags.
#define CSD1V1_C_SIZEH             0x00000003
#define CSD1V1_C_SIZEH_SHIFT       10

#define CSD2V1_C_SIZEL             0xffc00000
#define CSD2V1_C_SIZEL_SHIFT       22
#define CSD2V1_VDD_R_CURR_MIN      0x00380000
#define CSD2V1_VDD_R_CURR_MAX      0x00070000
#define CSD2V1_VDD_W_CURR_MIN      0x0000e000
#define CSD2V1_VDD_W_CURR_MAX      0x00001c00
#define CSD2V1_C_SIZE_MULT         0x00000380
#define CSD2V1_C_SIZE_MULT_SHIFT   7

// CSD Version 2 flags.
#define CSD2V2_C_SIZE              0x3fffff00
#define CSD2V2_C_SIZE_SHIFT        8

// SCR flags
// NOTE: SCR is big-endian, so flags appear byte-wise reversed from the spec.
#define SCR_STRUCTURE              0x000000f0
#define SCR_STRUCTURE_V1           0x00000000

#define SCR_SD_SPEC                0x0000000f
#define SCR_SD_SPEC_1_101          0x00000000
#define SCR_SD_SPEC_11             0x00000001
#define SCR_SD_SPEC_2_3            0x00000002

#define SCR_DATA_AFTER_ERASE       0x00008000

#define SCR_SD_SECURITY            0x00007000
#define SCR_SD_SEC_NONE            0x00000000
#define SCR_SD_SEC_NOT_USED        0x00001000
#define SCR_SD_SEC_101             0x00002000  // SDSC
#define SCR_SD_SEC_2               0x00003000  // SDHC
#define SCR_SD_SEC_3               0x00004000  // SDXC

#define SCR_SD_BUS_WIDTHS          0x00000f00
#define SCR_SD_BUS_WIDTH_1         0x00000100
#define SCR_SD_BUS_WIDTH_4         0x00000400

#define SCR_SD_SPEC3               0x00800000
#define SCR_SD_SPEC_2              0x00000000
#define SCR_SD_SPEC_3              0x00100000

#define SCR_EX_SECURITY            0x00780000

#define SCR_CMD_SUPPORT            0x03000000
#define SCR_CMD_SUPP_SET_BLKCNT    0x02000000
#define SCR_CMD_SUPP_SPEED_CLASS   0x01000000

// SD card types
#define SD_TYPE_MMC  1
#define SD_TYPE_1    2
#define SD_TYPE_2_SC 3
#define SD_TYPE_2_HC 4

// SD card functions supported values.
#define SD_SUPP_SET_BLOCK_COUNT 0x80000000
#define SD_SUPP_SPEED_CLASS     0x40000000
#define SD_SUPP_BUS_WIDTH_4     0x20000000
#define SD_SUPP_BUS_WIDTH_1     0x10000000

typedef struct EMMCCommand
{
    char const * name;
    unsigned int code;
    unsigned char resp;
    unsigned char rca;
    int delay;
} EMMCCommand;

// SD card descriptor
typedef struct SDDescriptor
{
    // Static information about the SD Card.
    unsigned long long capacity;
    unsigned int cid[4];
    unsigned int csd[4];
    unsigned int scr[2];
    unsigned int ocr;
    unsigned int support;
    unsigned int fileFormat;
    unsigned char type;
    unsigned char uhsi;
    unsigned char init;
    unsigned char absent;

    // Dynamic information.
    unsigned int rca;
    unsigned int cardState;
    unsigned int status;

    EMMCCommand* lastCmd;
    unsigned int lastArg;
} SDDescriptor;

// EMMC registers
//static volatile unsigned int* const EMMC_ARG2        = (unsigned int*)(0x20300000);
static volatile unsigned int* const EMMC_BLKSIZECNT  = (unsigned int*)(0x20300004);
static volatile unsigned int* const EMMC_ARG1        = (unsigned int*)(0x20300008);
static volatile unsigned int* const EMMC_CMDTM       = (unsigned int*)(0x2030000c);
static volatile unsigned int* const EMMC_RESP0       = (unsigned int*)(0x20300010);
static volatile unsigned int* const EMMC_RESP1       = (unsigned int*)(0x20300014);
static volatile unsigned int* const EMMC_RESP2       = (unsigned int*)(0x20300018);
static volatile unsigned int* const EMMC_RESP3       = (unsigned int*)(0x2030001c);
static volatile unsigned int* const EMMC_DATA        = (unsigned int*)(0x20300020);
static volatile unsigned int* const EMMC_STATUS      = (unsigned int*)(0x20300024);
static volatile unsigned int* const EMMC_CONTROL0    = (unsigned int*)(0x20300028);
static volatile unsigned int* const EMMC_CONTROL1    = (unsigned int*)(0x2030002c);
static volatile unsigned int* const EMMC_INTERRUPT   = (unsigned int*)(0x20300030);
static volatile unsigned int* const EMMC_IRPT_MASK   = (unsigned int*)(0x20300034);
static volatile unsigned int* const EMMC_IRPT_EN     = (unsigned int*)(0x20300038);
//static volatile unsigned int* const EMMC_CONTROL2    = (unsigned int*)(0x2030003c);
//static volatile unsigned int* const EMMC_BOOT_TIMEOUT= (unsigned int*)(0x20300070);
//static volatile unsigned int* const EMMC_EXRDFIFO_EN = (unsigned int*)(0x20300084);
//static volatile unsigned int* const EMMC_SPI_INT_SPT = (unsigned int*)(0x203000f0);
static volatile unsigned int* const EMMC_SLOTISR_VER = (unsigned int*)(0x203000fc);

// Command table.
// TODO: TM_DAT_DIR_CH required in any of these?
static EMMCCommand sdCommandTable[] =
{
    { "GO_IDLE_STATE", 0x00000000|CMD_RSPNS_NO                             , RESP_NO , RCA_NO  ,0},
    { "ALL_SEND_CID" , 0x02000000|CMD_RSPNS_136                            , RESP_R2I, RCA_NO  ,0},
    { "SEND_REL_ADDR", 0x03000000|CMD_RSPNS_48                             , RESP_R6 , RCA_NO  ,0},
    { "SET_DSR"      , 0x04000000|CMD_RSPNS_NO                             , RESP_NO , RCA_NO  ,0},
    { "SWITCH_FUNC"  , 0x06000000|CMD_RSPNS_48                             , RESP_R1 , RCA_NO  ,0},
    { "CARD_SELECT"  , 0x07000000|CMD_RSPNS_48B                            , RESP_R1b, RCA_YES ,0},
    { "SEND_IF_COND" , 0x08000000|CMD_RSPNS_48                             , RESP_R7 , RCA_NO  ,100},
    { "SEND_CSD"     , 0x09000000|CMD_RSPNS_136                            , RESP_R2S, RCA_YES ,0},
    { "SEND_CID"     , 0x0A000000|CMD_RSPNS_136                            , RESP_R2I, RCA_YES ,0},
    { "VOLT_SWITCH"  , 0x0B000000|CMD_RSPNS_48                             , RESP_R1 , RCA_NO  ,0},
    { "STOP_TRANS"   , 0x0C000000|CMD_RSPNS_48B                            , RESP_R1b, RCA_NO  ,0},
    { "SEND_STATUS"  , 0x0D000000|CMD_RSPNS_48                             , RESP_R1 , RCA_YES ,0},
    { "GO_INACTIVE"  , 0x0F000000|CMD_RSPNS_NO                             , RESP_NO , RCA_YES ,0},
    { "SET_BLOCKLEN" , 0x10000000|CMD_RSPNS_48                             , RESP_R1 , RCA_NO  ,0},
    { "READ_SINGLE"  , 0x11000000|CMD_RSPNS_48 |CMD_IS_DATA  |TM_DAT_DIR_CH, RESP_R1 , RCA_NO  ,0},
    { "READ_MULTI"   , 0x12000000|CMD_RSPNS_48 |TM_MULTI_DATA|TM_DAT_DIR_CH, RESP_R1 , RCA_NO  ,0},
    { "SEND_TUNING"  , 0x13000000|CMD_RSPNS_48                             , RESP_R1 , RCA_NO  ,0},
    { "SPEED_CLASS"  , 0x14000000|CMD_RSPNS_48B                            , RESP_R1b, RCA_NO  ,0},
    { "SET_BLOCKCNT" , 0x17000000|CMD_RSPNS_48                             , RESP_R1 , RCA_NO  ,0},
    { "WRITE_SINGLE" , 0x18000000|CMD_RSPNS_48 |CMD_IS_DATA  |TM_DAT_DIR_HC, RESP_R1 , RCA_NO  ,0},
    { "WRITE_MULTI"  , 0x19000000|CMD_RSPNS_48 |TM_MULTI_DATA|TM_DAT_DIR_HC, RESP_R1 , RCA_NO  ,0},
    { "PROGRAM_CSD"  , 0x1B000000|CMD_RSPNS_48                             , RESP_R1 , RCA_NO  ,0},
    { "SET_WRITE_PR" , 0x1C000000|CMD_RSPNS_48B                            , RESP_R1b, RCA_NO  ,0},
    { "CLR_WRITE_PR" , 0x1D000000|CMD_RSPNS_48B                            , RESP_R1b, RCA_NO  ,0},
    { "SND_WRITE_PR" , 0x1E000000|CMD_RSPNS_48                             , RESP_R1 , RCA_NO  ,0},
    { "ERASE_WR_ST"  , 0x20000000|CMD_RSPNS_48                             , RESP_R1 , RCA_NO  ,0},
    { "ERASE_WR_END" , 0x21000000|CMD_RSPNS_48                             , RESP_R1 , RCA_NO  ,0},
    { "ERASE"        , 0x26000000|CMD_RSPNS_48B                            , RESP_R1b, RCA_NO  ,0},
    { "LOCK_UNLOCK"  , 0x2A000000|CMD_RSPNS_48                             , RESP_R1 , RCA_NO  ,0},
    { "APP_CMD"      , 0x37000000|CMD_RSPNS_NO                             , RESP_NO , RCA_NO  ,100},
    { "APP_CMD"      , 0x37000000|CMD_RSPNS_48                             , RESP_R1 , RCA_YES ,0},
    { "GEN_CMD"      , 0x38000000|CMD_RSPNS_48                             , RESP_R1 , RCA_NO  ,0},

    // APP commands must be prefixed by an APP_CMD.
    { "SET_BUS_WIDTH", 0x06000000|CMD_RSPNS_48                             , RESP_R1 , RCA_NO  ,0},
    { "SD_STATUS"    , 0x0D000000|CMD_RSPNS_48                             , RESP_R1 , RCA_YES ,0}, // RCA???
    { "SEND_NUM_WRBL", 0x16000000|CMD_RSPNS_48                             , RESP_R1 , RCA_NO  ,0},
    { "SEND_NUM_ERS" , 0x17000000|CMD_RSPNS_48                             , RESP_R1 , RCA_NO  ,0},
    { "SD_SENDOPCOND", 0x29000000|CMD_RSPNS_48                             , RESP_R3 , RCA_NO  ,1000},
    { "SET_CLR_DET"  , 0x2A000000|CMD_RSPNS_48                             , RESP_R1 , RCA_NO  ,0},
    { "SEND_SCR"     , 0x33000000|CMD_RSPNS_48|CMD_IS_DATA|TM_DAT_DIR_CH   , RESP_R1 , RCA_NO  ,0},
};

// static const char* STATUS_NAME[] =
//   { "idle", "ready", "identify", "standby", "transmit", "data", "receive", "prog", "dis" };

// static const char* SD_TYPE_NAME[] =
//   {
//   "Unknown", "MMC", "Type 1", "Type 2 SC", "Type 2 HC"
//   };

// The SD card descriptor.
static SDDescriptor s_sdcard;

static int s_host_ver = 0; // Set by sdcard_init().
static int s_emmc_clock_rate = 0; // Set by init_emmc_clock_rate().

// static inline void wait(int32_t count)
// {
// 	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
// 		 : "=r"(count): [count]"0"(count) : "cc");
// }

/** Completely clear the interrupt register.
 */
static void clear_interrupt_reg()
{
    *EMMC_INTERRUPT = *EMMC_INTERRUPT; // Write the bits set to 1 to clear.
}

/* Wait for interrupt.
 *
 * - Waits up to one second for the interrupt.
 * - Returns non-zero value on error.
 */
static int wait_for_interrupt(unsigned int const mask)
{
    static uint32_t const max_ticks = 1000000; // 1 second, using a 1 MHz clock.
    int const waitMask = mask | INT_ERROR_MASK;
    bool wait_timeout = true;

    uint32_t const start_tick = armtimer_get_tick();

    // Wait for the specified interrupt or some error:
    //
    while(armtimer_get_tick() - start_tick < max_ticks)
    {
        if(*EMMC_INTERRUPT & waitMask)
        {
            wait_timeout = false;
            break;
        }
    }

    int const irq_val = *EMMC_INTERRUPT;

    if(wait_timeout
        || (irq_val & INT_CMD_TIMEOUT)
        || (irq_val & INT_DATA_TIMEOUT))
    {
        clear_interrupt_reg();

        return SD_TIMEOUT;
    }

    if(irq_val & INT_ERROR_MASK)
    {
        clear_interrupt_reg();

        return SD_ERROR;
    }

    // Clear the interrupt we were waiting for,
    // leaving all other (non-error) interrupts:
    //
    *EMMC_INTERRUPT = mask;

    return SD_OK;
}

/* Wait for any command that may be in progress.
 */
static int sdWaitForCommand()
  {
  // Check for status indicating a command in progress.
  int count = 1000000;
  while( (*EMMC_STATUS & SR_CMD_INHIBIT) && !(*EMMC_INTERRUPT & INT_ERROR_MASK) && count-- )
    armtimer_busywait_microseconds(1);
  if( count <= 0 || (*EMMC_INTERRUPT & INT_ERROR_MASK) )
    {
    //LOG_ERROR("EMMC: Wait for command aborted: %08x %08x %08x\n",*EMMC_STATUS,*EMMC_INTERRUPT,*EMMC_RESP0);
    return SD_BUSY;
    }

  return SD_OK;
  }

/* Wait for any data that may be in progress.
 */
static int sdWaitForData()
  {
  // Check for status indicating data transfer in progress.
  // Spec indicates a maximum wait of 500ms.
  // For now this is done by waiting for the DAT_INHIBIT flag to go from the status register,
  // or until an error is flagged in the interrupt register.
	  //  printf("EMMC: Wait for data started: %08x %08x %08x; dat: %d\n",*EMMC_STATUS,*EMMC_INTERRUPT,*EMMC_RESP0,datSet);
  int count = 0;
  while( (*EMMC_STATUS & SR_DAT_INHIBIT) && !(*EMMC_INTERRUPT & INT_ERROR_MASK) && ++count < 500000 )
    armtimer_busywait_microseconds(1);
  if( count >= 500000 || (*EMMC_INTERRUPT & INT_ERROR_MASK) )
    {
    //LOG_ERROR("EMMC: Wait for data aborted: %08x %08x %08x\n",*EMMC_STATUS,*EMMC_INTERRUPT,*EMMC_RESP0);
    return SD_BUSY;
    }
  //  printf("EMMC: Wait for data OK: count = %d: %08x %08x %08x\n",count,*EMMC_STATUS,*EMMC_INTERRUPT,*EMMC_RESP0);

  return SD_OK;
  }

/* Send command and handle response.
 */
static int sdSendCommandP( EMMCCommand* cmd, int arg )
  {
  // Check for command in progress
  if( sdWaitForCommand() != 0 )
    return SD_BUSY;

  //if( sdDebug ) LOG_DEBUG("EMMC: Sending command %s code %08x arg %08x\n",cmd->name,cmd->code,arg);
  s_sdcard.lastCmd = cmd;
  s_sdcard.lastArg = arg;

  //  printf("EMMC: Sending command %08x:%s arg %d\n",cmd->code,cmd->name,arg);

  int result;

  // Clear interrupt flags.  This is done by setting the ones that are currently set.
  //  printf("EMMC_INTERRUPT before clearing: %08x\n", *EMMC_INTERRUPT);
  *EMMC_INTERRUPT = *EMMC_INTERRUPT;

  // Set the argument and the command code.
  // Some commands require a delay before reading the response.
  //  printf("EMMC_STATUS:%08x\nEMMC_INTERRUPT: %08x\nEMMC_RESP0 : %08x\n", *EMMC_STATUS, *EMMC_INTERRUPT, *EMMC_RESP0);
  //  printf("ARG: %08x, CODE: %08x\n", arg, cmd->code);
  *EMMC_ARG1 = arg;
  *EMMC_CMDTM = cmd->code;
  if( cmd->delay ) armtimer_busywait_microseconds(cmd->delay);

  // Wait until command complete interrupt.
  if( (result = wait_for_interrupt(INT_CMD_DONE)) )
  {
      //console_deb_writeline("sdcard : Waiting for interrupt failed (1)!");
      return result;
  }

  // Get response from RESP0.
  int resp0 = *EMMC_RESP0;
  //  printf("EMMC: Sent command %08x:%s arg %d resp %08x\n",cmd->code,cmd->name,arg,resp0);

  // Handle response types.
  switch( cmd->resp )
    {
    // No response.
    case RESP_NO:
      return SD_OK;

    // RESP0 contains card status, no other data from the RESP* registers.
    // Return value non-zero if any error flag in the status value.
    case RESP_R1:
    case RESP_R1b:
      s_sdcard.status = resp0;
      // Store the card state.  Note that this is the state the card was in before the
      // command was accepted, not the new state.
      s_sdcard.cardState = (resp0 & ST_CARD_STATE) >> R1_CARD_STATE_SHIFT;
      return resp0 & R1_ERRORS_MASK;

    // RESP0..3 contains 128 bit CID or CSD shifted down by 8 bits as no CRC
    // Note: highest bits are in RESP3.
    case RESP_R2I:
    case RESP_R2S:
      s_sdcard.status = 0;
      unsigned int* data = cmd->resp == RESP_R2I ? s_sdcard.cid : s_sdcard.csd;
      data[0] = *EMMC_RESP3;
      data[1] = *EMMC_RESP2;
      data[2] = *EMMC_RESP1;
      data[3] = resp0;
      return SD_OK;

    // RESP0 contains OCR register
    // TODO: What is the correct time to wait for this?
    case RESP_R3:
      s_sdcard.status = 0;
      s_sdcard.ocr = resp0;
      return SD_OK;

    // RESP0 contains RCA and status bits 23,22,19,12:0
    case RESP_R6:
      s_sdcard.rca = resp0 & R6_RCA_MASK;
      s_sdcard.status = ((resp0 & 0x00001fff)     ) |   // 12:0 map directly to status 12:0
                      ((resp0 & 0x00002000) << 6) |   // 13 maps to status 19 ERROR
                      ((resp0 & 0x00004000) << 8) |   // 14 maps to status 22 ILLEGAL_COMMAND
                      ((resp0 & 0x00008000) << 8);    // 15 maps to status 23 COM_CRC_ERROR
      // Store the card state.  Note that this is the state the card was in before the
      // command was accepted, not the new state.
      s_sdcard.cardState = (resp0 & ST_CARD_STATE) >> R1_CARD_STATE_SHIFT;
      return s_sdcard.status & R1_ERRORS_MASK;

    // RESP0 contains voltage acceptance and check pattern, which should match
    // the argument.
    case RESP_R7:
      s_sdcard.status = 0;
      return resp0 == arg ? SD_OK : SD_ERROR;
    }

  return SD_ERROR;
  }

/* Send APP_CMD.
 */
static int sdSendAppCommand()
  {
      //console_deb_writeline("sdSendAppCommand: Entered function..");

  int resp;
  // If no RCA, send the APP_CMD and don't look for a response.
  if( !s_sdcard.rca )
    sdSendCommandP(&sdCommandTable[IX_APP_CMD],0x00000000);

  // If there is an RCA, include that in APP_CMD and check card accepted it.
  else
    {
    if( (resp = sdSendCommandP(&sdCommandTable[IX_APP_CMD_RCA],s_sdcard.rca)) )
    {
        //console_deb_writeline("sdcard : Error: Sending command failed (1)!");
        return resp;
    }
    // Debug - check that status indicates APP_CMD accepted.
    if( !(s_sdcard.status & ST_APP_CMD) )
      return SD_ERROR;
    }

  return SD_OK;
  }

/* Send a command with no argument.
 * RCA automatically added if required.
 * APP_CMD sent automatically if required.
 */
static int sdSendCommand( int index )
  {
  // Issue APP_CMD if needed.
  int resp;
  if( index >= IX_APP_CMD_START)
  {
      //console_deb_writeline("sdSendCommand: Calling sdSendAppCommand()..");
      if((resp = sdSendAppCommand()))
      {
          return resp;
      }
  }

  // Get the command and set RCA if required.
  EMMCCommand* cmd = &sdCommandTable[index];
  int arg = 0;
  if( cmd->rca == RCA_YES )
  {
    arg = s_sdcard.rca;
  }
    if( (resp = sdSendCommandP(cmd,arg)) )
    {
        //console_deb_writeline("sdcard : Error: Sending command failed (2)!");
        return resp;
    }
  // Check that APP_CMD was correctly interpreted.
  if( index >= IX_APP_CMD_START && s_sdcard.rca && !(s_sdcard.status & ST_APP_CMD) )
    return SD_ERROR_APP_CMD;

  return resp;
  }

/* Send a command with a specific argument.
 * APP_CMD sent automatically if required.
 */
static int sdSendCommandA( int index, int arg )
  {
  // Issue APP_CMD if needed.
  int resp;
    if( index >= IX_APP_CMD_START)
    {
        if((resp = sdSendAppCommand()) )
        {
            return resp;
        }
    }

  // Get the command and pass the argument through.
  if( (resp = sdSendCommandP(&sdCommandTable[index],arg)) )
  {
      //console_deb_writeline("sdcard : Error: Sending command failed (3)!");
      return resp;
  }

  // Check that APP_CMD was correctly interpreted.
  if( index >= IX_APP_CMD_START && s_sdcard.rca && !(s_sdcard.status & ST_APP_CMD) )
    return SD_ERROR_APP_CMD;

  return resp;
  }

/* Read card's SCR
 */
static int sdReadSCR()
  {
  // SEND_SCR command is like a READ_SINGLE but for a block of 8 bytes.
  // Ensure that any data operation has completed before reading the block.
  if( sdWaitForData() )
  {
      //console_deb_writeline("sdcard : Timeout 2.");
      return SD_TIMEOUT;
  }

  // Set BLKSIZECNT to 1 block of 8 bytes, send SEND_SCR command
  *EMMC_BLKSIZECNT = (1 << 16) | 8;
  int resp;
  if( (resp = sdSendCommand(IX_SEND_SCR)) ) return resp;

  // Wait for READ_RDY interrupt.
  if( (resp = wait_for_interrupt(INT_READ_RDY)) )
    {
    //LOG_ERROR("EMMC: Timeout waiting for ready to read\n");
    //console_deb_writeline("sdcard : Waiting for interrupt failed (2)!");
    return resp;
    }

  // Allow maximum of 100ms for the read operation.
  int numRead = 0, count = 100000;
  while( numRead < 2 )
    {
    if( *EMMC_STATUS & SR_READ_AVAILABLE )
      s_sdcard.scr[numRead++] = *EMMC_DATA;
    else
      {
      armtimer_busywait_microseconds(1);
      if( --count == 0 ) break;
      }
    }

  // If SCR not fully read, the operation timed out.
  if( numRead != 2 )
    {
    //LOG_ERROR("EMMC: SEND_SCR ERR: %08x %08x %08x\n",*EMMC_STATUS,*EMMC_INTERRUPT,*EMMC_RESP0);
    //LOG_ERROR("EMMC: Reading SCR, only read %d words\n",numRead);
    //console_deb_writeline("sdcard : Timeout 3.");
    return SD_TIMEOUT;
    }

  // Parse out the SCR.  Only interested in values in scr[0], scr[1] is mfr specific.
  if( s_sdcard.scr[0] & SCR_SD_BUS_WIDTH_4 ) s_sdcard.support |= SD_SUPP_BUS_WIDTH_4;
  if( s_sdcard.scr[0] & SCR_SD_BUS_WIDTH_1 ) s_sdcard.support |= SD_SUPP_BUS_WIDTH_1;
  if( s_sdcard.scr[0] & SCR_CMD_SUPP_SET_BLKCNT ) s_sdcard.support |= SD_SUPP_SET_BLOCK_COUNT;
  if( s_sdcard.scr[0] & SCR_CMD_SUPP_SPEED_CLASS ) s_sdcard.support |= SD_SUPP_SPEED_CLASS;

  return SD_OK;
  }

static int fls_long (unsigned long x) {
	int r = 32;
	if (!x)  return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
    }
    if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}

/* Get the clock divider for the given requested frequency.
 * This is calculated relative to the SD base clock.
 */
static uint32_t sdGetClockDivider ( uint32_t freq )
{
    uint32_t divisor;
    uint32_t closest = 41666666 / freq; // Pi SD frequency is always 41.66667Mhz on baremetal
    uint32_t shiftcount = fls_long(closest - 1);      // Get the raw shiftcount
    if (shiftcount > 0) shiftcount--;               // Note the offset of shift by 1 (look at the spec)
    if (shiftcount > 7) shiftcount = 7;               // It's only 8 bits maximum on HOST_SPEC_V2
    if (s_host_ver > HOST_SPEC_V2) divisor = closest;   // Version 3 take closest
      else divisor = (1 << shiftcount);            // Version 2 take power 2

    if (divisor <= 2) {                           // Too dangerous to go for divisor 1 unless you test
      divisor = 2;                           // You can't take divisor below 2 on slow cards
      shiftcount = 0;                           // Match shift to above just for debug notification
    }

    //LOG_DEBUG("Divisor selected = %u, pow 2 shift count = %u\n", divisor, shiftcount);
    uint32_t hi = 0;
    if (s_host_ver > HOST_SPEC_V2) hi = (divisor & 0x300) >> 2; // Only 10 bits on Hosts specs above 2
    uint32_t lo = (divisor & 0x0ff);               // Low part always valid
    uint32_t cdiv = (lo << 8) + hi;                  // Join and roll to position
    return cdiv;                              // Return cdiv
}

/* Set the SD clock to the given frequency.
 */
static int sdSetClock(int freq)
{
    // Wait for any pending inhibit bits:

    int count = 100000;

    while((*EMMC_STATUS & (SR_CMD_INHIBIT | SR_DAT_INHIBIT)) && --count)
    {
        armtimer_busywait_microseconds(1);
    }
    if(count <= 0)
    {
        //LOG_ERROR("EMMC: Set clock: timeout waiting for inhibit flags. Status %08x.\n",*EMMC_STATUS);
        return SD_ERROR_CLOCK;
    }

    // Switch clock off:

    *EMMC_CONTROL1 &= ~C1_CLK_EN;
    armtimer_busywait_microseconds(10);

    // Request the new clock setting and enable the clock:

    int const cdiv = sdGetClockDivider(freq);

    *EMMC_CONTROL1 = (*EMMC_CONTROL1 & 0xFFFF003F) | cdiv;
    armtimer_busywait_microseconds(10);

    // Enable the clock:

    *EMMC_CONTROL1 |= C1_CLK_EN;
    armtimer_busywait_microseconds(10);

    // Wait for clock to be stable:

    count = 10000;
    while(!(*EMMC_CONTROL1 & C1_CLK_STABLE) && count--)
    {
        armtimer_busywait_microseconds(10);
    }
    if(count <= 0)
    {
        //LOG_ERROR("EMMC: ERROR: failed to get stable clock.\n");
        return SD_ERROR_CLOCK;
    }

    //printf("EMMC: Set clock, status %08x CONTROL1: %08x\n",*EMMC_STATUS,*EMMC_CONTROL1);

    return SD_OK;
}

/* Reset card.
 */
static int reset_card()
{
    static int const reset_type = C1_SRST_HC;
    static int const max_iter = 10000;

    int resp, i = 0;

    // Send reset command to host controller and wait for completion:

    *EMMC_CONTROL0 = 0;
    *EMMC_CONTROL1 |= reset_type;

    while(i < max_iter)
    {
        armtimer_busywait_microseconds(10);

        if((*EMMC_CONTROL1 & reset_type) == 0)
        {
            break;
        }

        ++i;
    }
    if(i == max_iter)
    {
        return SD_ERROR_RESET;
    }

    // Enable internal clock and set data timeout:

    // TODO: Correct value for timeout?

    *EMMC_CONTROL1 |= C1_CLK_INTLEN | C1_TOUNIT_MAX;
    armtimer_busywait_microseconds(10);

    // Set clock to setup frequency:
    //
    if((resp = sdSetClock(FREQ_SETUP)))
    {
        return resp;
    }

    // Enable interrupts for command completion values:
    //
    *EMMC_IRPT_EN   = 0xFFFFFFFF;//INT_ALL_MASK;
    *EMMC_IRPT_MASK = 0xFFFFFFFF;//INT_ALL_MASK;

    // Reset card registers:
    //
    s_sdcard.rca = 0;
    s_sdcard.ocr = 0;
    s_sdcard.lastArg = 0;
    s_sdcard.lastCmd = 0;
    s_sdcard.status = 0;
    s_sdcard.type = 0;
    s_sdcard.uhsi = 0;

    return sdSendCommand(IX_GO_IDLE_STATE);
}

/* Common routine for APP_SEND_OP_COND.
 * This is used for both SC and HC cards based on the parameter.
 */
static int sdAppSendOpCond( int arg )
{
    // Send APP_SEND_OP_COND with the given argument (for SC or HC cards).

    //  printf("EMMC: Sending ACMD41 SEND_OP_COND status %08x\n",*EMMC_STATUS);

    int resp;

    // Note: The host shall set ACMD41 timeout more than 1 second to abort
    //       repeat of issuing ACMD41.
    //
    static uint32_t const max_tick = 1500000; // 1.5 Seconds (1 MHz clock used).
    uint32_t const start_tick = armtimer_get_tick();

    if((resp = sdSendCommandA(IX_APP_SEND_OP_COND, arg)))
    {
        if(resp != SD_TIMEOUT )
        {
            console_deb_writeline(
                "sdAppSendOpCond : Error: ACMD41 returned a non-timeout error (1)!");
            return resp;
        }
    }

    while(
        !(s_sdcard.ocr & R3_COMPLETE)
        && (armtimer_get_tick() - start_tick < max_tick))
    {
    	//    printf("EMMC: Retrying ACMD SEND_OP_COND status %08x\n",*EMMC_STATUS);

        if((resp = sdSendCommandA(IX_APP_SEND_OP_COND, arg)))
        {
            if(resp != SD_TIMEOUT )
            {
                console_deb_writeline(
                    "sdAppSendOpCond : Error: ACMD41 returned a non-timeout error (2)!");
                return resp;
            }
        }
    }

    // Return timeout error if still not busy.
    if(!(s_sdcard.ocr & R3_COMPLETE))
    {
        console_deb_writeline("sdAppSendOpCond : Error: Timeout error!");
        return SD_TIMEOUT;
    }

    // Check that at least one voltage value was returned.
    if(!(s_sdcard.ocr & ACMD41_VOLTAGE))
    {
        console_deb_writeline("sdAppSendOpCond : Error: No voltage returned!");
        return SD_ERROR_VOLTAGE;
    }

    return SD_OK;
}

/* Switch voltage to 1.8v where the card supports it.
 */
static int sdSwitchVoltage()
{
  //LOG_DEBUG("EMMC: Pi does not support switch voltage, fixed at 3.3volt\n");
  return SD_OK;
}

/** Initialize GPIO.
 */
static void sd_init_gpio()
{
    baregpio_set_func(GPIO_DAT3,gpio_func_alt3);
    baregpio_set_pud(GPIO_DAT3,gpio_pud_up);
    baregpio_set_func(GPIO_DAT2,gpio_func_alt3);
    baregpio_set_pud(GPIO_DAT2,gpio_pud_up);
    baregpio_set_func(GPIO_DAT1,gpio_func_alt3);
    baregpio_set_pud(GPIO_DAT1,gpio_pud_up);
    baregpio_set_func(GPIO_DAT0,gpio_func_alt3);
    baregpio_set_pud(GPIO_DAT0,gpio_pud_up);
    baregpio_set_func(GPIO_CMD,gpio_func_alt3);
    baregpio_set_pud(GPIO_CMD,gpio_pud_up);
    baregpio_set_func(GPIO_CLK,gpio_func_alt3);
    baregpio_set_pud(GPIO_CLK,gpio_pud_up);
}

/** Initialize static variable holding EMMC clock rate.
 *
 *  - Returns, if successful or not.
 */
static int init_emmc_clock_rate()
{
    uint32_t const r = mailbox_read_clockrate(mailbox_id_clockrate_emmc);

    if(r == UINT32_MAX)
    {
        return SD_ERROR;
    }

    s_emmc_clock_rate = (int)r;

    return SD_OK;
}

// /* Parse CID
//  */
// static void sdParseCID()
//   {
//   // For some reason cards I have looked at seem to have everything
//   // shifted 8 bits down.
//   int manId = (s_sdcard.cid[0]&0x00ff0000) >> 16;
//   char appId[3];
//   appId[0] = (s_sdcard.cid[0]&0x0000ff00) >> 8;
//   appId[1] = (s_sdcard.cid[0]&0x000000ff);
//   appId[2] = 0;
//   char name[6];
//   name[0] = (s_sdcard.cid[1]&0xff000000) >> 24;
//   name[1] = (s_sdcard.cid[1]&0x00ff0000) >> 16;
//   name[2] = (s_sdcard.cid[1]&0x0000ff00) >> 8;
//   name[3] = (s_sdcard.cid[1]&0x000000ff);
//   name[4] = (s_sdcard.cid[2]&0xff000000) >> 24;
//   name[5] = 0;
//   int revH = (s_sdcard.cid[2]&0x00f00000) >> 20;
//   int revL = (s_sdcard.cid[2]&0x000f0000) >> 16;
//   int serial = ((s_sdcard.cid[2]&0x0000ffff) << 16) +
//                ((s_sdcard.cid[3]&0xffff0000) >> 16);
//
//   // For some reason cards I have looked at seem to have the Y/M in
//   // bits 11:0 whereas the spec says they should be in bits 19:8
//   int dateY = ((s_sdcard.cid[3]&0x00000ff0) >> 4) + 2000;
//   int dateM = (s_sdcard.cid[3]&0x0000000f);
//
//   LOG_DEBUG("EMMC: SD Card %s %dMb UHS-I %d mfr %d '%s:%s' r%d.%d %d/%d, #%08x RCA %04x",
//             SD_TYPE_NAME[s_sdcard.type], (int)(s_sdcard.capacity>>20),s_sdcard.uhsi,
//             manId, appId, name, revH, revL, dateM, dateY, serial, s_sdcard.rca>>16);
//   }

/* Parse CSD
 */
static void sdParseCSD()
{
  int csdVersion = s_sdcard.csd[0] & CSD0_VERSION;

  // For now just work out the size.
  if( csdVersion == CSD0_V1 )
	  {
		  int csize = ((s_sdcard.csd[1] & CSD1V1_C_SIZEH) << CSD1V1_C_SIZEH_SHIFT) +
                ((s_sdcard.csd[2] & CSD2V1_C_SIZEL) >> CSD2V1_C_SIZEL_SHIFT);
		  int mult = 1 << (((s_sdcard.csd[2] & CSD2V1_C_SIZE_MULT) >> CSD2V1_C_SIZE_MULT_SHIFT) + 2);
		  long long blockSize = 1 << ((s_sdcard.csd[1] & CSD1VN_READ_BL_LEN) >> CSD1VN_READ_BL_LEN_SHIFT);
		  long long numBlocks = (csize+1LL)*mult;

		  s_sdcard.capacity = numBlocks * blockSize;
	  }
  else // if( csdVersion == CSD0_V2 )
	  {
		  long long csize = (s_sdcard.csd[2] & CSD2V2_C_SIZE) >> CSD2V2_C_SIZE_SHIFT;

		  s_sdcard.capacity = (csize+1LL)*512LL*1024LL;
	  }

  // Get other attributes of the card.
  s_sdcard.fileFormat = s_sdcard.csd[3] & CSD3VN_FILE_FORMAT;
}

// ************************
// *** PUBLIC FUNCTIONS ***
// ************************

/* Clear multiple contiguous blocks.
 * Assumes that the erase operation writes zeros to the file.
 */
int sdcard_blocks_clear( long long address, int numBlocks )
  {
  if( !s_sdcard.init ) return SD_NO_RESP;

  // Ensure that any data operation has completed before doing the transfer.
  if( sdWaitForData() )
  {
      //console_deb_writeline("sdcard : Timeout 5.");
      return SD_TIMEOUT;
  }

  // Address is different depending on the card type.
  // HC pass address as block # which is just address/512.
  // SC pass address straight through.
  int startAddress = s_sdcard.type == SD_TYPE_2_HC ? (int)(address>>9) : (int)address;
  int endAddress = startAddress + (s_sdcard.type == SD_TYPE_2_HC ? (numBlocks - 1) : ((numBlocks-1)*512));

  int resp;
  //  printf("EMMC: erasing blocks from %d to %d\n",startAddress,endAddress);
  if( (resp = sdSendCommandA(IX_ERASE_WR_ST,startAddress)) ) return resp;
  if( (resp = sdSendCommandA(IX_ERASE_WR_END,endAddress)) ) return resp;
  if( (resp = sdSendCommand(IX_ERASE)) ) return resp;
  //  printf("EMMC: sent erase command, status %08x int %08x\n",*EMMC_STATUS,*EMMC_INTERRUPT);

  // Wait for data inhibit status to drop.
  int count = 1000000;
  while( *EMMC_STATUS & SR_DAT_INHIBIT )
    {
    if( --count == 0 )
      {
      //LOG_ERROR("EMMC: Timeout waiting for erase: %08x %08x\n",*EMMC_STATUS,*EMMC_INTERRUPT);
      //console_deb_writeline("sdcard : Timeout 6.");
      return SD_TIMEOUT;
      }

    armtimer_busywait_microseconds(10);
    }

  //  printf("EMMC: completed erase command int %08x\n",*EMMC_INTERRUPT);

  return SD_OK;
  }

/** Transfer multiple contiguous blocks between the given address on the card and the buffer.
 */
int sdcard_blocks_transfer( long long address, int numBlocks, unsigned char* buffer, int write )
{
	//	printf("check s_sdcard.init\n"); // TEST
if( !s_sdcard.init ) return SD_NO_RESP;

//	printf("sdWaitForData() .init\n"); // TEST
// Ensure that any data operation has completed before doing the transfer.
if( sdWaitForData() )
{
    //console_deb_writeline("sdcard : Timeout 7.");
    return SD_TIMEOUT;
}

// Work out the status, interrupt and command values for the transfer.
int readyInt = write ? INT_WRITE_RDY : INT_READ_RDY;
//int readyData = write ? SR_WRITE_AVAILABLE : SR_READ_AVAILABLE;
int transferCmd = write ? ( numBlocks == 1 ? IX_WRITE_SINGLE : IX_WRITE_MULTI) :
                          ( numBlocks == 1 ? IX_READ_SINGLE : IX_READ_MULTI);

// If more than one block to transfer, and the card supports it,
// send SET_BLOCK_COUNT command to indicate the number of blocks to transfer.
int resp;
if( numBlocks > 1 &&
    (s_sdcard.support & SD_SUPP_SET_BLOCK_COUNT) &&
    (resp = sdSendCommandA(IX_SET_BLOCKCNT,numBlocks)) ) return resp;

// Address is different depending on the card type.
// HC pass address as block # which is just address/512.
// SC pass address straight through.
int blockAddress = s_sdcard.type == SD_TYPE_2_HC ? (int)(address>>9) : (int)address;

// Set BLKSIZECNT to number of blocks * 512 bytes, send the read or write command.
// Once the data transfer has started and the TM_BLKCNT_EN bit in the CMDTM register is
// set the EMMC module automatically decreases the BLKCNT value as the data blocks
// are transferred and stops the transfer once BLKCNT reaches 0.
// TODO: TM_AUTO_CMD12 - is this needed?  What effect does it have?
*EMMC_BLKSIZECNT = (numBlocks << 16) | 512;
//	printf("sdSendCommandA() .init\n"); // TEST
if( (resp = sdSendCommandA(transferCmd,blockAddress)) ) return resp;

// Transfer all blocks.
int blocksDone = 0;
while( blocksDone < numBlocks )
  {
  // Wait for ready interrupt for the next block.
  if( (resp = wait_for_interrupt(readyInt)) )
    {
		  //		  printf("EMMC: Timeout waiting for ready to read\n"); //TEST
    //LOG_ERROR("EMMC: Timeout waiting for ready to read\n");
    //console_deb_writeline("sdcard : Waiting for interrupt failed (3)!");
    return resp;
    }

  // Handle non-word-aligned buffers byte-by-byte.
  // Note: the entire block is sent without looking at status registers.
  int done = 0;
  if( (int)buffer & 0x03 )
    {
    while( done < 512 )
      {
      if( write )
        {
        int data = (buffer[done++]      );
        data +=    (buffer[done++] << 8 );
        data +=    (buffer[done++] << 16);
        data +=    (buffer[done++] << 24);
        *EMMC_DATA = data;
        }
      else
        {
        int data = *EMMC_DATA;
        buffer[done++] = (data      ) & 0xff;
        buffer[done++] = (data >> 8 ) & 0xff;
        buffer[done++] = (data >> 16) & 0xff;
        buffer[done++] = (data >> 24) & 0xff;
        }
      }
    }

  // Handle word-aligned buffers more efficiently.
  else
    {
    unsigned int* intbuff = (unsigned int*)buffer;
    while( done < 128 )
      {
      if( write )
        *EMMC_DATA = intbuff[done++];
      else
        intbuff[done++] = *EMMC_DATA;
      }
    }

  blocksDone++;
  buffer += 512;
  }

// If not all bytes were read, the operation timed out.
if( blocksDone != numBlocks )
  {
		//		printf("Error 23\n");
  //LOG_ERROR("EMMC: Transfer error only done %d/%d blocks\n",blocksDone,numBlocks);
  //LOG_DEBUG("EMMC: Transfer: %08x %08x %08x %08x\n",*EMMC_STATUS,*EMMC_INTERRUPT,*EMMC_RESP0,*EMMC_BLKSIZECNT);

  // if( !write && numBlocks > 1 && (resp = sdSendCommand(IX_STOP_TRANS)) )
	// 	//		printf("Error 24\n");
  //   LOG_DEBUG("EMMC: Error response from stop transmission: %d\n",resp);
  //
  if( !write && numBlocks > 1)
  {
      resp = sdSendCommand(IX_STOP_TRANS);
  }

  //console_deb_writeline("sdcard : Timeout 8.");
  return SD_TIMEOUT;
 }

// For a write operation, ensure DATA_DONE interrupt before we stop transmission.
if( write && (resp = wait_for_interrupt(INT_DATA_DONE)) )
  {
		//		printf("Error 25\n");
  //LOG_ERROR("EMMC: Timeout waiting for data done\n");
  //console_deb_writeline("sdcard : Waiting for interrupt failed (4)!");
  return resp;
  }

// For a multi-block operation, if SET_BLOCKCNT is not supported, we need to indicate
// that there are no more blocks to be transferred.
if( numBlocks > 1 && !(s_sdcard.support & SD_SUPP_SET_BLOCK_COUNT) &&
    (resp = sdSendCommand(IX_STOP_TRANS)) ) return resp;

return SD_OK;
}

void sdcard_deinit()
{
    memset(&s_sdcard, 0, sizeof (SDDescriptor));
}

int sdcard_init()
{
    int resp;

    if(s_sdcard.init)
    {
        return SD_OK;
    }

    sd_init_gpio();

    // TODO: Check version >= 1 and <= 3?
    //
    s_host_ver = (*EMMC_SLOTISR_VER & HOST_SPEC_NUM) >> HOST_SPEC_NUM_SHIFT;

    // Get base clock speed:
    //
    if((resp = init_emmc_clock_rate()))
    {
        console_deb_writeline(
            "sdcard_init: Error: Init EMMC clock rate failed!");
        return resp;
    }

    // Reset the card:
    //
    if((resp = reset_card()))
    {
        console_deb_writeline("sdcard_init: Error: Card reset failed!");
        return resp;
    }

    // Send SEND_IF_COND,0x000001AA (CMD8) voltage range 0x1 check pattern 0xAA
    // If voltage range and check pattern don't match, look for older card.
    resp = sdSendCommandA(IX_SEND_IF_COND, 0x000001AA);
    if(resp == SD_OK)
    {
        // Card responded with voltage and check pattern.
        // Resolve voltage and check for high capacity card.
        if( (resp = sdAppSendOpCond(ACMD41_ARG_HC)) ) return resp;

        // Check for high or standard capacity.
        if( s_sdcard.ocr & R3_CCS )
          s_sdcard.type = SD_TYPE_2_HC;
        else
          s_sdcard.type = SD_TYPE_2_SC;
    }
    else
    {
        if( resp == SD_BUSY )
        {
            console_deb_writeline("sdcard_init: Error: SD card is busy!");
            return resp;
        }
        else // No response to SEND_IF_COND, treat as an old card:
        {
            // If there appears to be a command in progress, reset the card:
            //
            if(*EMMC_STATUS & SR_CMD_INHIBIT)
            {
                if((resp = reset_card()))
                {
                    console_deb_writeline(
                        "sdcard_init: Error: Reset (because command seems to be in progress) failed!");
                    return resp;
                }
            }
            // wait(50);

            // Resolve voltage.
            if((resp = sdAppSendOpCond(ACMD41_ARG_SC)))
            {
                console_deb_writeline(
                    "sdcard_init: Error: Resolving voltage failed!");
                return resp;
            }

            s_sdcard.type = SD_TYPE_1;
        }
    }

    // If the switch to 1.8A is accepted, then we need to send a CMD11.
    // CMD11: Completion of voltage switch sequence is checked by high level of DAT[3:0].
    // Any bit of DAT[3:0] can be checked depends on ability of the host.
    // Appears for PI its any/all bits.
    if( (s_sdcard.ocr & R3_S18A) &&
      (resp = sdSwitchVoltage()) ) return resp;

    // Send ALL_SEND_CID (CMD2)
    if( (resp = sdSendCommand(IX_ALL_SEND_CID)) ) return resp;

    // Send SEND_REL_ADDR (CMD3)
    // TODO: In theory, loop back to SEND_IF_COND to find additional cards.
    if( (resp = sdSendCommand(IX_SEND_REL_ADDR)) ) return resp;

    // From now on the card should be in standby state.
    // Actually cards seem to respond in identify state at this point.
    // Check this with a SEND_STATUS (CMD13)
    //if( (resp = sdSendCommand(IX_SEND_STATUS)) ) return resp;
    //  printf("Card current state: %08x %s\n",s_sdcard.status,STATUS_NAME[s_sdcard.cardState]);

    // Send SEND_CSD (CMD9) and parse the result.
    if( (resp = sdSendCommand(IX_SEND_CSD)) ) return resp;
    sdParseCSD();
    if( s_sdcard.fileFormat != CSD3VN_FILE_FORMAT_DOSFAT &&
      s_sdcard.fileFormat != CSD3VN_FILE_FORMAT_HDD )
    {
    //LOG_ERROR("EMMC: Error, unrecognised file format %02x\n",s_sdcard.fileFormat);
    return SD_ERROR;
    }

    // At this point, set the clock to full speed.
    if( (resp = sdSetClock(FREQ_NORMAL)) ) return resp;

    // Send CARD_SELECT  (CMD7)
    // TODO: Check card_is_locked status in the R1 response from CMD7 [bit 25], if so, use CMD42 to unlock
    // CMD42 structure [4.3.7] same as a single block write; data block includes
    // PWD setting mode, PWD len, PWD data.
    if( (resp = sdSendCommand(IX_CARD_SELECT)) ) return resp;

    // Get the SCR as well.
    // Need to do this before sending ACMD6 so that allowed bus widths are known.
    if( (resp = sdReadSCR()) ) return resp;

    // Send APP_SET_BUS_WIDTH (ACMD6)
    // If supported, set 4 bit bus width and update the CONTROL0 register.
    if( s_sdcard.support & SD_SUPP_BUS_WIDTH_4 )
    {
    if( (resp = sdSendCommandA(IX_SET_BUS_WIDTH,s_sdcard.rca|2)) ) return resp;
    *EMMC_CONTROL0 |= C0_HCTL_DWITDH;
    }

    // Only needs to be set for SDSC cards.
    // For SDHC and SDXC cards block length is fixed at 512 anyway:
    //
    if((resp = sdSendCommandA(IX_SET_BLOCKLEN, 512)))
    {
        return resp;
    }

    s_sdcard.init = 1;
    return SD_OK;
}
