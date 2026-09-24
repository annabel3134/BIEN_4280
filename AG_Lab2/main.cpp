
#include "mbed.h"
#include "USBSerial.h"
#include "AGabriel_binaryutils.hpp"
#include "nrf_pwm.h"

//Define all registers
#define DIR      (uint32_t*)0x50000514
#define GPIO_OUT (uint32_t*)0x50000504

// LED Declaration
#define LED_RED_PIN   (uint8_t)24
#define LED_GREEN_PIN (uint8_t)16
#define LED_BLUE_PIN  (uint8_t)6


// Struct for messages - for queue and mempool
struct ice_cream_message_t
{
    float duty;
};


//Declare mempool
MemoryPool<ice_cream_message_t, 32> iceCreamPool;

//Declare all queues - since diff duty diff queues
Queue<ice_cream_message_t, 9> vanillaQueue;
Queue<ice_cream_message_t, 9> chocolateQueue;
Queue<ice_cream_message_t, 9> strawberryQueue;


//Initialize Threads
Thread vanillaThread;
Thread chocolateThread;
Thread strawberryThread;
Thread iceCreamManThread;


// Ticker used for vanilla - faster than usual bitset and such (sleep)
Ticker vanillaTicker;

float vanillaDuty = 1.0f / 3.0f;

#define VANILLA_TICK 100 //100 micro seconds

//Function called by ticker
void vanillaISR()
{
    int counter = 0;

    int Ticks = (int)(vanillaDuty * 100.0f); //Calculating time on

    // Turn LED on and off by duty cycle
    if (counter < Ticks)
    {
        clearbit(GPIO_OUT, LED_GREEN_PIN);
    }
    else
    {
        setbit(GPIO_OUT, LED_GREEN_PIN);
    }

    counter++;

    if (counter >= 100)
    {
        counter = 0;
    }
}

//vanilla thread
void vanilla()
{
    //Initialize led
    setbit(DIR, LED_GREEN_PIN);
    setbit(GPIO_OUT, LED_GREEN_PIN);

    vanillaTicker.attach_us(&vanillaISR, VANILLA_TICK);

    while (true)
    {
        //get event from queue continuously
        osEvent evt = vanillaQueue.get();

        //if new message then change duty cycle
        if (evt.status == osEventMessage)
        {
            ice_cream_message_t *msg =
                (ice_cream_message_t *)evt.value.p;

            vanillaDuty = msg->duty;

            iceCreamPool.free(msg);
        }
    }
}


// Chocolate thread
void chocolate()
{
    //Initialize LED (diff way for this one)
    PwmOut blue(PinName(LED_BLUE_PIN));

    //set period
    blue.period_ms(10);

    while (true)
    {
        //get message from queue
        osEvent evt = chocolateQueue.get();
        //If new message then change duty cycle
        if (evt.status == osEventMessage)
        {
            ice_cream_message_t *msg =
                (ice_cream_message_t *)evt.value.p;

            float duty = msg->duty;

            // Turn led on for set time (active low)
            blue.write(1.0f - duty);

            iceCreamPool.free(msg);
        }
    }
}


