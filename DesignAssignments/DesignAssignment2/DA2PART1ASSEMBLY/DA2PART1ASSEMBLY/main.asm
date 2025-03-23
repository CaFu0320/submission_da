;
; DA2PART1ASSEMBLY.asm
;
; Created: 3/20/2025 4:22:28 PM
; Author : Carlos Funes
;
.INCLUDE <M328PBDEF.INC> 

.ORG 0 ;RESET LOCATION
     JMP MAIN ;JUMPING TO MAIN

DELAY_150MS:        ;DELAY SUBROUTINE
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

    SBI DDRB, 5 ;PORTB5 AS OUTPUT
    CBI PORTB, 5 ;PORTB5 INITIALIZED TO OFF        
    CBI DDRC, 1 ; PORTC1 AS INPUT        
    SBI PORTC, 1 ;ENABLING PULL UP RESISTORS FOR PUSHBUTTON

WHILE:
    SBIC PINC, 1 ;CHECKING IF PUSHBUTTON PC1 IS PRESSED
    RJMP CHECKER ;IF BUTTON IS RELEASED              
    RJMP WHILE ;LOOPING FOREVER IF BUTTON IS NOT PRESSED           

CHECKER:
    SBIS PINC, 1 ;CHECK IF BUTTON IS NOT PRESSED          
    RJMP LED_ON ;IF BUTTON IS PRESSED, JUMP TO LED            
    RJMP CHECKER ;ONCE BUTTON IS PRESSED, STOP THE LOOP           

LED_ON:
    SBI PORTB, 5 ;TURN ON PB5 LED
    LDI R16, 0x14 ;1.5 SECOND DELAY 
L0:
    CALL DELAY_150MS ;CALLING DELAY SUBROUTINE
    DEC R16 ;DECREMENT R16
    CPI R16, 0 ;CHECK IF R16 = 0
    BRNE L0 ;CONTINUE TO LOOP IF R16 /= 0
    CBI PORTB, 5 ;AFTER 1.5 SECONDS, TURN OFF LED 
    RJMP WHILE ;JUMPING BACK TO THE MAIN LOOP
