#include "Addio/Embedded/Time/Timing/System_Timer/system_timer.h"
#include "Addio/Embedded/Time/Timing/timing.h"
#include "check_mcu_core.h"

/*
*	Note:	If you are using the system timer for something else, you are still able to use the timing functions.
*			Just make sure you increment "system_timer_overflows" in your event handler,
*			and adjust "SYSTEM_TIMER_COUNTER_TOP," to your "SysTick->LOAD" value.
*			That is all that is required to get "millis" and "micros" function working.
*/

#if __has_include("RTE_Components.h") && __has_include("core_cm7.h") && IS_MCU_CORE_CM7
#include <RTE_Components.h> //Contains ATMEL_START definition
#if defined(ATMEL_START)

/*
*	Include IC header
*/
//SAME70
#if defined(__SAME70Q21__)
#include <same70q21.h>
#elif defined(__SAME70Q20__)
#include <same70q20.h>
#elif defined(__SAME70Q19__)
#include <same70q19.h>
#elif defined(__SAME70N21__)
#include <same70n21.h>
#elif defined(__SAME70N20__)
#include <same70n20.h>
#elif defined(__SAME70N19__)
#include <same70n19.h>
#elif defined(__SAME70J21__)
#include <same70j21.h>
#elif defined(__SAME70J20__)
#include <same70j20.h>
#elif defined(__SAME70J19__)
#include <same70j19.h>

//SAMS70
#elif defined(__SAMS70Q21__)
#include <sams70q21.h>
#elif defined(__SAMS70Q20__)
#include <sams70q20.h>
#elif defined(__SAMS70Q19__)
#include <sams70q19.h>
#elif defined(__SAMS70N21__)
#include <sams70n21.h>
#elif defined(__SAMS70N20__)
#include <sams70n20.h>
#elif defined(__SAMS70N19__)
#include <sams70n19.h>
#elif defined(__SAMS70J21__)
#include <sams70j21.h>
#elif defined(__SAMS70J20__)
#include <sams70j20.h>
#elif defined(__SAMS70J19__)
#include <sams70j19.h>

//SAMV70
#elif defined(__SAMV70Q21__)
#include <samv70q21.h>
#elif defined(__SAMV70Q20__)
#include <samv70q20.h>
#elif defined(__SAMV70Q19__)
#include <samv70q19.h>
#elif defined(__SAMV70N21__)
#include <samv70n21.h>
#elif defined(__SAMV70N20__)
#include <samv70n20.h>
#elif defined(__SAMV70N19__)
#include <samv70n19.h>
#elif defined(__SAMV70J21__)
#include <samv70j21.h>
#elif defined(__SAMV70J20__)
#include <samv70j20.h>
#elif defined(__SAMV70J19__)
#include <samv70j19.h>

//SAMV71
#elif defined(__SAMV71Q21__)
#include <samv71q21.h>
#elif defined(__SAMV71Q20__)
#include <samv71q20.h>
#elif defined(__SAMV71Q19__)
#include <samv71q19.h>
#elif defined(__SAMV71N21__)
#include <samv71n21.h>
#elif defined(__SAMV71N20__)
#include <samv71n20.h>
#elif defined(__SAMV71N19__)
#include <samv71n19.h>
#elif defined(__SAMV71J21__)
#include <samv71j21.h>
#elif defined(__SAMV71J20__)
#include <samv71j20.h>
#elif defined(__SAMV71J19__)
#include <samv71j19.h>

#else
#error unsupported
#endif

#include <core_cm7.h> //Requires IC header.
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