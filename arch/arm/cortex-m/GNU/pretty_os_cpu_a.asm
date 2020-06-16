
/*------------------------- External References ------------------*/
.extern OS_Running

/*----------------------------- Equals ---------------------------*/

NVIC_INT_CTRL=0xE000ED04 			 /* Interrupt control state register. */
NVIC_SYSPRI3=0xE000ED22     	 	 /* System priority register (priority 3), advanced with 2 bytes. */
NVIC_PENDSV_PRI=0xFF        		 /* PendSV priority value (lowest = 0xFF). */
NVIC_PENDSVSET=0x10000000   	 	 /* Value to trigger PendSV exception. */

/*------------------------- Public Functions ---------------------*/

.global OS_CPU_ContexSwitch
.global OS_CPU_FirstStart
.global OS_CPU_InterruptContexSwitch

OS_CPU_ContexSwitch:
	bx	lr

OS_CPU_InterruptContexSwitch:
	bx	lr

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



