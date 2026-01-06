// tinytemp.c code for ECE304
// Saad Ahmed Malik and Divyesh Biswas
// Code for a temperature sensor using ATtiny85 and SSD1306 OLED and TMP36
// Outputs temperature values in Fahrenheit and Celsius with indicator for too hot
// Most of this code is based off of megaTemp.c and megaTempSleep.c from the professor's GitHub
// The rest of the code was based off of our 231 lab from last year

// including libraries and tinyOlLED.h
#include "tinyOLED.h"
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#define NUMSAMPLES 25   // #ADC Samples to average
#define VREF 1.1        // ADC reference voltage
#define MAXTEMP 70      // Too-Hot warning set at this temp (Deg F)

// defining functions
void WDT_OFF(void);
void OLED_command(uint8_t cmd);
void adc_init(void);
unsigned int get_adc(void);

int main(void) {

    WDT_OFF(); // Make sure the WDT is off at startup

    // initiaslizng variables
    unsigned int digitalValue;
    unsigned long int totalValue = 0;
    float rawTempC, rawTempF;
    int boundedTempC10, boundedTempF10;
    char buffer[6];

    // initializing ADC and OLED from megatempsleep.c

    adc_init();
    OLED_init();

    // dimming the oled for reduced current draw
    OLED_command(0x81);   // setting contrast
    OLED_command(0x10);   // contrast value

    // average ADC samples
    for (int i = 0; i < NUMSAMPLES; i++)
        totalValue += get_adc();
    digitalValue = totalValue / NUMSAMPLES;

    // calculating temp
    rawTempC = digitalValue * VREF / 10.24 - 50.0;
    rawTempF = rawTempC * 1.8 + 32.0;

    // setting bounds for Celsius
    boundedTempC10 = (int)(
        (rawTempC < 0.0) ? 0.0 :
        (rawTempC > 45.0) ? 45.0 :
        rawTempC * 10.0);
    
    // setting bounds for Fahrenheit

    boundedTempF10 = (int)(
        (rawTempF < 32.0) ? 32.0 :
        (rawTempF > 113.0) ? 113.0 :
        rawTempF * 10.0);

    // clearnig OLED before we display
    OLED_clear();

    // displays too hot if tempF exceeds MAXTEMP
    if (rawTempF > MAXTEMP) {
        OLED_cursor(20, 0);
        const char *warn = "TOO HOT!";
        for (unsigned char i = 0; i < strlen(warn); i++) {
            OLED_printC(warn[i]);
        }
    }

    // dispaly code for farenheit
    OLED_cursor(60, 2);
    itoa(boundedTempF10 / 10, buffer, 10);
    for (uint8_t *p = (uint8_t*)buffer; *p; p++)
        OLED_printC(*p);
    OLED_printC('.');
    itoa(boundedTempF10 % 10, buffer, 10);
    OLED_printC(buffer[0]);
    OLED_cursor(85, 2);
    OLED_printC('F');

    // display code for celsius
    OLED_cursor(20, 3);
    itoa(boundedTempC10 / 10, buffer, 10);
    for (uint8_t *p = (uint8_t*)buffer; *p; p++)
        OLED_printC(*p);
    OLED_printC('.');
    itoa(boundedTempC10 % 10, buffer, 10);
    OLED_printC(buffer[0]);
    OLED_cursor(45, 3);
    OLED_printC('C');

    // Set up watchdog timer for 8 seconds and enter sleep mode
    wdt_enable(WDTO_8S);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_mode();   // puts the MCU to sleep

    return 0;
}

// from megaTempsleep.c which is not using avr.io

// Disable WTD and clear reset flag immediately at startup
void WDT_OFF()
{
    MCUSR &= ~(1 << WDRF);
    WDTCR = (1 << WDCE) | (1 << WDE);
    WDTCR = 0x00;
}

// sends command bytes to the OLED for brightness control
void OLED_command(uint8_t cmd) 
{
    I2C_start(OLED_ADDR);
    I2C_write(OLED_CMD_MODE);
    I2C_write(cmd);
    I2C_stop();
}

// adc initialization for attiny
void adc_init(void)
{
    DDRB &= ~(1 << 3);  // configures pin3 as input
    ADCSRA = 0x83;  // sets up adc control and status register
    ADMUX = 0x83;   // configs multuiplexer selection register
}

// from megaTemp.c
// Read ADC value from ADC3
unsigned int get_adc(void)
{
    ADCSRA |= (1 << ADSC);
    while ((ADCSRA & (1 << ADIF)) == 0);
    ADCSRA |= (1 << ADIF);
    return ADCL | (ADCH << 8);
}
