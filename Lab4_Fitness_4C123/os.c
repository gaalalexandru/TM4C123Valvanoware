// os.c
// Runs on LM4F120/TM4C123/MSP432
// A priority/blocking real-time operating system 
// Lab 4 starter file.
// Daniel Valvano
// March 25, 2016
// Hint: Copy solutions from Lab 3 into Lab 4
#include <stdint.h>
#include "os.h"
#include "CortexM.h"
#include "BSP.h"
#include "../inc/tm4c123gh6pm.h"

// function definitions in osasm.s
void StartOS(void);

void static runperiodicevents(void);
#define NUMTHREADS  8        // maximum number of threads
#define NUMPERIODIC 2        // maximum number of periodic threads
#define STACKSIZE   100      // number of 32-bit words in stack per thread
struct tcb{
  int32_t *sp;      // pointer to stack (valid for threads not running
  struct tcb *next; // linked-list pointer
  int32_t *blocked;	// pointer to blocked semaphore, nonzero if blocked on this semaphore
  int32_t sleep;	// time to sleep, nonzero if this thread is sleeping
  uint32_t priority; // priority of the thread, 0 - highest priority, 254 - lowest
};
typedef struct tcb tcbType;
tcbType tcbs[NUMTHREADS];
tcbType *RunPt;
int32_t Stacks[NUMTHREADS][STACKSIZE];
void static runperiodicevents(void);

// ******** OS_Init ************
// Initialize operating system, disable interrupts
// Initialize OS controlled I/O: periodic interrupt, bus clock as fast as possible
// Initialize OS global variables
// Inputs:  none
// Outputs: none
void OS_Init(void){
  DisableInterrupts();
  BSP_Clock_InitFastest();// set processor clock to fastest speed
// perform any initializations needed, 
// set up periodic timer to run runperiodicevents to implement sleeping
  BSP_PeriodicTask_Init(runperiodicevents,1000,4);	//Start one HW Timer with periodic interrupt at 1000 Hz
}

void SetInitialStack(int i){
	//first set for each stack the stack pointer
	tcbs[i].sp = &Stacks[i][STACKSIZE-16];	//Thread Stack Pointer	R13 = SP
	//fill in bottom positions of the stack with register values, as if thread was already running and interrupted
	Stacks[i][STACKSIZE-1] = 0x01000000; //Thumb bit on last stack element
	//Stacks[i][STACKSIZE-2] = PC; //The Program Counter will be set later with the address of the function it points to, R15 = PC
	Stacks[i][STACKSIZE-3] = 0x14141414; //Initial Link Register dummy value, R14 = LR
	Stacks[i][STACKSIZE-4] = 0x12121212; //R12
	Stacks[i][STACKSIZE-5] = 0x03030303; //R3
	Stacks[i][STACKSIZE-6] = 0x02020202; //R2
	Stacks[i][STACKSIZE-7] = 0x01010101; //R1
	Stacks[i][STACKSIZE-8] = 0x00000000; //R0
	Stacks[i][STACKSIZE-9] = 0x11111111; //R11
	Stacks[i][STACKSIZE-10] = 0x10101010; //R10
	Stacks[i][STACKSIZE-12] = 0x09090909; //R9
	Stacks[i][STACKSIZE-13] = 0x08080808; //R8
  Stacks[i][STACKSIZE-13] = 0x07070707; //R7
  Stacks[i][STACKSIZE-14] = 0x06060606; //R6
  Stacks[i][STACKSIZE-15] = 0x05050505; //R5
  Stacks[i][STACKSIZE-16] = 0x04040404; //R4	
}

