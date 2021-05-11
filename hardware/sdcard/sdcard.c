
// Marcel Timm, RhinoDevel, 2019sep04

// *****************************************************************************
// Originally taken from:
//
// https://raw.githubusercontent.com/moizumi99/RPiHaribote/master/haribote/sdcard.c
//
// Includes comments by other authors, too.
//
// *****************************************************************************

// Taken from here
// https://www.raspberrypi.org/forums/viewtopic.php?f=72&t=94133
// Update: Control2 is removed according the the advice from LdB. Thanks a lot.
//
// Data transfer notes:
// The EMMC module restricts the maximum block size to the size of the internal data FIFO which is 1k bytes
// 0x80  Extension FIFO config - what's that?
// This register allows fine tuning the dma_req generation for paced DMA transfers when reading from the card.

#include "../mailbox/mailbox.h"
#include "../armtimer/armtimer.h"
#include "../gpio/gpio.h"
#include "../../lib/mem/mem.h"
#include "../../lib/console/console.h"
#include "../../lib/assert.h"
#include "../peribase.h"

#include "sdcard.h"
#include "sdcard_defines.h"

struct EMMCCommand
{
    unsigned int code;
    unsigned char resp;
    unsigned char rca;
    int delay;
};

struct SDDescriptor
{
    // Static informations:

    uint32_t csd3;
    unsigned int ocr;
    unsigned int support;
    unsigned char type;

    // Dynamic informations:

    unsigned int rca;
    unsigned int status;
};

// EMMC registers used:
//
static volatile uint32_t* const EMMC_BLKSIZECNT  = (uint32_t*)(PERI_BASE + 0x00300004);
static volatile uint32_t* const EMMC_ARG1        = (uint32_t*)(PERI_BASE + 0x00300008);
static volatile uint32_t* const EMMC_CMDTM       = (uint32_t*)(PERI_BASE + 0x0030000c);
static volatile uint32_t* const EMMC_RESP0       = (uint32_t*)(PERI_BASE + 0x00300010);
static volatile uint32_t* const EMMC_DATA        = (uint32_t*)(PERI_BASE + 0x00300020);
static volatile uint32_t* const EMMC_STATUS      = (uint32_t*)(PERI_BASE + 0x00300024);
static volatile uint32_t* const EMMC_CONTROL0    = (uint32_t*)(PERI_BASE + 0x00300028);
static volatile uint32_t* const EMMC_CONTROL1    = (uint32_t*)(PERI_BASE + 0x0030002c);
static volatile uint32_t* const EMMC_INTERRUPT   = (uint32_t*)(PERI_BASE + 0x00300030);
static volatile uint32_t* const EMMC_IRPT_MASK   = (uint32_t*)(PERI_BASE + 0x00300034);
static volatile uint32_t* const EMMC_IRPT_EN     = (uint32_t*)(PERI_BASE + 0x00300038);
static volatile uint32_t* const EMMC_SLOTISR_VER = (uint32_t*)(PERI_BASE + 0x003000fc);

