//Part 3
#include "mbed.h"
#include "USBSerial.h"
#include "AGabriel_binaryutils.hpp"

#define HUMIDITY (1 << 0) //bit 0
#define TEMPERATURE (1 << 1) //bit 1
#define TICK 500000 //.5 sec
#define LED_WAIT_TIME 500 //.5 second

//Define all registers
#define DIR      (uint32_t*)0x50000514
#define GPIO_OUT (uint32_t*)0x50000504

// LED Declaration
#define LED_GREEN_PIN (uint8_t)16 //16
#define LED_BLUE_PIN  (uint8_t)6 //6

#define HS3003_ADDR (uint8_t)(0x44 << 1)
#define I2C_PULLUP_PIN 0

#define DIR1      (uint32_t*)0x50000814
#define GPIO_OUT1 (uint32_t*)0x50000804
#define ENTER_PROGRAMMING 0xA0
#define EXIT_PROGRAMMING  0x80
#define WHOAMI_UPPER 0x1E
#define WHOAMI_LOWER 0x1F
#define SENSOR_VDD_PIN 22

USBSerial serial;

I2C i2c(P0_14, P0_15);

Ticker eventTicker;
Thread Read_Humidity;
Thread Read_Temperature;

EventFlags flags;

Mutex serialMutex;
Mutex i2cMutex;

int counting = 0;
uint16_t rawHum = 0;
uint16_t rawTemp = 0;

float hum = 0.0f;
float temp = 0.0f;

char data[4];

//The sensor needs to be turned on within 10ms of the program starting
DigitalOut sensorVDD(P0_22, 0);

//Need ISR for event flags
void eventISR(){
    if (counting == 0){
    flags.set(HUMIDITY); //cue humidity thread
    counting++;
    } else {
    flags.set(TEMPERATURE);//cue temperature thread
    counting = 0;
    }
}

void readingData()
{
    //write to i2c
    int write_result = i2c.write(HS3003_ADDR, NULL, 0);

    //print if cannot write
    if (write_result != 0) {
        serialMutex.lock();
        serial.printf("Measurement request FAILED\r\n");
        serialMutex.unlock();
        return;
    }

    // takes a second to actually read data
    thread_sleep_for(40);

    // Read 4 bytes of measurement data
    int read_result = i2c.read(HS3003_ADDR, data, 4);

    //if you cannot read the result, print read failed
    if (read_result != 0) {
        serialMutex.lock();
        serial.printf("Measurement read FAILED\r\n");
        serialMutex.unlock();
        return;
    }

    // Humidity - bits [13:0]
    rawHum = ((((uint16_t)(uint8_t)data[0]) << 8) |(uint8_t)data[1]) & 0x3FFF; //mask using 0x3FFF

    // Temperature - bits [15:2]
    uint16_t tempData = (((uint16_t)(uint8_t)data[2]) << 8) |(uint8_t)data[3]; //mask and shift

    rawTemp = (tempData >> 2) & 0x3FFF; //mask

    //equations
    hum = ((float)rawHum / 16383.0f) * 100.0f; //humidity (from sensor datasheet)

    temp = ((float)rawTemp / 16383.0f) * 165.0f - 40.0f; //temperature (from sensor datasheet)
}


//humidity thread
void read_humidity()
{
    while (1) {
        //when flag is set come here
        flags.wait_any(HUMIDITY);

        //read data
        i2cMutex.lock();
        readingData();
        i2cMutex.unlock();

        serialMutex.lock();
        //collect humidity
        int humWhole = (int)hum;
        int humDecimal = (int)((hum - humWhole) * 100);

        //print humidity
        serial.printf("Humidity = %d.%02d %%\r\n",humWhole,humDecimal);

        serialMutex.unlock();

        //LED still goes
        clearbit(GPIO_OUT, LED_GREEN_PIN);

        thread_sleep_for(LED_WAIT_TIME);

        setbit(GPIO_OUT, LED_GREEN_PIN);

        flags.clear(HUMIDITY);
    }
}

//Temperature thread (same as humidity but diff variables)
void read_temperature()
{
    while (1) {
        //when flag is set come here
        flags.wait_any(TEMPERATURE);
        //read data
        i2cMutex.lock();
        readingData();
        i2cMutex.unlock();

        serialMutex.lock();
        //collect data
        int tempWhole = (int)temp;
        int tempDecimal = (int)((temp - tempWhole) * 100);

        //make positive decimal
        if(tempDecimal<0){
            tempDecimal= tempDecimal*-1;
        }

        //print temperature
        serial.printf("Temperature = %d.%02d C\r\n",tempWhole,tempDecimal);

        serialMutex.unlock();

        //still do LEDs
        clearbit(GPIO_OUT, LED_BLUE_PIN);

        thread_sleep_for(LED_WAIT_TIME);

        setbit(GPIO_OUT, LED_BLUE_PIN);

        flags.clear(TEMPERATURE);
    }
}

//WhoamI function
void whoAmI()
{
    //need to enter programming mode
    char command[3] = {ENTER_PROGRAMMING,0x00,0x00};

    char reg;
    char upper[2] = {0, 0};
    char lower[2] = {0, 0};

    // Make sure sensor is OFF (power)
    sensorVDD = 0;

    serial.printf("Sensor OFF\r\n");
    thread_sleep_for(100);

    // Turn sensor ON
    sensorVDD = 1;

    serial.printf("Sensor ON\r\n");

    // Programming command must be sent within 10 ms
    int write_result = i2c.write(HS3003_ADDR, command, 3);

    serial.printf("Programming Write = %d\r\n", write_result);

    if (write_result != 0) {
        serial.printf("Programming mode FAILED\r\n");
        return;
    }

    serial.printf("Programming mode SUCCESS\r\n");

    wait_us(120);

    // Upper 16 bits
    reg = WHOAMI_UPPER;

    int upper_write = i2c.write(HS3003_ADDR, &reg, 1);
    int upper_read  = i2c.read(HS3003_ADDR, upper, 2);

    serial.printf("Upper W=%d R=%d\r\n",upper_write,upper_read);

    // Lower 16 bits
    reg = WHOAMI_LOWER;

    int lower_write = i2c.write(HS3003_ADDR, &reg, 1);
    int lower_read  = i2c.read(HS3003_ADDR, lower, 2);

    serial.printf("Lower W=%d R=%d\r\n",lower_write,lower_read);

    serial.printf("Upper Bytes: %02X %02X\r\n",(uint8_t)upper[0],(uint8_t)upper[1]);

    serial.printf("Lower Bytes: %02X %02X\r\n",(uint8_t)lower[0],(uint8_t)lower[1]);

    // Exit programming mode
    command[0] = EXIT_PROGRAMMING;

    //do test result
    int normal_result = i2c.write(HS3003_ADDR,command,3);

    serial.printf("Normal Mode Write = %d\r\n",normal_result);
}


// main() runs in its own thread in the OS
int main()
{
    // Configure LEDs
    setbit(DIR, LED_GREEN_PIN);
    setbit(DIR, LED_BLUE_PIN);

    setbit(GPIO_OUT, LED_GREEN_PIN);
    setbit(GPIO_OUT, LED_BLUE_PIN);

    // Configure I2C pull-up
    setbit(DIR1, I2C_PULLUP_PIN);
    setbit(GPIO_OUT1, I2C_PULLUP_PIN);

    // call whoami func
    whoAmI();

    // Start humidity and temperature threads
    Read_Humidity.start(read_humidity);
    Read_Temperature.start(read_temperature);

    // Alternate between humidity and temperature (flags)
    eventTicker.attach_us(&eventISR, TICK);

    while (true) {
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