//******** OS_AddThreads ***************
// Add eight main threads to the scheduler
// Inputs: function pointers to eight void/void main threads
//         priorites for each main thread (0 highest)
// Outputs: 1 if successful, 0 if this thread can not be added
// This function will only be called once, after OS_Init and before OS_Launch
int OS_AddThreads(void(*thread0)(void), uint32_t p0,
                  void(*thread1)(void), uint32_t p1,
                  void(*thread2)(void), uint32_t p2,
                  void(*thread3)(void), uint32_t p3,
                  void(*thread4)(void), uint32_t p4,
                  void(*thread5)(void), uint32_t p5,
                  void(*thread6)(void), uint32_t p6,
                  void(*thread7)(void), uint32_t p7){
	int32_t sr;	//I bit status
	int32_t i;	//thread index
	sr = StartCritical();	//Disable Interrupts
	
	//initialize TCB circular list
	tcbs[0].next = &tcbs[1];	//main thread 0 points to main thread 1
	tcbs[1].next = &tcbs[2];	//main thread 1 points to main thread 2
	tcbs[2].next = &tcbs[3];	//main thread 2 points to main thread 3	
	tcbs[3].next = &tcbs[4];	//main thread 3 points to main thread 4
	tcbs[4].next = &tcbs[5];	//main thread 4 points to main thread 5
	tcbs[5].next = &tcbs[6];	//main thread 5 points to main thread 6
	tcbs[6].next = &tcbs[7];	//main thread 6 points to main thread 7
	tcbs[7].next = &tcbs[0];	//main thread 7 points to main thread 8
	
	//initialize threads as not blocked									
	for(i=0; i< NUMTHREADS; i++){tcbs[i].blocked = 0;}
	
	// initialize RunPt
	RunPt = &tcbs[0];

	// initialize four stacks, including initial PC
	SetInitialStack(0);	//SetInitialStack initial stack of main thread 0
	Stacks[0][STACKSIZE-2] = (int32_t)(thread0);	//Set address of thread0 as PC
	SetInitialStack(1);	//SetInitialStack initial stack of main thread 1
	Stacks[1][STACKSIZE-2] = (int32_t)(thread1);	//Set address of thread1 as PC	
	SetInitialStack(2);	//SetInitialStack initial stack of main thread 2
	Stacks[2][STACKSIZE-2] = (int32_t)(thread2);	//Set address of thread2 as PC
	SetInitialStack(3);	//SetInitialStack initial stack of main thread 3
	Stacks[3][STACKSIZE-2] = (int32_t)(thread3);	//Set address of thread3 as PC
	SetInitialStack(4);	//SetInitialStack initial stack of main thread 4
	Stacks[4][STACKSIZE-2] = (int32_t)(thread4);	//Set address of thread4 as PC
	SetInitialStack(5);	//SetInitialStack initial stack of main thread 5
	Stacks[5][STACKSIZE-2] = (int32_t)(thread5);	//Set address of thread5 as PC	
	SetInitialStack(6);	//SetInitialStack initial stack of main thread 6
	Stacks[6][STACKSIZE-2] = (int32_t)(thread6);	//Set address of thread5 as PC	
	SetInitialStack(7);	//SetInitialStack initial stack of main thread 6
	Stacks[7][STACKSIZE-2] = (int32_t)(thread7);	//Set address of thread5 as PC	
	
	//initialize priority for each thread
	tcbs[0].priority = p0;
	tcbs[1].priority = p1;
	tcbs[2].priority = p2;
	tcbs[3].priority = p3;
	tcbs[4].priority = p4;
	tcbs[5].priority = p5;
	tcbs[6].priority = p6;
	tcbs[7].priority = p7;
	
	EndCritical(sr);	//Enable Interrupts
	return 1;         // successful
}


void static runperiodicevents(void){
// **DECREMENT SLEEP COUNTERS
// In Lab 4, handle periodic events in RealTimeEvents
	int32_t i;
	for (i=0;i<NUMTHREADS;i++){ if(tcbs[i].sleep != 0) {	//search for sleeping main threads
			tcbs[i].sleep --;	//decrement sleep period by 1ms
		}
	}
}

