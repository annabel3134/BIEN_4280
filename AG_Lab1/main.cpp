/* Name: Annabel Gabriel
   Date: 9/15/2026
   Description: Main Function - Blinks Light at Set Speeds and Coorosponding different colors while stating the status
*/

#include "mbed.h"
#include "USBSerial.h"
#include "AGabriel_binaryutils.hpp"
#include "Mail.h"

#define DIR (uint32_t*)0x50000514 //DIR (for setup)
#define OUT (uint32_t*)0x50000504 //Out pin

#define Register (uint8_t)13 //Register 13 for for P0.13

#define LED_RED_PIN (uint8_t)24
#define LED_GREEN_PIN (uint8_t)16
//yellow is red and green at the same time

Thread ledThread;
Thread testerThread;

enum Blink_Speed{NO_ERROR = 1000, //100% duty cycle
ATTN_REQ = 250, //25% duty cycle
FATAL_ERROR = 100}; //10% duty cycle

enum States{
    STATE_NO_ERROR, 
    STATE_ATTN_REQ,
    STATE_FATAL_ERROR
};

typedef struct{
    States state;
}mail_t;

Mail<mail_t, 16> mail_box;

// Set up colored LEDs
//int delay = NO_ERROR;
States Current_State = STATE_NO_ERROR;

// main() runs in its own thread in the OS
USBSerial serial;

// Week 2 Code
//LED DIAG HANDLER FUNCTION/THREAD
void led_diag_handler(){

    while(1){
    //receive mail
    osEvent event = mail_box.get(0);
    if (event.status == osEventMail) {
    mail_t *mail = (mail_t *)event.value.p;

    //what is current state?
    Current_State = mail->state;
    mail_box.free(mail);

    switch(Current_State){
        case STATE_FATAL_ERROR:
        serial.printf("State: Fatal Error - 100 ms blink rate\r\n");
        break;

        case STATE_ATTN_REQ:
        serial.printf("State: Attention Required - 250 ms blink rate\r\n");
        break;

        case STATE_NO_ERROR:
        serial.printf("State: No Error - 1000 ms blink rate\r\n");
        break;

    }

    }
    //if mail is received that led should switch change led state + color
    switch(Current_State){
        case STATE_NO_ERROR:
            //switch color to green
            setbit(OUT, LED_RED_PIN);

            //Flash at set rate
            setbit(OUT, LED_GREEN_PIN);
            //setbit(OUT, Register);
            thread_sleep_for(NO_ERROR);
            
            clearbit(OUT, LED_GREEN_PIN);
            //clearbit(OUT, Register);
            thread_sleep_for(NO_ERROR);
            break;

        case STATE_ATTN_REQ:
            //switch color to yellow
            //red and green at same time?
            setbit(OUT, LED_GREEN_PIN);

            //FLASH COLORS  
            setbit(OUT, LED_GREEN_PIN);
            setbit(OUT, LED_RED_PIN);
            //setbit(OUT, Register);
            thread_sleep_for(ATTN_REQ);

            clearbit(OUT, LED_GREEN_PIN);
            clearbit(OUT, LED_RED_PIN);
            //clearbit(OUT, Register);
            thread_sleep_for(ATTN_REQ);
            break;

        case STATE_FATAL_ERROR:
            //switch color to red
            setbit(OUT, LED_GREEN_PIN);
            setbit(OUT, LED_RED_PIN);

            //flash
            setbit(OUT, LED_RED_PIN);
            //setbit(OUT, Register);
            thread_sleep_for(FATAL_ERROR);

            clearbit(OUT, LED_RED_PIN);
            //clearbit(OUT, Register);
            thread_sleep_for(FATAL_ERROR);
            break;
        }
    }
}

// DIAG_TESTER function/thread
void diag_tester(){
    States state = STATE_NO_ERROR;

    while(1){
        mail_t *msg = mail_box.alloc();

        if (msg != NULL) {

            msg->state = state;
            mail_box.put(msg);
    }
    //change to next state
        state = (States)(state + 1);
        if (state > STATE_FATAL_ERROR){
            state = STATE_NO_ERROR;
        }
    thread_sleep_for(5000);

    }
}

// Main function
int main(){
//configure leds as outs
    setbit(DIR, LED_RED_PIN);
    setbit(DIR, LED_GREEN_PIN);
    setbit(DIR, Register);
    //setbit(DIR, LED_YELLOW_PIN);

//initialize state varibles to zero/default state

    clearbit(OUT, LED_RED_PIN);
    clearbit(OUT, LED_GREEN_PIN);
    //clearbit(OUT, LED_YELLOW_PIN);

    ledThread.start(led_diag_handler);
    testerThread.start(diag_tester);

    serial.printf("Intitialization Done\r\n");

while(1){
    //Always blink - but change colors (using clearbit and setbit)
    //If cases, print the states to tabby
    serial.printf("Empty Main Loop\r\n");
    thread_sleep_for(5000);

    }
}









// Week 1 Code
/*
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
    }*/