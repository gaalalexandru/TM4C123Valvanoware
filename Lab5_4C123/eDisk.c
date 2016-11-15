// eDisk.c
// Runs on TM4C123
// Mid-level implementation of the solid state disk device
// driver.  Below this is the low level, hardware-specific
// flash memory interface.  Above this is the high level
// file system implementation.
// Daniel and Jonathan Valvano
// August 29, 2016

/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2016

   "Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers",
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2016

   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2016

   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2016

 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include <stdint.h>
#include "eDisk.h"
#include "FlashProgram.h"

//*************** eDisk_Init ***********
// Initialize the interface between microcontroller and disk
// Inputs: drive number (only drive 0 is supported)
// Outputs: status
//  RES_OK        0: Successful
//  RES_ERROR     1: Drive not initialized
enum DRESULT eDisk_Init(uint32_t drive){
  // if drive is 0, return RES_OK, otherwise return RES_ERROR
  // for some configurations the physical drive needs initialization
  // however for the internal flash, no initialization is required
  //    so this function doesn't do anything
  if(drive == 0){             // only drive 0 is supported
     return RES_OK;
  }
  return RES_ERROR;

}

//*************** eDisk_ReadSector ***********
// Read 1 sector of 512 bytes from the disk, data goes to RAM
// Inputs: pointer to an empty RAM buffer
//         sector number of disk to read: 0,1,2,...255
// Outputs: result
//  RES_OK        0: Successful
//  RES_ERROR     1: R/W Error
//  RES_WRPRT     2: Write Protected
//  RES_NOTRDY    3: Not Ready
//  RES_PARERR    4: Invalid Parameter
// Finished function
enum DRESULT eDisk_ReadSector(
	uint8_t *buff,     // Pointer to a RAM buffer into which to store
	uint8_t sector){   // sector number to read from
// starting ROM address of the sector is	EDISK_ADDR_MIN + 512*sector
// return RES_PARERR if EDISK_ADDR_MIN + 512*sector > EDISK_ADDR_MAX
// copy 512 bytes from ROM (disk) into RAM (buff)

	uint32_t start_address;
	uint8_t *ptROM;
	uint16_t i;
	
	start_address = (EDISK_ADDR_MIN + (512*sector));  //calculate start address
	ptROM = (uint8_t *)start_address;	//copy start address in ptROM
	
	if((start_address <= EDISK_ADDR_MAX)&&(sector <= 255)) {  //valid sector number
		for(i=0;i<512;i++) {
			*buff = *ptROM;  //copy first byte to buffer
			ptROM++;  //increment with 1 byte
			buff++;  //increment with 1 byte
		}
		return RES_OK;  //successefull read
	}
  return RES_PARERR;  //if not valid start address
}

//*************** eDisk_WriteSector ***********
// Write 1 sector of 512 bytes of data to the disk, data comes from RAM
// Inputs: pointer to RAM buffer with information
//         sector number of disk to write: 0,1,2,...,255
// Outputs: result
//  RES_OK        0: Successful
//  RES_ERROR     1: R/W Error
//  RES_WRPRT     2: Write Protected
//  RES_NOTRDY    3: Not Ready
//  RES_PARERR    4: Invalid Parameter
// Function finished
enum DRESULT eDisk_WriteSector(
	const uint8_t *buff,  // Pointer to the data to be written
	uint8_t sector){      // sector number

	uint32_t start_address;
	uint8_t result;
		
// starting ROM address of the sector is	EDISK_ADDR_MIN + 512*sector
// return RES_PARERR if EDISK_ADDR_MIN + 512*sector > EDISK_ADDR_MAX
// write 512 bytes from RAM (buff) into ROM (disk)
// you can use Flash_FastWrite or Flash_WriteArray

	start_address = (EDISK_ADDR_MIN + (512*sector));  //calculate start address
	
	if((start_address <= EDISK_ADDR_MAX)&&(sector <= 255)) {  //valid sector number	
		result = Flash_WriteArray((uint32_t *)buff,start_address,WORDSPERSECTOR);
		if (result == WORDSPERSECTOR) { return RES_OK; }  //successefull write
		else { return RES_ERROR; }  //unsuccessefull write
	}	
  return RES_PARERR;	//if not valid start address
}

//*************** eDisk_Format ***********
// Erase all files and all data by resetting the flash to all 1's
// Inputs: none
// Outputs: result
//  RES_OK        0: Successful
//  RES_ERROR     1: R/W Error
//  RES_WRPRT     2: Write Protected
//  RES_NOTRDY    3: Not Ready
//  RES_PARERR    4: Invalid Parameter
// Function finished
enum DRESULT eDisk_Format(void){
// erase all flash from EDISK_ADDR_MIN to EDISK_ADDR_MAX
	uint32_t address;
	uint8_t result;
	
  address = EDISK_ADDR_MIN; // start of disk
  while(address <= EDISK_ADDR_MAX){
    result = Flash_Erase(address); // erase 1k block
    address = address+BLOCKSIZE;
  }
	if(result == RES_OK) { return RES_OK; }
	else { return RES_ERROR; }
}