// Command table:
//
// - TODO: TM_DAT_DIR_CH required in any of these?
//
static struct EMMCCommand sdCommandTable[] =
{
    {0x00000000|CMD_RSPNS_NO                             , RESP_NO , RCA_NO  ,0},
    {0x02000000|CMD_RSPNS_136                            , RESP_R2I, RCA_NO  ,0},
    {0x03000000|CMD_RSPNS_48                             , RESP_R6 , RCA_NO  ,0},
    {0x07000000|CMD_RSPNS_48B                            , RESP_R1b, RCA_YES ,0},
    {0x08000000|CMD_RSPNS_48                             , RESP_R7 , RCA_NO  ,100},
    {0x09000000|CMD_RSPNS_136                            , RESP_R2S, RCA_YES ,0},
    {0x0C000000|CMD_RSPNS_48B                            , RESP_R1b, RCA_NO  ,0},
    {0x10000000|CMD_RSPNS_48                             , RESP_R1 , RCA_NO  ,0},
    {0x11000000|CMD_RSPNS_48 |CMD_IS_DATA  |TM_DAT_DIR_CH, RESP_R1 , RCA_NO  ,0},
    {0x12000000|CMD_RSPNS_48 |TM_MULTI_DATA|TM_DAT_DIR_CH, RESP_R1 , RCA_NO  ,0},
    {0x17000000|CMD_RSPNS_48                             , RESP_R1 , RCA_NO  ,0},
    {0x18000000|CMD_RSPNS_48 |CMD_IS_DATA  |TM_DAT_DIR_HC, RESP_R1 , RCA_NO  ,0},
    {0x19000000|CMD_RSPNS_48 |TM_MULTI_DATA|TM_DAT_DIR_HC, RESP_R1 , RCA_NO  ,0},
    {0x37000000|CMD_RSPNS_NO                             , RESP_NO , RCA_NO  ,100},
    {0x37000000|CMD_RSPNS_48                             , RESP_R1 , RCA_YES ,0},

    // APP commands must be prefixed by an APP_CMD.
    {0x06000000|CMD_RSPNS_48                             , RESP_R1 , RCA_NO  ,0},
    {0x29000000|CMD_RSPNS_48                             , RESP_R3 , RCA_NO  ,1000},
    {0x33000000|CMD_RSPNS_48|CMD_IS_DATA|TM_DAT_DIR_CH   , RESP_R1 , RCA_NO  ,0}
};

static bool s_is_initialized = false; // Set by sdcard_init().
static struct SDDescriptor s_sdcard; // Prepared by sdcard_init().

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
    // Check for status indicating a command in progress:

    int count = 1000000;

    while((*EMMC_STATUS & SR_CMD_INHIBIT)
        && !(*EMMC_INTERRUPT & INT_ERROR_MASK)
        && count--)
    {
        armtimer_busywait_microseconds(1);
    }
    if(count <= 0 || (*EMMC_INTERRUPT & INT_ERROR_MASK))
    {
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

  int count = 0;
  while( (*EMMC_STATUS & SR_DAT_INHIBIT) && !(*EMMC_INTERRUPT & INT_ERROR_MASK) && ++count < 500000 )
    armtimer_busywait_microseconds(1);
  if( count >= 500000 || (*EMMC_INTERRUPT & INT_ERROR_MASK) )
    {
    return SD_BUSY;
    }

  return SD_OK;
  }

/* Send command and handle response.
 */
