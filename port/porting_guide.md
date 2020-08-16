### Introduction

This document guides you with steps of how possibly porting **PrettyOS** to a new target whether on BSP or CPU level.

### Porting Directories Structures

Initially, the ideal port directory structure would 
be something like this: 

```
port
 +
 |
 +->[CPU Family Name]
 |   +
 |   +> [CPU Target Name]
 |      +
 |      +--> [Compiler Name]
 |             +----->pretty_arch.h                                                             [Required       ]
 |             |----->pretty_os_cpu.c                                                           [Required       ]
 |             +----->pretty_os_cpu_a.asm (pure assembly portion)                               [Mostly Required]
 |
 +>[bsp]
  |
  +--->[Board Name]
        +->bsp.c ( Contains board HWs initialization and any BSP APIs definitions)              [Required       ]
        |->board_name_compiler_name.ld (linker script)                                          [Mostly Required]
        +->board_name_compiler_name.s (startup script)                                          [Mostly Required]
        +-> other files.c/.h                                                                    [optional       ]
```

Your porting structure could be different a little by making sub directories of different compilers under directory of **[board name]**.

From the above structure, we conclude that there are some required files to be exist in every port.

##### pretty_arch.h
Contains the target CPU configurations in shape of C macros such as CPU data bus size/CPU address bus size,
how the stack grows, the scheme which interrupts are disabled or enabled. 
And the prototypes of low level APIs to be implemented for the target CPU whether in C or Assembly or both for handling the tasks
context switches and the OS system time ticker.

