#ifndef SYSTEM_TIMER_H_
#define SYSTEM_TIMER_H_

#include <stdint.h>


void system_timer_init();

/*
*	Gets the current amount of milliseconds that have elapsed since program start.
*/
uint64_t millis();

/*
*	Gets the current amount of microseconds that have elapsed since program start.
*/
uint64_t micros();

//void system_timer_enable();
//void system_timer_disable();





#endif /* SYSTEM_TIMER_H_ */