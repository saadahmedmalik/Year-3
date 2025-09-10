/*
Saad Malik
*/

// Import Header Files
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

// Define Pins
#define TrigPin PC0
#define EchoPin PC1

// Define Constants
#define  RANGECONST 1.098
#define staticDisplay 500
#define displayDelay 5

//Function Prototypes
int rfDistance(void);
void timer(void);

int main(void){
    int count, range;
    unsigned char D1, D2, D3, D4; 
    unsigned char Digits[] = {0x3F, 0x86, 0x5B, 0x4F, 0x66, 0x6D, 0x07, 0x7F,0x6F};
    DDRD = 0xFF; DDRB = 0xFF; PORTB = 0xFF;
    while(1){
        range = rfDistance();
        D4 = range%10; D3 =(range/10)%10; D2=(range/100)%10; D1 = (range/1000);
        for(count=0; count<staticDisplay/displayDelay; count++){
            PORTD = Digits[D4]; PORTB = ~(1<<1); _delay_ms(displayDelay);
            PORTD = Digits[D3]; PORTB = ~(1<<2); _delay_ms(displayDelay);
            PORTD = Digits[D2]; PORTB = ~(1<<3); _delay_ms(displayDelay);
            PORTD = Digits[D1]; PORTB = ~(1<<4); _delay_ms(displayDelay);
        }


    }

}

void timer(void){
    TCCR0A=0; 
    TCCR0B=5; 
    TCNT0=0;
}

int rfDistance(void){
    unsigned char rec, fec, ewc; int range;
    DDRC = 1<<TrigPin; PORTC &= (1<<TrigPin);
    timer();

    while(1){
        TCNT0=0;
        PORTC |= (1<<TrigPin);
        _delay_us(20);
        PORTC &= ~(1<<TrigPin);
        while((PINC &(1<<EchoPin))==0);
        rec = TCNT0;
        while(!(PINC & (1<<EchoPin))==0);
        fec = TCNT0;
        if(fec>rec){
            ewc = fec - rec;
            range = ewc * RANGECONST;    
            if(range>200)
                range = 200;
            if(range<5)
                range = 5;
            return range;}
        _delay_ms(500);
        

    }
}

// End of file