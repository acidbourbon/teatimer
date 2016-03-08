// ======================================================================
// The perfect tea timer
//
// Copyright (C) 2016 Michael Wiebusch
// Visit http://acidbourbon.wordpress.com for more information to this
// and other projects of mine!
//
// This is free software, licensed under the terms of the GNU General
// Public License as published by the Free Software Foundation.
// ======================================================================



#include <avr/interrupt.h>
#include <avr/io.h> 
#include <util/delay.h>
#include <avr/sleep.h>

#include "tones.h"


#define SW1 PB2
#define SW2 PB3
#define LED PB1
#define BUZZ PB4
#define IOPORT PORTB
#define IOPIN  PINB
#define IODDR  DDRB

#define pause 0

#define KEY_DELAY 2
#define INC_COUNTER_SW1 60
#define INC_COUNTER_SW2 600



uint16_t countdown_seconds;
uint8_t key_delay_counter;
uint8_t armed;


void led_on(void){
  IOPORT &= ~(1<<LED);
}
void led_off(void){
  IOPORT |= (1<<LED);
}

ISR(TIM0_COMPA_vect){
  static uint8_t counter = 0;
  counter++;
  if(counter >=125){
    counter = 0;
    if(countdown_seconds > 0){
      countdown_seconds--;
    }
    led_on();
  }
  
  if(counter == 2){
    led_off();
  }
  
  if(key_delay_counter){
    key_delay_counter--;
  }
  
}


ISR(PCINT0_vect){
  
  if( key_delay_counter == 0){
    if( (IOPIN & (1<<SW1)) == 0){ //switch1 pressed
      armed = 1;
      countdown_seconds+=INC_COUNTER_SW1;
      play_coin();
    }
    if( (IOPIN & (1<<SW2)) == 0){ //switch2 pressed
      armed = 1;
      countdown_seconds+=INC_COUNTER_SW2;
      play_shroom();
    }
  }
  key_delay_counter = KEY_DELAY;
//   cancel_counter = 0;
}

void play(uint16_t note, uint8_t length) {
  
  if(note == 0){
    for(uint16_t i=0; i< length; i++){
      for(uint16_t j=0; j<1000;j++){
        _delay_us(1);
      }
    }
  } else {
    for(uint16_t i=0; i< length*1000/note; i++){
      for(uint16_t j=0; j<note;j++){
        _delay_us(1);
      }
        IOPORT ^= 1<<BUZZ;
    }
  }
}


void play_bigben(void){
  const uint8_t length=40;
  play(e4,length);
  play(c4,length);
  play(d4,length);
  play(g3,length);
  play(pause,length);
  play(g3,length);
  play(d4,length);
  play(e4,length);
  play(c4,length);
}

void play_coin(void){
  const uint8_t length=10;
  play(g4,length);
  play(c5,length);
}

void play_shroom(void){
  const uint8_t length=10;
  play(c4,length);
  play(e4,length);
  play(g4,length);
  play(c5,length);
}

void play_cancel(void){
  const uint8_t length=20;
  play(g2,length);
  play(pause,length/4);
  play(g2,length);
}
  
  
void init_io(void){
  IODDR |= (1<<LED) | (1<<BUZZ); //LED and Buzzer pins are outputs
  IOPORT |= (1<<SW1)|(1<<SW2); // pullups on Switch 1 and 2
}



void init_timer(void) {
  TCCR0A = (1<<WGM01)|(0<<WGM00)|(0<<COM0B1)|(0<<COM0B0);// WGM01=1: clear timer on compare match
  TCCR0B = (0<<WGM02)|(0<<CS02) | (1<<CS01)| (1<<CS00); // clock scaler = 64
  TIMSK  = (0<<TOIE0)|(1<<OCIE0A); // compare match interrupt enabled
  // TOIE0 = 1 : overflow interrupt enabled for timer0
  // OCIE0A = 1 : compare match interrupt enabled for timer0
  OCR0A  = 125; // count to 125, because it's 1000/8
  
}

void disable_timer(void){
  TIMSK  = (0<<TOIE0)|(0<<OCIE0A); // compare match interrupt disabled
}

void init_button_interrupt(void) {
  
  GIMSK |= (1<<PCIE); // enable pin changed interrupt 
  PCMSK |= (1<<PCINT2)|(1<<PCINT3); // pin changed mask: enable PB2 and PB3 as interrupt sources
  
}


void system_sleep() {
  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();

  sleep_mode();                        // System actually sleeps here

  sleep_disable();                     // System continues execution here when woken up by interrup
  
}


int main(void)
{
  
  countdown_seconds = 4;
  armed = 0;
  key_delay_counter = 0;
    
    
  init_io();
  init_button_interrupt();
  led_off();
  
  sei();
    
  while(1){
    

    init_timer();
    uint8_t cancel_counter = 0;
    _delay_ms(1000);
    
    while(countdown_seconds && armed){ // do some stuff while you're ticking
      
      // poll the switches, disable countdown if pressed long ...
      if( (IOPIN & (1<<SW1)) == 0 || (IOPIN & (1<<SW2)) == 0 ){ //any switch pressed??
        //if any button pressed, increase cancel_counter
        cancel_counter++;
      } else {
        cancel_counter = 0;
      }
      if(cancel_counter > 150) { // pressed for 1.5 sec?
        cancel_counter = 0;
        countdown_seconds = 0;
        armed = 0;
      }
      _delay_ms(10);
      
    }
    
    
   
    disable_timer();
    if(armed){
      led_off();
      play_bigben();
    } else {
      led_on();
      play_cancel();
      led_off();
    }

    
    while(1){
      countdown_seconds = 0;
      armed = 0;
      cancel_counter = 0;
      key_delay_counter = 0;
      led_off();
      system_sleep();
      if(armed){break;}; //someone armed the clock again!
      //else reset everything and go to sleep all over again
    }
      
  };

  return 0; /* never reached */
}







