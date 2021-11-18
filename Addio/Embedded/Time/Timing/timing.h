#ifndef TIMING_H_
#define TIMING_H_

#include <stdint.h>
#include <stdbool.h>

/*
*	Gets the current amount of milliseconds that have elapsed since program start.
*/
uint64_t millis();

/*
*	Gets the current amount of microseconds that have elapsed since program start.
*/
uint64_t micros();

///*
//*	Gets the current amount of seconds that have elapsed since program start.
//*/
//uint64_t seconds();
//
///*
//*	Gets the current amount of minutes that have elapsed since program start.
//*/
//uint64_t minutes();


bool has_time_elapsed_ms(unsigned long ms, unsigned long start_ms);
bool has_time_elapsed_us(unsigned long us, unsigned long start_us);
bool has_time_elapsed_sec(unsigned long sec, unsigned long start_ms);
bool has_time_elapsed_min(unsigned long min, unsigned long start_ms);

#endif /* TIMING_H_ */