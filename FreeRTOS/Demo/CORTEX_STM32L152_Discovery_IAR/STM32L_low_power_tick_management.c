/*
    FreeRTOS V8.2.3 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/* Standard includes. */
#include <limits.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* ST library functions. */
#include "stm32l1xx.h"

/*
 * When configCREATE_LOW_POWER_DEMO is set to 1 then the tick interrupt
 * is generated by the TIM2 peripheral.  The TIM2 configuration and handling
 * functions are defined in this file.  Note the RTC is not used as there does
 * not appear to be a way to read back the RTC count value, and therefore the
 * only way of knowing exactly how long a sleep lasted is to use the very low
 * resolution calendar time.
 *
 * When configCREATE_LOW_POWER_DEMO is set to 0 the tick interrupt is
 * generated by the standard FreeRTOS Cortex-M port layer, which uses the
 * SysTick timer.
 */
#if configCREATE_LOW_POWER_DEMO == 1

/* The frequency at which TIM2 will run. */
#define lpCLOCK_INPUT_FREQUENCY 	( 1000UL )

/* STM32 register used to ensure the TIM2 clock stops when the MCU is in debug
mode. */
#define DBGMCU_APB1_FZ 	( * ( ( volatile unsigned long * ) 0xE0042008 ) )

/*-----------------------------------------------------------*/

/*
 * The tick interrupt is generated by the TIM2 timer.
 */
void TIM2_IRQHandler( void );

/*-----------------------------------------------------------*/

/* Calculate how many clock increments make up a single tick period. */
static const uint32_t ulReloadValueForOneTick = ( ( lpCLOCK_INPUT_FREQUENCY / configTICK_RATE_HZ ) - 1 );

/* Holds the maximum number of ticks that can be suppressed - which is
basically how far into the future an interrupt can be generated. Set during
initialisation. */
static TickType_t xMaximumPossibleSuppressedTicks = 0;

/* Flag set from the tick interrupt to allow the sleep processing to know if
sleep mode was exited because of an tick interrupt or a different interrupt. */
static volatile uint32_t ulTickFlag = pdFALSE;

/*-----------------------------------------------------------*/

/* The tick interrupt handler.  This is always the same other than the part that
clears the interrupt, which is specific to the clock being used to generate the
tick. */
void TIM2_IRQHandler( void )
{
	/* Clear the interrupt. */
	TIM_ClearITPendingBit( TIM2, TIM_IT_Update );

	/* The next block of code is from the standard FreeRTOS tick interrupt
	handler.  The standard handler is not called directly in case future
	versions contain changes that make it no longer suitable for calling
	here. */
	( void ) portSET_INTERRUPT_MASK_FROM_ISR();
	{
		if( xTaskIncrementTick() != pdFALSE )
		{
			portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
		}

		/* Just completely clear the interrupt mask on exit by passing 0 because
		it is known that this interrupt will only ever execute with the lowest
		possible interrupt priority. */
	}
	portCLEAR_INTERRUPT_MASK_FROM_ISR( 0 );

	/* In case this is the first tick since the MCU left a low power mode the
	reload value is reset to its default. */
	TIM2->ARR = ( uint16_t ) ulReloadValueForOneTick;

	/* The CPU woke because of a tick. */
	ulTickFlag = pdTRUE;
}
/*-----------------------------------------------------------*/

/* Override the default definition of vPortSetupTimerInterrupt() that is weakly
defined in the FreeRTOS Cortex-M3 port layer with a version that configures TIM2
to generate the tick interrupt. */
void vPortSetupTimerInterrupt( void )
{
NVIC_InitTypeDef NVIC_InitStructure;
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	/* Enable the TIM2 clock. */
  	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2, ENABLE );

	/* Ensure clock stops in debug mode. */
	DBGMCU_APB1_FZ |= DBGMCU_APB1_FZ_DBG_TIM2_STOP;

	/* Scale the clock so longer tickless periods can be achieved.  The	SysTick
	is not used as even when its frequency is divided by 8 the maximum tickless
	period with a system clock of 16MHz is only 8.3 seconds.  Using	a prescaled
	clock on the 16-bit TIM2 allows a tickless period of nearly	66 seconds,
	albeit at low resolution. */
	TIM_TimeBaseStructure.TIM_Prescaler = ( uint16_t ) ( SystemCoreClock / lpCLOCK_INPUT_FREQUENCY );
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = ( uint16_t ) ( lpCLOCK_INPUT_FREQUENCY / configTICK_RATE_HZ );
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseInit( TIM2, &TIM_TimeBaseStructure );

	/* Enable the TIM2 interrupt.  This must execute at the lowest interrupt
	priority. */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_LOWEST_INTERRUPT_PRIORITY; /* Must be set to lowest priority. */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	TIM_ITConfig( TIM2, TIM_IT_Update, ENABLE );
	TIM_SetCounter( TIM2, 0 );
	TIM_Cmd( TIM2, ENABLE );

	/* See the comments where xMaximumPossibleSuppressedTicks is declared. */
	xMaximumPossibleSuppressedTicks = ( ( unsigned long ) USHRT_MAX ) / ulReloadValueForOneTick;
}
/*-----------------------------------------------------------*/

