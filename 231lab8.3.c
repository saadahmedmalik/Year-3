// Saad Malik, ECE 231 Lab 8.3 //

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

#define TrigPin PC0
#define EchoPin PC1

#define  RANGECONST 1.098 // Appropriately defining variable names to include numerical values to be used later 
#define staticDisplay 500
#define displayDelay 5

int rfDistance(void); // Function prototype for for distance measurement 
void timer(void); // Function prototype for timer configuration

int main(void){
    int count, range;
    unsigned char D1, D2, D3, D4; 
    unsigned char Digits[] = {0x3F, 0x86, 0x5B, 0x4F, 0x66, 0x6D, 0x07, 0x7F,0x6F};
    DDRD = 0xFF; DDRB = 0xFF; PORTB = 0xFF;
    while(1){
        range = rfDistance(); // Measure distance using ultrasonic sensor 
        D4 = range%10;  // Extracting individual digits from the range 
        D3 =(range/10)%10; 
        D2=(range/100)%10; 
        D1 = (range/1000);
        for(count=0; count<staticDisplay/displayDelay; count++){
            PORTD = Digits[D4]; PORTB = ~(1<<1); _delay_ms(displayDelay); // Displaying digits on the 4-digit 7-segment display
            PORTD = Digits[D3]; PORTB = ~(1<<2); _delay_ms(displayDelay);
            PORTD = Digits[D2]; PORTB = ~(1<<3); _delay_ms(displayDelay);
            PORTD = Digits[D1]; PORTB = ~(1<<4); _delay_ms(displayDelay);
        }
    }
}

void timer(void){
    TCCR0A=0; // Timer0 control register A to 0
    TCCR0B=5; // Timer0 control register B for prescaler of 1024 
    TCNT0=0; // Reset Timer0 counter 
}

int rfDistance(void){
    unsigned char rec, fec, ewc; int range;
    DDRC = 1<<TrigPin; // Set TrigPin as output
    PORTC &= (1<<TrigPin); // Set TrigPin low
    timer();

    while(1){
        TCNT0=0;
        PORTC |= (1<<TrigPin); // Send trigger pulse 
        _delay_us(20); // Delay for 20 microseconds 
        PORTC &= ~(1<<TrigPin); // Clear trigger pulse 
        while((PINC &(1<<EchoPin))==0);
        rec = TCNT0;
        while(!(PINC & (1<<EchoPin))==0);
        fec = TCNT0;
        if(fec>rec){ // Calculate value based on time difference 
            ewc = fec - rec;
            range = ewc * RANGECONST;    
            if(range>200) // Ensuring value is within a reasonable range 
                range = 200;
            if(range<5)
                range = 5;
            return range;} // Return calculated value 
        _delay_ms(500); // Appropriate delay to prevent rapid unwanted measurements 
    }
}
