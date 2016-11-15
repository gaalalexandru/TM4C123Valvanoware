// eFile.c
// Runs on either TM4C123 or MSP432
// High-level implementation of the file system implementation.
// Daniel and Jonathan Valvano
// August 29, 2016
#include <stdint.h>
#include "eDisk.h"

uint8_t Buff[512]; // temporary buffer used during file I/O
uint8_t Directory[256], FAT[256];
int32_t bDirectoryLoaded =0; // 0 means disk on ROM is complete, 1 means RAM version active

// Return the larger of two integers.
int16_t max(int16_t a, int16_t b){
  if(a > b){
    return a;
  }
  return b;
}
//*****MountDirectory******
// if directory and FAT are not loaded in RAM,
// bring it into RAM from disk
void MountDirectory(void){ 
	uint16_t i = 0;
// if bDirectoryLoaded is 0, 
//    read disk sector 255 and populate Directory and FAT
//    set bDirectoryLoaded=1
// if bDirectoryLoaded is 1, simply return
// **write this function**

	
	if (bDirectoryLoaded == 0) { //If DIR & FAT is in ROM
		eDisk_ReadSector(Buff,255);
		for(i=0;i<256;i++) {
			Directory[i] = Buff[i];	//Read first part of buffer to directory
		}
		for(i=256;i<512;i++) {
			FAT[i-256] = Buff[i];	//Read first part of buffer to FAT
		}
		bDirectoryLoaded = 1;
	}
	else { return; }
	return;
	/* 
	//Alternative aproach to mount directory
	volatile uint8_t *DirAddr = (volatile uint8_t *)(EDISK_ADDR_MAX - 511); // address of directory
	volatile uint8_t *FATAddr = (volatile uint8_t *)(EDISK_ADDR_MAX - 255); // address of FAT
	if(bDirectoryLoaded==0){
		for (int i = 0; i < 256; i++){
			Directory[i] = *DirAddr;
			FAT[i] = *FATAddr;
			DirAddr++;
			FATAddr++;
		}
		bDirectoryLoaded=1;
	}
	return;	*/
}

// Return the index of the last sector in the file
// associated with a given starting sector.
// Note: This function will loop forever without returning
// if the file has no end (i.e. the FAT is corrupted).
uint8_t lastsector(uint8_t start){
// **write this function**
	while ((FAT[start] != 255)&&(start != 255)){
		start = FAT[start];
	}		
	if(FAT[start] == 255){  //last sector of that file
		return start;
	}
	if (start == 255){  // navigated through all FAT
		return 255; 
	}
	return 255;
}

// Return the index of the first free sector.
// Note: This function will loop forever without returning
// if a file has no end or if (Directory[255] != 255)
// (i.e. the FAT is corrupted).
uint8_t findfreesector(void){
// **write this function**
	int8_t fs = -1;  //free sector
	uint8_t i = 0;
	uint8_t ls = 0;  //last sector
	while(ls != 255) {//until one file end is found
		ls = lastsector(Directory[i]); //Find the last sector of the current file in directory
		if (ls == 255) { return (fs+1);} //return the next sector number
		fs = max(fs,ls);
		
		i++;  //move to next file in directory  
	}
  return 0; // replace this line
}

// Append a sector index 'n' at the end of file 'num'.
// This helper function is part of OS_File_Append(), which
// should have already verified that there is free space,
// so it always returns 0 (successful).
// Note: This function will loop forever without returning
// if the file has no end (i.e. the FAT is corrupted).
uint8_t appendfat(uint8_t num, uint8_t n){
// **write this function**
  //AleGaa
	uint8_t i = 0;
	i = Directory[num];
	if(i == 255) { Directory[num] = n; } //New file added
	else {
		while(FAT[i] != 255) {  //Not last sector of the file
			i = FAT[i];  //Move to next linked sector
		}
		FAT[i] = n;  //New sector added to the file
	}
  return 0; // replace this line
}