//******** OS_Launch ***************
// Start the scheduler, enable interrupts
// Inputs: number of clock cycles for each time slice
// Outputs: none (does not return)
// Errors: theTimeSlice must be less than 16,777,216
void OS_Launch(uint32_t theTimeSlice){
  STCTRL = 0;                  // disable SysTick during setup
  STCURRENT = 0;               // any write to current clears it
  SYSPRI3 =(SYSPRI3&0x00FFFFFF)|0xE0000000; // priority 7
  STRELOAD = theTimeSlice - 1; // reload value
  STCTRL = 0x00000007;         // enable, core clock and interrupt arm
  StartOS();                   // start on the first task
}
// runs every ms
void Scheduler(void){      // every time slice
	// look at all threads in TCB list choose
	// highest priority thread not blocked and not sleeping 
	// If there are multiple highest priority (not blocked, not sleeping) run these round robin
	// ROUND ROBIN, skip blocked and sleeping threads
	uint32_t maxprio = 255;
	tcbType *tempPt;
	tcbType *bestPt;
	tempPt = RunPt;
	do {
		tempPt = tempPt->next;
		if(((tempPt->priority) < maxprio) && (tempPt->blocked == 0) && (tempPt->sleep == 0)) { 
			//If priority is higher and not blocked and not sleeping
			maxprio = tempPt->priority;
			bestPt = tempPt;
		}
	} while (RunPt != tempPt); //search through all linked list
	RunPt = bestPt; //move to next suitable thread
}

//******** OS_Suspend ***************
// Called by main thread to cooperatively suspend operation
// Inputs: none
// Outputs: none
// Will be run again depending on sleep/block status
void OS_Suspend(void){
  STCURRENT = 0;        // any write to current clears it
  INTCTRL = 0x04000000; // trigger SysTick
// next thread gets a full time slice
}

// ******** OS_Sleep ************
// place this thread into a dormant state
// input:  number of msec to sleep
// output: none
// OS_Sleep(0) implements cooperative multitasking
void OS_Sleep(uint32_t sleepTime){
// set sleep parameter in TCB
// suspend, stops running
	RunPt->sleep = sleepTime;
	OS_Suspend();
}

// ******** OS_InitSemaphore ************
// Initialize counting semaphore
// Inputs:  pointer to a semaphore
//          initial value of semaphore
// Outputs: none
void OS_InitSemaphore(int32_t *semaPt, int32_t value){
  //Assign initial value to *semaPt
	*semaPt = value;
}

// ******** OS_Wait ************
// Decrement semaphore and block if less than zero
// Lab2 spinlock (does not suspend while spinning)
// Lab3 block if less than zero
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Wait(int32_t *semaPt){
	DisableInterrupts();
	*semaPt = (*semaPt) - 1;
	if(*semaPt < 0){
		RunPt->blocked = semaPt;	//Point to semaphore which is blocked
		EnableInterrupts();
		OS_Suspend();	//Switch threads by generating a systick interrupt
	}
	EnableInterrupts();
}

// ******** OS_Signal ************
// Increment semaphore
// Lab2 spinlock
// Lab3 wakeup blocked thread if appropriate
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Signal(int32_t *semaPt){
	tcbType	*threadPt;	//local thread pointer
	DisableInterrupts();
	(*semaPt) = (*semaPt) + 1;
	if(*semaPt <= 0){
		threadPt = RunPt->next;	//point to next thread
		while((threadPt->blocked) != semaPt) {	threadPt = threadPt->next; }//search for a thread that is blocked on this semaphore
		threadPt->blocked = 0;	//unblock 1st blocked thread found
	}
	EnableInterrupts();
}

#define FSIZE 10    // can be any size
uint32_t PutI;      // index of where to put next
uint32_t GetI;      // index of where to get next
uint32_t Fifo[FSIZE];
int32_t CurrentSize;// 0 means FIFO empty, FSIZE means full
uint32_t LostData;  // number of lost pieces of data

