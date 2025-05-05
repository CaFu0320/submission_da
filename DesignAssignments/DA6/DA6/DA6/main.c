/*
 * DA6
 * 
 * Created: 4/28/2025 11:50:07 PM
 * Author: Carlos Funes
 */

#define F_CPU 16000000UL //defining CPU frequency
#include <avr/io.h> //including AVR I/O register definitions 
#include <stdint.h> //integer types
#include <avr/interrupt.h> //including interrupt handling functions 
#include <util/delay.h> //for delay_ms()
#include <stdio.h> //I/O functions
#include <stdlib.h> //for functions
#include <string.h> //string handling

//UART config
#define BAUDRATE 9600 //baud rate
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1) // Prescaler calculation
char uart_buffer[64];          // Buffer for UART messages

//function prototypes
void USART_init(void); //UART
void Servo_init(void); //servo motor
void Ultrasonic_init(void); // Initialize ultrasonic sensor
void SPI_init(void); // Initialize SPI for 7-segment
void USART_putstring(const char* str); //sending string via UART
uint16_t angle_to_OCR(uint16_t angle); //converting angle to PWM value
double read_distance_mm(void); //reading distance from sensor
void display_number_on_7seg(uint16_t val); //displaying number on 7-segment
void display_number_once(uint16_t val); //single display update

//UART initialization
void USART_init(void) {
    UBRR0H = (uint8_t)(BAUD_PRESCALLER >> 8); //high byte of baud rate
    UBRR0L = (uint8_t)(BAUD_PRESCALLER); //low byte
    UCSR0B = (1 << RXEN0) | (1 << TXEN0); //enabling receiver and transmitter
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); //8-bit data, 1 stop bit, no parity
}

//sending single character via UART
void USART_send(unsigned char data) {
    while (!(UCSR0A & (1 << UDRE0))); //waiting until buffer is empty
    UDR0 = data; //loading data into transmit register
}

//sending string via UART
void USART_putstring(const char* str) {
    while (*str) { //looping until end of string
        USART_send(*str++); //sending each character
    }
}

//servo motor initialization 
void Servo_init(void) {
    DDRB |= (1 << PB1); //setting PB1 as output (OC1A)
    //configuring Timer1 for fast PWM mode
    TCCR1A = (1 << COM1A1) | (1 << WGM11); //non-inverting, fast PWM
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10); //prescaler 64
    ICR1 = 4999; //PWM period for 50Hz (20ms)
}

//angle (0-180) to OCR1A value (105-610)
uint16_t angle_to_OCR(uint16_t angle) {
    return (uint16_t)(105 + (angle * 505.0 / 180.0));
}

//ultrasonic sensor initialization (PD5 trigger, PE2 echo)
volatile uint16_t TimerOverflow = 0; //tracking Timer3 overflows

ISR(TIMER3_OVF_vect) { //timer3 overflow interrupt
    TimerOverflow++; //increment overflow count
}

void Ultrasonic_init(void) {
    DDRD |= (1 << PD5); //PD5 as trigger output
    DDRE &= ~(1 << PE2); //PE2 (ICP3) as input
    TCCR3A = 0; //normal timer3 operation
    TIMSK3 |= (1 << TOIE3); //enabling timer3 overflow interrupt
}

//reading distance in mm
double read_distance_mm(void) {
    PORTD |= (1 << PD5); //sending 10us trigger pulse
    _delay_us(10);
    PORTD &= ~(1 << PD5);
    
    TCCR3B = (1 << ICES3) | (1 << CS10); //capturing rising edge, no prescaler
    TIFR3 = (1 << ICF3) | (1 << TOV3); //clearing flags
    TimerOverflow = 0; //resetting overflow counter
    while (!(TIFR3 & (1 << ICF3))); //waiting for rising edge
    
    TCNT3 = 0; //resetting timer
    TCCR3B &= ~(1 << ICES3); //capturing falling edge
    TIFR3 = (1 << ICF3) | (1 << TOV3); //clearing flags
    TimerOverflow = 0;
    while (!(TIFR3 & (1 << ICF3))); //waiting for falling edge
    
    uint32_t pulse = (uint32_t)ICR3 + (65536 * TimerOverflow);
    return (pulse / 58.0 / 16.0) * 10.0; //converting to mm
}

