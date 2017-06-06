#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include "timer.h"
#include "bit.h"
#include "scheduler.h"
#include "io.c"
#include "pwm.c"
#include "joystickADC.c"
#include "pcd8544.h"
#include "pcd8544.c"
#include "pieces.h"
#include "pieces.c"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//							Shared Variables							   //
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

uint16_t x;				// hold x value of joystick
uint16_t y;				// hold y value of joystick
unsigned char blaster;	// button for blaster
unsigned char select_but; //button for start
unsigned char direction;	//will hold 1 for right, 0 for left, 2 for stationary
unsigned char shot_fired;	//set to 0 when missile makes contact with an enemy

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//							State Machines								   //
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

enum SM1_States { s1, read_x };
enum SM2_States { s2, shot };
enum SM3_States { s3, display, setup, game };

enum SM4_States { s4, move };
	
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//							Read Joystick								   //
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

int SMTick1(int state) {

	x = JoyStickY_ADC(SetADC_Ch(1));	// inverted because board orientation

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
			direction = 2;
			PORTB = 0x00;
		}
		else if(x <= 2){
			direction = 0;
			PORTB = 0x01;
		}
		else if(x >= 6){
			direction = 1;
			PORTB = 0x02;
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
			state = shot;
		}
		else if(!blaster){
			state = s2;
		}
		break;

		case shot:
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
			//shot_fired =0;
			break;

		case shot:
			set_PWM(523.25);
			Set_up_shot();
			Draw_shot();
			shot_fired = 1;
			
			//Move_shot();
			/*
			while(shot_hit == 1){
				shot_hit = Move_shot();
				Draw_shot();
			}
			*/
			
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
				state = setup;
			}
			else if(!select_but){
				state = display;
			}
			break;

		case setup:
			state = game;
			break;

		case game:
			state = game;
			break;


		default:
			state = s3;
			break;
	}
	
	switch(state) {
		
		case s3:
			break;

		case display:
			LCDClear();
			unsigned char *Menu = menu;
			LCD_Full_Image(Menu);
			for(int i = 0; i<1000; i++);
			break;

		case setup:
			LCDClear();
			Set_up_enemy();
			Draw_enemy();

			Set_up_player();
			Draw_player();
			
		case game:
			Move_enemy();
			Draw_enemy();
			Move_player(direction);
			Draw_player();
			break;
		
		default: break;
	}
	return state;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//							Missile move								   //
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

int SMTick4(int state) {
	
	unsigned char hit = 1;		// 0  when bullet is dead
	switch (state) {
		case s4:
			if(shot_fired){
				state = move;
			}
			else{
				state = s4;
			}
			break;

		case move:
			if(shot_fired){
				state = move;
			}
			else{
				state = s4;
			}
			break;
		
		default:
			state = s4;
			break;
	}
	
	switch(state) {
		case s4: break;
		
		case move:
			hit = Move_shot();
			if(hit ==0){
				shot_fired = 0;
			}
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
	unsigned long int SMTick1_calc = 60;		// joystick
	unsigned long int SMTick2_calc = 400;		// blaster
	unsigned long int SMTick3_calc = 250;		// display
	unsigned long int SMTick4_calc = 180;		// missile move
	
	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
	tmpGCD = findGCD(tmpGCD, SMTick3_calc);
	tmpGCD = findGCD(tmpGCD, SMTick4_calc);
	
	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;
	
	//Recalculate GCD periods for scheduler
	unsigned long int SMTick1_period = SMTick1_calc/GCD;
	unsigned long int SMTick2_period = SMTick2_calc/GCD;
	unsigned long int SMTick3_period = SMTick3_calc/GCD;
	unsigned long int SMTick4_period = SMTick4_calc/GCD;
	
	//Declare an array of tasks
	static task task1, task2, task3, task4;
	task *tasks[] = { &task1, &task2, &task3, &task4};
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
	
	// Task 4
	task4.state = -1;//Task initial state.
	task4.period = SMTick4_period;//Task Period.
	task4.elapsedTime = SMTick4_period;//Task current elapsed time.
	task4.TickFct = &SMTick4;//Function pointer for the tick.

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
