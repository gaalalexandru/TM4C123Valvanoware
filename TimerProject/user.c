//*****************************************************************************
// user.c
// Runs on LM4F120/TM4C123/MSP432
// An example user program that initializes the simple operating system
//   Schedule three independent threads using preemptive round robin
//   Each thread rapidly toggles a pin on profile pins and increments its counter
//   THREADFREQ is the rate of the scheduler in Hz

// Daniel Valvano
// February 8, 2016

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2016

   "Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers",
   ISBN: 978-1466468863, , Jonathan Valvano, copyright (c) 2016
   Programs 4.4 through 4.12, section 4.2

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
#include "../inc/BSP.h"
#include "../inc/profile.h"
#include "CortexM.h"
#include "BSP.h"


#define THREADFREQ 1000   // frequency in Hz

unsigned long sec = 0;
unsigned long min = 0;
unsigned long hour = 0;
unsigned long msec = 0;

//******** SisTickInit ***************
// start the sistick timer, enable interrupts
// Inputs: number of clock cycles for each time slice
//         (maximum of 24 bits)
// Outputs: none (does not return)
void SisTick_Init(uint32_t theTimeSlice){
  STCTRL = 0;                  // disable SysTick during setup
  STCURRENT = 0;               // any write to current clears it
  SYSPRI3 =(SYSPRI3&0x00FFFFFF)|0xE0000000; // priority 7
  STRELOAD = theTimeSlice - 1; // reload value
  STCTRL = 0x00000007;         // enable, core clock and interrupt arm
}

void main(void)
{
	DisableInterrupts();
	Profile_Init();       // enable digital I/O on profile pins
	BSP_Buzzer_Init(0); //512 is half
  BSP_Clock_InitFastest();// set processor clock to fastest speed
	SisTick_Init(BSP_Clock_GetFreq()/THREADFREQ);// doesn't return, interrupts enabled in here
	EnableInterrupts();	
	while(1)
	{
	}
}

void Alarm(char status)
{
	if(status)
	{
		BSP_Buzzer_Set(700);
	}
	else
	{
		BSP_Buzzer_Set(0);
	}
}

void SysTick_Handler(void)
{
	Profile_Toggle0();    // toggle bit every millisecond
	if(msec == 1000)	//check milli second counter
	{
		msec = 0;
		//sec ++;
		Alarm(0);
		Profile_Toggle1();    // toggle bit every second
		if(sec == 60)	//check second counter
		{
			sec = 0;
			Profile_Toggle2();    // toggle bit every minute
			if(min == 60)	//check minute counter
			{
				min = 0;
				hour ++;
			}
			else
			{
				min ++;
			}
		}
		else
		{
			sec ++;
		}
	}
	else
	{
		msec ++;
	}
	if(((msec % 1000)==0)&&((sec % 30) == 0))
	{
		Alarm(1);
	}
}
