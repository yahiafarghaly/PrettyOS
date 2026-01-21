# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

PrettyOS is a preemptive hard real-time kernel (RTOS) for embedded devices. It's written in C with minimal assembly for context switching. The kernel supports two scheduling modes: Static Priority (RMS - Rate Monotonic Scheduling) and EDF (Earliest Deadline First).

## Build System

There is no Makefile or CMake. Projects are built by integrating kernel source files with port-specific files and compiling with the appropriate toolchain.

**For POSIX/Linux development:**
```bash
# Compile an application (example pattern)
gcc -I kernel/ -I port/posix/cpu/GNU/ -I port/posix/bsp/ -I Applications/ \
    kernel/*.c port/posix/cpu/GNU/*.c port/posix/bsp/*.c your_app.c \
    -lpthread -lrt -o your_app

# POSIX port requires real-time priority configuration:
# Add to /etc/security/limits.conf: "username - rtprio unlimited"
```

**For ARM Cortex-M4 (TI LM4F120):**
- Use embedded toolchain (arm-none-eabi-gcc)
- Link with `port/arm/cortex-m4/bsp/ek-lm4f120xl/ek-lm4f120xl.lds`
- Include startup file from BSP directory

## Architecture

```
kernel/              Core RTOS (~7K lines)
├── pretty_os.h      Public API - only header needed by applications
├── pretty_config.h  Configuration (enable/disable features, limits)
├── pretty_core.c    Scheduler and OS initialization
├── pretty_task.c    Task creation/deletion/suspension
├── pretty_mutex.c   Mutex with OCPP (Original Ceiling Priority Protocol)
├── pretty_sem.c     Semaphores
├── pretty_mailbox.c Message mailboxes
├── pretty_flags.c   Event flags
└── pretty_memory.c  Fixed-size block memory partitions

port/
├── arm/cortex-m4/   ARM Cortex-M4 port
│   ├── cpu/GNU/     Context switch (ASM), stack init, timer setup
│   └── bsp/         Board support (LM4F120)
└── posix/           Linux development port (pthreads-based)
    ├── cpu/GNU/     POSIX thread wrapper
    └── bsp/         Linux BSP

Applications/        21 example programs demonstrating all features
├── bsp.h            Portable BSP interface
└── [examples]/      tasks, semaphores, mutex, mailbox, Eventflags, memory, etc.
```

## Key Concepts

**Scheduler Modes:**
- **Static Priority:** One task per priority level (0=Idle reserved, 1=OS internal, 2-127=user tasks). Uses priority bitmap with CLZ optimization.
- **EDF:** Tasks sorted by deadline. When enabled, Mutex/Semaphore/Mailbox/EventFlags are automatically disabled in config.

**Application Pattern:**
```c
#include <bsp.h>
#include <pretty_os.h>

OS_tSTACK stkTask_Idle[STACK_SIZE];

void my_task(void* args) { /* task code */ }

int main() {
    BSP_HardwareSetup();
    OS_Init(stkTask_Idle, sizeof(stkTask_Idle));
    OS_TaskCreate(&my_task, NULL, stack, size, priority);
    OS_Run(BSP_CPU_FrequencyGet());  // Never returns
}
```

## Configuration

Edit `kernel/pretty_config.h` to enable/disable features:
- `OS_CONFIG_EDF_EN` - Switch between EDF and Static Priority scheduler
- `OS_CONFIG_MUTEX_EN`, `OS_CONFIG_SEMAPHORE_EN`, `OS_CONFIG_MAILBOX_EN`, `OS_CONFIG_FLAG_EN` - IPC services
- `OS_CONFIG_MEMORY_EN` - Memory partition management
- `OS_CONFIG_TASK_COUNT` - Max tasks (default 128, must be power of 2)
- `OS_CONFIG_TICKS_PER_SEC` - System tick rate (default 100 Hz)
- `OS_CONFIG_APP_*` - Application hooks

## Porting

See `port/porting_guide.md`. Required files for new port:
- `pretty_arch.h` - CPU config macros, low-level API prototypes
- `pretty_os_cpu.c` - Stack init, timer setup
- `pretty_os_cpu_a.asm` - Context switch assembly
- `bsp.c` - Board hardware init, BSP API implementations

Key low-level APIs to implement: `OS_CPU_FirstStart`, `OS_CPU_ContexSwitch`, `OS_CPU_InterruptContexSwitch`, `OS_CPU_TaskStackInit`, `OS_CPU_SystemTimerSetup`