A template file which you can start with is using a previous port file and modify it for your CPU port.
a typical file is [arm/cortex-m4](https://github.com/yahiafarghaly/PrettyOS/blob/master/port/arm/cortex-m4/cpu/GNU/pretty_arch.h) port. It's a documented port file and easy to be read.

##### pretty_os_cpu.c and pretty_os_cpu_a.asm
These two files are the definitions of the prettyOS low level APIs declared in **pretty_os.h** .
The low level APIs implementation can be in one file whether in CPU assembly or in C or both.

usually, in both
since **pretty_os_cpu.c** is used for creating stack frame of a task and setup the OS system ticker handler. 
and **pretty_os_cpu_a.asm** is used for doing the real context switch of tasks in assembly.

##### bsp.c
This is the C APIs implementation for the desired target of [bsp.h](https://github.com/yahiafarghaly/PrettyOS/blob/master/Applications/bsp.h) such that the applications examples of prettyOS be portable across different targets.

### A deep look for prettyOS low level APIs
Initially, there are external global variables & functions of which prettyOS kernel internally use.
These *externs* must be included in your port files.

```
extern void OS_TaskReturn (void);       /* This is called if task intentionally returned.                                       */
extern void OS_TimerTick  (void);       /* This calls prettyOS internal of signalling the occurance of CPU tick.                */
extern void OS_IntEnter   (void);       /* This is used to notify prettyOS of interrupt enterence.                              */
extern void OS_IntExit    (void);       /* This is used to notify prettyIS of interrupt exit/end.                               */

.extern OS_Running                      /* extern in GNU assembly, Represent the status of OS (Running = OS_TRUE)               */
.extern OS_currentTask                  /* extern in GNU assembly, Represent the task which will be switched-out                */
.extern OS_nextTask                     /* extern in GNU assembly, Represent the task which will be switched-in (preempted).    */

```

The implementation of the following APIs is very CPU-specific.

###### OS_CPU_FirstStart
This function is responsible for doing the following:
- Setup an interrupt handler for performing the context switch.
- Set OS_Running to OS_TRUE.
- Trigger this interrupt handler to start the first task.
- Enable CPU interrupts.

###### OS_CPU_ContexSwitch
- Perform context switch from a task level code. (i.e not from inside an ISR)

###### OS_CPU_InterruptContexSwitch
- Perform context switch from an interrupt level. such as switching out from an interrupt to a task.

###### OS_CPU_TaskStackInit
- This is used for setup the stack for the created task. i.e the order of CPU registers are
pushed/poped from stack to CPU pyhsical registers map.

###### OS_CPU_SystemTimerSetup
- Initialize the timer which will be used as a system ticker for the OS.

###### OS_CPU_SystemTimerHandler
- This is the interrupt vector function for timer tick occurrance. It's used to notify prettyOS of the occurrance of the tick. 
The code for this handler is usually the same for all ports and it is written in this way.

```
void OS_CPU_SystemTimerHandler  (void)
{
    CPU_SR_ALLOC();

    OS_CRTICAL_BEGIN();

    OS_IntEnter();          /* Notify that we are entering an ISR.          */

    OS_CRTICAL_END();

    OS_TimerTick();         /* Signal the tick to the OS_timerTick().       */

    OS_IntExit();           /* Notify that we are leaving the ISR.          */
}
``` 

###### CPU_CountLeadZeros
- This calls the CPU assembly instruction of **clz** (count leading zeros), If it's supported by your target CPU.
If it's not supported, then it will call the [C implementation of the clz](https://github.com/yahiafarghaly/PrettyOS/blob/master/kernel/pretty_clz.c) provided with the kernel code.
by setting *CPU_CONFIG_COUNT_LEAD_ZEROS_ASM_PRESENT to 0* in **pretty_arch.h** .

###### CPU_SR_ALLOC & OS_CRTICAL_BEGIN & OS_CRTICAL_END
- These are macros which are used for protecting the critical sections of code.
-- CPU_SR_ALLOC : is used to allocate a local variable in case of a local variable for saving the interrupt status.
-- OS_CRTICAL_BEGIN : Disable CPU Interrupts.
-- OS_CRTICAL_END : Enable CPU Interrupts.


For now, you should have a good overview of the essential bones for the porting. 

Let's go for the heart where the magic happens :) . 
##### [the context switch ]
Here, I am providing a pseudo code for the context switch which you will replace with the equivalent assembly for performing the context switch.

```
 void ContextSwitch (void)
 {
        CPU_Disable_Interrupts();
        if (OS_currentTask->TASK_SP != ((void*)0))      /* Is it first context switch ?                                 */
        {       
                                                        /* No, Suspend the current task.                                */
                PUSH the calling function registers into the memory stack. (which is the current task)
                Save the new Stack Pointer in task structure [Defined in offest zero in the C structure, Look to OS_TASK_TCB]
                OS_currentTask->TASK_SP = R13;          /* Where R13 is the stack pointer for some CPU architecture.    */
        }
                                                        /* Yes, ...                                                     */
        R13 = OS_nextTask->TASK_SP;                     /* Move the CPU Stack Pointer to the switched-in task last SP.  */
        OS_currentTask = OS_nextTask;                   /* Make current equals to the next.                             */
        
        POP the next task registers from memory stack to the CPU registers. Such that the CPU will jump to the new Program Counter
        after this context is ended.

        CPU_Enable_Interrupts();     

        return;
 }
```

The above code is usually placed inside an interrupt handler for doing the context switch in a privileged mode.

At last, there are few low level APIs hooks which are user definable but must be included in the port file
if configured to be added since it's called internally by prettyOS.

These are the list of hook functions:
| Hook Function          |  Short Description	                                                        |
| ---------------------- |:----------------------------------------------------------------------------:|
| OS_CPU_Hook_Init       | Called at the early stages of OS_Init() before doing any OS initialization.  |
| OS_CPU_Hook_TaskCreated | Called when a task is created.                                               |
| OS_CPU_Hook_TaskDeleted | Called when a task is deleted.                                               |
| OS_CPU_Hook_ContextSwitch  | Called when OS performs a task context switch                                |
| OS_CPU_Hook_TimeTick  | Called when OS_TimerTick() is called before decrementing any tasks ticks.    |
| OS_CPU_Hook_Idle       | Called when OS is running its idle task.                                     |

This document ends here, hope it's helpful for starting to port to other architectures and target boards.
Beside this document, I recommend reading the code of one the supported ported target. It will definitally help clarifying 
much more.