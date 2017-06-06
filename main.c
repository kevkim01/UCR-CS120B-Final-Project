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
unsigned char game_start = 0;	// start game
unsigned char blaster;	// button for blaster
unsigned char select_but; //button for start
unsigned char direction;	//will hold 1 for right, 0 for left, 2 for stationary
unsigned char shot_fired;	//set to 0 when missile makes contact with an enemy
unsigned char fire;			// used to signal when boss shoots

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//							State Machines								   //
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

enum SM1_States { s1, read_x };		// for reading the joystick analog values
enum SM2_States { s2, shot };		// for reading when the blaster button is pushed
enum SM3_States { s3, display, setup, game, boss_setup, boss_battle };		// displays the game: spawn and boss
enum SM4_States { s4, move };		// responsible for propagation of user bullet
enum SM5_States { s5, travel };		// responsible for auto firing and propagation of boss bullets
	
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
//							Blaster user   								//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

int SMTick2(int state) {

	if(game_start == 1){
		blaster = ~PINA & 0x04;
	}
	else{
		blaster = 0;
	}

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
			break;

		case shot:
			shot_fired = 1;
			set_PWM(523.25);
			Set_up_shot();
			Draw_shot();
		
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
	unsigned char killed_enemies;
	
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
			killed_enemies = Check_enemies();
			if(killed_enemies == 8){
				state = boss_setup;
			}
			else{
				state = game;
			}
			break;
			
		case boss_setup:
			state = boss_battle;
			break;
			
		case boss_battle:
			state = boss_battle;
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
			game_start = 1;
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
		
		case boss_setup:
			Draw_enemy();
			Set_up_boss();
			Draw_boss();
			Move_player(direction);
			Draw_player();
			break;
			
		case boss_battle:
			fire = 1;
			Move_boss();
			Draw_boss();
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
//							boss Missile								   //
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

unsigned char shot1_hit = 1;		// 0  when bullet is dead
unsigned char shot2_hit = 1;		// 0  when bullet is dead
unsigned char p_hit = 1;			// 1 when player avoids
int SMTick5(int state) {
	
	
	switch (state) {
		case s5:
			if(fire){
				state = travel;
			}
			else{
				state = s5;
			}
			break;

		case travel:
			if(shot1_hit == 1 || shot2_hit == 1){
				state = travel;
			}
			else if(shot1_hit == 0 && shot2_hit == 0){
				state = s5;
			}
			break;
			
			default:
				state = s5;
				break;
	}
	
	switch(state) {
		case s5: 
			Set_up_boss_shot(); 
			shot1_hit = 1;
			shot2_hit = 1;
			break;
		
		case travel:
			p_hit = Move_boss_shot();
			if(p_hit == 0){
				shot1_hit = 0;
			}
			else if(p_hit == 1){
				shot2_hit = 0;
			}
			else if(p_hit == 2){
				shot1_hit = 0;
				shot2_hit = 0;
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
	unsigned long int SMTick5_calc = 800;		// boss missile move
	
	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
	tmpGCD = findGCD(tmpGCD, SMTick3_calc);
	tmpGCD = findGCD(tmpGCD, SMTick4_calc);
	tmpGCD = findGCD(tmpGCD, SMTick5_calc);
	
	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;
	
	//Recalculate GCD periods for scheduler
	unsigned long int SMTick1_period = SMTick1_calc/GCD;
	unsigned long int SMTick2_period = SMTick2_calc/GCD;
	unsigned long int SMTick3_period = SMTick3_calc/GCD;
	unsigned long int SMTick4_period = SMTick4_calc/GCD;
	unsigned long int SMTick5_period = SMTick5_calc/GCD;
	
	//Declare an array of tasks
	static task task1, task2, task3, task4, task5;
	task *tasks[] = { &task1, &task2, &task3, &task4, &task5};
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

	// Task 5
	task5.state = -1;//Task initial state.
	task5.period = SMTick5_period;//Task Period.
	task5.elapsedTime = SMTick5_period;//Task current elapsed time.
	task5.TickFct = &SMTick5;//Function pointer for the tick.

	TimerSet(GCD);
	TimerOn();
	LCD_init();
	PWM_on();
	ADC_init();
	LCDInit();
	
	unsigned short i; // Scheduler for-loop iterator
	while(1) {
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
	return 0;
}
