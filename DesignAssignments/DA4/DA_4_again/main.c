/*
 * DA4
 * Created: 3/19/2025 11:30:20 AM
 * Author : Carlos Funes
 */
#define F_CPU 16000000UL //defining CPU frequency
#include <avr/io.h>  //including AVR I/O register definitions 
#include <avr/interrupt.h> //including interrupt handling functions 
#include <stdio.h> //including standard I/O functions for snprintf

//UART configuration
void uart_init(unsigned int ubrr) {
	UBRR0H = (unsigned char)(ubrr >> 8); //setting high byte of UART baud rate
	UBRR0L = (unsigned char)ubrr; //setting low byte of UART baud rate
	UCSR0B = (1 << TXEN0); //turning on the UART transmitter
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); //8 data bits, 1 stop bit for communication
}

void uart_transmit(char data) { //character sending
	while (!(UCSR0A & (1 << UDRE0))); //wait for empty transmit buffer
	UDR0 = data; //putting data into buffer to send
}

void uart_print(const char* str) { //string sending
	while (*str) { //looping through each character
		uart_transmit(*str++); //sending each character
	}
}

//ADC configuration with timer auto trigger
void adc_init(void) {
	//configuring ADC
	ADMUX = (1 << REFS0); //reference voltage (AVcc)
	ADCSRB = (1 << ADTS1) | (1 << ADTS0); //timer0 Compare Match A trigger source
	ADCSRA = (1 << ADEN) | (1 << ADATE) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

	//enable ADC, auto-trigger, interrupt, prescaler = 1024
	
	DIDR0 = (1 << ADC0D); //disable digital input on ADC0
}

//timer0 configuration for 10ms interval
void timer0_init(void) {
	TCCR0A = (1 << WGM01); //CTC mode
	TCCR0B = (1 << CS02) | (1 << CS00); // prescaler 1024
	OCR0A = 155; //10ms
	TIMSK0 = (1 << OCIE0A); //enable compare match interrupt
}

volatile uint16_t adc_value = 0; //stores latest ADC result
volatile uint8_t adc_ready = 0; //flag indicating new ADC value is ready

//interrupt service routines

//timer0 Compare Match A interrupt
ISR(TIMER0_COMPA_vect) {
	//ADC triggered automatically by hardware
}
//interrupt for ADC conversion completion
ISR(ADC_vect) {
	adc_value = ADC; //storing ADC value
	adc_ready = 1; //setting flag for notifying new data is ready to be displayed
}

int main(void) {
	uart_init(103); //9600 baud @ 16MHz
	adc_init(); //initialize ADC with auto-trigger
	timer0_init(); //initialize timer for auto-trigger
	sei(); //enable global interrupts
	
	char buffer[20]; //holding the formatted voltage value
	float voltage; //store the formatted voltage

	while (1) {  //loop to display data 
		if (adc_ready) {
			adc_ready = 0; //clear flag
			
			//convert to voltage (0-5V)
			voltage = (adc_value / 1023.0) * 5.0;
			
			//format with 0.1V resolution
			snprintf(buffer, sizeof(buffer), "%.1fV\n", voltage);
			uart_print(buffer);
		}
	}
}
