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

/*
 * Author   : Yahia Farghaly Ashour
 *
 * Purpose  : Wrapper implementation of BSP APIs according to POSIX standards on a Linux machine.
 *
 * Language:  C
 */

/*
*******************************************************************************
*                               Includes Files                                *
*******************************************************************************
*/
#ifndef _XOPEN_SOURCE
	#define _XOPEN_SOURCE	600
#endif

#include <bsp.h>                    /* BSP Exposed APIs.                        		*/
#include <stdio.h>					/* Standard I/O C routines.							*/
#include <stdlib.h>					/* Standard C routines.								*/
#include <math.h>					/* For round() function. Add -lm for gcc linker. 	*/
#include <time.h>   				/* for nanosleep 									*/
#include <unistd.h> 				/* for sysconf 										*/
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/resource.h>
/*
*******************************************************************************
*                               BSP Globals                                   *
*******************************************************************************
*/

/*[The Best Choice]: This macro enables delaying of requested ms in terms of usleep() function call. */
#define USE_USLEEP_IN_TERMS_OF_MILLISEC		1U

#define USE_POSIX_SLEEP_IN_DELAY_LOOP		(0 || USE_USLEEP_IN_TERMS_OF_MILLISEC)	/* 1 to enable using POSIX sleep functions. 0 otherwise.*/

static double __delay_loop_time_nanosecond  = 0.0f;
static double __system_cpu_frequency_MHz	= 0.0f;

static void __delay_loop (void);
static void __delay_loop_calibration (void);

void
BSP_HardwareSetup(void) {

	system("clear");				/* Clear the console.						*/
	printf("[BSP]: Initialization starts ... \n");

	printf("[BSP]: #Cores = %d\n", (int)sysconf( _SC_NPROCESSORS_ONLN ));
	fflush(stdout);

	/* Extract Core#1 speed from /proc/cpuinfo .								*/
	{
		FILE *fp;
		char* command = "cat /proc/cpuinfo | grep 'cpu MHz' | cut -d: -f2 | head -n1";
		char output[20] = { 0 };
		fp = popen(command,"r");
		fscanf(fp,"%s",&output[0]);	/* Store the output value in the array char	*/
		fclose(fp);

		__system_cpu_frequency_MHz = strtof(output,NULL);
	}

	printf("[BSP]: Processor Core#1 Frequency  = %f MHz\n",__system_cpu_frequency_MHz);
	fflush(stdout);
#if(USE_POSIX_SLEEP_IN_DELAY_LOOP == 0U)
	printf("[BSP]: Calibrating __delay_loop() for the current processor speed ...\n");

	__delay_loop_calibration();

	printf("\n[BSP]: Max resolution of __delay_loop() = %f milliseconds\n",__delay_loop_time_nanosecond*1e-6);

	int sec = 10;
	printf("[BSP]: Testing %d seconds delay ...\n",sec);
	fflush(stdout);

	sleep(2);
	for(; sec > 0; sec--)
	{
		printf("[BSP]: ... Remaining %d seconds ... \r",sec);
		fflush(stdout);
		BSP_DelayMilliseconds(1000);
	}

	printf("\n");
	fflush(stdout);
#endif
	printf("[BSP]: Done ..\n");

	fflush(stdout);
}

/*
 * Calculate the average time (in nanoseconds) which delay_loop() takes to be completely executed.
 * This implementation assumes an almost stable running frequency of CPU.
 * Otherwise, __delay_loop() cannot be taken seriously for busy waiting delays. Instead, set USE_POSIX_SLEEP_IN_DELAY_LOOP = 1U;
 *  */
