#define F_CPU 8000000UL // clock frequency = 8 MHz
#include <xc.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// delay subroutine of 0.15 sec, based off the 8MHz oscillating frequency
void delay_subroutine(void){
	for (volatile uint8_t i = 0; i < 255; i++) {            // 130050 empty instructions each executed at 0.125us
		for (volatile uint8_t j = 0; j < 255; j++) {        // will result in the loop ending after around 150ms = 0.15s
			for (volatile uint8_t h = 0; h < 2; h++) {      // 255 x 255 x 2 = 130050
			}
		}
	}
}

ISR (INT0_vect) {                    // begin Interrupt Service Routine for INT0
	PORTB |= (1 << 5);               // turn on LED connected to PB5
	for (int t = 0; t < 10; t++) {   // LED stays on 3 seconds
		delay_subroutine();               // calling delay subroutine 20 times (0.15 x 20 = 3)
	}
}

int main(void)
{
	DDRB |= (1 << 5);   // set PORTB.5 as an output pin for the LED
	PORTB &= ~(1 << 5); // set PB5 initially to zero (LED initially off)
	DDRC &= ~(1 << 1);  // set PORTC.1 as an input pin
	PORTC |= (1 << 1);  // enable pull-up resistor for pushbutton/switch
	DDRD &= ~(1 << 2);  // make PD2 an input (for INTO interrupt)
	PORTD |= (1 << 2);  // enable pull-up resistor for PD2 (for pushbutton/switch)

	EICRA |= (1 << ISC01) | (1 << ISC00); // configure INT0 to trigger on a rising edge
	EIMSK |= (1 << INT0); // enable INT0 interrupt
	sei(); // set the global interrupt enable bit in SREG

	while(1) // LED at PB5 stays off during normal operation
	{
		if (!(PINC & (1 << 1))) {       // if pushbutton connected to PC1 is pressed
			PORTB |= (1 << 5);          // turn on LED connected to PB5
			for (int l = 0; l < 10; l++){   // LED stays on for 1.5 seconds
				delay_subroutine();              // calling delay subroutine 10 times (0.15 x 10 = 1.5)
			}
			PORTB &= ~(1 << 5);    // LED turns back off
			} else {                   // else
			PORTB &= ~(1 << 5);    // LED stays off
		}
	}
}