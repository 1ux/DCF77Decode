#ifndef DEBUG_PROJECT_H
#define DEBUG_PROJECT_H

	#include <Arduino.h>

	#define SUCCESS 0
	#define ERROR_INVALID_VALUE -1
	#define ERROR_TIMEOUT -2

	//DEBUG_SERIAL activates the debug output via the serial interface. Please note that the RAM usage increases. 
	#define DEBUG_SERIAL 

	//#define DEBUG_WATCHDOG //If you need a watchdog, use this option.
	#ifdef DEBUG_WATCHDOG
	  	#include <WDT.h>
	#endif

#endif