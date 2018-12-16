
// Marcel Timm, RhinoDevel, 2018dec12

#include <stdbool.h>

#include "watchdog.h"
#include "../mem/mem.h"
#include "../peribase.h"

// Power management, reset controller and watchdog registers:
//
static uint32_t const PM_BASE = PERI_BASE + 0x100000;
static uint32_t const PM_RSTC = PM_BASE + 0x1C;
static uint32_t const PM_WDOG = PM_BASE + 0x24;
//static uint32_t const PM_WDOG_RESET = 0;
static uint32_t const PM_PASSWORD = 0x5A000000;
static uint32_t const PM_WDOG_TIME_SET = 0xFFFFF;
static uint32_t const PM_RSTC_WRCFG_CLR = 0xFFFFFFCF;
//static uint32_t const PM_RSTC_WRCFG_SET = 0x30;
static uint32_t const PM_RSTC_WRCFG_FULL_RESET = 0x20;
//static uint32_t const PM_RSTC_RESET = 0x102;

static void start(uint32_t const timeout)
{
    // Setup watchdog for reset:
    //
    // - Watchdog timer = Timer clock / 16.
    // - Needs password (31:16) and value (11:0).

    uint32_t const pm_wdog = PM_PASSWORD | (timeout & PM_WDOG_TIME_SET),
        pm_rstc = PM_PASSWORD
            | (mem_read(PM_RSTC) & PM_RSTC_WRCFG_CLR)
            | PM_RSTC_WRCFG_FULL_RESET;

    mem_write(PM_WDOG, pm_wdog);
    mem_write(PM_RSTC, pm_rstc);
}

// static void stop()
// {
//     mem_write(PM_RSTC, PM_PASSWORD | PM_RSTC_RESET);
// }

void watchdog_reset_system()
{
    start(1);

	while(true)
    {
        // Infinite loop.
    }
}
