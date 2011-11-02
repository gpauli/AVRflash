/************************************************************************

 AVRFlash flashes an LED on Port  PB1
	
***********************************************************************

	Hardware
	
	prozessor:	ATMEGA8, ATMEGA168 ( see #define )
	clock:		16MHz Crystal

 ATMEL ATMEGA8 & 168 / ARDUINO

                                +-\/-+
 RESET/PCINT14            PC6  1|    |28  PC5 (AI 5)  ADC5/SCL/PCINT13
 RXD/PCINT16        (D 0) PD0  2|    |27  PC4 (AI 4)  ADC4/SDA/PCINT12
 TXD/PCINT17        (D 1) PD1  3|    |26  PC3 (AI 3)  ADC3/PCINT11
 INT0/PCINT18       (D 2) PD2  4|    |25  PC2 (AI 2)  ADC2/PCINT10
 INT1/OC2B/PCINT19  (D 3) PD3  5|    |24  PC1 (AI 1)  ADC1/PCINT9
 XCK/T0/PCINT20     (D 4) PD4  6|    |23  PC0 (AI 0)  ADC0/PCINT8
                          VCC  7|    |22  GND
                          GND  8|    |21  AREF
 XTAL1/TOSC1/PCINT6       PB6  9|    |20  AVCC
 XTAL2/TOSC2/PCINT7       PB7 10|    |19  PB5 (D 13)  SCK/PCINT5
 T1/OC0B/PCINT21    (D 5) PD5 11|    |18  PB4 (D 12)  MISO/PCINT4
 AIN0/OC0A/PCINT22  (D 6) PD6 12|    |17  PB3 (D 11)  MOSI/OC2A/PCINT3
 AIN1/PCINT23       (D 7) PD7 13|    |16  PB2 (D 10)  SS/OC1B/PCINT2
 ICP1/CLKO/PCINT0   (D 8) PB0 14|    |15  PB1 (D 9)   OC1A/PCINT1
                                +----+



****************************************************************************

	date	authors					version		comment
	======	======================	=======		==============================
	Okt.31	(gp) Gerd Pauli 	V1.0		First implemetation
	
	Versions:

	V1.0
	- first implementation


***************************************************************************/
/***************************************************************************
 *   
 *   (c) 2011 Gerd Pauli, gerd(at)pauli.info
 *
 ***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation version 2 of the License,                *
 *   If you extend the program please maintain the list of authors.        *
 *   If you want to use this software for commercial purposes and you      *
 *   don't want to make it open source, please contact the authors for     *
 *   licensing.                                                            *
 ***************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "oddebug.h"

#define  FALSE	0
#define  TRUE	1

#define FLASH_LED_DDR  DDRB
#define FLASH_LED_PORT PORTB
#define FLASH_LED_PIN  PINB
#define FLASH_LED      PINB1
#define FLASH_LED1      PINB2
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))

// CPU clock 16MHz
// #define F_CPU	16000000

#define FINT (F_CPU/512)	// 512=256PWM steps / 0.5 step per PWM interrupt
// FINT = 31250Hz @F_CPU16MHz

#define FS (FINT/2)
// FS = 15625Hz @F_CPU16MHz

void analogWrite(int val)
{
  // We need to make sure the PWM output is enabled for those pins
  // that support it, as we turn it off when digitally reading or
  // writing with them.  Also, make sure the pin is in output mode
  // for consistenty with Wiring, which doesn't require a pinMode
  // call for the analog output pins.
  // OC1A is Timer for FLASH_LED

  if (val == 0) {
    // disconnect pin from timer
    cbi(TCCR1A, COM1A1);
    // set low
    cbi(FLASH_LED_PORT,FLASH_LED);
  }
  else if (val == 255) {
    // disconnect pin from timer
    cbi(TCCR1A, COM1A1);
    // set high
    sbi(FLASH_LED_PORT,FLASH_LED);
  }
  else {
    // connect pwm to pin on timer 1
    sbi(TCCR1A, COM1A1);
    OCR1A = val; // set pwm duty
  }
}

int main(void) {
  // this provides a heartbeat on pin 9, so you can tell the software is running.
  uint8_t hbval=128;
  int8_t hbdelta=8;
  uint8_t osc = 0;
  uint8_t c = 0;
  sei();
  /* set LED pin as output */
  sbi(FLASH_LED_DDR,FLASH_LED);
  sbi(FLASH_LED_DDR,FLASH_LED1);
  osc = OSCCAL;
  odDebugInit();
  DBG1(0x01,&osc,1);

  TCCR1B = 0;
  // set timer 1 prescale factor to 64
  sbi(TCCR1B, CS11);
  sbi(TCCR1B, CS10);
  // put timer 1 in 8-bit phase correct pwm mode
  sbi(TCCR1A, WGM10);

  while (1) {
    DBG1(0x02,0,0);
  JB:
    //sbi(FLASH_LED_PORT,FLASH_LED);
    //cbi(FLASH_LED_PORT,FLASH_LED);
    //asm("nop\n");
    if (hbval > 192) hbdelta = -hbdelta;
    if (hbval < 32) hbdelta = -hbdelta;
    hbval += hbdelta;
    analogWrite(hbval);
    _delay_ms(40);
    if ( !(UCSRA & (1<<RXC))) {
      goto JB;
    }
    c = UDR;
    DBG1(0x03,&c,1);
    if ( c == '+' ) {
      osc++;
    } else if ( c == '-' ) {
      osc--;
    } else if ( c == '0' ) {
      hbval=0;
      hbdelta=0;
    } else if ( c == '1' ) {
      hbval=255;
      hbdelta=0;
    } else if ( c == 'd' ) {
      hbval=128;
      hbdelta=8;
    } else if ( c == 'o' ) {
      hbval=254;
      hbdelta=0;
    }
    OSCCAL = osc;
    DBG1(0x11,&osc,1);
    DBG1(0x12,&hbval,1);
    DBG1(0x13,&hbdelta,1);
  }
}

