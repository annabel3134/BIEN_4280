#include "mbed.h"
#include "USBSerial.h"
#include "AGabriel_binaryutils.hpp"
#include "Mail.h"
#include "PwmOut.h"
#include "nrf_pwm.h"
#include "hal/pwmout_api.h"

#define DIR (uint32_t*)0x50000514 //DIR (for setup)
#define OUT (uint32_t*)0x50000504 //Out pin

#define Register (uint8_t)13 //Register 13 for for P0.13

#define LED_RED_PIN (uint8_t)24
#define LED_GREEN_PIN (uint8_t)16
#define LED_BLUE_PIN (uint8_t)8

//Information Being Sent
typedef struct{

    float duty_cycle;
} message_t;

int period = 2;

//prepare memory pool
MemoryPool<message_t, 9> poolV;
MemoryPool<message_t, 9> poolC;
MemoryPool<message_t, 9> poolS;

//prepare queue
Queue<message_t, 9> queueV;
Queue<message_t, 9> queueC;
Queue<message_t, 9> queueS;

//Initialize all threads
Thread ice_cream_man_Thread;
Thread vanilla_Thread;
Thread chocolate_Thread;
Thread strawberry_Thread;

//Producer Thread
void ice_cream_man(){
    uint32_t i = 0;
    while (1){
        i++;

        message_t *messageV = poolV.alloc();
        message_t *messageC = poolC.alloc();
        message_t *messageS = poolS.alloc();

        //data setting to diff values
        messageV->duty_cycle = 100;
        messageC->duty_cycle = 250;
        messageS->duty_cycle = 1000;

        //put info on queues
        queueV.put(messageV); 
        queueC.put(messageC);
        queueS.put(messageS);

        thread_sleep_for(100);
    }
}


//Consumer Thread
//Named Vanilla - will rapidly flash green light. 
//should rely on a queue for percentage PWM
void vanilla(){
    
    while(1){
    osEvent evt = queueV.get();

    if(evt.status == osEventMessage)
    {
            //receive message
            message_t* messageV = (message_t*)evt.value.p;

            float dutyV = messageV->duty_cycle;

            while(1){
            //Flash at set rate
            setbit(OUT, LED_GREEN_PIN);
            //setbit(OUT, Register);
            thread_sleep_for(dutyV);
            
            clearbit(OUT, LED_GREEN_PIN);
            //clearbit(OUT, Register);
            thread_sleep_for(dutyV);
            }
            poolV.free(messageV);
        }
    }   
}

//use pwm out class to do the same as vanilla (diff speed)
void chocolate(){
    //receive message
            PwmOut led(LED_BLUE_PIN);
            LED_BLUE_PIN.period(period);

            while(1){

                osEvent evt = queueC.get();

                if(evt.status == osEventMessage)
                {

                message_t* messageC = (message_t*)evt.value.p;

                float dutyC_Percent = (messageC->duty_cycle)/1000;
    

                led.write(dutyC_Percent);

                poolC.free(messageC);


                }

            }
   


}

//use HAL to do the same
void strawberry(){
    pwmout_t pwm;
    pwmout_init(&pwm, LED_RED_PIN);

    while(1){
    osEvent evt = queueV.get();

    if(evt.status == osEventMessage)
    {
            //receive message
            message_t* messageS = (message_t*)evt.value.p;

            float dutyS_Percent = (messageS->duty_cycle)/1000;
            pwmout_write(&pwm, dutyS_Percent);
            pwmout_period_ms(&pwm, period);

            pwmout_free(&pwm);
        }
    }

}

// main() runs in its own thread in the OS
int main()
{
    ice_cream_man_Thread.start(ice_cream_man);
    vanilla_Thread.start(vanilla);
    chocolate_Thread.start(chocolate);
    strawberry_Thread.start(strawberry);

    while (true) {
        thread_sleep_for(2000);
    }
}