static int sdSendCommandP( struct EMMCCommand * cmd, int arg )
  {
  // Check for command in progress
  if( sdWaitForCommand() != 0 )
    return SD_BUSY;

  int result;

  clear_interrupt_reg();

  // Set the argument and the command code.
  // Some commands require a delay before reading the response.

  *EMMC_ARG1 = arg;
  *EMMC_CMDTM = cmd->code;
  if( cmd->delay ) armtimer_busywait_microseconds(cmd->delay);

  // Wait until command complete interrupt.
  if( (result = wait_for_interrupt(INT_CMD_DONE)) )
  {
      return result;
  }

  // Get response from RESP0.
  int const resp0 = *EMMC_RESP0;

  // Handle response types.
  switch(cmd->resp)
    {
    // No response.
    case RESP_NO:
      return SD_OK;

    // RESP0 contains card status, no other data from the RESP* registers.
    // Return value non-zero if any error flag in the status value.
    case RESP_R1:
    case RESP_R1b:
      s_sdcard.status = resp0;
      return resp0 & R1_ERRORS_MASK;

    // RESP0..3 contains 128 bit CID or CSD shifted down by 8 bits as no CRC.
    //
    // Note: Highest bits are in RESP3:
    //
    case RESP_R2I: // CID
    {
        s_sdcard.status = 0;
        return SD_OK; // Not interested in CID.
    }
    case RESP_R2S: // CSD
    {
        s_sdcard.status = 0;
        s_sdcard.csd3 = resp0; // (interested in lowest bits, only)
        return SD_OK;
    }

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
  int resp;
  // If no RCA, send the APP_CMD and don't look for a response.
  if( !s_sdcard.rca )
    sdSendCommandP(&sdCommandTable[IX_APP_CMD],0x00000000);

  // If there is an RCA, include that in APP_CMD and check card accepted it.
  else
    {
    if( (resp = sdSendCommandP(&sdCommandTable[IX_APP_CMD_RCA],s_sdcard.rca)) )
    {
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
      if((resp = sdSendAppCommand()))
      {
          return resp;
      }
  }

  // Get the command and set RCA if required.
  struct EMMCCommand * cmd = &sdCommandTable[index];
  int arg = 0;
  if( cmd->rca == RCA_YES )
  {
    arg = s_sdcard.rca;
  }
    if( (resp = sdSendCommandP(cmd,arg)) )
    {
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
    unsigned int scr[2];

  // SEND_SCR command is like a READ_SINGLE but for a block of 8 bytes.
  // Ensure that any data operation has completed before reading the block.
  if( sdWaitForData() )
  {
      return SD_TIMEOUT;
  }

  // Set BLKSIZECNT to 1 block of 8 bytes, send SEND_SCR command
  *EMMC_BLKSIZECNT = (1 << 16) | 8;
  int resp;
  if( (resp = sdSendCommand(IX_SEND_SCR)) ) return resp;

  // Wait for READ_RDY interrupt.
  if( (resp = wait_for_interrupt(INT_READ_RDY)) )
    {
    return resp;
    }

  // Allow maximum of 100ms for the read operation.
  int numRead = 0, count = 100000;
  while( numRead < 2 )
    {
    if( *EMMC_STATUS & SR_READ_AVAILABLE )
      scr[numRead++] = *EMMC_DATA;
    else
      {
      armtimer_busywait_microseconds(1);
      if( --count == 0 ) break;
      }
    }

  // If SCR not fully read, the operation timed out.
  if( numRead != 2 )
    {
    return SD_TIMEOUT;
    }

  // Parse out the SCR.  Only interested in values in scr[0], scr[1] is mfr specific.
  if( scr[0] & SCR_SD_BUS_WIDTH_4 ) s_sdcard.support |= SD_SUPP_BUS_WIDTH_4;
  if( scr[0] & SCR_SD_BUS_WIDTH_1 ) s_sdcard.support |= SD_SUPP_BUS_WIDTH_1;
  if( scr[0] & SCR_CMD_SUPP_SET_BLKCNT ) s_sdcard.support |= SD_SUPP_SET_BLOCK_COUNT;
  if( scr[0] & SCR_CMD_SUPP_SPEED_CLASS ) s_sdcard.support |= SD_SUPP_SPEED_CLASS;

  return SD_OK;
}

/* Common routine for APP_SEND_OP_COND.
 * This is used for both SC and HC cards based on the parameter.
 */
static int sdAppSendOpCond( int arg )
{
    // Send APP_SEND_OP_COND with the given argument (for SC or HC cards).

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
#ifndef NDEBUG
            console_write("sdAppSendOpCond : Returning with code ");
            console_write_dword_dec((uint32_t)resp);
            console_writeline(" (1).");
#endif //NDEBUG
            return resp;
        }
    }

    while(
        !(s_sdcard.ocr & R3_COMPLETE)
        && (armtimer_get_tick() - start_tick < max_tick))
    {
        if((resp = sdSendCommandA(IX_APP_SEND_OP_COND, arg)))
        {
            if(resp != SD_TIMEOUT )
            {
#ifndef NDEBUG
                console_write("sdAppSendOpCond : Returning with code ");
                console_write_dword_dec((uint32_t)resp);
                console_writeline(" (2).");
#endif //NDEBUG
                return resp;
            }
        }
    }

    // Return timeout error if still not busy.
    if(!(s_sdcard.ocr & R3_COMPLETE))
    {
        console_deb_writeline("sdAppSendOpCond : Error: Timeout!");
        return SD_TIMEOUT;
    }

    // Check that at least one voltage value was returned.
    if(!(s_sdcard.ocr & ACMD41_VOLTAGE))
    {
        console_deb_writeline("sdAppSendOpCond : Error: Voltage!");
        return SD_ERROR_VOLTAGE;
    }

    return SD_OK;
}

/** Initialize GPIO.
 */
static void init_gpio()
{
    gpio_set_func(GPIO_DAT3, gpio_func_alt3);
    gpio_set_pud(GPIO_DAT3, gpio_pud_up);

    gpio_set_func(GPIO_DAT2, gpio_func_alt3);
    gpio_set_pud(GPIO_DAT2, gpio_pud_up);

    gpio_set_func(GPIO_DAT1, gpio_func_alt3);
    gpio_set_pud(GPIO_DAT1, gpio_pud_up);

    gpio_set_func(GPIO_DAT0, gpio_func_alt3);
    gpio_set_pud(GPIO_DAT0, gpio_pud_up);

    gpio_set_func(GPIO_CMD, gpio_func_alt3);
    gpio_set_pud(GPIO_CMD, gpio_pud_up);
    
    gpio_set_func(GPIO_CLK, gpio_func_alt3);
    gpio_set_pud(GPIO_CLK, gpio_pud_up);
}

/** Check and return, if EMMC clock rate has the expected (hard-coded) value.
 */
static int is_clockrate_emmc_compatible()
{
    uint32_t const r = mailbox_read_clockrate(mailbox_id_clockrate_emmc);

    if(r == UINT32_MAX)
    {
        console_deb_writeline(
            "is_clockrate_emmc_compatible: Error: Failed to read EMMC clock rate!");
        return SD_ERROR;
    }

#ifndef NDEBUG
    console_write("is_clockrate_emmc_compatible: Rate is ");
    console_write_dword_dec(r);
    console_writeline(".");
#endif //NDEBUG

    if(r != SD_REQ_CLOCKRATE_EMMC)
    {
      console_deb_writeline(
            "is_clockrate_emmc_compatible: Error: Clock rate is not compatible!");
        return SD_ERROR;
    }
    return SD_OK;
}

/** Get the clock divider for the given requested frequency.
 *  This is calculated relative to the SD base clock.
 */
static uint32_t get_clock_divider(uint32_t const freq)
{
    // SD card frequency should always be ~41.667MHz, as long as EMMC frequency
    // is at 250MHz (250Mhz / 6 = ~41.667MHz).
    //
    uint32_t const closest_divider = 41666666 / freq;

    // Host versions higher than 2 take closest divider (using 2 as min., here):
    //
    uint32_t const divider = closest_divider < 2 ? 2 : closest_divider;

    // 10 bits on host versions above v2:
    //
    uint32_t const lo = (divider & 0x000000FF) << 8; // 11111111 00000000
    uint32_t const hi = (divider & 0x00000300) >> 2; // 00000000 11000000

    return lo + hi;
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
        return SD_ERROR_CLOCK;
    }

    // Switch clock off:

    *EMMC_CONTROL1 &= ~C1_CLK_EN;
    armtimer_busywait_microseconds(10);

    // Request the new clock setting and enable the clock:

    int const cdiv = get_clock_divider(freq);

#ifndef NDEBUG
    console_write("sdSetClock: Calc. clock divider for wanted frequency val. ");
    console_write_dword_dec((uint32_t)freq);
    console_write(" is ");
    console_write_dword_dec((uint32_t)cdiv);
    console_writeline(".");
#endif //NDEBUG

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
        return SD_ERROR_CLOCK;
    }

    return SD_OK;
}

