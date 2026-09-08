/* Name: Annabel Gabriel
   Date: 9/1/2026
   Description: Main Function - Demonstrates Binary Utility Functions
*/

#include "mbed.h"
#include "AGabriel_binaryutils.hpp"
#include "USBSerial.h"

// main() runs in its own thread in the OS
USBSerial serial;

int main(){

uint32_t solo = 0;

    while(1){
//serial.printf("Connected!\r\n"); //debug line

uint32_t mask1 = 0xFFF; //0-11th bits
uint32_t mask2 = 0xF0; //4th-7th bits 
char number[33]; //array for final binary output

setbit(&solo, 24); //sets 24th bit
setbit(&solo, 16); //sets 16th bit
setbit(&solo, 17); //sets 17th bit
setbits(&solo, mask1); //sets 0-11th bits
clearbit(&solo, 11); //clears bit 11
clearbits(&solo, mask2); //clears bits 4-7
serial.printf("Binary Solo: %s\r\n", display_binary(solo, number)); //prints binary representation
serial.printf("Decimal Solo: %u\r\n", solo); //prints decimal representation

ThisThread::sleep_for(1000ms); //sleeps instead of running very fast

}}