// ******** OS_FIFO_Init ************
// Initialize FIFO.  The "put" and "get" indices initially
// are equal, which means that the FIFO is empty.  Also
// initialize semaphores to track properties of the FIFO
// such as size and busy status for Put and Get operations,
// which is important if there are multiple data producers
// or multiple data consumers.
// Inputs:  none
// Outputs: none
void OS_FIFO_Init(void){ //Init the FIFO with indexes and CurrentSize and LostData set to 0
	PutI = 0;
	GetI = 0;
	OS_InitSemaphore(&CurrentSize,0);
}

// ******** OS_FIFO_Put ************
// Put an entry in the FIFO.  Consider using a unique
// semaphore to wait on busy status if more than one thread
// is putting data into the FIFO and there is a chance that
// this function may interrupt itself.
// Inputs:  data to be stored
// Outputs: 0 if successful, -1 if the FIFO is full
int OS_FIFO_Put(uint32_t data){
	if(CurrentSize == FSIZE) { //FIFO is full
		LostData++;
		return -1; //Error
	}
	else {
		Fifo[PutI] = data;	//store data in FIFO at put index
		PutI = (PutI + 1)%FSIZE; //Increment Put index and wrap around if necessary
		OS_Signal(&CurrentSize);
		return 0;	//Success
	}
}

// ******** OS_FIFO_Get ************
// Get an entry from the FIFO.  Consider using a unique
// semaphore to wait on busy status if more than one thread
// is getting data from the FIFO and there is a chance that
// this function may interrupt itself.
// Inputs:  none
// Outputs: data retrieved
uint32_t OS_FIFO_Get(void){uint32_t data;
	OS_Wait(&CurrentSize);	//Wait till there is data in FIFO, block if empty
	data = Fifo[GetI];	//Get stored data from Fifo
	GetI = (GetI + 1) % FSIZE;	//Incremet Get index and wrap around
  return data;
}
// *****periodic events****************
int32_t *PeriodicSemaphore0;
uint32_t Period0; // time between signals
int32_t *PeriodicSemaphore1;
uint32_t Period1; // time between signals
void RealTimeEvents(void){int flag=0;
  static int32_t realCount = -10; // let all the threads execute once
  // Note to students: we had to let the system run for a time so all user threads ran at least one
  // before signalling the periodic tasks
  realCount++;
  if(realCount >= 0){
		if((realCount%Period0)==0){
      OS_Signal(PeriodicSemaphore0);
      flag = 1;
		}
    if((realCount%Period1)==0){
      OS_Signal(PeriodicSemaphore1);
      flag=1;
	}
    if(flag){
      OS_Suspend(); // run the scheduler
    }
  }
}
// ******** OS_PeriodTrigger0_Init ************
// Initialize periodic timer interrupt to signal 
// Inputs:  semaphore to signal
//          period in ms
// priority level at 0 (highest)
// Outputs: none
void OS_PeriodTrigger0_Init(int32_t *semaPt, uint32_t period){
	PeriodicSemaphore0 = semaPt;
	Period0 = period;
	BSP_PeriodicTask_InitC(&RealTimeEvents,1000,0);
}
// ******** OS_PeriodTrigger1_Init ************
// Initialize periodic timer interrupt to signal 
// Inputs:  semaphore to signal
//          period in ms
// priority level at 0 (highest)
// Outputs: none
void OS_PeriodTrigger1_Init(int32_t *semaPt, uint32_t period){
	PeriodicSemaphore1 = semaPt;
	Period1 = period;
	BSP_PeriodicTask_InitC(&RealTimeEvents,1000,0);
}

