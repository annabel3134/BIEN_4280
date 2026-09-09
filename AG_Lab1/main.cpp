/* Name: Annabel Gabriel
   Date: 9/8/2026
   Description: Main Function - Blinks Light at Set Speeds
*/

#include "mbed.h"
#include "USBSerial.h"
#include "AGabriel_binaryutils.hpp"

#define Register_Address (uint32_t*)0x50000504 //Out pin
#define DIR (uint32_t*)0x50000514 //DIR (for setup)
#define Register (uint8_t)13 //Register 13 for for P0.13
enum Blink_Speed{NO_ERROR = 1000, //100% duty cycle
ATTN_REQ = 250, //75% duty cycle
FATAL_ERROR = 100}; //50% duty cycle



// main() runs in its own thread in the OS
USBSerial serial;

int main(){

//initialize state varibles to zero/default state
int state = 0;
int delay = NO_ERROR;

    while (true) {
        serial.printf("State = %d\r\n", state);
        switch (state) {
            case 0:
                delay = NO_ERROR; //1000 ms
                break;

            case 1:
                delay = ATTN_REQ; //500 ms
                break;

            case 2:
                delay = FATAL_ERROR; //100 ms
                break;
        }

        // Blink for 2 seconds in current state
        for (int i = 0; i < 2000 / (2 * delay); i++) {

            setbit(Register_Address, Register);
            thread_sleep_for(delay);

            clearbit(Register_Address, Register);
            thread_sleep_for(delay);
        }

        // Switch state for cases above
        state += 1;
        if (state > 2){

            state = 0;
        }
        }
    }