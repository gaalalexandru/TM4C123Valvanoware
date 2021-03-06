;/*****************************************************************************/
; OSasm.s: low-level OS commands, written in assembly                       */
; Runs on LM4F120/TM4C123/MSP432
; Lab 4 starter file
; March 25, 2016

;


        AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8

        EXTERN  RunPt            ; currently running thread
        EXPORT  StartOS
        EXPORT  SysTick_Handler
        IMPORT  Scheduler
        EXPORT  PendSV_Handler

SysTick_Handler                ; 1) Saves R0-R3,R12,LR,PC,PSR
    CPSID   I                  ; 2) Prevent interrupt during switch
	PUSH {R4-R11}		;saves registers R4 to R11
	LDR R0,=RunPt		;load address of RunPt to R0
	LDR R1,[R0]		;R1 = RunPt
	STR SP,[R1]		;save current SP to tcbs.sp
	;LDR R1,[R1,#4]		;move to next tcbs
	;STR R1,[R0]		;R1 = RunPt, for the upcomming tcbs
	PUSH {R0,LR}
	BL Scheduler
	POP {R0,LR}
	LDR R1,[R0]		; 6) R1 = RunPt, new thread
	LDR SP,[R1]		;load new SP, SP = RunPt.sp
	POP {R4-R11}		;load registers from stack
    CPSIE   I                  ; 9) tasks run with interrupts enabled
    BX      LR                 ; 10) restore R0-R3,R12,LR,PC,PSR

StartOS
	LDR R0,=RunPt	;load address of RunPt to R0
	LDR R2,[R0]		;R2 = RunPt
	LDR SP,[R2]		;Load value of RunPt.sp to SP
	POP {R4-R11}
	POP {R0-R3}
	POP {R12}
	ADD SP,SP,#4	;move to next element
	POP {LR}
	ADD SP,SP,#4	;move to next element
	CPSIE   I              ; Enable interrupts at processor level
    BX      LR                 ; start first thread

PendSV_Handler
    LDR     R0, =RunPt         ; run this thread next
    LDR     R2, [R0]           ; R2 = value of RunPt
    LDR     SP, [R2]           ; new thread SP; SP = RunPt->stackPointer;
    POP     {R4-R11}           ; restore regs r4-11
    LDR     LR,=0xFFFFFFF9
    BX      LR                 ; start next thread
	
    ALIGN
    END