/* Override the default definition of vPortSuppressTicksAndSleep() that is
weakly defined in the FreeRTOS Cortex-M3 port layer with a version that manages
the TIM2 interrupt, as the tick is generated from TIM2 compare matches events. */
void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
{
uint32_t ulCounterValue, ulCompleteTickPeriods;
eSleepModeStatus eSleepAction;
TickType_t xModifiableIdleTime;
const TickType_t xRegulatorOffIdleTime = 30;

	/* THIS FUNCTION IS CALLED WITH THE SCHEDULER SUSPENDED. */

	/* Make sure the TIM2 reload value does not overflow the counter. */
	if( xExpectedIdleTime > xMaximumPossibleSuppressedTicks )
	{
		xExpectedIdleTime = xMaximumPossibleSuppressedTicks;
	}

	/* Calculate the reload value required to wait xExpectedIdleTime tick
	periods. */
	ulCounterValue = ulReloadValueForOneTick * xExpectedIdleTime;

	/* Stop TIM2 momentarily.  The time TIM2 is stopped for is not accounted for
	in this implementation (as it is in the generic implementation) because the
	clock is so slow it is unlikely to be stopped for a complete count period
	anyway. */
	TIM_Cmd( TIM2, DISABLE );

	/* Enter a critical section but don't use the taskENTER_CRITICAL() method as
	that will mask interrupts that should exit sleep mode. */
	__asm volatile ( "cpsid i" );
	__asm volatile ( "dsb" );
	__asm volatile ( "isb" );

	/* The tick flag is set to false before sleeping.  If it is true when sleep
	mode is exited then sleep mode was probably exited because the tick was
	suppressed for the entire xExpectedIdleTime period. */
	ulTickFlag = pdFALSE;

	/* If a context switch is pending then abandon the low power entry as
	the context switch might have been pended by an external interrupt that
	requires processing. */
	eSleepAction = eTaskConfirmSleepModeStatus();
	if( eSleepAction == eAbortSleep )
	{
		/* Restart tick. */
		TIM_Cmd( TIM2, ENABLE );

		/* Re-enable interrupts - see comments above the cpsid instruction()
		above. */
		__asm volatile ( "cpsie i" );
	}
	else if( eSleepAction == eNoTasksWaitingTimeout )
	{
		/* A user definable macro that allows application code to be inserted
		here.  Such application code can be used to minimise power consumption
		further by turning off IO, peripheral clocks, the Flash, etc. */
		configPRE_STOP_PROCESSING();

		/* There are no running state tasks and no tasks that are blocked with a
		time out.  Assuming the application does not care if the tick time slips
		with respect to calendar time then enter a deep sleep that can only be
		woken by (in this demo case) the user button being pushed on the
		STM32L discovery board.  If the application does require the tick time
		to keep better track of the calender time then the RTC peripheral can be
		used to make rough adjustments. */
		PWR_EnterSTOPMode( PWR_Regulator_LowPower, PWR_SLEEPEntry_WFI );

		/* A user definable macro that allows application code to be inserted
		here.  Such application code can be used to reverse any actions taken
		by the configPRE_STOP_PROCESSING().  In this demo
		configPOST_STOP_PROCESSING() is used to re-initialise the clocks that
		were turned off when STOP mode was entered. */
		configPOST_STOP_PROCESSING();

		/* Restart tick. */
		TIM_SetCounter( TIM2, 0 );
		TIM_Cmd( TIM2, ENABLE );

		/* Re-enable interrupts - see comments above the cpsid instruction()
		above. */
		__asm volatile ( "cpsie i" );
		__asm volatile ( "dsb" );
		__asm volatile ( "isb" );
	}
	else
	{
		/* Trap underflow before the next calculation. */
		configASSERT( ulCounterValue >= TIM_GetCounter( TIM2 ) );

		/* Adjust the TIM2 value to take into account that the current time
		slice is already partially complete. */
		ulCounterValue -= ( uint32_t ) TIM_GetCounter( TIM2 );

		/* Trap overflow/underflow before the calculated value is written to
		TIM2. */
		configASSERT( ulCounterValue < ( uint32_t ) USHRT_MAX );
		configASSERT( ulCounterValue != 0 );

		/* Update to use the calculated overflow value. */
		TIM_SetAutoreload( TIM2, ( uint16_t ) ulCounterValue );
		TIM_SetCounter( TIM2, 0 );

		/* Restart the TIM2. */
		TIM_Cmd( TIM2, ENABLE );

		/* Allow the application to define some pre-sleep processing.  This is
		the standard configPRE_SLEEP_PROCESSING() macro as described on the
		FreeRTOS.org website. */
		xModifiableIdleTime = xExpectedIdleTime;
		configPRE_SLEEP_PROCESSING( xModifiableIdleTime );

		/* xExpectedIdleTime being set to 0 by configPRE_SLEEP_PROCESSING()
		means the application defined code has already executed the wait/sleep
		instruction. */
		if( xModifiableIdleTime > 0 )
		{
			/* The sleep mode used is dependent on the expected idle time
			as the deeper the sleep the longer the wake up time.  See the
			comments at the top of main_low_power.c.  Note xRegulatorOffIdleTime
			is set purely for convenience of demonstration and is not intended
			to be an optimised value. */
			if( xModifiableIdleTime > xRegulatorOffIdleTime )
			{
				/* A slightly lower power sleep mode with a longer wake up
				time. */
				PWR_EnterSleepMode( PWR_Regulator_LowPower, PWR_SLEEPEntry_WFI );
			}
			else
			{
				/* A slightly higher power sleep mode with a faster wake up
				time. */
				PWR_EnterSleepMode( PWR_Regulator_ON, PWR_SLEEPEntry_WFI );
			}
		}

		/* Allow the application to define some post sleep processing.  This is
		the standard configPOST_SLEEP_PROCESSING() macro, as described on the
		FreeRTOS.org website. */
		configPOST_SLEEP_PROCESSING( xModifiableIdleTime );

		/* Stop TIM2.  Again, the time the clock is stopped for in not accounted
		for here (as it would normally be) because the clock is so slow it is
		unlikely it will be stopped for a complete count period anyway. */
		TIM_Cmd( TIM2, DISABLE );

		/* Re-enable interrupts - see comments above the cpsid instruction()
		above. */
		__asm volatile ( "cpsie i" );
		__asm volatile ( "dsb" );
		__asm volatile ( "isb" );

		if( ulTickFlag != pdFALSE )
		{
			/* Trap overflows before the next calculation. */
			configASSERT( ulReloadValueForOneTick >= ( uint32_t ) TIM_GetCounter( TIM2 ) );

			/* The tick interrupt has already executed, although because this
			function is called with the scheduler suspended the actual tick
			processing will not occur until after this function has exited.
			Reset the reload value with whatever remains of this tick period. */
			ulCounterValue = ulReloadValueForOneTick - ( uint32_t ) TIM_GetCounter( TIM2 );

			/* Trap under/overflows before the calculated value is used. */
			configASSERT( ulCounterValue <= ( uint32_t ) USHRT_MAX );
			configASSERT( ulCounterValue != 0 );

			/* Use the calculated reload value. */
			TIM_SetAutoreload( TIM2, ( uint16_t ) ulCounterValue );
			TIM_SetCounter( TIM2, 0 );

			/* The tick interrupt handler will already have pended the tick
			processing in the kernel.  As the pending tick will be processed as
			soon as this function exits, the tick value	maintained by the tick
			is stepped forward by one less than the	time spent sleeping.  The
			actual stepping of the tick appears later in this function. */
			ulCompleteTickPeriods = xExpectedIdleTime - 1UL;
		}
		else
		{
			/* Something other than the tick interrupt ended the sleep.  How
			many complete tick periods passed while the processor was
			sleeping? */
			ulCompleteTickPeriods = ( ( uint32_t ) TIM_GetCounter( TIM2 ) ) / ulReloadValueForOneTick;

			/* Check for over/under flows before the following calculation. */
			configASSERT( ( ( uint32_t ) TIM_GetCounter( TIM2 ) ) >= ( ulCompleteTickPeriods * ulReloadValueForOneTick ) );

			/* The reload value is set to whatever fraction of a single tick
			period remains. */
			ulCounterValue = ( ( uint32_t ) TIM_GetCounter( TIM2 ) ) - ( ulCompleteTickPeriods * ulReloadValueForOneTick );
			configASSERT( ulCounterValue <= ( uint32_t ) USHRT_MAX );
			if( ulCounterValue == 0 )
			{
				/* There is no fraction remaining. */
				ulCounterValue = ulReloadValueForOneTick;
				ulCompleteTickPeriods++;
			}
			TIM_SetAutoreload( TIM2, ( uint16_t ) ulCounterValue );
			TIM_SetCounter( TIM2, 0 );
		}

		/* Restart TIM2 so it runs up to the reload value.  The reload value
		will get set to the value required to generate exactly one tick period
		the next time the TIM2 interrupt executes. */
		TIM_Cmd( TIM2, ENABLE );

		/* Wind the tick forward by the number of tick periods that the CPU
		remained in a low power state. */
		vTaskStepTick( ulCompleteTickPeriods );
	}
}

#endif /* configCREATE_LOW_POWER_DEMO == 1 */

