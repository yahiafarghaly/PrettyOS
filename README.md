<p align="center">
  <img src="logo.png">
</p>
<p align="center">
  <b>A Preemptive Hard Real time kernel for embedded devices</b>.
</p>

#### ☑ List of Supported Features

- **Static** and **Dynamic** Priority Schedulers
    - **Preemptive Scheduling** using a **static** priority scheduling class.
        - An RMS ([Rate Monotonic Scheduling](https://en.wikipedia.org/wiki/Rate-monotonic_scheduling)) can be effective for use.
        - Number of tasks at each priority level is 1.
    - **EDF** (Earliest Deadline First) 
        - Limited Support for kernel services.

- **Configurable** Number of Tasks.

- **Lock/Unlock** Scheduler.

- Support **Memory Management** .
    - Using a basic memory manager for fixed-sized allocatable objects in a memory partition (i.e region).  
    
- For **Static Priority** Scheduling

    - **Runtime Priority** Change.
    - **Suspend/Resume** Tasks.
    - **Mutex** Support. 
        - Including **OCPP** ( [Original Ceiling Priority Protocol](https://en.wikipedia.org/wiki/Priority_ceiling_protocol) ) to overcome priority inversion scenarios.
    - Support **Semaphores**, **Message Mailboxes** and **EventFlags** .  

- **Hooks APIs** at Application and CPU port level.

- Software based Tasks' **stack overflow detection**.

#### 💻 Porting availability
| System      			| BSP / CPU Port 	| Notes                                 |
| ----------------------|:-----------------:|:-------------------------------------:|
| TI Stellaris LM4F120 	|✔️ 			    |                                       |
| Linux machine         | ✔️                |Requires POSIX.1b standards as minimal |

To add another port, Please read this [porting guide](port/porting_guide.md) first.

#### 🗃️ Include the RTOS
You include only a single header file [pretty_os.h](kernel/pretty_os.h) which contains the list
of the public APIs with a proper description for each one.

#### 📝 License
Copyright © 2020 - present, Yahia Farghaly Ashour.<br>
This project is [MIT](https://github.com/yahiafarghaly/PrettyOS/blob/master/LICENSE) Licensed.
