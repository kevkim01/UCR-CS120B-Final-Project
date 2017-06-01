#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "timer.h"
#include <stdio.h>
#include "bit.h"
#include "scheduler.h"
#include "io.c"
#include "pwm.c"
#include "joystickADC.c"
#include "pcd8544.h"
#include "pcd8544.c"
/*
#include "5110.h"
#include "5110.c"
#include "timeout.h"
*/

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//							Shared Variables							   //
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

uint16_t x;				// hold x value of joystick
uint16_t y;				// hold y value of joystick
unsigned char blaster;	// button for blaster
unsigned char select_but; //button for start

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//							State Machines								   //
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

enum SM1_States { s1, read_x };
enum SM2_States { s2, sound};
enum SM3_States { s3, display, game};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//							Read Joystick								   //
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

int SMTick1(int state) {

	x = JoyStickY_ADC(SetADC_Ch(1));	// inverted because board orientation
	//y = JoyStickX_ADC(SetADC_Ch(0));	// inverted because board orientation

	switch (state) {
		case s1:
		state = read_x;
		break;

		case read_x:
		state = read_x;
		break;
		
		default:
		state = s1;
		break;
	}
	
	switch(state) {
		case s1: break;
		
		case read_x:
		if(x == 4){
			PORTB = 0x00;
		}
		else if(x == 1){
			PORTB = 0x04;
		}
		else if(x == 7){
			PORTB = 0x08;
		}
		break;

		default: break;
	}
	return state;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//							Blaster sounds									//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

int SMTick2(int state) {

	blaster = ~PINA & 0x04;

	switch (state) {
		case s2:
		if(blaster){
			state = sound;
		}
		else if(!blaster){
			state = s2;
		}
		break;

		case sound:
		if(blaster){
			state = s2;
		}
		else if(!blaster){
			state = s2;
		}
		break;

		default:
		state = s2;
		break;
	}
	
	switch(state) {
		case s2:
		set_PWM(0);
		break;

		case sound:
		set_PWM(523.25);
		break;
		
		default: break;
	}
	return state;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//							NOKIA DISPLAY								   //
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

int SMTick3(int state) {
	
	select_but = ~PINA & 0x08;
	PORTD = 0x08;
	switch (state) {
		case s3:
			state = display;
			break;

		case display:
			if(select_but){
				state = game;
			}
			else if(!select_but){
				state = display;
			}
			break;
		
		default:
			state = s3;
			break;
	}
	
	switch(state) {
		case s3:
			break;

		case display:
			//LCDInit();
			//LCDClear();
			LCDBitmap(menu);
			for(int i = 0; i<1000; i++);
			break;
		
		case game:
			//LCDInit();
			//LCDClear();
			LCDChar('a');
			for(int i = 0; i<1000; i++);
			break;
		
		default: break;
	}
	return state;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//							MAIN FUNCTION								   //
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

int main()
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	// Period for the tasks
	unsigned long int SMTick1_calc = 10;
	unsigned long int SMTick2_calc = 100;
	unsigned long int SMTick3_calc = 10;
	
	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
	tmpGCD = findGCD(tmpGCD, SMTick3_calc);
	
	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;
	
	//Recalculate GCD periods for scheduler
	unsigned long int SMTick1_period = SMTick1_calc/GCD;
	unsigned long int SMTick2_period = SMTick2_calc/GCD;
	unsigned long int SMTick3_period = SMTick3_calc/GCD;
	
	//Declare an array of tasks
	static task task1, task2, task3;
	task *tasks[] = { &task1, &task2, &task3};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	
	// Task 1
	task1.state = -1;//Task initial state.
	task1.period = SMTick1_period;//Task Period.
	task1.elapsedTime = SMTick1_period;//Task current elapsed time.
	task1.TickFct = &SMTick1;//Function pointer for the tick.
	
	// Task 2
	task2.state = -1;//Task initial state.
	task2.period = SMTick2_period;//Task Period.
	task2.elapsedTime = SMTick2_period;//Task current elapsed time.
	task2.TickFct = &SMTick2;//Function pointer for the tick.
	
	// Task 3
	task3.state = -1;//Task initial state.
	task3.period = SMTick3_period;//Task Period.
	task3.elapsedTime = SMTick3_period;//Task current elapsed time.
	task3.TickFct = &SMTick3;//Function pointer for the tick.

	// Set the timer and turn it on
	TimerSet(GCD);
	TimerOn();
	LCD_init();
	PWM_on();
	ADC_init();
	LCDInit();
	
	
	
	unsigned short i; // Scheduler for-loop iterator
	while(1) {
		
		// Scheduler code
		for ( i = 0; i < numTasks; i++ ) {
			// Task is ready to tick
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				// Setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// Reset the elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		
		while(!TimerFlag);
		TimerFlag = 0;
	}
	
	// Error: Program should not exit!
	return 0;
}