/* Reset card.
 */
static int reset_card()
{
    static int const reset_type = C1_SRST_HC;
    static int const max_iter = 10000;

    int resp = SD_ERROR, i = 0;

    // Send reset command to host controller and wait for completion:

    *EMMC_CONTROL0 = 0;
    *EMMC_CONTROL1 = *EMMC_CONTROL1 | reset_type;

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
        console_deb_writeline("reset_card : Error: Max. iteration reached!");
        return SD_ERROR_RESET;
    }

    // Enable internal clock and set data timeout:

    *EMMC_CONTROL1 = *EMMC_CONTROL1 | (C1_CLK_INTLEN | C1_TOUNIT_MAX);
    //
    // Correct timeout value?
    //
    armtimer_busywait_microseconds(10);

    // Set clock to setup frequency:
    //
    resp = sdSetClock(FREQ_SETUP);
    if(resp != SD_OK)
    {
        console_deb_writeline("reset_card : Error: Set clock failed!");
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
    s_sdcard.status = 0;
    s_sdcard.type = 0;

    return sdSendCommand(IX_GO_IDLE_STATE);
}

// ************************
// *** PUBLIC FUNCTIONS ***
// ************************

/** Transfer multiple contiguous blocks between the given address on the card 
 *  and the buffer.
 */