//****edge-triggered event************
int32_t *edgeSemaphore;
// ******** OS_EdgeTrigger_Init ************
// Initialize button1, PD6, to signal on a falling edge interrupt
// Inputs:  semaphore to signal
//          priority
// Outputs: none
void OS_EdgeTrigger_Init(int32_t *semaPt, uint8_t priority){
	uint32_t clock;
	uint32_t bit_prio;
	
	edgeSemaphore = semaPt;
//***IMPLEMENT THIS***
// 1) activate clock for Port D
	SYSCTL_RCGCGPIO_R |= 0x00000008; // activate clock for Port D (bit3) 
// allow time for clock to stabilize
  while((SYSCTL_PRGPIO_R&0x08) == 0){};// allow time for clock to stabilize	
	
// 2) no need to unlock PD6
	
// 3) disable analog on PD6
	GPIO_PORTD_AMSEL_R &= ~0x40;     // 3) disable analog on PD6
	
// 4) configure PD6 as GPIO
	//GPIO_PORTD_PCTL_R &= ~0x0F000000;
	GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R&0xF0FFFFFF)+0x00000000;
// 5) make PD6 input
	GPIO_PORTD_DIR_R &= ~0x40;    // (c) make PD6 input
	
// 6) disable alt funct on PD6
	GPIO_PORTD_AFSEL_R &= ~0x40;
	
// disable pull-up on PD6
	//GPIO_PORTD_PUR_R |= 0x40;
  GPIO_PORTD_PUR_R &= ~0x40;       // disable pull-up on PD6
// 7) enable digital I/O on PD6  
	GPIO_PORTD_DEN_R |= 0x40;        // 7) enable digital I/O on PD6
// (d) PD6 is edge-sensitive 
	GPIO_PORTD_IS_R &= ~0x40;  //(d) PD6 is edge sensitive
	
//     PD6 is not both edges 
	GPIO_PORTD_IBE_R &= ~0x40;  //(d) PD6 not both edges
	
//     PD6 is falling edge event 
	GPIO_PORTD_IEV_R &= ~0x40;  // d) PD6 falling edge

// (e) clear PD6 flag
	GPIO_PORTD_ICR_R |= 0x40;  //(e) PD6 clear flags
 
// (f) arm interrupt on PD6
	GPIO_PORTD_IM_R |= 0x40;  //(f) arm interrupt on PD6
	
// priority on Port D edge trigger is NVIC_PRI0_R	31 � 29
	//NVIC_PRI0_R = (NVIC_PRI7_R&0x00FFFFFF)|0xA0000000; // (g) priority 5
	bit_prio = (priority << 28);
	NVIC_PRI0_R = (NVIC_PRI0_R&0x00FFFFFF)|bit_prio; // (g) priority 5
// enable is bit 3 in NVIC_EN0_R
	NVIC_EN0_R |= 0x00000008; //enable bit 3
 }

// ******** OS_EdgeTrigger_Restart ************
// restart button1 to signal on a falling edge interrupt
// rearm interrupt
// Inputs:  none
// Outputs: none
void OS_EdgeTrigger_Restart(void){
//***IMPLEMENT THIS***
// rearm interrupt 3 in NVIC
// clear flag6
	GPIO_PORTD_IM_R |= 0x40;  //(f) arm interrupt on PD6
	NVIC_EN0_R |= 0x00000008; //enable bit 3
	GPIO_PORTD_ICR_R |= 0x40;  //(e) PD6 clear flags	
}
void GPIOPortD_Handler(void){
//***IMPLEMENT THIS***
	// step 1 acknowledge by clearing flag
  // step 2 signal semaphore (no need to run scheduler)
  // step 3 disarm interrupt to prevent bouncing to create multiple signals
	uint8_t status;	
	status = GPIO_PORTD_RIS_R;
	if(status == 0x40) {
		GPIO_PORTD_ICR_R |= 0x40;  //PD6 clear flags
    OS_Signal(edgeSemaphore);  //button press occurred
		GPIO_PORTD_IM_R &= ~0x40;  //disarm interrupt on PD6
		NVIC_EN0_R &= ~0x00000008; //enable bit 3		
	}
  OS_Suspend();	
}


