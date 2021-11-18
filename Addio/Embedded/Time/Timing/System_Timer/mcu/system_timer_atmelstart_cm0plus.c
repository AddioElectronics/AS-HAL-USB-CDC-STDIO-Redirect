#include "../system_timer.h"
#include "../../timing.h"

/*
*	Note:	If you are using the system timer for something else, you are still able to use the timing functions.
*			Just make sure you increment "system_timer_overflows" in your event handler,
*			and adjust "SYSTEM_TIMER_COUNTER_TOP," to your "SysTick->LOAD" value.
*			That is all that is required to get "millis" and "micros" function working.
*/


#if __has_include("RTE_Components.h") && __has_include("core_cm0plus.h")
#include <RTE_Components.h> //Contains ATMEL_START definition
#if defined(ATMEL_START)

/*
*	Include IC header
*/
#if defined(__SAMD21J18A__)
#include <samd21j18a.h>
#elif defined(__SAMD21J17A__)
#include <samd21j17a.h>
#elif defined(__SAMD21J16A__)
#include <samd21j16a.h>
#elif defined(__SAMD21J15A__)
#include <samd21j15a.h>
#elif defined(__SAMD21G18A__)
#include <samd21g18a.h>
#elif defined(__SAMD21G17A__)
#include <samd21g17a.h>
#elif defined(__SAMD21G18AU__)
#include <samd21g18au.h>
#elif defined(__SAMD21G17AU__)
#include <samd21g17au.h>
#elif defined(__SAMD21G16A__)
#include <samd21g16a.h>
#elif defined(__SAMD21G15A__)
#include <samd21j15a.h>
#elif defined(__SAMD21E18A__)
#include <samd21e18a.h>
#elif defined(__SAMD21E17A__)
#include <samd21e17a.h>
#elif defined(__SAMD21E16A__)
#include <samd21e16a.h>
#elif defined(__SAMD21E15A__)
#include <samd21e15a.h>
#elif //Add your IC here.

#else
#error unsupported
#endif

#include <core_cm0plus.h> //Requires IC header.
#include <peripheral_clk_config.h>

/*
*	The top value of the counter.
*/
#define SYSTEM_TIMER_COUNTER_TOP	SysTick_LOAD_RELOAD_Msk

/*
*	How many ticks equal a unit of time.
*/
#define US_TICKS (CONF_CPU_FREQUENCY/1000000)
#define MS_TICKS (CONF_CPU_FREQUENCY/1000)
//#define SEC_TICKS (CONF_CPU_FREQUENCY)
//#define MIN_TICKS (CONF_CPU_FREQUENCY * 60)


/*
*	How many times the system timer has overflowed.
*/
volatile uint64_t system_timer_overflows = 0;


void system_timer_init()
{
	SysTick->LOAD  = (uint32_t)(SYSTEM_TIMER_COUNTER_TOP);             /* set reload register */
	NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);  /* set Priority for Systick Interrupt */
	SysTick->VAL   = (uint32_t)(SYSTEM_TIMER_COUNTER_TOP);             /* Load the SysTick Counter Value */
	SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
	SysTick_CTRL_TICKINT_Msk   |
	SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick Int and SysTick Timer */
}


//void __attribute__((weak)) SysTick_Handler();	//Remove GCC compiler warning
void /*__attribute__((weak))*/ SysTick_Handler()
{
	system_timer_overflows++;
}


#pragma region Timing Functions


uint64_t millis()
{
	//Timer counts down, we must subtract
	return ((SYSTEM_TIMER_COUNTER_TOP - SysTick->VAL) + (SYSTEM_TIMER_COUNTER_TOP * system_timer_overflows) ) / MS_TICKS;
}

uint64_t micros()
{
	return ((SYSTEM_TIMER_COUNTER_TOP - SysTick->VAL) + (SYSTEM_TIMER_COUNTER_TOP * system_timer_overflows) ) / US_TICKS;
}

//uint64_t seconds()
//{
	////Timer counts down, we must subtract
	//return ((SYSTEM_TIMER_COUNTER_TOP - SysTick->VAL) + (SYSTEM_TIMER_COUNTER_TOP * system_timer_overflows) ) / SEC_TICKS;
//}
//
//uint64_t minutes()
//{
	////Timer counts down, we must subtract
	//return ((SYSTEM_TIMER_COUNTER_TOP - SysTick->VAL) + (SYSTEM_TIMER_COUNTER_TOP * system_timer_overflows) ) / MIN_TICKS;
//}


#pragma endregion Timing Functions

#endif //defined(ATMEL_START)
#endif //__has_include("RTE_Components.h") && __has_include("core_cm0plus.h")