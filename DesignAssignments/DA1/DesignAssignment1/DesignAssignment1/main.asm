;
; DA1.asm
;
; Created: 1/30/2025 12:12:03 PM
; Author : venki
;


; Replace with your application code
.ORG 0
    ; Initialize the stack pointer
    LDI R20,HIGH(RAMEND)
    OUT SPH,R20
    LDI R20,LOW(RAMEND)
    OUT SPL,R20
    ; Initialize Y-pointer for EEPROM Key retrieval
    LDI YH,HIGH(0x00)
    LDI YL,LOW(0x00)

    ; Initialize the X-pointer for Decrypted Message starting @ 0x200
    LDI XH,HIGH(0x2FF) //CHANGE to 0x2ff for avoiding memory conflict
    LDI XL,LOW(0x2FF)

    ; Initialize the Z-pointer for Key starting @ 0x600
    LDI ZH,HIGH(KEY<<1)
    LDI ZL,LOW(KEY<<1)


L1:    ;load KeyPin EEPROM

    LPM R20,Z+
    CALL STORE_IN_EEPROM    ; R20 has the EEPROM Data
    INC YL ; increment to next EEPROM Location
    CPI YL, 17 ; 8 bits x 16 = 128 bits
    BRNE L1 ; loop stops after 16 iterations

	RJMP TEAfunction


LOAD_FROM_EEPROM:
    SBIC EECR, EEPE
    RJMP LOAD_FROM_EEPROM
    OUT EEARH,YH
    OUT EEARL,YL
    SBI EECR,EERE
    IN R20,EEDR
    RET

STORE_IN_EEPROM:
    SBIC EECR, EEPE
    RJMP STORE_IN_EEPROM
    OUT EEARH,YH
    OUT EEARL,YL
    OUT EEDR,R20
    SBI EECR,EEMPE
    SBI EECR,EEPE
    RET

