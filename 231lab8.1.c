// Saad Malik, ECE 231, Lab Assignment 8.1: Blinking LEDs with differeing states of operation //

#include <avr/io.h>
#include <util/delay.h>

int main(void){
    DDRD = 1<<DDD6|1<<DDD7;                       // Set D6, D7 as output
    PORTD = 1<<PORTD3|1<<PORTD4|1<<PORTD5;        // Set pull-up on D3 & D4
                   
    while(1){

        if((!(PIND & (1<<PORTD3)) && !(PIND & (1<<PORTD4)))){
            PORTD &= ~(1<<DDD6);
            PORTD &= ~(1<<DDD7);
        }
        else if(!(PIND & (1<<PORTD5))){
            PORTD &= ~(1<<DDD6);
            _delay_ms(200);
            PORTD |= (1<<DDD6);
            PORTD &= ~(1<<DDD7);
            _delay_ms(200);
            PORTD |= (1<<DDD7);
        }
        else {
            if (PIND & (1<<PORTD3)){
                PORTD &= ~(1<<DDD6);
            }
            else{
                PORTD &= ~(1<<DDD6);
                _delay_ms(200);
                PORTD |= (1<<DDD6);
                _delay_ms(200);
            }
            if (PIND & (1<<PORTD4)){
                PORTD &= ~(1<<DDD7);
            }
            else{
                PORTD &= ~(1<<DDD7);
                _delay_ms(200);
                PORTD |= (1<<DDD7);
                _delay_ms(200);
            }
        }    
    }
    return(0);
}

