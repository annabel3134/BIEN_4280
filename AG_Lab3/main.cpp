//Part 3
#include "mbed.h"
#include "USBSerial.h"
#include "AGabriel_binaryutils.hpp"

#define HUMIDITY (1 << 0) //bit 0
#define TEMPERATURE (1 << 1) //bit 1
#define TICK 1000000 //100000 us
#define LED_WAIT_TIME 1000 //second

//Define all registers
#define DIRSET (uint32_t*) 0x50000514  // Set GPIO direction (Input/Output)
#define OUTSET (uint32_t*) 0x50000508  // Set LED Pin (Turn ON)
#define GPIO_OUT (uint32_t*)0x50000504

// LED Declaration
#define LED_GREEN_PIN (uint8_t)16 //16
#define LED_BLUE_PIN  (uint8_t)6 //6

#define I2C_SCL (uint8_t)15
#define I2C_SDA (uint8_t)14

//I2C i2c(I2C_SDA, I2C_SCL);
I2C i2c(P14, P15);

const int addr7bit = 0x48; // 7-bit I2C address
const int writeAddr8Bit = 0x48 << 1; // 8-bit I2C address, 0x90
const int readAddr8Bit = writeAddr8Bit | 1; // 8-bit I2C address, 0x91

#define HS3003_ADDR (0x44 << 1)

#define denominator 1U << 14 //2^14

USBSerial serial;

Ticker eventTicker;
Thread Read_Humidity;
Thread Read_Temperature;

EventFlags flags;

Mutex serialMutex;
int counting = 0;
float hum = 0;
float temp = 0;

//Need ISR for event flags
void eventISR(){
    if (counting == 0){
    flags.set(HUMIDITY);
    counting++;
    } else {
    flags.set(TEMPERATURE);
    counting = 0;
    }
}

void read_humidity(){   

    while(1){
    flags.wait_any(HUMIDITY);

    

    serialMutex.lock();
    serial.printf("Humidity = %f\r\n", hum);
    serialMutex.unlock();

    clearbit(GPIO_OUT, LED_GREEN_PIN);

    thread_sleep_for(LED_WAIT_TIME);

    setbit(GPIO_OUT, LED_GREEN_PIN);

    flags.clear(HUMIDITY);
    }   
}

void read_temperature(){
    while(1){
    flags.wait_any(TEMPERATURE);

    serialMutex.lock();
    serial.printf("Reading Temperature\r\n");
    serialMutex.unlock();

    clearbit(GPIO_OUT, LED_BLUE_PIN);

    thread_sleep_for(LED_WAIT_TIME);

    setbit(GPIO_OUT, LED_BLUE_PIN);

    flags.clear(TEMPERATURE);
    }   
}

// main() runs in its own thread in the OS
int main()
{
    //set_i2c_pullup();

    char who_am_i_addr[] = {0x1E};  // Register to read ID
    char who_am_i_data[2];

        // Set pin direction
    setbit(DIRSET, LED_GREEN_PIN);
    setbit(DIRSET, LED_BLUE_PIN);
    //setbit(DIRSET, LED_RED_PIN);

    // Turn led off
    setbit(GPIO_OUT, LED_GREEN_PIN);
    setbit(GPIO_OUT, LED_BLUE_PIN);
    //setbit(GPIO_OUT, LED_RED_PIN);

    //Turn on Threads + Ticker
    Read_Humidity.start(read_humidity);
    Read_Temperature.start(read_temperature);
    eventTicker.attach_us(&eventISR, TICK);

    i2c.frequency(100000);

    *DIRSET |= (1 << 0);
    *OUTSET |= (1 << 0);

    char data[4];

    while (true) {

        i2c.write(HS3003_ADDR, data, 1);
        i2c.read(HS3003_ADDR, data, 2);

        uint32_t maskHum = 0x3FFF; //bits 1-13
        uint32_t maskTemp = ((1u << 16) - 1) & ~((1u << 2) - 1); //bits 2-15

        uint32_t rawHum = ((uint32_t)(data[0] & maskHum) << 8) | data[1];
        uint32_t rawTemp = ((uint32_t)((data[0] & maskTemp)>> 2) << 8) | data[1];

        float hum = ((rawHum)/(denominator - 1)) * 100; //calculate humidity
        float temp = ((rawTemp)/(denominator - 1)) * (165 - 40); //calculate temperature

        thread_sleep_for(1000);
    }
}





//Part 2 complete
/*#include "mbed.h"
#include "USBSerial.h"
#include "AGabriel_binaryutils.hpp"

#define HUMIDITY (1 << 0) //bit 0
#define TEMPERATURE (1 << 1) //bit 1
#define TICK 1000000 //100000 us
#define LED_WAIT_TIME 1000 //second

//Define all registers
#define DIR      (uint32_t*)0x50000514
#define GPIO_OUT (uint32_t*)0x50000504

// LED Declaration
#define LED_RED_PIN   (uint8_t)24 //24
#define LED_GREEN_PIN (uint8_t)16 //16
#define LED_BLUE_PIN  (uint8_t)6 //6

USBSerial serial;

Ticker eventTicker;
Thread Read_Humidity;
Thread Read_Temperature;

EventFlags flags;

Mutex serialMutex;
int counting = 0;

//Need ISR for event flags
void eventISR(){
    if (counting == 0){
    flags.set(HUMIDITY);
    counting++;
    } else {
    flags.set(TEMPERATURE);
    counting = 0;
    }
}

void read_humidity(){
    while(1){
    flags.wait_any(HUMIDITY);

    serialMutex.lock();
    serial.printf("Reading Humidity\r\n");
    serialMutex.unlock();

    clearbit(GPIO_OUT, LED_GREEN_PIN);

    thread_sleep_for(LED_WAIT_TIME);

    setbit(GPIO_OUT, LED_GREEN_PIN);

    flags.clear(HUMIDITY);
    }   
}

void read_temperature(){
    while(1){
    flags.wait_any(TEMPERATURE);

    serialMutex.lock();
    serial.printf("Reading Temperature\r\n");
    serialMutex.unlock();

    clearbit(GPIO_OUT, LED_BLUE_PIN);

    thread_sleep_for(LED_WAIT_TIME);

    setbit(GPIO_OUT, LED_BLUE_PIN);

    flags.clear(TEMPERATURE);
    }   
}

// main() runs in its own thread in the OS
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

    //Turn on Threads + Ticker
    Read_Humidity.start(read_humidity);
    Read_Temperature.start(read_temperature);
    eventTicker.attach_us(&eventISR, TICK);

    while (true) {
        thread_sleep_for(1000);
    }
}

*/ 