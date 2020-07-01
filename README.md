# PrettyOS
---------
A Preemptive dynamic priority based real hard time kernel for embedded devices.

### List of supported Features

| Features      | Status        |
| ------------- |:-------------:|
| Preemptive Multitasking      | **Yes** |
| Maximum number of tasks      | Configurable     |
| Number of tasks at each priority level | 1      |
| Priority Change at run time | **Yes** |
| Support Round robin scheduling | No |
| Scheduling lock/Unlock | **Yes** |
|Task suspend/resume| **Yes** |
|Catch a task that returns| No |
|Semaphores| **Yes** |
| Mutual exclusion semaphores | No |
|Software timers| No|
| User definable hook functions | No |
| Compile-time configurable | No |
| Message mailboxes | No |

### Porting availability
| System      | BSP        | Hardware layer|
| ------------- |:-------------:|:-------------:|
| TI stellaris LM4F120 | Done | Done |