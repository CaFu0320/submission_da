/*
 * main.c
 *
 * Created: 3/20/2025 3:15:20 PM
 *  Author: Carlos Funes
 */
#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void delay_150ms(void) {
	volatile uint16_t i = 0;
	volatile uint16_t j = 0;
	while(i < 660) {         // outer loop remains at 660 iterations
		j = 0;
		while(j < 200) {    // inner loop: 200 iterations
			j++;
		}
		i++;
	}
}

ISR(INT0_vect) {
	// Interrupt-based handling: turn on LED and delay for 3 sec.
	PORTB |= (1 << 5);            // Turn on LED at PB5
	for(int k = 0; k < 10; k++){
		delay_150ms();
	}                // Delay 3 seconds (blocking delay)
	PORTB &= ~(1 << 5);           // Turn off LED
}

int main(void) {
	// Configure LED output
	DDRB |= (1 << 5);             // Set PB5 as output
	PORTB &= ~(1 << 5);           // Make sure LED is initially off

	// Configure switch on INT0 (PD2)
	DDRD &= ~(1 << 2);            // Set PD2 as input
	PORTD |= (1 << 2);            // Enable internal pull-up on PD2

	// Configure INT0 to trigger on a rising edge (active high)
	EICRA |= (1 << ISC01) | (1 << ISC00);  // Rising edge trigger for INT0
	EIMSK |= (1 << INT0);                  // Enable external interrupt INT0

	sei();                          // Enable global interrupts

	while(1) {
		// this is just to keep the microcontroller running
	}
	return 0;
}