//Strawberry Thread
void strawberry()
{
    // PWM sequence
    uint16_t sequence[1];

    sequence[0] = 500;
    //Initialize pins being used (LED)
    uint32_t pins[NRF_PWM_CHANNEL_COUNT] =
    {
        LED_RED_PIN,
        NRF_PWM_PIN_NOT_CONNECTED,
        NRF_PWM_PIN_NOT_CONNECTED,
        NRF_PWM_PIN_NOT_CONNECTED
    };
    //Set pins
    nrf_pwm_pins_set(NRF_PWM1, pins);
    //Configure pins
    nrf_pwm_configure(
        NRF_PWM1,
        NRF_PWM_CLK_1MHz,
        NRF_PWM_MODE_UP,
        1000
    );
    //decoder
    nrf_pwm_decoder_set(
        NRF_PWM1,
        NRF_PWM_LOAD_COMMON,
        NRF_PWM_STEP_AUTO
    );

    nrf_pwm_sequence_t const seq =
    {
        .values.p_common = sequence,
        .length          = NRF_PWM_VALUES_LENGTH(sequence),
        .repeats         = 0,
        .end_delay       = 0
    };
    //set sequence
    nrf_pwm_sequence_set(
        NRF_PWM1,
        0,
        &seq
    );
    //Auto loop
    NRF_PWM1->LOOP = 0;
    //Enable the PWM
    nrf_pwm_enable(NRF_PWM1);
    //Clear the event (PWM)
    nrf_pwm_event_clear(
        NRF_PWM1,
        NRF_PWM_EVENT_SEQEND0
    );
    //Re-Trigger event (PWM)
    nrf_pwm_task_trigger(
        NRF_PWM1,
        NRF_PWM_TASK_SEQSTART0
    );

    while (true)
    {
        //continuously get messages
        osEvent evt = strawberryQueue.get();
        //If new message, change duty cycle
        if (evt.status == osEventMessage)
        {
            ice_cream_message_t *msg =
                (ice_cream_message_t *)evt.value.p;

            // convert duty cycle to be usable
            sequence[0] = (uint16_t)(msg->duty * 1000.0f);

            iceCreamPool.free(msg);
        }

        // Restart the PWM when it finishes - auto loop was not working :(
        if (nrf_pwm_event_check(
                NRF_PWM1,
                NRF_PWM_EVENT_SEQEND0))
        {
            nrf_pwm_event_clear(
                NRF_PWM1,
                NRF_PWM_EVENT_SEQEND0
            );

            nrf_pwm_task_trigger(
                NRF_PWM1,
                NRF_PWM_TASK_SEQSTART0
            );
        }

        thread_sleep_for(1);
    }
}


//Producer - get it...he's the ice cream man!
void ice_cream_man()
{
    while (true)
    {
        //send vanilla message through queue (33% duty)
        ice_cream_message_t *vanillaMsg =
            iceCreamPool.alloc();

        if (vanillaMsg != nullptr)
        {
            vanillaMsg->duty = 1.0f / 3.0f;

            if (vanillaQueue.put(vanillaMsg) != osOK)
            {
                iceCreamPool.free(vanillaMsg);
            }
        }


        //Send chocolate message (75% duty)
        ice_cream_message_t *chocolateMsg =
            iceCreamPool.alloc();

        if (chocolateMsg != nullptr)
        {
            chocolateMsg->duty = 3.0f / 4.0f;

            if (chocolateQueue.put(chocolateMsg) != osOK)
            {
                iceCreamPool.free(chocolateMsg);
            }
        }


        //Send strawberry message (50% duty)
        ice_cream_message_t *strawberryMsg =
            iceCreamPool.alloc();

        if (strawberryMsg != nullptr)
        {
            strawberryMsg->duty = 1.0f / 2.0f;

            if (strawberryQueue.put(strawberryMsg) != osOK)
            {
                iceCreamPool.free(strawberryMsg);
            }
        }

        // Sleep for a min
        thread_sleep_for(100);
    }
}


int main()
{
    // Set pin direction
    setbit(DIR, LED_GREEN_PIN);
    setbit(DIR, LED_BLUE_PIN);
    setbit(DIR, LED_RED_PIN);

    // Turn led off
    setbit(GPIO_OUT, LED_GREEN_PIN);
    setbit(GPIO_OUT, LED_BLUE_PIN);
    setbit(GPIO_OUT, LED_RED_PIN);

    // Start threads
    vanillaThread.start(vanilla);
    chocolateThread.start(chocolate);
    strawberryThread.start(strawberry);

    // Start the man
    iceCreamManThread.start(ice_cream_man);

    while (true)
    {
        //spinning empty loop
        thread_sleep_for(1000);
    }
}

