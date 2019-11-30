
// Marcel Timm, RhinoDevel, 2018dec11

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#include <stdint.h>

#include "../hardware/peribase.h"
#include "../hardware/armtimer/armtimer.h"
#include "../hardware/irqcontroller/irqcontroller.h"
#include "../hardware/miniuart/miniuart.h"
//#include "../hardware/pl011uart/pl011uart.h"

#include "../hardware/gpio/gpio_params.h"
#include "../hardware/gpio/gpio.h"

#include "../lib/console/console.h"
#include "ui/ui_terminal_to_commodore.h"
#include "ui/ui_commodore_to_terminal.h"
#include "ui/ui.h"
#include "config.h"
#include "../lib/alloc/alloc.h"
#include "../lib/assert.h"
#include "../lib/mem/mem.h"
#include "../lib/basic/basic.h"
#include "../lib/basic/basic_addr.h"
#include "tape/tape_init.h"
#include "statetoggle/statetoggle.h"

#include "cbm/cbm_receive.h"
#include "cbm/cbm_send.h"

#if PERI_BASE == PERI_BASE_PI1
    #define VIDEO_SUPPORT 1
#else //PERI_BASE == PERI_BASE_PI1
    #define VIDEO_SUPPORT 0
#endif //PERI_BASE == PERI_BASE_PI1

#if VIDEO_SUPPORT
    #include "../lib/video/video.h"
#endif //VIDEO_SUPPORT

#ifndef NDEBUG
    #include "../lib/ff14/source/ff.h"
    #include "../lib/dir/dir.h"
#endif //NDEBUG

extern void _enable_interrupts(); // See boot.S.

extern uint32_t __heap; // See memmap.ld.

static bool s_timer_irq_state = false;

/** IRQ interrupt handler.
 */
void __attribute__((interrupt("IRQ"))) handler_irq()
{
    armtimer_irq_clear();

    s_timer_irq_state = !s_timer_irq_state;

    gpio_set_output(GPIO_PIN_NR_ACT, s_timer_irq_state);
}

#ifndef MT_INTERACTIVE
    /**
     * - Must not be called. Just for error handling..
     */
    static uint8_t dummy_read()
    {
        assert(false);

        return 0;
    }

    static void toggle_gpio(
        uint32_t const pin_nr,
        int const count,
        uint32_t const milliseconds,
        bool const is_high)
    {
        uint32_t const microseconds = 1000 * milliseconds;
        int i = 0;

        while(true)
        {
            gpio_set_output(pin_nr, !is_high);
            armtimer_busywait_microseconds(microseconds);
            gpio_set_output(pin_nr, is_high);

            ++i;
            if(i == count)
            {
                return;
            }

            armtimer_busywait_microseconds(microseconds);
        }
    }
#endif //MT_INTERACTIVE

#if VIDEO_SUPPORT
#else //VIDEO_SUPPORT
    static void dummy_write(uint8_t const byte)
    {
        (void)byte; // Doing nothing.
    }
#endif //VIDEO_SUPPORT

/** Connect console (singleton) to wanted in-/output.
 */
static void init_console()
{
    struct console_params p;

    // Initialize console via MiniUART to read and video to write:

    // No read in non-interactive mode:
    //
#ifdef MT_INTERACTIVE
    p.read_byte = miniuart_read_byte;
#else //MT_INTERACTIVE
    p.read_byte = dummy_read;
#endif //MT_INTERACTIVE

    // p.write_byte = miniuart_write_byte;
    // //
    // // If MiniUART is also used for serial transfer, using UART as console
    // // may cause serial transfers to fail (e.g. assertions and debug output)!
    // //
    // p.write_newline_with_cr = true;
    //
#if VIDEO_SUPPORT
    p.write_byte = video_write_byte;
#else //VIDEO_SUPPORT
    p.write_byte = dummy_write;
#endif //VIDEO_SUPPORT

    p.write_newline_with_cr = false;

    miniuart_init();

    // // Initialize console via PL011 UART (use this for QEMU):
    //
    // p.read_byte = pl011uart_read_byte;
    //
    // p.write_byte = pl011uart_write_byte;
    // //
    // // If PL011 UART is also used for serial transfer, using UART as console
    // // may cause serial transfers to fail (e.g. assertions and debug output)!
    // //
    // p.write_newline_with_cr = true;
    //
    // pl011uart_init(); // (don't do this while using QEMU)

    console_init(&p);
}

