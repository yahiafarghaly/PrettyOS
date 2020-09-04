/*****************************************************************************
MIT License

Copyright (c) 2020 Yahia Farghaly Ashour

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

/*------------------- Code Generation Directives -----------------*/
/* Referrences for GNU assembly directives:
 *		[1]: https://sourceware.org/binutils/docs/as/ARM-Directives.html
 *		[2]: https://sourceware.org/binutils/docs/as/index.html
 */
	.cpu cortex-m4
	.syntax unified
	.text
/*------------------------- External References ------------------*/
	.extern OS_Running
	.extern OS_currentTask
	.extern OS_nextTask

/*----------------------------- Equals ---------------------------*/

.equ NVIC_INT_CTRL, 	0xE000ED04 			 /* Interrupt control state register. */
.equ NVIC_SYSPRI3,		0xE000ED22     	 	 /* System priority register (priority 3), advanced with 2 bytes. */
.equ NVIC_PENDSV_PRI,	0xFF        		 /* PendSV priority value (lowest = 0xFF). */
.equ NVIC_PENDSVSET,	0x10000000   	 	 /* Value to trigger PendSV exception. */
.equ OS_CONFIG_CPU_SOFT_STK_OVERFLOW_DETECTION,	1	/* Enable Software stack overflow detection.				*/

/*------------------------- Public Functions ---------------------*/

	.global CPU_CountLeadZeros
	.global OS_CPU_ContexSwitch
	.global OS_CPU_FirstStart
	.global OS_CPU_InterruptContexSwitch
	.global OS_CPU_PendSVHandler
	.global CPU_SR_Save
	.global CPU_SR_Restore
	.global OS_StackOverflow_Detected

/*----------------------------------------------------------------*/

/************ CPU_tSR CPU_SR_Save (void) *****************
 * Save the interrupt status and disable the system interrupts.
 *
 * Arguments    :   None.
 *
 * Returns      :   The interrupt status register value, which is moved to R0.
 */
	.type   CPU_SR_Save,%function
	.thumb_func
CPU_SR_Save:
	/* Store the value of PRIMASK inside R0. 													*/
	mrs		r0,primask
	/* Disable interrupts and configurable fault handlers. i.e set the value of PRIMASK to 1 .	*/
	cpsid	i
	/* Return ...																				*/
	bx		lr


/************ void CPU_SR_Restore (CPU_tSR cpu_sr) *****************
 * Restore the interrupt status.
 *
 * Arguments    :   cpu_sr		is the previous CPU interrupt status value prior to 'CPU_SR_Save' call.
 *
 * Returns      :   None.
 */
	.type   CPU_SR_Restore,%function
	.thumb_func
CPU_SR_Restore:
	/* Move back the value of the stored interrupt status (R0) to PRIMASK.
		This will re-enable interrupts if previously PRIMASK was equal to 0.					*/
	msr		primask,r0
	/* Return ...																				*/
	bx		lr

/************ CPU_tWORD CPU_CountLeadZeros(CPU_tWORD val) *****************
 * Counts the number of contiguous, most-significant, leading zero bits before the
 * first binary one bit in a data value.
 *
 * Arguments    :   val   Data value to count the leading zero bits.
 *
 * Returns      :   Number of contiguous, most-significant, leading zero bits in 'val'.
 */
	.type   CPU_CountLeadZeros,%function
	.thumb_func
CPU_CountLeadZeros:
	clz		r0, r0
    bx 		lr

/************************* void OS_CPU_ContexSwitch(void) *****************
 * This function triggers the PendSV exception handler to perform
 * a context switch (where the real work happens).
 */
	.type   OS_CPU_ContexSwitch,%function
	.thumb_func
OS_CPU_ContexSwitch:
	ldr		r0, =NVIC_INT_CTRL
	ldr	    r1, [r0]
	orr		r1, r1, #(NVIC_PENDSVSET)
    str     r1, [r0]
    bx 		lr

/************************* void OS_CPU_InterruptContexSwitch(void) *****************
 * This function triggers the PendSV exception handler to perform
 * a context switch from an interrupt level.
 */
	.type   OS_CPU_InterruptContexSwitch,%function
	.thumb_func
OS_CPU_InterruptContexSwitch:
	ldr		r0, =NVIC_INT_CTRL
	ldr	    r1, [r0]
	orr		r1, r1, #(NVIC_PENDSVSET)
    str     r1, [r0]
    bx 		lr

/************************* void OS_CPU_FirstStart(void) *****************
 * This function triggers a PendSV exception (which will cause a context switch)
 * 		to start the first task.
 *
 * It does the following:
 *     - Setup PendSV exception priority to lowest.
 *     - Set OS_Running to OS_TRUE.
 *     - Trigger PendSV exception.
 *     - Enable interrupts (tasks will run with interrupts enabled).
 */
	.type   OS_CPU_FirstStart,%function
	.thumb_func
