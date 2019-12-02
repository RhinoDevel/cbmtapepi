
// Marcel Timm, RhinoDevel, 2019sep04

// *****************************************************************************
// Originally taken from:
//
// https://raw.githubusercontent.com/moizumi99/RPiHaribote/master/haribote/sdcard.h
// *****************************************************************************

#ifndef MT_SDCARD
#define MT_SDCARD

#define SD_OK                  0
#define SD_ERROR               1
#define SD_TIMEOUT             2
#define SD_BUSY                3
#define SD_NO_RESP             5
#define SD_ERROR_RESET         6
#define SD_ERROR_CLOCK         7
#define SD_ERROR_VOLTAGE       8
#define SD_ERROR_APP_CMD       9
#define SD_ALREADY_INITIALIZED 10

int sdcard_blocks_clear(long long address, int num);
int sdcard_blocks_transfer(long long address, int num, unsigned char * buffer, int write);

/** Initialize SD card.
 *
 *  - Returns non-zero value on error.
 *  - Assumes SD card to be present!
 *  - Assumes SD card NEVER getting removed/reinserted!
 */
int sdcard_init();

#endif //MT_SDCARD

// Test code:
//
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
