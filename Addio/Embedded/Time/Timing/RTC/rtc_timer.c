#include "rtc_timer.h"

#include <hpl_rtc_base.h>



struct timer_task rtc_timer_task;

/*
*	How many times the system timer has overflowed.
*/
volatile uint64_t rtc_overflows = 0;

void rtc_timer_cb(const struct timer_task *const timer_task);

void rtc_timer_init()
{
	rtc_timer_task.cb = rtc_timer_cb;
	rtc_timer_task.interval = RTC_MODE0_COUNT_MASK;
	rtc_timer_task.mode = TIMER_TASK_REPEAT;
	timer_add_task()
}


void rtc_timer_cb(const struct timer_task *const timer_task)
{
	rtc_overflows ++;
}