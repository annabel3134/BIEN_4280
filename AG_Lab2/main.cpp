#include "mbed.h"
#include "USBSerial.h"
#include "AGabriel_binaryutils.hpp"
#include "Mail.h"
#include "PwmOut.h"
#include "nrf_pwm.h"

#define PWM_TOP 20000
#define DIR (uint32_t*)0x50000514 //DIR (for setup)
#define GPIO_OUT (uint32_t*)0x50000504 //Out pin

#define Register (uint8_t)13 //Register 13 for for P0.13
//used 4 for testing (A0)
#define LED_RED_PIN (uint8_t)24//24
#define LED_GREEN_PIN (uint8_t)16 //16
#define LED_BLUE_PIN (uint8_t)6//6


nrf_pwm_values_individual_t seq_values;

//Information Being Sent
typedef struct{

    float duty_cycle;
} message_t;

int period = 10;//ms

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

/*
        //data setting to diff values - Percent
        messageV->duty_cycle = .10;
        messageC->duty_cycle = .25;
        messageS->duty_cycle = .50;

        //put info on queues
        queueV.put(messageV); 
        queueC.put(messageC);
        queueS.put(messageS);
*/
    if(messageV != NULL)
{
    messageV->duty_cycle = .33;
    queueV.put(messageV);
}
if(messageC != NULL)
{
    messageC->duty_cycle = .75;
    queueC.put(messageC);
}
if(messageS != NULL)
{
    messageS->duty_cycle = .5;
    queueS.put(messageS);
}

        thread_sleep_for(period*3);
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
            int time_on = period*dutyV;
            int time_off = period - period*dutyV;

            //glow at set rate
            clearbit(GPIO_OUT, LED_GREEN_PIN);
            //clearbit(OUT, Register);
            thread_sleep_for(time_on);

            setbit(GPIO_OUT, LED_GREEN_PIN);
            //setbit(OUT, Register);
            thread_sleep_for(time_off);
            poolV.free(messageV);
            
            
            }
            
        }
    }   


//use pwm out class to do the same as vanilla (diff speed)
void chocolate(){
    //receive message
            PwmOut LED_BLUE{PinName(LED_BLUE_PIN)};

            LED_BLUE.period_ms(period);

            while(1){

                osEvent evt = queueC.get();

                if(evt.status == osEventMessage)
                {

                message_t* messageC = (message_t*)evt.value.p;

                float dutyC_Percent = (messageC->duty_cycle);
    

                LED_BLUE.write(1-dutyC_Percent);

                poolC.free(messageC);


                }

            }
   


}

//use HAL to do the same
void strawberry()
{
    NRF_PWM0->PRESCALER = PWM_PRESCALER_PRESCALER_DIV_8;

    NRF_PWM0->COUNTERTOP = PWM_TOP;

    NRF_PWM0->MODE = NRF_PWM_MODE_UP;

    NRF_PWM0->DECODER = PWM_DECODER_LOAD_Individual;

    NRF_PWM0->SEQ[0].PTR = (uint32_t)&seq_values;
    NRF_PWM0->SEQ[0].CNT = 1;
    NRF_PWM0->SEQ[0].REFRESH = 0;
    NRF_PWM0->SEQ[0].ENDDELAY = 0;

    NRF_PWM0->PSEL.OUT[0] = LED_RED_PIN;

    nrf_pwm_enable(NRF_PWM0);

    while (1)
    {
        osEvent evt = queueS.get();

        if (evt.status == osEventMessage)
        {
            message_t *messageS = (message_t *)evt.value.p;

            float dutyS_Percent = messageS->duty_cycle;

            uint16_t duty =
                (uint16_t)(PWM_TOP * dutyS_Percent);

            seq_values.channel_0 = 0x8000 | duty;

            NRF_PWM0->TASKS_SEQSTART[0] = 1;

            poolS.free(messageS);
        }
    }
}

// main() runs in its own thread in the OS
int main()
{
    setbit(DIR, LED_GREEN_PIN);
    //setbit(DIR, LED_BLUE_PIN);
    setbit(DIR, LED_RED_PIN);

    ice_cream_man_Thread.start(ice_cream_man);
    vanilla_Thread.start(vanilla);
    chocolate_Thread.start(chocolate);
    strawberry_Thread.start(strawberry);

    while (true) {
        thread_sleep_for(2000);
    }
}