OS_CPU_FirstStart:
	/* Set the PendSV exception priority to the lowest Value 0xFF */
	ldr     r0, =NVIC_SYSPRI3
    ldr     r1, =NVIC_PENDSV_PRI
    strb    r1, [r0]
    /* Set OS_Running = TRUE */
    ldr     r0, =OS_Running
    movs    r1, #1
    strb    r1, [r0]
	/* Trigger the PendSV exception (causes context switch) */
    ldr     r0, =NVIC_INT_CTRL
    ldr     r1, =NVIC_PENDSVSET
    str     r1, [r0]
    /* Enable processor interrupts */
    cpsie   i
/* ------- OS_CPU_FirstStart End ------- */

/************************* void OS_CPU_PendSVHandler(void) *****************
 * This where the real context switch happens.
 *
 * Description:
 * ------------
 * In an OS enivronment, PendSV is recommended to be used for context switching when
 * no other exceptions or interrupts are active.
 * The Cortex M3/M4/M4F auto-saves bunch of processor registers on any exception entry,
 * and restores them on exception return. This leaves us to manaully save R4-R11
 * registers and proper setup for the stack pointers.
 *
 * The context switch is identical if it happens from a thread, interrupt or exception.
 *
 * On entry into PendSV handler:
 *           - The following have been saved on the process stack (by processor):
 *                xPSR, PC, LR, R12, R0-R3
 *           - Processor mode is switched to Handler mode (from Thread mode)
 *			 - SP is switched to the MSP (Main stack) from PSP (Process Stack).
 *
 * Pseudo-code:
 * -----------
 *		- Disable processor interrupts.
 *		- if it's the first time:
 *			- skip saving the cpu registers and jump to `resume`
 *		- if not:
 *			- Get the current running task SP.
 *			- Push the task registers R4-R11 to the task stack pointed by SP.
 *			- Save the new SP to the stack pointer member variable of the current task
 *				structure.
 *			- goto `resume`
 *		resume:
 *		- Get the new SP from the next task (SP = OS_nextTask->TASK_SP)
 *		- Make the current task is equal to the next task.
 *		- Restore R4-R11 registers from the task stack.
 *		- Enable processor interrupts.
 *		- Do exception return which will restore the remaining registers.
 *
 * The function prototype must be placed on entry 14 of of the ARM Cortex-M4 vector table.
 */
	.type   OS_CPU_PendSVHandler,%function
	.thumb_func
OS_CPU_PendSVHandler:
	/* Disable processor interrupts. */
	cpsid	i
	/* Skip saving the registers the first time. */
	ldr		r0,=OS_currentTask
	ldr		r0,[r0,#0]							/* OS_currentTask->TASK_SP 									*/
	cbz		r0,OS_CPU_PendSV_resume				/* If it's first time, then Jump to OS_CPU_PendSV_resume 	*/
.if OS_CONFIG_CPU_SOFT_STK_OVERFLOW_DETECTION == 1
	/* If not the first time, then :
		1) Check a possible stack overflow. 																*/
	ldr 	r1,[r0,#4]							/* OS_currentTask->TASK_SP_Limit																		*/
	SUB		r2,r13,#32							/*Move Current SP address by 8 pushed words (R4-R11) i.e 32 bytes ( ARM stacks grows from High to Low) 	*/
	CMP		r1,r2								/* Is SP_Limit >= Future SP ? ( ARM SP moves from High address to Low ).								*/
	bge		OS_CPU_STACK_OVERFLOW_DETECT		/*If SP_Limit >= SP, Jump to OS_CPU_STACK_OVERFLOW_DETECT												*/
.endif
												/*Else, The push will be fine within the memory stack area.	*/
	/*	2) Suspend the current task :
	   		- Push The current task registers into the memory stack.
	   		- Update the current task stack pointer to the new value due to stack push. 					*/
	push	{r4-r11}							/* Push from R4 to R11 */
	str		sp,[r0,#0] 							/* OS_currentTask->TASK_SP = SP */
	/* Resume another task. */
OS_CPU_PendSV_resume:
	/* Retreive the next task stack pointer. */
	ldr		r0,=OS_nextTask
	ldr		r0,[r0,#0]
	ldr		sp,[r0,#0]							/* SP = OS_nextTask->TASK_SP */
	/*Make the current task points to the next one. */
   	ldr     r1,=OS_nextTask
    ldr     r1, [r1]
    ldr     r2,=OS_currentTask
    str     r1, [r2]							/* OS_currentTask = OS_nextTask */
    /* Pop the next(resumed) task registers from the memory stack to registers. */
    pop 	{r4-r11}							/* Pop from R4 to R11 */
 	/* Enable processor interrupts. */
	cpsie	i
	/* Exception return will restore remaining registers. */
	bx		lr
.if OS_CONFIG_CPU_SOFT_STK_OVERFLOW_DETECTION == 1
OS_CPU_STACK_OVERFLOW_DETECT:
	ldr		r0,=OS_currentTask				/* Pass the task TCB which will cause a stack overflow.			*/
	bl 		OS_StackOverflow_Detected		/* Call PrettyOS' function for post operation of stack overflow */
.endif