#ifndef NDEBUG
    static void ff_test()
    {
        FATFS fatfs;
        FIL f;
        UINT buf_uint;
        FRESULT res;
        struct dir_entry * * entry_arr = 0;
        int entry_count = -1;

        if(f_mount(&fatfs, "", 0) != FR_OK)
        {
            console_writeline("ff_test : Error: Mount failed!");
            return;
        }

        if(!dir_reinit("/"))
        {
            console_writeline("ff_test : Error: dir singleton init failed!");
            return;
        }

        console_write("ff_test : Folder name   : \"");
        console_write(dir_get_dir_path());
        console_writeline("\".");

        entry_arr = dir_create_entry_arr(&entry_count);
        if(entry_count == -1)
        {
            console_writeline("ff_test : Error: Entry array creation failed!");
            return;
        }

        if(entry_count == 0)
        {
            console_writeline("ff_test : No entries found.");
        }
        else
        {
            for(int i = 0;i < entry_count; ++i)
            {
                console_write(entry_arr[i]->is_dir
                    ? "ff_test : Subfolder name: \""
                    : "ff_test : File name     : \"");
                console_write(entry_arr[i]->name);
                console_writeline("\".");

                if(mem_cmp_byte(
                    (uint8_t const *)entry_arr[i]->name,
                    (uint8_t const *)"RHINODEV.TXT",
                    11 + 1))
                {
                    if(f_unlink("/RHINODEV.TXT") != FR_OK)
                    {
                        console_writeline(
                            "ff_test : Error: Failed to remove file!");
                        return;
                    }
                }
                else if(mem_cmp_byte(
                    (uint8_t const *)entry_arr[i]->name,
                    (uint8_t const *)"CMDLINE.TXT",
                    11 + 1))
                {
                    char stupid_buf[1024];
                    UINT bytes_read;
                    if(f_open(&f, "/CMDLINE.TXT", FA_READ) != FR_OK)
                    {
                        console_writeline("ff_test : Error: Opening file failed!");
                        return;
                    }

                    res = f_read(&f, stupid_buf, 1024, &bytes_read);
                    if(res!= FR_OK)
                    {
                        console_write(
                            "ff_test : Error: Reading from file failed (");
                        console_write_dword_dec((uint32_t)res);
                        console_writeline(")!");
                        return;
                    }
                    if(f_close(&f) != FR_OK)
                    {
                        console_writeline(
                            "ff_test : Error: Closing file failed (1)!");
                        return;
                    }

                    stupid_buf[bytes_read] = '\0';

                    console_write("ff_test : File content: \"");
                    console_write(stupid_buf);
                    console_writeline("\".");
                }
            }

            dir_free_entry_arr(entry_arr, entry_count);
            entry_arr = 0;
            entry_count = -1;
        }

        if(!dir_deinit())
        {
            console_writeline("ff_test : Error: dir singleton deinit. failed!");
            return;
        }

        res = f_open(&f, "/RHINODEV.TXT", FA_CREATE_NEW | FA_WRITE);
        if(res!= FR_OK)
        {
            console_write("ff_test : Error: Creating file failed (");
            console_write_dword_dec((uint32_t)res);
            console_writeline(")!");
            return;
        }

        res = f_write(&f, "RhinoDevel was here.", 20, &buf_uint);
        if(res!= FR_OK)
        {
            console_write(
                "ff_test : Error: Writing to file failed (");
            console_write_dword_dec((uint32_t)res);
            console_writeline(")!");
            return;
        }

        if(f_close(&f) != FR_OK)
        {
            console_writeline("ff_test : Error: Closing file failed (2)!");
            return;
        }

        console_write("ff_test : fs_type = 0x");
        console_write_byte(fatfs.fs_type);
        console_writeline(".");
        console_write("ff_test : n_rootdir = 0x");
        console_write_word(fatfs.n_rootdir);
        console_writeline(".");

        if(f_mount(0, "", 0) != FR_OK)
        {
            console_writeline("ff_test : Error: Unmounting failed!");
            return;
        }
    }

    // static void sdcard_test()
    // {
    // 	unsigned char carddata[512];
    // 	uint32_t fileSystemDescriptor_0, firstSectorNumbers_0;//, numberOfSectors_0;
    // 	int i;//, j, fatsize;
    // 	uint32_t bytePerSector;
    // 	//uint32_t reservedSectors;
    // 	uint32_t numberOfFATs;
    // 	//uint32_t rootEntries;
    // 	//uint32_t bigTotalSectors;
    // 	//uint32_t bigSectorsPerFAT;
    // 	//uint32_t extFlag;
    // 	//uint32_t FS_version, rootDirStrtClus, FSInfoSec, bkUpBootSec;
    // 	//uint32_t driveNumber, extBootSignature, serialNumber, volumeLabel;
    // 	char sig_0, sig_1;
    //
    // 	//uint32_t *fatdata;
    //
    // 	armtimer_busywait_microseconds(10000);
    //
    // 	/* Read MBR */
    // 	if ((i=sdcard_blocks_transfer((long long) 0, 1, carddata, 0)) != 0)
    //     {
    // 		console_write("Error: sdcard_blocks_transfer: ");
    //         console_write_dword_dec((uint32_t)i);
    //         console_writeline("!");
    // 		return;
    // 	}
    // 	//	printf("MBR\n");
    // 	//	dump(carddata, 512);
    // 	/* check the first partiion */
    // 	//	bootDescriptor_0 = carddata[446];
    // 	fileSystemDescriptor_0 = carddata[446+4];
    // 	console_write("File System Descriptor = 0x");
    //     console_write_byte((uint8_t)fileSystemDescriptor_0);
    //     console_writeline(".");
    // 	if (fileSystemDescriptor_0 != 0x04 &&fileSystemDescriptor_0 != 0x06 && fileSystemDescriptor_0 != 0xb && fileSystemDescriptor_0 != 0x0c && fileSystemDescriptor_0 != 0x0e) {
    // 		// 01: FAT12 (not supported)
    // 		// 04: FAT16 (<32MB)
    // 		// 05: Extended DOS (not supported)
    // 		// 06: FAT16 (>=32MB)
    // 		// 0b: FAT32 (>2GB)
    // 		// 0c: FAT32 (Int 32h)
    // 		// 0e: FAT16 (Int 32h)
    // 		console_writeline("Error. Only support FAT\n");
    // 		return;
    // 	}
    // 	firstSectorNumbers_0 = carddata[446+8] + (carddata[446+9]<<8) + (carddata[446+10]<<16) + (carddata[446+11]<<24);
    // 	//numberOfSectors_0 = carddata[446+12] + (carddata[446+13]<<8) + (carddata[446+14]<<16) + (carddata[446+15]<<24);
    // 	//	printf("first Sector Number: %08x\n", firstSectorNumbers_0);
    //
    // 	/* Read the BPB of the first partition */
    // 	if ((i=sdcard_blocks_transfer((long long) firstSectorNumbers_0*512, 1, carddata, 0)) != 0) {
    // 		console_write("Error: sdcard_blocks_transfer (BPB): ");
    //         console_write_dword_dec((uint32_t)i);
    //         console_writeline("!");
    // 		return;
    // 	}
    // 	//	printf("BPB\n");
    // 	//	dump(carddata, 512);
    // 	bytePerSector = carddata[11] + (carddata[12]<<8);
    // 	//sectorsPerCluster = carddata[13];
    // 	//reservedSectors = carddata[14] + (carddata[15]<<8);
    // 	numberOfFATs = carddata[16];
    // 	//rootEntries = carddata[17] + (carddata[18]<<8);
    // 	//sectorsPerFAT = carddata[22] + (carddata[23]<<8);
    // 	//bigTotalSectors = carddata[32] + (carddata[33]<<8) + (carddata[34]<<16) + (carddata[35]<<24);
    // 	sig_0 = carddata[510];
    // 	sig_1 = carddata[511];
    // 	//	printf("bytePerSector: %08x\n", bytePerSector);
    // 	//	printf("sectorsPerCluster: %08x\n", sectorsPerCluster);
    // 	//	printf("reservedSectors: %08x\n", reservedSectors);
    // 	//	printf("numberOfFATs: %08x\n", numberOfFATs);
    // 	//	printf("sectorsPerFAT: %08x\n", sectorsPerFAT);
    // 	//	printf("bigTotalSectors: %08x\n", bigTotalSectors);
    // 	//FAT_type = 0;
    // 	// if (sectorsPerFAT == 0) {
    // 	// 	FAT_type = 1; //FAT32
    // 	// 	// todo: check the number of clusters http://elm-chan.org/docs/fat.html#fsinfo
    // 	// 	// read FAT32 BPB
    // 	// 	bigSectorsPerFAT = carddata[36] + (carddata[37]<<8) + (carddata[38]<<16) + (carddata[39]<<24);
    // 	// 	sectorsPerFAT = bigSectorsPerFAT;
    // 	// 	extFlag = carddata[40] + (carddata[41]<<8);
    // 	// 	FS_version = carddata[42] + (carddata[43]<<8);
    // 	// 	rootDirStrtClus = carddata[44] + (carddata[45]<<8) + (carddata[46]<<16) + (carddata[47]<<24);
    // 	// 	FSInfoSec = carddata[48] + (carddata[49]<<8);
    // 	// 	bkUpBootSec = carddata[50] + (carddata[51]<<8);
    // 	// 	//		printf("bigSectorsPerFAT: %08x\n", bigSectorsPerFAT);
    // 	// 	//		printf("extFlag: %04x\n", extFlag);
    // 	// 	//		printf("FS Version: %04x\n", FS_version);
    // 	// 	//		printf("rootDirStrtClus: %08x\n", rootDirStrtClus);
    // 	// 	//		printf("FSInfoSec: %04x\n", FSInfoSec);
    // 	// 	//		printf("bkUpBootSec: %04x\n", bkUpBootSec);
    // 	// }
    // 	if (bytePerSector!=512) {
    // 		console_writeline("Error: byte per sector is not 512\n");
    // 		return;
    // 	}
    // 	if (numberOfFATs != 2) {
    // 		console_write("Warning: The number of FATs is not two but ");
    //         console_write_dword_dec((uint32_t)numberOfFATs);
    //         console_writeline(".");
    // 	}
    // 	//	if (rootEntries != 512) {
    // 	//		printf("Warning: The number of rootEntries is not 512 but %d\n", rootEntries);
    // 	//	}
    // 	if (sig_0 != 0x55 || sig_1 != 0xaa) {
    // 		console_write("Error: signature is not 55 AA but ");
    //         console_write_byte((uint8_t)sig_0);
    //         console_write(" ");
    //         console_write_byte((uint8_t)sig_1);
    //         console_writeline("!");
    // 		return;
    // 	}
    //
    // 	// /* read fat */
    // 	// fatadr = (firstSectorNumbers_0 + reservedSectors)*512;
    // 	// fatsize = sectorsPerFAT;
    // 	// fat_bytesize = (FAT_type==0) ? fatsize*512*2 : fatsize*512;
    // 	// //	printf("fatadr: %08x, fatsize: %08x\n", (unsigned) fatadr, fatsize);
    // 	// fatdata = (uint32_t *) memman_alloc_4k(memman, fat_bytesize);
    // 	// file_readfat(memman, fatdata, fatadr);
    // 	// //	printf("FAT\n");
    // 	// //	dump((char *) fatdata, 512);
    // 	// /* read rde (Rood DIrection Entry) */
    // 	// rdeadr = fatadr + fatsize*numberOfFATs*bytePerSector;
    // 	// rdesize = 32*rootEntries/bytePerSector;
    // 	// usradr = rdeadr + rdesize*512;
    // 	// usrsize = bigTotalSectors - reservedSectors - numberOfFATs*sectorsPerFAT - rootEntries*32/bytePerSector;
    // 	// //	printf("rdeadr: %08x, rdesize: %08x\n", (unsigned) rdeadr, rdesize);
    // 	// //	printf("usradr: %08x, usrsize: %08x\n", (unsigned) usradr, usrsize);
    //     //
    // 	// if (FAT_type==0) {
    // 	// 	rdedata = (char *) memman_alloc_4k(memman, 512*rdesize);
    // 	// 	if ((i = sdcard_blocks_transfer(rdeadr, rdesize, rdedata, 0))!=0) {
    // 	// 		console_write("Error: sdcard_blocks_transfer (ROOT):%d\n", i);
    // 	// 		return;
    // 	// 	}
    // 	// } else {
    // 	// 	rdedata = (char *) memman_alloc_4k(memman, 512*32);
    // 	// 	file_loadfile(rootDirStrtClus, 32*512, rdedata, fatdata, usradr);
    // 	// }
    // 	// //	printf("RDE\n");
    // 	// //	dump(rdedata, 512*8);
    // }
