#include "mbed.h"
#include "AGabriel_binaryutils.hpp"

//LED Declarations
#define DIR (uint32_t*)0x50000514
#define OUT (uint32_t*)0x50000504

#define LED_GREEN_PIN (uint8_t)16

volatile int why = 0;

//Initialization
Ticker cream_cheese;
Thread foo1;

//Global Variable Incrementing
void bagel()
{
    why++;
}

void foo()
{
    while(1)
    {
        //If 3 seconds has passed
        if(why >= 3)
        {
            //Turn Led on
            clearbit(OUT, LED_GREEN_PIN); 
            thread_sleep_for(100);

            //Turn LED off
            setbit(OUT, LED_GREEN_PIN);   

            //Reset back to 0 seconds
            why = 0;
        }

        thread_sleep_for(1);
    }
}

int main()
{
    //Initialize LED
    setbit(DIR, LED_GREEN_PIN);
    setbit(OUT, LED_GREEN_PIN);

    //Start Thread
    foo1.start(foo);

    //Attach thread to ticker
    cream_cheese.attach(&bagel, 1.0);

    //Empty spinning main loop
    while(true)
    {
        thread_sleep_for(10000);
    }
}