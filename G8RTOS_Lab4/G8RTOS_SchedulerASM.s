; G8RTOS_SchedulerASM.s - uP2 Fall 2022
; Holds all ASM functions needed for the scheduler
; Note: If you have an h file, do not have a C file and an S file of the same name

	; Functions Defined
	.def G8RTOS_Start, PendSV_Handler

	; Dependencies
	.ref CurrentlyRunningThread, G8RTOS_Scheduler

	.thumb		; Set to thumb mode
	.align 2	; Align by 2 bytes (thumb mode uses allignment by 2 or 4)
	.text		; Text section

; Need to have the address defined in file 
; (label needs to be close enough to asm code to be reached with PC relative addressing)
RunningPtr: .field CurrentlyRunningThread, 32

; G8RTOS_Start
;	Sets the first thread to be the currently running thread
;	Starts the currently running thread by setting Link Register to tcb's Program Counter
G8RTOS_Start:

	.asmfunc

	; your code
	LDR R0, RunningPtr
	LDR R0, [R0]
	LDR SP, [R0]

	;defaults
	POP {R4 - R11}
	POP {R0 - R3}
	POP {R12}

	;get pc and put to lr
	POP {LR}
	POP {LR}

	;"POP" PSR

	ADD SP, SP, #1

	;enable systcik and pendsv interrupts
	PUSH {R0-R1}

	MOV R0, #57360 ;0xE010
	MOV R1, #57344 ;0xE000
	LSL R1, R1, #16
	ADD R1, R1, R0
	LDR R0, [R1]
	ORR R0, #5
	STR R0, [R1]

	POP {R0-R1}


	BX LR
	.endasmfunc

; PendSV_Handler
; - Performs a context switch in G8RTOS
; 	- Saves remaining registers into thread stack
;	- Saves current stack pointer to tcb
;	- Calls G8RTOS_Scheduler to get new tcb
;	- Set stack pointer to new stack pointer from new tcb
;	- Pops registers from thread stack
PendSV_Handler:
	
	.asmfunc
	; 	- Saves remaining registers into thread stack
	CPSID I				; Disable interrupts as to not prevent ISR from completing
	PUSH {R4 - R11}		; Push R4-R11 into the stack (other ones are done automatically)

	; your code
	;	- Saves current stack pointer to tcb
	LDR R0, RunningPtr
	LDR R0, [R0]
	STR SP, [R0]
	;	- Calls G8RTOS_Scheduler to get new tcb
	PUSH {LR}
	BL G8RTOS_Scheduler
	POP {LR}
	;	- Set stack pointer to new stack pointer from new tcb
	LDR R0, RunningPtr
	LDR R0, [R0]
	LDR SP, [R0]
	;	- Pops registers from thread stack
	POP {R4 - R11}
	CPSIE I				; Re-enable interrupts

	;MVN LR, #14

	BX LR

	.endasmfunc
	; end of the asm file
	.align
	.end
