/*
 * DA2PART3CCODE.c
 *
 * Created: 3/20/2025 4:13:42 PM
 * Author : Carlos Funes
 */ 
//Question 3)
#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void delay_150ms(void) {
	volatile uint16_t i = 0;
	volatile uint16_t j = 0;
	while(i < 660) {         //outer loop remains at 660 iterations
		j = 0;
		while(j < 200) {    //inner loop: 200 * 20 = 4000 iterations
			j++;
		}
		i++;
	}
}

ISR(INT0_vect) {
	//interrupt-based handling: turn on LED and delay for 3 sec.
	PORTB |= (1 << 5);            //turn on LED at PB5
	for(int k = 0; k < 10; k++ ){
		delay_150ms();
	}
	                //delay 3 seconds (blocking delay)
	PORTB &= ~(1 << 5);           //turn off LED
}

int main(void) {
	DDRB |= (1 << 5);             //set PB5 as output
	PORTB &= ~(1 << 5);           //make sure LED is initially off
	
	DDRC &= ~(1 << 1); //set pc1 for input
	PORTC |= (1 << 1); //enable pull-up resistor for pc1

	DDRD &= ~(1 << 2);            //set PD2 as input
	PORTD |= (1 << 2);            //enable internal pull-up on PD2

	
	EICRA |= (1 << ISC01) | (1 << ISC00);  //rising edge trigger for INT0
	EIMSK |= (1 << INT0);                  //enable external interrupt INT0

	sei();                          //enable global interrupts

	while(1) {            //start infinite loop
		if (!(PINC & (1 << 1))) {  //check if button at pc1 is pressed (active low)
			PORTB |= (1 << 5);     //turn on led on pb5
			uint8_t count = 10;    //we want a total delay of 1.5 seconds (10 x 150 ms)
			while(count--) {       //loop 10 times
				delay_150ms();     //delay occurs here (~150 ms delay)
			}
			PORTB &= ~(1 << 5);    //turn off led on pb5 after delay
			} else {
			PORTB &= ~(1 << 5);    //ensure led remains off when button is not pressed
		}
	}

	return 0;
}
