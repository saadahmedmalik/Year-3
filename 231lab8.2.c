// Saad Ahmed Malik, ECE 231, Lab 8.2 //

#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include "i2c.h"
#include "SSD1306.h"
#include "my_adc_lib.h"
#include "my_uart_lib.h"
#include <math.h>

void floattostring(char* strArray, double num); // Function used primarily to convert float values to string values

double adcToC(unsigned int adc){ // Analog to Digital Converter with respect to Celcius temperature values
    double mv = (double)((adc/1024.0)*5);
    double C = (mv-0.5)*100;
    return C;
}

int main(void){

    char stringArray[10]; // Array to store temperature readings
    uart_init(); // Initialize UART 
    OLED_Init(); // Initialize OLED
    adc_init(); // Initialize Analog to Digital Converter
    float highTemp = 15.555556; // Threshold Temperature

    DDRD = 1<<PD3;
    PORTD = 1<<PD4;
    unsigned int digValue;

    while (1){

        unsigned char buttonPress = (PIND & (1<<PD4)); // Read Button State
        digValue = get_adc();
        double C = adcToC(digValue);
        double F = (C*(9.0/5))+32;

        if (C < highTemp){
            PORTD &= ~(1<<PD3); // Turn off LED if below threshold
        }
        else {
            PORTD |= (1<<PD3); // Turn on LED if above threshold
        }

        if(buttonPress){
            floattostring(stringArray, F);
            strcat(stringArray, " F"); 
            OLED_GoToLine(6); // Navigating between lines
            OLED_DisplayString(stringArray); // Display Temperature 
            send_string(stringArray);
            uart_send(13);
            uart_send(10);
        }
        else{
            floattostring(stringArray, C);
            strcat(stringArray, " C"); 
            OLED_GoToLine(6); // Navigating between lines
            OLED_DisplayString(stringArray); // Display Temperature 
            send_string(stringArray);
            uart_send(13);
            uart_send(10);
        }
        _delay_ms(1000); // Delay added of 1 second
    }
}

void floattostring(char* strArray, double num){ // Previously displayed/defined function now specified 

    int intComp = (int)num;
    double floatComp = num - (double)intComp;
    int displayDec = (int)(round(floatComp*100));
    sprintf(strArray, "%d.%d", intComp, displayDec);

}