//SPI and 7 segment display PB2-PB5
#define DATA    (1<<PB3) //data pin (PB3)
#define LATCH   (1<<PB2) //latch (PB2)
#define CLOCK   (1<<PB5) //clock (PB5)

//segment patterns for 0-9
const uint8_t SEGMENT_MAP[10] = {
    0xC0, 0xF9, 0xA4, 0xB0, 0x99, // 0-4
    0x92, 0x82, 0xF8, 0x80, 0x90  // 5-9
};
//digit enable patterns for 4-digit display
const uint8_t DIGIT_SELECT[4] = {0xF1, 0xF2, 0xF4, 0xF8};

void SPI_init(void) {
    DDRB |= (DATA | CLOCK | LATCH); //setting data/latch/clock pins as outputs
    PORTB &= ~(DATA | CLOCK | LATCH); //initialize low
    SPCR0 = (1 << SPE) | (1 << MSTR);  //enabling SPI communication, master mode
}

//sending number pattern via SPI
void SPI_send(uint8_t byte) {
    SPDR0 = byte; //loading data
    while (!(SPSR0 & (1 << SPIF))); //waiting until data is fully sent
}

//displaying 4-digit number with multiplexing
void display_number_on_7seg(uint16_t val) {
    uint8_t digits[4] = {
        (uint8_t)(val % 10),          //ones
        (uint8_t)((val / 10) % 10),   //tens
        (uint8_t)((val / 100) % 10),  //hundreds
        (uint8_t)(val / 1000)         //thousands
    };
    
    for (int8_t i = 3; i >= 0; i--) { //cycling through each digit position
        PORTB &= ~LATCH; //preparing to send data
        SPI_send(SEGMENT_MAP[digits[i]]); //sending segment pattern
        SPI_send(DIGIT_SELECT[i]); //selecting digit that lights up
        PORTB |= LATCH; //latching high to update
        _delay_ms(2); //display each digit for 2ms
    }
}

//single display update
void display_number_once(uint16_t val) {
    // Similar to above but without delay
    uint8_t digits[4] = {val % 10, (val/10)%10, (val/100)%10, val/1000};
    for (int8_t i = 3; i >= 0; i--) {
        PORTB &= ~LATCH;
        SPI_send(SEGMENT_MAP[digits[i]]);
        SPI_send(DIGIT_SELECT[i]);
        PORTB |= LATCH;
    }
}

//main program
int main(void) {
    uint16_t angle; //current servo angle
    double distance, min_distance; //distance measurements
    uint16_t display_min; //minimum distance to display
    char angleStr[8], distStr[8]; //UART buffers

    USART_init(); //starting serial communication
    Servo_init(); //preparing servo motor
    Ultrasonic_init(); //preparing distance sensor
    SPI_init(); //preparing display
    sei(); //enabling interrupts

    USART_putstring("Angle (Degrees), Distance (mm)\n"); //printing 

    while (1) {
        min_distance = 9999.0; //starting with a high minimum distance

        //sweeping servo 0 to 180
        for (angle = 0; angle <= 180; angle += 2) {
            OCR1A = angle_to_OCR(angle); // Moving servo to current angle
            distance = read_distance_mm(); //measuring distance

            if (distance < min_distance) min_distance = distance; //tracking closest object

            //formating and sending data via UART
            dtostrf((double)angle, 3, 0, angleStr); //converting angle to text
            dtostrf(distance, 6, 2, distStr); //converting distance to text
            snprintf(uart_buffer, sizeof(uart_buffer),
                "Angle: %s°   Distance: %s mm\n", angleStr, distStr); //sending this to the terminal
            USART_putstring(uart_buffer);

            display_min = (uint16_t)min_distance; //updating display with the closes distance
            display_number_on_7seg(display_min);
            _delay_ms(100); //waiting between steps
        }

        //servo 180 to 0 
        for (angle = 180; angle >= 0; angle -= 2) { //this is the same process from above but in reverse
            OCR1A = angle_to_OCR(angle);
            distance = read_distance_mm();

            dtostrf((double)angle, 3, 0, angleStr);
            dtostrf(distance, 6, 2, distStr);
            snprintf(uart_buffer, sizeof(uart_buffer), 
                "Angle: %s°   Distance: %s mm\n", angleStr, distStr);
            USART_putstring(uart_buffer);

            display_number_on_7seg((uint16_t)distance);
            _delay_ms(100);
        }
    }
}