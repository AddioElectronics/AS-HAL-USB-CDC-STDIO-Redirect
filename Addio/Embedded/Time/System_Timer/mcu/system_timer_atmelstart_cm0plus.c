#include "../system_timer.h"
#include "../../Timing/timing.h"

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


/*
*	How many times the system timer has overflowed.
*/
volatile uint64_t overflows = 0;


void system_timer_init()
{
	SysTick->LOAD  = (uint32_t)(SysTick_LOAD_RELOAD_Msk);             /* set reload register */
	NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); /* set Priority for Systick Interrupt */
	SysTick->VAL   = (uint32_t)(SysTick_LOAD_RELOAD_Msk);             /* Load the SysTick Counter Value */
	SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
	SysTick_CTRL_TICKINT_Msk   |
	SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick Int and SysTick Timer */
}


void SysTick_Handler()
{
	overflows++;
}

#pragma region Timing Functions


uint64_t millis()
{
	//Timer counts down, we must subtract
	return ((SysTick_LOAD_RELOAD_Msk - SysTick->VAL) + (SysTick_LOAD_RELOAD_Msk * overflows) ) / MS_TICKS;
}

uint64_t micros()
{
	return ((SysTick_LOAD_RELOAD_Msk - SysTick->VAL) + (SysTick_LOAD_RELOAD_Msk * overflows) ) / US_TICKS;
}

//uint64_t seconds()
//{
	////Timer counts down, we must subtract
	//return ((SysTick_LOAD_RELOAD_Msk - SysTick->VAL) + (SysTick_LOAD_RELOAD_Msk * overflows) ) / SEC_TICKS;
//}
//
//uint64_t minutes()
//{
	////Timer counts down, we must subtract
	//return ((SysTick_LOAD_RELOAD_Msk - SysTick->VAL) + (SysTick_LOAD_RELOAD_Msk * overflows) ) / MIN_TICKS;
//}


#pragma endregion Timing Functions

#endif //defined(ATMEL_START)
#endif //__has_include("RTE_Components.h") && __has_include("core_cm0plus.h")