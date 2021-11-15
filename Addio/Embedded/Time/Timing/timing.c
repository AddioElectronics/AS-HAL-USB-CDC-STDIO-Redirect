#include "timing.h"

//millis and micros are located in another file.


/*
*	Checks to see if an amount of milliseconds have passed between now and the start_time.
*
*	/param	ms			The amount of milliseconds which is considered a time out.
*	/param	start_ms	The initial value of the timer (milliseconds).
*
*	/returns True if a timeout has occurred. False if not enough time has elapsed.
*/
bool has_time_elapsed_ms(unsigned long ms, unsigned long start_ms)
{
	unsigned long time = millis();
	if(time - start_ms >= ms)
	{
		return true;
	}

	return false;
}

/*
*	Checks to see if an amount of microseconds have passed between now and the start_time.
*
*	/param	us			The amount of microseconds which is considered a time out.
*	/param	start_us	The initial value of the timer. (microseconds).
*
*	/returns True if a timeout has occurred. False if not enough time has elapsed.
*/
bool has_time_elapsed_us(unsigned long us, unsigned long start_us)
{
	unsigned long time = micros();
	if(time - start_us >= us)
	{
		return true;
	}

	return false;
}

/*
*	Checks to see if an amount of seconds have passed between now and the start_time.
*
*	/param	sec			The amount of seconds which is considered a time out.
*	/param	start_time	The initial value of the timer.
*
*	/returns True if a timeout has occurred. False if not enough time has elapsed.
*/
bool __attribute__((__always_inline__)) has_time_elapsed_sec(unsigned long sec, unsigned long start_ms)
{
	return has_time_elapsed_ms(sec * 1000, start_ms);
}

/*
*	Checks to see if an amount of seconds have passed between now and the start_time.
*
*	/param	min			The amount of seconds which is considered a time out.
*	/param	start_time	The initial value of the timer.
*
*	/returns True if a timeout has occurred. False if not enough time has elapsed.
*/
bool __attribute__((__always_inline__)) has_time_elapsed_min(unsigned long min, unsigned long start_ms)
{
	return has_time_elapsed_ms(min * 1000 * 60, start_ms);
}

///*
//*	Checks to see if an amount of seconds have passed between now and the start_time.
//*
//*	/param	sec			The amount of seconds which is considered a time out.
//*	/param	start_time	The initial value of the timer.
//*
//*	/returns True if a timeout has occurred. False if not enough time has elapsed.
//*/
//bool has_time_elapsed_sec(unsigned long sec, unsigned long start_time)
//{
	//unsigned long time = seconds();
	//if(time - start_time >= sec)
	//{
		//return true;
	//}
//
	//return false;
//}
//
///*
//*	Checks to see if an amount of seconds have passed between now and the start_time.
//*
//*	/param	min			The amount of seconds which is considered a time out.
//*	/param	start_time	The initial value of the timer.
//*
//*	/returns True if a timeout has occurred. False if not enough time has elapsed.
//*/
//bool has_time_elapsed_min(unsigned long min, unsigned long start_time)
//{
	//unsigned long time = minutes();
	//if(time - start_time >= min)
	//{
		//return true;
	//}
//
	//return false;
//}

#warning reminder to come re-comment this, its 5am and my english is becoming shit.