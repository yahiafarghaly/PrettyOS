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

#ifndef __PRETTY_CONFIG_H_
#define __PRETTY_CONFIG_H_

/*
*******************************************************************************
*																			  *
*																			  *
*                       OS Miscellaneous Configurations                       *
*                       													  *
*                                     										  *
*******************************************************************************
*/

/******************************************************************************/
/**********************	  Enabling/Disabling Configs    ***********************/
/******************************************************************************/


#define 	OS_CONFIG_ENABLE				(1U)		/* TRUE  value for Enabling  a macro.		*/
#define 	OS_CONFIG_DISABLE				(0U)		/* FALSE value for Disabling a macro. 		*/

/*===============  Enable/Disable   Mutex 	 service in the code.   ===========*/

#define 	OS_CONFIG_MUTEX_EN				(OS_CONFIG_ENABLE)

/*===============  Enable/Disable Semaphores service in the code. 	===========*/

#define 	OS_CONFIG_SEMAPHORE_EN			(OS_CONFIG_ENABLE)

/*===============  Enable/Disable Mailboxes service in the code. 	===========*/

#define 	OS_CONFIG_MAILBOX_EN			(OS_CONFIG_ENABLE)

/*===============  Enable/Disable Memory Management service in the code. ======*/

#define		OS_CONFIG_MEMORY_EN				(OS_CONFIG_ENABLE)

/*===============  Enable/Disable  OS_ERRNO  service in the code.   ===========*/

#define 	OS_CONFIG_ERRNO_EN   			(OS_CONFIG_ENABLE)

/*=========  Enable/Disable Storing TaskEntry/Args in TCB Structure. ==========*/

#define OS_CONFIG_TCB_TASK_ENTRY_STORE_EN	(OS_CONFIG_ENABLE)

/*=========  Enable/Disable Storing OSTCBExtension in TCB Structure. ==========*/

#define OS_CONFIG_TCB_EXTENSION_EN			(OS_CONFIG_ENABLE)


/********************************************************************************/
/**********************	      Parameterized Configs	      ***********************/
/********************************************************************************/


/*====================== Number of System Ticks/Second. =======================*/

#define OS_CONFIG_TICKS_PER_SEC     								(100U)		/* 100 or 1000 is acceptable.  			*/

/*=================== Max Number of Possible Created Tasks. ===================*/

#define OS_CONFIG_TASK_COUNT  										(128U)   	/* Required to be multiple of 8.   		*/

/*=================== Max Number of Possible Created Events. ===================*/

#define OS_CONFIG_MAX_EVENTS         								(10U)     	/* Max. of Event Objects				*/

/*=================== Max Number of Possible Memory Partition. =================*/

#define OS_CONFIG_MEMORY_PARTITION_COUNT							(10U)		/* Max. of Memory Partition Objects.	*/


/******************************************************************************/
/************************* A U T O GENERATED MACROS ***************************/
/******************************************************************************/

#define OS_AUTO_CONFIG_INCLUDE_EVENTS	(OS_CONFIG_SEMAPHORE_EN || OS_CONFIG_MUTEX_EN || OS_CONFIG_MAILBOX_EN)


/******************************************************************************/
/************************* Configurable DataTypes  ****************************/
/******************************************************************************/


typedef CPU_t16U        OS_SEM_COUNT;       	/* Max. Semaphore Count Limit. 		*/
typedef CPU_t32U		OS_MEMORY_BLOCK;		/* Max. Size of memory block.		*/


#endif /* __PRETTY_CONFIG_H_ */
