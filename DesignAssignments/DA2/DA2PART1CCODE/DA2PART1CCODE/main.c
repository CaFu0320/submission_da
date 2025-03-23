/*
 * main.c
 *
 * Created: 3/8/2025 3:15:20 PM
 *  Author: Carlos Funes
 */ 
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>

void delay_150ms(void) {
	volatile uint16_t i = 0;
	volatile uint16_t j = 0;
	while(i < 660) {       //outer loop
		j = 0;
		while(j < 200) {   //inner loop
			j++;
		}
		i++;
	}
} //this loop creates 660*200 = 132k iterations

int main(void) {            // main function entry point
	DDRC &= ~(1 << 1); //set pc1 for input
	PORTC |= (1 << 1); //enable pull-up resistor for pc1
	DDRB |= (1 << 5); //set pb5 for output
	PORTB &= ~(1 << 5); //make sure led on pb5 is initially off
	while(1) {            // start infinite loop
		if (!(PINC & (1 << 1))) {  // check if button at pc1 is pressed (active low)
			PORTB |= (1 << 5);     // turn on led on pb5
			uint8_t count = 10;    // we want a total delay of 1.5 seconds (10 x 150 ms)
			while(count--) {       // loop 10 times
				delay_150ms();     // delay occurs here (~150 ms delay)
			}
			PORTB &= ~(1 << 5);    // turn off led on pb5 after delay
			} else {
			PORTB &= ~(1 << 5);    // ensure led remains off when button is not pressed
		}
	}
	return 0;               // end of main function
}