int sdcard_blocks_transfer(long long address, int numBlocks, unsigned char* buffer, int write)
{
    if( !s_is_initialized ) return SD_NO_RESP;

    // Ensure that any data operation has completed before doing the transfer.
    if( sdWaitForData() )
    {
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
    //#define TM_AUTO_CMD12    0x00000004
    *EMMC_BLKSIZECNT = (numBlocks << 16) | 512;

    if( (resp = sdSendCommandA(transferCmd,blockAddress)) ) return resp;

    // Transfer all blocks.
    int blocksDone = 0;
    while( blocksDone < numBlocks )
      {
      // Wait for ready interrupt for the next block.
      if( (resp = wait_for_interrupt(readyInt)) )
        {
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
          if( !write && numBlocks > 1)
          {
              resp = sdSendCommand(IX_STOP_TRANS);
          }

      return SD_TIMEOUT;
    }

    // For a write operation, ensure DATA_DONE interrupt before we stop transmission.
    if( write && (resp = wait_for_interrupt(INT_DATA_DONE)) )
      {
      return resp;
      }

    // For a multi-block operation, if SET_BLOCKCNT is not supported, we need to indicate
    // that there are no more blocks to be transferred.
    if( numBlocks > 1 && !(s_sdcard.support & SD_SUPP_SET_BLOCK_COUNT) &&
        (resp = sdSendCommand(IX_STOP_TRANS)) ) return resp;

    return SD_OK;
}

int sdcard_init()
{
    int resp = SD_ERROR;

    if(s_is_initialized)
    {
        return SD_ALREADY_INITIALIZED;
    }

    // Check clock rate:
    //
    resp = is_clockrate_emmc_compatible();
    if(resp != SD_OK) // (function logs in debug mode)
    {
        return resp;
    }

    memset(&s_sdcard, 0, sizeof(struct SDDescriptor)); // Necessary?

    init_gpio();

    uint32_t const host_ver = (*EMMC_SLOTISR_VER & 0x00FF0000) >> 16;
    //
    if(host_ver <= HOST_SPEC_V2)
    {
        console_deb_writeline(
          "sdcard_init: Error: Unsupported host spec. version!");
        return SD_ERROR;
    }

    // Reset the card:
    //
    resp = reset_card();
    if(resp != SD_OK)
    {
        console_deb_writeline("sdcard_init: Error: Card reset failed!");
        return resp;
    }

    // Send SEND_IF_COND, 0x000001AA (CMD8) voltage range 0x1 check pattern
    // 0xAA. If voltage range and check pattern don't match, look for older
    // card:
    //
    resp = sdSendCommandA(IX_SEND_IF_COND, 0x000001AA);
    if(resp == SD_OK)
    {
        // Card responded with voltage and check pattern.
        // Resolve voltage and check for high capacity card:

        resp = sdAppSendOpCond(ACMD41_ARG_HC);
        if(resp != SD_OK)
        {
            return resp;
        }

        // Check for high or standard capacity:
        //
        s_sdcard.type = (s_sdcard.ocr & R3_CCS) == 0
            ? SD_TYPE_2_SC : SD_TYPE_2_HC;
    }
    else
    {
        if(resp == SD_BUSY)
        {
            console_deb_writeline("sdcard_init: Error: SD card is busy!");
            return resp;
        }

        // No response to SEND_IF_COND, treat as an old card:

        // If there appears to be a command in progress, reset the card:
        //
        if((*EMMC_STATUS & SR_CMD_INHIBIT) != 0)
        {
            console_deb_writeline(
                "sdcard_init: Resetting card a second time..");
            resp = reset_card();
            if(resp != SD_OK)
            {
                console_deb_writeline(
                    "sdcard_init: Error: Reset (because command seems to be in progress) failed!");
                return resp;
            }
        }

        // Resolve voltage:
        //
        resp = sdAppSendOpCond(ACMD41_ARG_SC);
        if(resp != SD_OK)
        {
#ifndef NDEBUG
            console_write(
                "sdcard_init: Error: Resolving voltage failed (error code: ");
            console_write_dword_dec(resp);
            console_writeline(")!");
#endif //NDEBUG
            return resp;
        }

        s_sdcard.type = SD_TYPE_1;
    }

    // Send ALL_SEND_CID (CMD2):
    //
    resp = sdSendCommand(IX_ALL_SEND_CID);
    if(resp != SD_OK)
    {
        return resp;
    }

    // Send SEND_REL_ADDR (CMD3):
    //
    resp = sdSendCommand(IX_SEND_REL_ADDR);
    if(resp != SD_OK)
    {
        return resp;
    }

    // From now on the card should be in stand-by state.
    // Actually cards seem to respond in identify state at this point.
    // Check this with a SEND_STATUS (CMD13).

    // Send SEND_CSD (CMD9) and parse the result:
    //
    resp = sdSendCommand(IX_SEND_CSD);
    if(resp != SD_OK)
    {
        return resp;
    }

    uint32_t const file_format = s_sdcard.csd3 & CSD3VN_FILE_FORMAT;

    if(file_format != CSD3VN_FILE_FORMAT_DOSFAT 
        && file_format != CSD3VN_FILE_FORMAT_HDD)
    {
        console_deb_writeline(
            "sdcard_init: Error: Unsupported SD card file format!");
        return SD_ERROR;
    }

    // At this point, set the clock to full speed:

    resp = sdSetClock(FREQ_NORMAL);
    if(resp != SD_OK)
    {
        return resp;
    }

    // Send CARD_SELECT (CMD7).
    //
    // TODO: Check card_is_locked status in the R1 response from CMD7 [bit 25],
    //       if so, use CMD42 to unlock.
    //
    // CMD42 structure [4.3.7] same as a single block write; data block includes
    // PWD setting mode, PWD len, PWD data.
    //
    resp = sdSendCommand(IX_CARD_SELECT);
    if(resp != SD_OK)
    {
        return resp;
    }

    // Get the SCR as well.
    // Need to do this before sending ACMD6 so that allowed bus widths are
    // known:
    //
    resp = sdReadSCR();
    if(resp != SD_OK)
    {
        return resp;
    }

    // Send APP_SET_BUS_WIDTH (ACMD6).
    // If supported, set 4 bit bus width and update the CONTROL0 register:
    //
    if((s_sdcard.support & SD_SUPP_BUS_WIDTH_4) != 0)
    {
        resp = sdSendCommandA(IX_SET_BUS_WIDTH, s_sdcard.rca | 2);
        if(resp != SD_OK)
        {
            return resp;
        }

        *EMMC_CONTROL0 = *EMMC_CONTROL0 | C0_HCTL_DWITDH;
    }

    // Only needs to be set for SDSC cards.
    // For SDHC and SDXC cards block length is fixed at 512 anyway:
    //
    resp = sdSendCommandA(IX_SET_BLOCKLEN, 512);
    if(resp != SD_OK)
    {
        return resp;
    }

    s_is_initialized = true;
    return SD_OK;
}
