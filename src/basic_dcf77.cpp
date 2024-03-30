#include "basic_dcf77.h"
#include "DebugProject.h"

uint8_t DCF77_PIN=13;

//This function makes the port pin assignment. If this function is not called, the default port is 13.
void setupDCF77(uint8_t pin)
{
	pinMode(pin, INPUT);
	DCF77_PIN=pin;
}

//Receives the demodulated DCF77 signals and writes them into an int array with the size DCF77_STRING_SIZE, which represents the bits of one minute.
int receiveDCF77(uint8_t* bitArray, uint8_t size) 
{
    unsigned long volatile highDuration;
    bool minuteMarkerDetected = false;
    uint8_t i = 0;

    // Check if the array size matches the expected DCF77 string size
    if (size != DCF77_STRING_SIZE) 
    {
        #ifdef DEBUG_SERIAL
            Serial.println("The passed bitArray has the wrong size. Note the DCF77_STRING_SIZE declaration.");
        #endif
        return ERROR_INVALID_VALUE;
    }
    #ifdef DEBUG_SERIAL
        Serial.println("\n\n --=Debug Output Mode=--\n");
    #endif

    // Wait until the started minute is over
    do 
    {
        // pulseIn() returns 0 when TIMEOUT_DURATION is reached
        highDuration = pulseIn(DCF77_PIN, HIGH, TIMEOUT_DURATION);
        minuteMarkerDetected = (highDuration == 0);
        #ifdef DEBUG_SERIAL
            Serial.println("Wait for new minute...");
        #endif
    } while (!minuteMarkerDetected);

    // Receive and decode the DCF77 signal for one minute
    for (i = 0; i < DCF77_STRING_SIZE; i++) 
    {
        // Measure the duration of the HIGH pulse
        highDuration = pulseIn(DCF77_PIN, HIGH, TIMEOUT_DURATION);
        
        // Check for DCF77 signal loss
        if (highDuration == 0) 
        {
            #ifdef DEBUG_SERIAL
                Serial.println("\nunexpected DCF77 connection timeout");
            #endif
            return ERROR_TIMEOUT;
        }

        // Decode the received bit based on the pulse duration
        if (highDuration > BIT_0_DURATION && highDuration <= BIT_1_DURATION) 
        {
            bitArray[i] = 1;
        } 
        else if (highDuration <= BIT_0_DURATION && highDuration > min_BIT_0_DURATION) 
        {
            bitArray[i] = 0;
        }
        
        #ifdef DEBUG_SERIAL
            // Print the decoded bit for debugging purposes
            Serial.print(bitArray[i]);
            //Serial.println(highDuration);
        #endif
    }
    
    // signal was successfully received and decoded
    return SUCCESS;
}

//This function interprets a DCF77 bit array range via a weighting into a decimal number. It is an internal function that is only used by decodeDCF77().
static int BitScaleDCF77(uint8_t *bitstring, uint8_t length) 
{
    static const int weights[] = {1, 2, 4, 8, 10, 20, 40, 80};
    int value = 0;
    static const int weights_len = 8;
    for (int i = 0; i < length && i < weights_len; i++) 
    {
        value += weights[i] * bitstring[i];
    }
    return value;
}

static int checkParity(uint8_t *bitArray)
{
    //DCF77 uses even parity
    uint8_t minuteParity = 0;
    uint8_t hourParity = 0;
    uint8_t dateParity = 0;

    // Calculate parity for minute
    for (uint8_t i = 21; i < 28; ++i)
    {
        minuteParity ^= bitArray[i];
    }

    // Calculate parity for hour
    for (uint8_t i = 29; i < 35; ++i)
    {
        hourParity ^= bitArray[i];
    }
    // Calculate parity for date
    for (uint8_t i = 36; i < 58; ++i)
    {
        dateParity ^= bitArray[i];
    }

    // Check the parity bits for minutes and hours
    if ((minuteParity != bitArray[28]) || (hourParity != bitArray[35])|| (dateParity != bitArray[58]))
    {
        return ERROR_INVALID_VALUE; // Parity error
    }

    return SUCCESS;
}

//Extracts and interprets the date and time from the binary DCF77 string and writes them into a TimeStampDCF77 structure.
int decodeDCF77(uint8_t *bitArray, uint8_t size, TimeStampDCF77 *time) 
{
    if (size != DCF77_STRING_SIZE) 
    {
        #ifdef DEBUG_SERIAL
            Serial.println("The passed bitArray has the wrong size. Note the DCF77_STRING_SIZE declaration.");
        #endif
        return ERROR_INVALID_VALUE; // The size of the array is invalid
    }

    // Decode the bit strings according to the DCF77 specification
    time->hour = BitScaleDCF77(bitArray + 29, 6);
    time->minute = BitScaleDCF77(bitArray + 21, 7);
    time->day = BitScaleDCF77(bitArray + 36, 6);
    time->weekday = BitScaleDCF77(bitArray + 42, 3);
    time->month = BitScaleDCF77(bitArray + 45, 5);
    time->year = BitScaleDCF77(bitArray + 50, 8);
    time->transmitter_fault = BitScaleDCF77(bitArray + 15, 1);
    time->A1 = BitScaleDCF77(bitArray + 16, 1);

    if(checkParity(bitArray)==ERROR_INVALID_VALUE)
    {
        #ifdef DEBUG_SERIAL
            Serial.println("\nParity error in hour or minute.");
        #endif
        return ERROR_INVALID_VALUE;
    }

    // Check if day, month, or year have invalid (00) values
    if (time->day == 0 || time->month == 0 || time->year == 0) 
    {
        #ifdef DEBUG_SERIAL
            Serial.println("\nInvalid date received.");
        #endif
        return ERROR_INVALID_VALUE; // The date is not plausible
    }

    #ifdef DEBUG_SERIAL
        Serial.print("\nUhrzeit: ");
        Serial.print(time->hour);
        Serial.print(":");
        Serial.println(time->minute);
        Serial.print("Datum: ");
        Serial.print(time->day);
        Serial.print(".");
        Serial.print(time->month);
        Serial.print(".20");
        Serial.println(time->year);
    #endif 

    return SUCCESS;
}