#endif //NDEBUG

/**
 * - Entry point (see boot.S).
 */
void kernel_main(uint32_t r0, uint32_t r1, uint32_t r2)
{
    // To be able to compile, although these are not used (stupid?):
    //
    (void)r0;
    (void)r1;
    (void)r2;

    // Initialize GPIO singleton:
    //
    gpio_init((struct gpio_params){
        .wait_microseconds = armtimer_busywait_microseconds,

        .mem_read = mem_read,
        .mem_write = mem_write,

        .peri_base = PERI_BASE
    });

    // Initialize console in-/output:
    //
    init_console();

    // Does not work for Raspberry Pi 2, yet:
    //
    // Initialize video:
    //
#if VIDEO_SUPPORT
    video_init();
#endif //VIDEO_SUPPORT

    // Initialize memory (heap) manager for dynamic allocation/deallocation:
    //
    alloc_init(&__heap, MT_HEAP_SIZE);

    // Initialize for tape transfer:
    //
    tape_init(
        armtimer_start_one_mhz,
        armtimer_get_tick,
        armtimer_busywait_microseconds);

    irqcontroller_irq_src_enable_armtimer();
    _enable_interrupts();
    //
    // Timer counts down 250.000 times in one second (with 250 kHz):
    //
    armtimer_start(250000 * 1, 500); // 0.5 seconds, hard-coded for 250MHz clock.

#ifndef NDEBUG
    ff_test();

    // uint32_t const sdcard_init_result = (uint32_t)sdcard_init();
    //
    // console_write("SD card init result is: 0x");
    // console_write_dword(sdcard_init_result);
    // console_writeline(".");
    //
    // sdcard_test();
#endif //NDEBUG

#ifdef MT_INTERACTIVE
    // Start user interface (via console):
    //
    ui_enter();
#else //MT_INTERACTIVE
    // "YMODEM plus toggle button" mode:
    //
    statetoggle_init(MT_GPIO_PIN_NR_BUTTON, MT_GPIO_PIN_NR_LED, false);
    while(true)
    {
        if(statetoggle_get_state())
        {
            toggle_gpio(MT_GPIO_PIN_NR_LED, 3, 200, true); // Hard-coded

            ui_commodore_to_terminal(false); // (return value ignored)
        }
        else
        {
            toggle_gpio(MT_GPIO_PIN_NR_LED, 3, 200, false); // Hard-coded

            ui_terminal_to_commodore(false); // (return value ignored)
        }

        if(statetoggle_is_requested())
        {
            statetoggle_toggle();
        }
    }

    // TODO: Implement correctly:
    //
    // "File system and SAVE control" mode:
    //
    // // TODO: Fit state toggle (and cancel by user) stuff to this mode!
    // //
    // statetoggle_init(MT_GPIO_PIN_NR_BUTTON, MT_GPIO_PIN_NR_LED, true);
    // while(true)
    // {
    //     // Wait for SAVE (either as control command, or to really save):
    //
    //     struct tape_input * const ti = cbm_receive(0);
    //
    //     if(ti == 0)
    //     {
    //         console_deb_writeline("kernel_main : Error: Receive from commodore failed!");
    //         continue; // Try again..
    //     }
    //
    //     // Decide, what to do, based on name given:
    //
    //     static uint8_t const cmd_dir[] = "$               "; // List current directory content.
    //     static uint8_t const cmd_dir_up[] = "$..             "; // Go to upper directory and list content.
    //     ////static uint8_t const cmd_dir_pwd[] = "$.              "; // Print current directory.
    //     // '$', followed by some subdirectory name. => Go to subdirectory and list content.
    //     // '$', followed by some file name. => LOAD that file next.
    //
    //     // Toggle mode and wait for 3 seconds:
    //     //
    //     statetoggle_toggle();
    //     toggle_gpio(MT_GPIO_PIN_NR_LED, 6, 500, false); // Hard-coded
    //
    //     if(mem_cmp_byte(ti->name, cmd_dir, MT_TAPE_INPUT_NAME_LEN))
    //     {
    //         // Send directory list by expected following LOAD command:
    //
    //         uint32_t count = 2 + 6;
    //         uint8_t * bytes = alloc_alloc(count);
    //         char* name = "dummylistdir";
    //
    //         bytes[0] = 0xE8;
    //         bytes[1] = 0x07;
    //         // bytes[0] = 0x34;
    //         // bytes[1] = 0x03;
    //         bytes[2] = 169; // Immediate LDA.
    //         bytes[3] = 83; // Heart symbol (yes, it is romantic).
    //         bytes[4] = 141; // Absolute STA.
    //         bytes[5] = 0; // Lower byte of 1024 (0x0400 - C64 video RAM start).
    //         bytes[6] = 4; // Higher byte of 1024.
    //         bytes[7] = 96; // RTS.
    //
    //         // TODO: Prepare current directory list as next LOAD result.
    //
    //         // TODO: If this does not work, let user toggle mode by button
    //         //       to indicate that LOAD and return were entered.
    //
    //         cbm_send(bytes, name, count, 0);// Return value ignored.
    //
    //         alloc_free(bytes);
    //         bytes = 0;
    //
    //         statetoggle_toggle();
    //         toggle_gpio(MT_GPIO_PIN_NR_LED, 3, 200, true); // Hard-coded
    //     }
    //     else if(mem_cmp_byte(ti->name, cmd_dir_up, MT_TAPE_INPUT_NAME_LEN))
    //     {
    //         char* a = "12345678";
    //         char* b = "ABCDEFGH";
    //         char* c = "IJKLMNOPQRSTUVWXYZ";
    //         char* d = "THIS IS A DUMMY DIRECTORY OUTPUT";
    //         char* e = "11111111112222222222333333333344444444445555555555666666666677777777778888888888";
    //         uint32_t const dummy_arr_len = 5;
    //         char const * * const dummy_arr = alloc_alloc(dummy_arr_len * 4 /* byte */);
    //
    //         char* name = "dummydirup";
    //         uint32_t len = 0;
    //
    //         dummy_arr[0] = a;
    //         dummy_arr[1] = b;
    //         dummy_arr[2] = c;
    //         dummy_arr[3] = d;
    //         dummy_arr[4] = e;
    //
    //         //uint8_t * const bytes = basic_get_sample(MT_BASIC_ADDR_C64, &len);
    //         //
    //         uint8_t * const bytes = basic_get_prints(
    //             MT_BASIC_ADDR_C64, dummy_arr, dummy_arr_len, &len);
    //
    //         cbm_send(bytes, name, len, 0); // Return value ignored.
    //
    //         alloc_free(dummy_arr);
    //         alloc_free(bytes);
    //
    //         statetoggle_toggle();
    //         toggle_gpio(MT_GPIO_PIN_NR_LED, 3, 200, true); // Hard-coded
    //     }
    //     //
    //     // TODO: Add more commands, here.
    //     //
    //     else
    //     {
    //         // Really save to file:
    //
    //         // TODO: Save to file.
    //
    //         console_deb_writeline("kernel_main : Should save to file, now..");
    //     }
    //
    //     alloc_free(ti->bytes);
    //     alloc_free(ti);
    // }
#endif //MT_INTERACTIVE
}
