/*
Library overview:

This Arduino library implements all bits publicly specified by the „Physikalisch-Technische Bundesanstalt“ 
that are within a 60 second bit stream of the time signal, with the exception of the leap second. 
It also checks for transmission errors via even parity and queries the 15 bit ("call bit") to rule out problems with the transmitter. 
The focus is on clean code, excellent documentation and ease of use of this library. 
The blocking pulseIn() function, which is part of the standard Arduino library, 
is used for this. There are no other dependencies required.
A DCF77 receiver module is required. 
This module demodulates the AM long wave signal from Germany 
and generates 59 square wave pulses with two different duty cycles.

Serial Output:
It is now XX:XX o'clock CET
Today is XX.XX.20XX
Weekday: XX
*/

#include "src/basic_dcf77.h"
#include "src/DebugProject.h"


uint8_t bitArray[DCF77_STRING_SIZE]; //Memory location for received DCF77 bit string
TimeStampDCF77 time;  //Data type for decoded DCF77 string
int ReceiveDCF77;     //DCF77 receiving status

char buffer[40];      //A cache for a pretty and formatted text output

void setup()
{
  //If necessary, activate the debug output for DCF77 in DebugProject.h.
	Serial.begin(115200);
  delay(7000);    //Depending on hardware, it may take some time until the DCF77 module is ready
  setupDCF77(12); //set MCU digital input Pin 12 for DCF77
}


void loop() 
{
	ReceiveDCF77 = receiveDCF77(bitArray,DCF77_STRING_SIZE); //Start receiving a DCF77 string
  
	if(ReceiveDCF77==SUCCESS)
  {
    if(decodeDCF77(bitArray,DCF77_STRING_SIZE,&time)==SUCCESS)
    {
      snprintf(buffer, sizeof(buffer), "It is now %02d:%02d o'clock %s", time.hour, time.minute, 
      (time.CEST) ? "CEST" : "CET");
      Serial.println(buffer);
      snprintf(buffer, sizeof(buffer), "Today is %02d.%02d.20%02d",time.day, time.month, time.year);
      Serial.println(buffer);
      snprintf(buffer, sizeof(buffer), "Weekday: %02d\n", time.weekday);
      Serial.println(buffer);
      if(time.transmitter_fault!=SUCCESS)
        Serial.println("Either their signal is very noisy, or something is wrong in Germany.");
      else if(time.A1)
        Serial.println("CET/CEST Time change is coming up.");
    }
    else Serial.println("No doubt, signal unstable, please readjust antenna.");
  }
  else if(ReceiveDCF77==ERROR_TIMEOUT)
    Serial.println("\nDCF77 signal unstable, please wait or readjust antenna.");
}