//********OS_File_New*************
// Returns a file number of a new file for writing
// Inputs: none
// Outputs: number of a new file
// Errors: return 255 on failure or disk full
// Function finished
uint8_t OS_File_New(void){
  static uint8_t i = 0;
	if(!bDirectoryLoaded) {
		MountDirectory(); 	//Read DIR and FAT from ROM to RAM
	}
	//AleGaa
	while ((i != 255)&&(Directory[i] != 255)){	//find a free spot
		i++;
	}
	if (Directory[i] == 255) {	//Current directory number / position is free / available
		return i;
	}
	if (i == 255) {  //Directory full
			return 255;
		}
  return 255;
}

//********OS_File_Size*************
// Check the size of this file
// Inputs:  num, 8-bit file number, 0 to 254
// Outputs: 0 if empty, otherwise the number of sectors
// Errors:  none
// Function finished
uint8_t OS_File_Size(uint8_t num){
  uint8_t	size = 1;
	uint8_t file_sector;
	
	file_sector = Directory[num];  //get first sector number
	
	if(file_sector == 255) {  //empty
		return 0;
	}
	
	while(FAT[file_sector] != 255) {  //while not last sector of the file
		file_sector = FAT[file_sector];  //move to next sector of the file
		size++;
	}
  return size; // replace this line
}

//********OS_File_Append*************
// Save 512 bytes into the file
// Inputs:  num, 8-bit file number, 0 to 254
//          buf, pointer to 512 bytes of data
// Outputs: 0 if successful
// Errors:  255 on failure or disk full
uint8_t OS_File_Append(uint8_t num, uint8_t buf[512]){
// **write this function**
  uint8_t n = 0;
	n = findfreesector();
	if (n == 255) {
		return 0;
	}
	else {
		eDisk_WriteSector(buf,n);
		appendfat(num,n);
	}
  return 0;
}

//********OS_File_Read*************
// Read 512 bytes from the file
// Inputs:  num, 8-bit file number, 0 to 254
//          location, logical address, 0 to 254
//          buf, pointer to 512 empty spaces in RAM
// Outputs: 0 if successful
// Errors:  255 on failure because no data
// Function finished
uint8_t OS_File_Read(uint8_t num, uint8_t location,
                     uint8_t buf[512]){
  uint8_t file_sector = 0;
	uint8_t count = 0;

	file_sector = Directory[num];  //first sector of the file num
	if(location == 0) {  //need to read the 1st block of the file
		eDisk_ReadSector(buf,file_sector);  //read from ROM
		return 0;  //successful read
	}

	while(count != location) {
		if(FAT[file_sector] == 255) {  //if the current block is the last and attempting to read other one
			return 255;  //no data
		}
		file_sector = FAT[file_sector];  //move to next block of the file
		count++;
	}
	eDisk_ReadSector(buf,file_sector);  //read from ROM	
  return 0; //successful read
}

//********OS_File_Flush*************
// Update working buffers onto the disk
// Power can be removed after calling flush
// Inputs:  none
// Outputs: 0 if success
// Errors:  255 on disk write failure
// Finished function
uint8_t OS_File_Flush(void){
	uint16_t i;
	if (bDirectoryLoaded) {
		for(i=0;i<256;i++) {
			Buff[i] = Directory[i];  //save directory in first 256 bytes
   		Buff[i+256] = FAT[i];
		}
		if (eDisk_WriteSector(Buff, 255) !=RES_OK ) { 	//write to ROM
			return 255;
		}
		else {
			bDirectoryLoaded=0;  //Dir and FAT are now in ROM
		}
	}
  return 0; // replace this line
}

//********OS_File_Format*************
// Erase all files and all data
// Inputs:  none
// Outputs: 0 if success
// Errors:  255 on disk write failure
// Finished function
uint8_t OS_File_Format(void){
// call eDiskFormat
// clear bDirectoryLoaded to zero

	eDisk_Format();
	bDirectoryLoaded = 0;
  return 0; // replace this line
}
