/*
 * Simple code to flash a beacon.
 * 
 * ATtiny85 internal clock at 8 MHz
 * 
 * flashes on OC1A on PB1, which is physical pin 6 on the 8-pin DIP
 * supplemental flash at 1/1000 on PB4 (pin 3)
 * button on PB3 (pin 2) to cycle through modes
 */
#include <button.h>

void setup() 
{
  cli();

  //'pin' refers to physical pin on 8-pin DIP
  DDRB |= (1 << PB1);  //pin 6 as output
  DDRB |= (1 << PB4);  //pin 3 as output
  PORTB |= (1 << PB3); //pullup pin 2 for button

  //prescalers:
  // 1 = 0x01
  // 8 = 0x04
  // 16 = 0x05
  // 256 = 0x09
  // 1024 = 0x0B
  // 16k = 0x0F

  //@ 8 MHz:
  //freq = 8e6 / (2 * PRE * (TOP + 1))
  // ==> TOP = 8e6 / (2 * freq * PRE) 

  //initialize Time/Counter 1 to 1 kHz
  TCCR1 = 0x95; //CTC on OC1A | PRESCALER
  OCR1C = 249;  //TOP - 1

  TIMSK |= 0x40; //set OCIEA for counter

  sei();
}

Button cycleButton(PB3);
uint8_t mode = 0; //0 = 1 kHz; 1 = 5 kHz; 2 = DC

void loop() 
{  
  if(cycleButton.CheckButton())
  {
    mode++; mode %= 3;
    cli();
    switch(mode)
    {
      case 0: 
        TCCR1 = 0x95; OCR1C = 249; // 1 kHz
        break;
      case 1: 
        TCCR1 = 0x95; OCR1C = 49; // 5 kHz
        break;
      case 2: 
        TCCR1 = 0x00; PORTB |= (1 << PB1); //0 Hz, lit
        PORTB |= (1 << PB4); //turn on LED
        break;     
    }
    sei();
  }
}

volatile uint16_t counter = 0; //use a counter to strobe indicator at 1/1000th frequency of beacon

ISR(TIMER1_COMPA_vect) 
{
  counter++;  
  counter %= 1000;
  if(!counter) PINB = (1 << PB4); //toggle indicator LED on pin 3
  //PORTB ^= (1 << PB4); //toggle indicator LED on pin 3
} 
