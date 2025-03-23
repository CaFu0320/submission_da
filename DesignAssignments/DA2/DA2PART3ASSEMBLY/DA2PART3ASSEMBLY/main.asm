;
; DA2PART3ASSEMBLY.asm
;
; Created: 3/20/2025 5:16:02 PM
; Author : Carlos Funes
;
.INCLUDE <M328PBDEF.INC>

.ORG 0 ;RESET LOCATION
     JMP MAIN ;JUMPING TO MAIN

.ORG 0x02 ;INT0 VECTOR
JMP ISR_INT0VECTOR

DELAY_150MS:
LDI R17, 0x30 ;THIS IS OUTER LOOP
L1:
    LDI R18, 0x40 ;MIDDLE LOOP
L2:
	LDI R19, 0x60 ;INNER LOOP
L3: 
	DEC	R19 ;DECREMENTING R19
	CPI R19, 0 ;
	BRNE L3 ;BRANCH TO L3 IF R19 /= 0
	DEC R18 ;DECREMENTING R18
	CPI R18, 0
	BRNE L2 ;BRANCH TO L2 IF R18 /= 0 
	DEC R17 ;DECREMENTING R17
	CPI R17, 0
	BRNE L1 ;BRANCH TO L1 IF R17 /= 0
	RET	;RETURN TO THE PROGRAM

MAIN:
    LDI R20, HIGH(RAMEND) ;STARTING STACK POINTER
    OUT SPH, R20
    LDI R20, LOW(RAMEND)
    OUT SPL, R20

	LDI R20, 0x03 ;MAKING INT0 BE TRIGGERED AT RISING EDGE, LAST 2 BITS MAKE THIS HAPPEN
	STS EICRA, R20 ;STORING R20 INTO EICRA
	SBI EIMSK, INT0 ;ENABLING INT0 INTERRUPT
	SEI

    SBI DDRB, 5 ;PORTB5 AS OUTPUT
    CBI PORTB, 5 ;PORTB5 INITIALIZED TO OFF        
    CBI DDRD, 2 ; PORTD2 AS INPUT        
    SBI PORTD, 2 ;ENABLING PULL UP RESISTORS FOR PUSHBUTTON
	CBI DDRC, 1 ; PORTC1 AS INPUT        
    SBI PORTC, 1 ;ENABLING PULL UP RESISTORS FOR PUSHBUTTON

    LDI R20,(1<<ISC01)|(1<<ISC00) ;SETTING INT0 RISING EDGE TRIGGER
    STS EICRA,R20 ;STORING TO INT CONTROL REGISTER
    SBI EIMSK,INT0 ;ENABLING INT0 INTERRUPT
    SEI ;GLOBAL INTERRUPT

POLL_LOOP:

    SBIC PINC,1 ;CHECKING IF BUTTON PC1 PRESSED 
    RJMP POLL_LOOP ;KEEP POLLING IF NOT PRESSED

    CALL DELAY_150MS ;DEBOUNCE DELAY 0.15 SEC

WAIT_RELEASE:

    SBIS PINC,1 ;CHECK IF BUTTON RELEASED
    RJMP WAIT_RELEASE ;WAIT UNTIL BUTTON RELEASED

    SBI PORTB,5 ;TURN ON LED
    LDI R16,20 ;SET 1.5 SEC DELAY
DELAY_1_5S:

    CALL DELAY_150MS ;CALLING 0.15 SEC DELAY
    DEC R16 ;DECREMENT COUNTER
    BRNE DELAY_1_5S ;REPEAT 20 TIMES
    CBI PORTB,5 ;TURN OFF LED
    RJMP POLL_LOOP ;RETURN TO POLLING LOOP

ISR_INT0VECTOR:

    SBI PORTB,5 ;TURN ON LED ON INTERRUPT
    PUSH R16 ;SAVE REGISTER STATE
    LDI R16,20 ;SET 3 SEC DELAY 
DELAY_3S:

    CALL DELAY_150MS ;CALLING 0.15 SEC DELAY
    DEC R16 ;DECREMENT COUNTER
    BRNE DELAY_3S ;REPEATING 20 TIMES
    CBI PORTB,5 ;TURN OFF LED
    POP R16 ;RESTORING REGISTER STATE
    RETI ;RETURN FROM INTERRUPT