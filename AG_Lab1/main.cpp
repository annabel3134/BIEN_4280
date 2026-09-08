/* Name: Annabel Gabriel
   Date: 9/1/2026
   Description: Main Function - Demonstrates Binary Utility Functions
*/

#include "mbed.h"
#include "USBSerial.h"
#include "AGabriel_binaryutils.hpp"

#define LED1 LED_BUILTIN
#define delay 1000 //1 HZ = 1 sec delay
#define Register_Address (uint32_t*)0x50000504 //P0
#define DIR (uint32_t*)0x50000514
#define Register (uint8_t)13

// main() runs in its own thread in the OS
USBSerial serial;

int main(){

    while(1){

    setbit(DIR, Register);

    setbit(Register_Address, Register);
    thread_sleep_for(delay);

    clearbit(Register_Address, Register);
    thread_sleep_for(delay);


    }
}