void __delay_loop_calibration (void)
{
#if (USE_POSIX_SLEEP_IN_DELAY_LOOP == 0U)
	#define TRIAL_PER_CALIBRATE 100U
	#define	CALIBRATE_COUNT		10U

	struct timespec start 	= { 0 , 0};
	struct timespec end 	= { 0 , 0};
	double 		  __total_delay_loop_time_nanosecond 		= 0.0f;

	static double __accumlated_delay_loop_time_nanosecond 	= 0.0f;
	static size_t  __calibrate_cnt							= CALIBRATE_COUNT;


	__delay_loop_time_nanosecond 						= 0.0f;

	if(__calibrate_cnt == CALIBRATE_COUNT)
	{
		printf("\n[__delay_loop_calibration]: #Trial/Calibration = %d , #Calibration = %d\n",TRIAL_PER_CALIBRATE,CALIBRATE_COUNT);
		fflush(stdout);
	}

	for(size_t t = TRIAL_PER_CALIBRATE; t > 0; --t)
	{
		clock_gettime(CLOCK_REALTIME,&start);

		__delay_loop();

		clock_gettime(CLOCK_REALTIME,&end);

		__delay_loop_time_nanosecond = (double)((end.tv_nsec - start.tv_nsec) + (end.tv_sec - start.tv_sec)*1e9);

		__total_delay_loop_time_nanosecond += __delay_loop_time_nanosecond;
	}

	__delay_loop_time_nanosecond = __total_delay_loop_time_nanosecond / (double)TRIAL_PER_CALIBRATE;

	if(__calibrate_cnt == 0)
	{
		__delay_loop_time_nanosecond = __accumlated_delay_loop_time_nanosecond/(double)CALIBRATE_COUNT;
		printf("\n");
		fflush(stdout);
		return;
	}
	else
	{
		__accumlated_delay_loop_time_nanosecond += __delay_loop_time_nanosecond;
		printf("\r[__delay_loop_calibration]... %f ..",__delay_loop_time_nanosecond);
		fflush(stdout);
		__calibrate_cnt -= 1;
		usleep(100*1000);				/*	wait 100 ms	*/
		__delay_loop_calibration();
	}
#else
	printf("[__delay_loop_calibration]: no need. The application use POSIX sleep() for busy waiting loop.\n");
	fflush(stdout);
#endif
}

/*
 * Execute a "nop" instruction for 255 iterations.
 * */
void __delay_loop (void)
{
	for(size_t i = 255U; i > 0; --i)
	{
		__asm volatile ("nop");		/* GNU assembly for x86 architecture.*/
	}
}

void
BSP_DelayMilliseconds (unsigned long ms) {

#if (USE_POSIX_SLEEP_IN_DELAY_LOOP == 1U)
#if (USE_USLEEP_IN_TERMS_OF_MILLISEC == 0U)
	/*[To Investigate & Fix]: Using nanosleep() or usleep(ms*1000) as a busy-wait loop
	 * is not functioning well (in terms of delaying not context switch) with the underlying porting code.
	 * 	*/
	#if (_POSIX_C_SOURCE >= 199309L)
		struct timespec ts;
		ts.tv_sec = ms / 1000U;
		ts.tv_nsec = (ms % 1000U) * 1000000U;
		nanosleep(&ts, NULL);
	#else
		usleep(ms * 1000U);
	#endif
#else
		/* The simple solution for the above wrong behavior. */
		for(unsigned int t = ms; t > 0; t--)
		{
			usleep(1000U);
		}
#endif

#else

	double ticks = (double)(((double)ms*1e6) / (__delay_loop_time_nanosecond));	/* Number of iterations to execute __delay_loop() to reaches the requested delay time (ms)*/
	ticks = (unsigned long)round(ticks);

	for(unsigned long t = (unsigned long)ticks; t > 0; t--)
	{
		__delay_loop();
	}

#endif
}

/*
 * Simple implementation like what should happens if it was on a bare metal.
 * */
void
BSP_UART_SendByte(const unsigned char cData)
{
	putchar((int)cData);				/* Send the char to stdout.												*/
	fflush(stdout);						/* Flush the write buffer. So it doesn't wait for buffer to be full.	*/
}

int
BSP_Write_to_Console (const char *format, ...)
{
#include <stdarg.h>
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(format);

    fflush(stdout);
    return 0;
}

void
BSP_UART_ClearVirtualTerminal (void)
{

}

void
BSP_LED_RedOn(void) {
	/* 	EMPTY	*/
}

void
BSP_LED_RedOff(void) {
	/* 	EMPTY	*/
}

void
BSP_LED_BlueOn(void) {
	/* 	EMPTY	*/
}

void
BSP_LED_BlueOff(void) {
	/* 	EMPTY	*/
}

void
BSP_LED_GreenOn(void) {
	/* 	EMPTY	*/
}

void
BSP_LED_GreenOff(void) {
	/* 	EMPTY	*/
}

unsigned long
BSP_CPU_FrequencyGet(void)
{
	/* Return in Hz.		*/
    return (unsigned long)(__system_cpu_frequency_MHz*1000*1000);
}

void
BSP_onFailure(char const *module, int location)
{
    printf("BSP Failure at module: %s, LOC: %d\n",module,location);
    for(;;);
}

void
BSP_CPU_WFI (void)
{
	/* 	EMPTY	*/
}

void BSP_CPU_NOP (void)
{
	/* 	EMPTY	*/
}
