
// Marcel Timm, RhinoDevel, 2019sep06

// *****************************************************************************
// Originally taken from:
//
// https://raw.githubusercontent.com/moizumi99/RPiHaribote/master/haribote/sdcard.c
//
// Includes comments by other authors, too.
// *****************************************************************************

#ifndef MT_SDCARD_DEFINES
#define MT_SDCARD_DEFINES

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

// CONTROL register settings used:
//
#define C0_HCTL_DWITDH   0x00000002
//
#define C1_SRST_HC       0x01000000
#define C1_TOUNIT_MAX    0x000e0000
#define C1_CLK_EN        0x00000004
#define C1_CLK_STABLE    0x00000002
#define C1_CLK_INTLEN    0x00000001

// Frequencies used:
//
#define FREQ_SETUP           400000  // 400 Khz
#define FREQ_NORMAL        25000000  // 25 Mhz

// SLOTISR_VER values used:
//
#define HOST_SPEC_NUM              0x00ff0000
#define HOST_SPEC_NUM_SHIFT        16
#define HOST_SPEC_V2               1

// STATUS register settings used:
//
#define SR_READ_AVAILABLE    0x00000800  // ???? undocumented
#define SR_WRITE_AVAILABLE   0x00000400  // ???? undocumented
#define SR_DAT_INHIBIT       0x00000002
#define SR_CMD_INHIBIT       0x00000001

// Arguments used for specific commands:
//
// TODO: What's the correct voltage window for the RPi SD interface?
//       2.7v-3.6v (given by 0x00ff8000) or something narrower?
// TODO: For now, don't offer to switch voltage.
//
#define ACMD41_HCS           0x40000000
#define ACMD41_SDXC_POWER    0x10000000
#define ACMD41_S18R          0x01000000
#define ACMD41_VOLTAGE       0x00ff8000
#define ACMD41_ARG_HC        (ACMD41_HCS|ACMD41_SDXC_POWER|ACMD41_VOLTAGE|ACMD41_S18R)
#define ACMD41_ARG_SC        (ACMD41_VOLTAGE|ACMD41_S18R)

// R1 (status) values used:
//
#define ST_CARD_STATE        0x00001e00  // 12:9
#define ST_APP_CMD           0x00000020  // 5

#define R1_CARD_STATE_SHIFT  9
#define R1_ERRORS_MASK       0xfff9c004  // All bits which indicate errors.

// R3 (ACMD41 APP_SEND_OP_COND) used:
//
#define R3_COMPLETE    0x80000000
#define R3_CCS         0x40000000

// R6 (CMD3 SEND_REL_ADDR) used:
//
#define R6_RCA_MASK    0xffff0000

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

// Command indexes in the command table (just the used ones are listed, here):
//
#define IX_GO_IDLE_STATE    0
#define IX_ALL_SEND_CID     1
#define IX_SEND_REL_ADDR    2
#define IX_CARD_SELECT      5
#define IX_SEND_IF_COND     6
#define IX_SEND_CSD         7
#define IX_STOP_TRANS       10
#define IX_SET_BLOCKLEN     13
#define IX_READ_SINGLE      14
#define IX_READ_MULTI       15
#define IX_SET_BLOCKCNT     18
#define IX_WRITE_SINGLE     19
#define IX_WRITE_MULTI      20
#define IX_APP_CMD          29
#define IX_APP_CMD_RCA      30 // APP_CMD used once we have the RCA.

// Commands hereafter require APP_CMD (just the used ones are listed, here):
//
#define IX_APP_CMD_START    32
#define IX_SET_BUS_WIDTH    32
#define IX_APP_SEND_OP_COND 36
#define IX_SEND_SCR         38

// CSD flags:
//
// Note: All flags are shifted down by 8 bits as the CRC is not included.
//
// Most flags are common:
//  - in V1 the size is 12 bits with a 3 bit multiplier
//  - in V1 currents for read and write are specified
//  - in V2 the size is 22 bits, no multiplier, no currents

#define CSD1VN_READ_BL_LEN         0x00000f00
#define CSD1VN_READ_BL_LEN_SHIFT   8

#define CSD3VN_FILE_FORMAT         0x0000000c
#define CSD3VN_FILE_FORMAT_HDD     0x00000000
#define CSD3VN_FILE_FORMAT_DOSFAT  0x00000004

// CSD Version 1 flags.
#define CSD1V1_C_SIZEH             0x00000003
#define CSD1V1_C_SIZEH_SHIFT       10

#define CSD2V1_C_SIZEL             0xffc00000
#define CSD2V1_C_SIZEL_SHIFT       22
#define CSD2V1_C_SIZE_MULT         0x00000380
#define CSD2V1_C_SIZE_MULT_SHIFT   7

// CSD Version 2 flags.
#define CSD2V2_C_SIZE              0x3fffff00
#define CSD2V2_C_SIZE_SHIFT        8

// SCR flags used:
// NOTE: SCR is big-endian, so flags appear byte-wise reversed from the spec.

#define SCR_SD_BUS_WIDTH_1         0x00000100
#define SCR_SD_BUS_WIDTH_4         0x00000400

#define SCR_CMD_SUPP_SET_BLKCNT    0x02000000
#define SCR_CMD_SUPP_SPEED_CLASS   0x01000000

// SD card types used:
#define SD_TYPE_1    2
#define SD_TYPE_2_SC 3
#define SD_TYPE_2_HC 4

// SD card functions supported values.
#define SD_SUPP_SET_BLOCK_COUNT 0x80000000
#define SD_SUPP_SPEED_CLASS     0x40000000
#define SD_SUPP_BUS_WIDTH_4     0x20000000
#define SD_SUPP_BUS_WIDTH_1     0x10000000

#endif //MT_SDCARD_DEFINES
