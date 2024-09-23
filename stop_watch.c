/*
 * stop_watch.c
 *
 *  Created on: Sep 10, 2024
 *      Author: Patrick
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#define COUNT_UP 1// Constants to define counting direction
#define COUNT_DOWN 0
short sec_ones=0,sec_tens=0,min_ones=0,min_tens=0,hour_ones=0,hour_tens=0;// Global variables to keep track of time
unsigned char mode=COUNT_UP;// Mode for counting up or down
ISR(TIMER1_COMPA_vect)// Interrupt Service Routine for Timer1
{
	if (mode == COUNT_UP) {
		sec_ones++;

		if (sec_ones == 10) {
			sec_ones = 0;
			sec_tens++;

			if (sec_tens == 6) {
				sec_tens = 0;
				min_ones++;

				if (min_ones == 10) {
					min_ones = 0;
					min_tens++;

					if (min_tens == 6) {
						min_tens = 0;
						hour_ones++;
					}
				}
			}
		}
	}
	else if(mode==COUNT_DOWN)
	{
		if(!(sec_ones == 0 && sec_tens == 0 && min_ones == 0 && min_tens == 0 && hour_ones == 0 && hour_tens == 0))
		{
			sec_ones--;

			if (sec_ones < 0) {
				sec_ones = 9;
				sec_tens--;

				if (sec_tens<0) {
					sec_tens = 5;
					min_ones--;

					if (min_ones < 0) {
						min_ones = 9;
						min_tens--;

						if (min_tens < 0) {
							min_tens = 5;
							hour_ones--;

							if (hour_ones < 0) {
								hour_ones = 9;
								hour_tens--;
							}
						}
					}
				}
			}
		}
		else
		{
			PORTD|=(1<<PD0);// Turn on buzzer if countdown has finished
		}
	}
}
ISR(INT0_vect)// Interrupt Service Routine for External Interrupt 0 (Reset)
{
	sec_ones=0;// Reset time to zero
	sec_tens=0;
	min_ones=0;
	min_tens=0;
	hour_ones=0;
	hour_tens=0;
}
ISR(INT1_vect)// Interrupt Service Routine for External Interrupt 1 (Stop Timer)
{
	TCCR1B =0;//stops timer1
}
ISR(INT2_vect)// Interrupt Service Routine for External Interrupt 2 (Start Timer)
{
	TCCR1B = (1<<WGM12) | (1<<CS12) | (1<<CS10);// Start Timer1 in CTC mode with prescaler
}
void timer1_ctc(void)// Function to initialize Timer1 in CTC mode
{
	TCNT1 = 0;
	OCR1A = 15625;// Set CTC compare value for 1 second
	TIMSK |= (1<<OCIE1A); // Enable Timer1 compare interrupt
	TCCR1A = (1<<FOC1A);// Set Timer1 to CTC mode
	TCCR1B = (1<<WGM12) | (1<<CS12) | (1<<CS10);// Prescaler 1024
}
void INT1_Init (void)// Initialize External Interrupt 1
{
	DDRD  &= ~(1<<PD3);// Set PD3 as input
	MCUCR |= (1<<ISC11)|(1<<ISC10);// Trigger on rising edge
	GICR  |= (1<<INT1);// Enable INT1
}
void INT0_Init (void)// Initialize External Interrupt 0
{
	DDRD  &= ~(1<<PD2);// Set PD2 as input
	MCUCR |= (1<<ISC01);// Trigger on falling edge
	GICR  |= (1<<INT0);// Enable INT0
}
void INT2_Init (void)// Initialize External Interrupt 2
{
	DDRD  &= ~(1<<PB2);// Set PB2 as input
	MCUCR |= (1<<ISC01);// Trigger on falling edge
	GICR  |= (1<<INT2);// Enable INT2
}
int main(void)
{
	DDRC|=(1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3);// Set Data Direction Registers
	DDRA|=(1<<PA0)|(1<<PA1)|(1<<PA2)|(1<<PA3)|(1<<PA4)|(1<<PA5);
	DDRD|=(1<<PD4)|(1<<PD5)|(1<<PD0);
	DDRB&=~(1<<PB7)&~(1<<PB1)&~(1<<PB0)&~(1<<PB4)&~(1<<PB3)&~(1<<PB5)&~(1<<PB6);
	PORTB |= (1<<PB7)|(1<<PB1)|(1<<PB0)|(1<<PB4)|(1<<PB3)|(1<<PB5)|(1<<PB6)|(1<<PB2);// Enable pull-up resistors on input pins
	PORTD |= (1<<PD2);
	PORTD&=~(1<<PD5);
	PORTD|=(1<<PD4);
	PORTC=0;
	unsigned char mode_toggle=0,decrement_sec=0,increment_sec=0,decrement_min=0,increment_min=0,decrement_hour=0,increment_hour=0; // Initialize flags for button presses
	INT1_Init();// Initialize external interrupts and timer
	INT2_Init ();
	timer1_ctc();
	INT0_Init();
	SREG |= (1<<7);// Enable global interrupts
	while(1)
	{
		if (!(PINB & (1 << PB7))) {
			if(mode_toggle==0){
				mode = (mode == COUNT_UP) ? COUNT_DOWN : COUNT_UP;
				if(mode == COUNT_UP)
				{
					PORTD&=~(1<<PD0);
				}
				else
				{

				}
				mode_toggle=1;
			}
		}
		else
		{
			mode_toggle=0;
		}
		PORTD = (mode == COUNT_UP) ? (PORTD | (1 << PD4)) & ~(1 << PD5) : (PORTD | (1 << PD5)) & ~(1 << PD4);
		if (!(PINB & (1 << PB5)))
		{
			if(decrement_sec==0)
			{
				sec_ones--;
				if(sec_ones==0&&sec_tens!=0)
				{
					sec_tens--;
					sec_ones=9;
				}
				decrement_sec=1;
			}
		}
		else
		{
			decrement_sec=0;
		}
		if (!(PINB & (1 << PB3)))
		{
			if(decrement_min==0){
				min_ones--;
				if(min_ones==0&&min_tens!=0)
				{
					min_tens--;
					min_ones=9;
				}
				decrement_min=1;
			}
		}
		else
		{
			decrement_min=0;
		}
		if (!(PINB & (1 << PB0)))
		{
			if(decrement_hour==0)
			{
				hour_ones--;
				if(hour_ones==0&&hour_tens!=0)
				{
					hour_tens--;
					hour_ones=9;
				}
				decrement_hour=1;
			}
		}
		else
		{
			decrement_hour=0;
		}
		if(!(PINB & (1<<PB6)))
		{
			if(increment_sec== 0)
			{
				sec_ones++;
				if(sec_ones==10)
				{
					sec_tens++;
					sec_ones=0;
				}
				increment_sec= 1;
			}
		}
		else
		{
			increment_sec= 0;
		}

		if (!(PINB & (1 << PB4)))
		{
			if(increment_min==0)
			{
				min_ones++;
				if(min_ones==10)
				{
					min_tens++;
					min_ones=0;
				}
				increment_min=1;
			}
		}
		else
		{
			increment_min=0;
		}
		if (!(PINB & (1 << PB1)))
		{
			if(increment_hour==0)
			{
				hour_ones++;
				if(hour_ones==10)
				{
					hour_tens++;
					hour_ones=0;
				}
				increment_hour=1;
			}
		}
		else
		{
			increment_hour=0;
		}
		PORTA&=~(1<<PA1);
		PORTA&=~(1<<PA2);
		PORTA&=~(1<<PA3);
		PORTA&=~(1<<PA4);
		PORTA&=~(1<<PA5);
		PORTC = (PORTC & 0xF0) | (hour_tens& 0x0F);
		PORTA|=(1<<PA0);
		_delay_ms(2);
		PORTA&=~(1<<PA0);
		PORTA&=~(1<<PA2);
		PORTA&=~(1<<PA3);
		PORTA&=~(1<<PA4);
		PORTA&=~(1<<PA5);
		PORTC = (PORTC & 0xF0) | (hour_ones& 0x0F);
		PORTA|=(1<<PA1);
		_delay_ms(2);
		PORTA&=~(1<<PA0);
		PORTA&=~(1<<PA1);
		PORTA&=~(1<<PA3);
		PORTA&=~(1<<PA4);
		PORTA&=~(1<<PA5);
		PORTC = (PORTC & 0xF0) | (min_tens& 0x0F);
		PORTA|=(1<<PA2);
		_delay_ms(2);
		PORTA&=~(1<<PA0);
		PORTA&=~(1<<PA1);
		PORTA&=~(1<<PA2);
		PORTA&=~(1<<PA4);
		PORTA&=~(1<<PA5);
		PORTC = (PORTC & 0xF0) | (min_ones& 0x0F);
		PORTA|=(1<<PA3);
		_delay_ms(2);
		PORTA&=~(1<<PA0);
		PORTA&=~(1<<PA1);
		PORTA&=~(1<<PA2);
		PORTA&=~(1<<PA3);
		PORTA&=~(1<<PA5);
		PORTC = (PORTC & 0xF0) | (sec_tens& 0x0F);
		PORTA|=(1<<PA4);
		_delay_ms(2);
		PORTA&=~(1<<PA0);
		PORTA&=~(1<<PA1);
		PORTA&=~(1<<PA2);
		PORTA&=~(1<<PA3);
		PORTA&=~(1<<PA4);
		PORTC = (PORTC & 0xF0) | (sec_ones& 0x0F);
		PORTA|=(1<<PA5);
		_delay_ms(2);
	}
}

