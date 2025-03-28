/*
 * DA3allofthecodescombined.c
 *
 * Created: 3/26/2025 8:21:33 PM
 * Author : Carlos Funes
 */ 
#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint16_t timer3_counter = 0; //counter for Timer3 interrupts
volatile uint16_t timer4_counter = 0; //counter for Timer4 interrupts

int main(void) {
	//configuring Timer0 for 0.125ms delay
	DDRB |= (1 << PB5); //PB5 as output
	TCCR0A = 0; //normal mode
	TCNT0 = 0x83; //initial value for 125us overflow
	TCCR0B |= (1 << CS01); //prescaler 8
	uint16_t timer0_counter = 0; //software counter for overflows

	//configuring Timer3 for 0.25ms interrupts
	DDRB |= (1 << PB4); //PB4 as output
	TCCR3A = 0; //normal port 
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30); //CTC mode, prescaler 64
	OCR3A = 30; //compare match value for ~0.248ms
	TIMSK3 |= (1 << OCIE3A); //enable compare match interrupt

	//configuring Timer4 for 0.1ms interrupts (Normal mode, PB3 LED)
	DDRB |= (1 << PB3); //PB3 as output
	TCNT4 = 0xFF9C; //preload for 100us overflow (65436)
	TCCR4A = 0; //normal mode
	TCCR4B = (1 << CS41); //prescaler 8
	TIMSK4 |= (1 << TOIE4); //enable overflow interrupt

	sei(); //global interrupts

	while (1) {
		//polling Timer0 overflow flag
		if (TIFR0 & (1 << TOV0)) { //checking overflow flag
			TIFR0 = (1 << TOV0); //clearing flag
			timer0_counter++;
			if (timer0_counter >= 12000) {
				PORTB ^= (1 << PB5); //toggling PB5 every 1.5s
				timer0_counter = 0; //reseting counter
			}
		}
	}
}

//Timer3 toggling PB4 every 2 seconds
ISR(TIMER3_COMPA_vect) {
	timer3_counter++;
	if (timer3_counter >= 16128) { //2 seconds on and off
		PORTB ^= (1 << PB4); //toggling PB4
		timer3_counter = 0; //reseting counter
	}
}

//Timer4 toggling PB3 every 1 second
ISR(TIMER4_OVF_vect) {
	TCNT4 = 0xFF9C; //reloading preload value
	timer4_counter++;
	if (timer4_counter >= 20000) { //1 second on and off
		PORTB ^= (1 << PB3); //toggling PB3
		timer4_counter = 0; //reseting counter
	}
}