TEAfunction:
	LDI ZH,HIGH(MESSAGE<<1) ;loading high byte of message address
    LDI ZL,LOW(MESSAGE<<1)  ;loading low byte of message address
	;loading first 32-bit block of the message into registers R12-R15
	LPM R20, Z+   ;loading first byte of message
	MOV R15, R20
	LPM R20, Z+   ;second byte
	MOV R14, R20
	LPM R20, Z+   ;third byte
	MOV R13, R20  
	LPM R20, Z+   ;fourth byte
	MOV R12, R20   //LOADING R 15 TO 12
	;this is k0, GOING FROM R1 R2 R3 TO R4
	CALL LOAD_FROM_EEPROM      ;loading first key byte into R1
	MOV R1, R20
	DEC YL                     ;moving to next EEPROM address 
	CALL LOAD_FROM_EEPROM      
	MOV R2, R20                ;loading second key byte into R2
	DEC YL
	CALL LOAD_FROM_EEPROM      
	MOV R3, R20 ;loading second key byte into R3
	DEC YL
	CALL LOAD_FROM_EEPROM     
	MOV R4, R20 ;loading second key byte into R4
	DEC YL
	;THIS IS WHERE THE FIRST 32 BITS ARE 
	CALL LOAD_FROM_EEPROM      ;now this is k1, GOING FROM R5 R6 R7 TO R8, same logic as k0
	MOV R5, R20
	DEC YL
	CALL LOAD_FROM_EEPROM     
	MOV R6, R20
	DEC YL
	CALL LOAD_FROM_EEPROM      ;
	MOV R7, R20
	DEC YL
	CALL LOAD_FROM_EEPROM      ;
	MOV R8, R20
	DEC YL
	;DELTA CONSTANTS (sum) (looping once) for the first four bytes
	LDI R19, 0X9E 
	LDI R18, 0X37
	LDI R17, 0X79
	LDI R16, 0XB9
	;z << 4
	LSL R12
	ROL R13
	ROL R14
	ROL R15

	LSL R12
	ROL R13
	ROL R14
	ROL R15

	LSL R12
	ROL R13
	ROL R14
	ROL R15

	LSL R12
	ROL R13
	ROL R14
	ROL R15   
    ;(Z<<4 + K[0]) IS STORED R4:1
	ADD R1, R12
	ADC R2, R13
	ADC R3, R14
	ADC R4, R15
	;(Z >> 5)
	LSR R15
	ROR R14
	ROR R13
	ROR R12

	LSR R15
	ROR R14
	ROR R13
	ROR R12

	LSR R15
	ROR R14
	ROR R13
	ROR R12

	LSR R15
	ROR R14
	ROR R13
	ROR R12
	;NEEDING TO PRESERVE THE VALUES OF Z
	;z + sum where delta = sum
	ADD R16, R12
	ADC R17, R13
	ADC R18, R14
	ADC R19, R15
	;z>>5
	LSR R15
	ROR R14
	ROR R13
	ROR R12

	LSR R15
	ROR R14
	ROR R13
	ROR R12

	LSR R15
	ROR R14
	ROR R13
	ROR R12

	LSR R15
	ROR R14
	ROR R13
	ROR R12

	LSR R15
	ROR R14
	ROR R13
	ROR R12
	;z>>5 + k[1]
	ADD R5, R12  
	ADC R6, R13
	ADC R7, R14
	ADC R8, R15

	;this is x oring (Z<<4 + K[0])^(Z>>5 + K[1])
	EOR R4, R19
	EOR R3, R18
	EOR R2, R17
	EOR R1, R16
	;x oring z>>5 + k[1] with the value above
	EOR R4, R8
	EOR R3, R7
	EOR R2, R6
	EOR R1, R5

	//after doing this, these are our new least significant bits for our message (text[0])
	//start by loading r1

	ST X, R1 ;storing encrypted text back
	DEC XL ;decrementing pointer
	ST X, R2
	DEC XL
	ST X, R3
	DEC XL
	ST X, R4
	DEC XL
	//----------------------------------------------------------------------------------------------------------------------
	;PART 2, TEXT[1] , same logic as text[0] is followed
	LPM R20, Z+  
	MOV R15, R20
	LPM R20, Z+
	MOV R14, R20
	LPM R20, Z+
	MOV R13, R20
	LPM R20, Z+
	MOV R12, R20  
	CALL LOAD_FROM_EEPROM ;this is where k[2] is loaded     
	MOV R1, R20
	DEC YL
	CALL LOAD_FROM_EEPROM      
	MOV R2, R20
	DEC YL
	CALL LOAD_FROM_EEPROM      
	MOV R3, R20
	DEC YL
	CALL LOAD_FROM_EEPROM      
	MOV R4, R20
	DEC YL
	
	CALL LOAD_FROM_EEPROM ;this is where k[3] is loaded       
	MOV R5, R20
	DEC YL
	CALL LOAD_FROM_EEPROM      
	MOV R6, R20
	DEC YL
	CALL LOAD_FROM_EEPROM      
	MOV R7, R20
	DEC YL
	CALL LOAD_FROM_EEPROM      
	MOV R8, R20
	DEC YL
	LDI R19, 0X9E  
	LDI R18, 0X37
	LDI R17, 0X79
	LDI R16, 0XB9
	;y << 4
	LSL R12
	ROL R13
	ROL R14
	ROL R15

	LSL R12
	ROL R13
	ROL R14
	ROL R15

	LSL R12
	ROL R13
	ROL R14
	ROL R15

	LSL R12
	ROL R13
	ROL R14
	ROL R15 

	ADD R1, R12
	ADC R2, R13
	ADC R3, R14
	ADC R4, R15
	;y<<4 + k[0]

	;y is shifted back to receive original value
	LSR R15
	ROR R14
	ROR R13
	ROR R12

	LSR R15
	ROR R14
	ROR R13
	ROR R12

	LSR R15
	ROR R14
	ROR R13
	ROR R12

	LSR R15
	ROR R14
	ROR R13
	ROR R12
	
	ADD R16, R12
	ADC R17, R13
	ADC R18, R14
	ADC R19, R15
	;y + sum where sum = delta

	;y>>5
	LSR R15
	ROR R14
	ROR R13
	ROR R12

	LSR R15
	ROR R14
	ROR R13
	ROR R12

	LSR R15
	ROR R14
	ROR R13
	ROR R12

	LSR R15
	ROR R14
	ROR R13
	ROR R12

	LSR R15
	ROR R14
	ROR R13
	ROR R12

	;y>>5 + k[3]

	ADD R5, R12
	ADC R6, R13
	ADC R7, R14
	ADC R8, R15

	;this is x oring (y<<4 + k[2])^(y + sum)
	EOR R4, R19
	EOR R3, R18
	EOR R2, R17
	EOR R1, R16
	;this is x oring ((y<<4 + k[2])^(y + sum))^(y>>5 + k[3])
	EOR R4, R8
	EOR R3, R7
	EOR R2, R6
	EOR R1, R5

	//after doing this, these are our new least significant bits for our message (text[1])
	//start by loading r1

	ST X, R1
	DEC XL
	ST X, R2
	DEC XL
	ST X, R3
	DEC XL
	ST X, R4
	DEC XL

END: RJMP END

; Message: "jiLSTLNdaRrxrmiElGjSeiZBNSIrXEOInKAljICoLQvnCSTuTqApIrpqhyjBNAYy"     <<<<<You type something in english, you get hex values

.ORG 0x200
MESSAGE: .DB 0x6a, 0x69, 0x4c, 0x53, 0x54, 0x4c, 0x4e, 0x64, 0x61, 0x52, 0x72, 0x78, 0x72, 0x6d, 0x69, 0x45
;.DB 6c, 47, 6a, 53, 65, 69, 5a, 42, 4e, 53, 49, 72, 58, 45, 4f, 49
;.DB 6e, 4b, 41, 6c, 6a, 49, 43, 6f, 4c, 51, 76, 6e, 43, 53, 54, 75
;.DB 54, 71, 41, 70, 49, 72, 70, 71, 68, 79, 6a, 42, 4e, 41, 59, 79


; KEY: "YKTFgWnvaloBflrr"
.ORG 0x300
KEY:.DB 0x59, 0x4b, 0x54, 0x46, 0x67, 0x57, 0x6e, 0x76, 0x61, 0x6c, 0x6f, 0x42, 0x66, 0x6c, 0x72, 0x72
