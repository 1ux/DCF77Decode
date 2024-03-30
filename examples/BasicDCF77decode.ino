/*
Library overview:

This library implements the minimum functional scope of DCF77 decoding and was deliberately kept small/minimalistic. 
This range of functions supports the decoding of: Minute, hour, day, month and year.
There is no error detection or other DCF77 refinements. With a stable signal, you should have no problems.




*/
#include "src/basic_dcf77.h"
#include "src/DebugProject.h"

uint8_t bitArray[DCF77_STRING_SIZE]; //for DCF77 receiving
TimeStampDCF77 time;

#define DEBUG_SERIAL

void setup() 
{
  // put your setup code here, to run once:
	#ifdef DEBUG_SERIAL
		Serial.begin(115200);
		Serial.println("Debug Mode on\n");
	#endif
  

}

void loop() 
{
  // put your main code here, to run repeatedly:
	receiveDCF77(bitArray,DCF77_STRING_SIZE);
	decodeDCF77(bitArray,DCF77_STRING_SIZE,&time);
}
