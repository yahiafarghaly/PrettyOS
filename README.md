<p align="center">
  <img src="logo.png">
</p>
<p align="center">
  <b>A Preemptive dynamic priority based hard real time kernel for embedded devices</b>.
</p>

#### ☑ List of supported Features

| Features      | Status        |
| ------------- |:-------------:|
| Preemptive Multitasking Scheduling | **Yes** |
| Maximum number of tasks      | Configurable     |
| Number of tasks at each priority level | 1      |
| Priority Change at run time | **Yes** |
| Scheduling lock/Unlock | **Yes** |
|Task suspend/resume| **Yes** |
|Catch a task that returns| **Yes** |
|Semaphores| **Yes** |
| Mutual exclusion semaphores(Mutex) | **Yes** with Original Ceiling Priority Protocol (**OCPP**)|
| Message mailboxes | **Yes** |
| Memory Management | **Yes** - Basic Memory Partition Manager |
| User definable hook functions | **Yes** - In both Application level and CPU Port Level |
|Software timers| No|
| Support Round robin Scheduling | No |


#### 💻 Porting availability
| System      			    | BSP        	  | CPU port 		  | Notes         |
| ----------------------|:-------------:|:-------------:|:-------------:|
| TI stellaris LM4F120 	| Done 			    | Done 			    |               |
| Linux machine         | Done          | Done          |Requires POSIX.1b standards as minimal |

To add another port, Please read this [porting guide](port/porting_guide.md) first.

#### Include the RTOS
You include only a single header file [pretty_os.h](kernel/pretty_os.h) which contains the list
of the public APIs with a proper description for each one.


#### 📝 License
Copyright © 2020 - present, Yahia Farghaly Ashour.<br>
This project is [MIT](https://github.com/yahiafarghaly/PrettyOS/blob/master/LICENSE) Licensed.
