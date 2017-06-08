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

uint16_t x;						// hold x value of joystick
unsigned char game_start = 0;	// start game
unsigned char blaster;			// button for blaster
unsigned char select_but;		//button for start
unsigned char direction;		// will hold 1 for right, 0 for left, 2 for stationary
unsigned char shot_fired;		// set to 0 when missile makes contact with an enemy
unsigned char fire;				// used to signal when boss shoots

unsigned char fire_pawn1;		// used to signal when pawns shoot
unsigned char fire_pawn2;		// used to signal when pawns shoot
unsigned char fire_pawn3;		// used to signal when pawns shoot

unsigned char win;				// holds value of win or loss
unsigned char count;			// counts until game reset

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//							State Machines								   //
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

enum SM1_States { s1, read_x };		// for reading the joystick analog values
enum SM2_States { s2, shot };		// for reading when the blaster button is pushed
enum SM3_States { s3, display, setup, game, warn, boss_setup, boss_battle, wait_end, end_game };		// displays the game: spawn and boss
enum SM4_States { s4, move };		// responsible for propagation of user bullet
enum SM5_States { s5, travel };		// responsible for auto firing and propagation of boss bullets
enum SM6_States { s6, travel1 };	// essentially same as state 5 but for pawn enemies
enum SM7_States { s7, travel2 };	// essentially same as state 5 but for pawn enemies
enum SM8_States { s8, travel3 };	// essentially same as state 5 but for pawn enemies

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
		}
		else if(x <= 2){
			direction = 0;
		}
		else if(x >= 6){
			direction = 1;
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
	unsigned char killed_enemies;		// checks to see when all pawns eliminated
	unsigned char killed_boss;			// checks to see when the boss is eliminated
	unsigned char killed_player;		// checks to see when the player is eliminated

	unsigned char temp1;
	unsigned char temp2;
	unsigned char temp3;

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
		killed_player = Check_player();

		if(killed_enemies == 8){		// if kill all pawns then go to boss fight
			state = boss_setup;				// changed
		}

		else if(killed_player == 0){	// if player is killed then go to end game sequence
			win = 0;
			state = wait_end;
		}
		else{							// else keep fighting pawns
			state = game;
		}
		break;

		case boss_setup:
		state = boss_battle;
		break;
		
		case boss_battle:
		killed_boss = Check_boss();
		killed_player = Check_player();

		if(killed_boss == 0){			// if boss is dead then go to end game sequence
			win = 1;
			state = wait_end;
		}

		else if(killed_player == 0){	// if player is killed then go to end game sequence
			win = 0;
			state = wait_end;
		}

		else{
			state = boss_battle;
		}
		break;

		case wait_end:
		state = end_game;
		break;

		case end_game:
		if(count < 15){
			state = end_game;
		}
		else{
			count = 0;
			state = s3;
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
		LCDClear();
		unsigned char *Menu = menu;
		LCD_Full_Image(Menu);
		for(int i = 0; i<1000; i++);
		break;

		case setup:
		killed_player = Check_player();
		PORTB = 0x00;

		game_start = 1;
		LCDClear();
		Set_up_enemy();
		Draw_enemy();
		Set_up_player();
		Draw_player();
		
		case game:
		
		killed_player = Check_player();
		if(killed_player == 0){PORTB = 0x00;}
		else if(killed_player == 1){PORTB = 0x01;}
		else if(killed_player == 2){PORTB = 0x03;}
		else if(killed_player == 3){PORTB = 0x07;}

		temp1 = Check_for_fire1();
		temp2 = Check_for_fire2();
		temp3 = Check_for_fire3();

		if(temp1 == 1){
			fire_pawn1 = 1;
		}
		else{
			fire_pawn1 = 0;
		}

		if(temp2 == 1){
			fire_pawn2 = 1;
		}
		else{
			fire_pawn2 = 0;
		}

		if(temp3 == 1){
			fire_pawn3 = 1;
		}
		else{
			fire_pawn3 = 0;
		}

		Move_enemy();
		Draw_enemy();
		Move_player(direction);
		Draw_player();
		break;

		case boss_setup:

		killed_player = Check_player();
		if(killed_player == 0){PORTB = 0x00;}
		else if(killed_player == 1){PORTB = 0x01;}
		else if(killed_player == 2){PORTB = 0x03;}
		else if(killed_player == 3){PORTB = 0x07;}
		fire_pawn1 = 0;
		fire_pawn2 = 0;
		fire_pawn3 = 0;

		Draw_enemy();
		Set_up_boss();
		Draw_boss();
		Move_player(direction);
		Draw_player();
		break;
		
		case boss_battle:
		
		killed_player = Check_player();
		if(killed_player == 0){PORTB = 0x00;}
		else if(killed_player == 1){PORTB = 0x01;}
		else if(killed_player == 2){PORTB = 0x03;}
		else if(killed_player == 3){PORTB = 0x07;}

		fire = 1;
		Move_boss();
		Draw_boss();
		Move_player(direction);
		Draw_player();
		break;

		case wait_end:
		PORTB = 0x00;
		fire = 0;
		fire_pawn1 = 0;
		fire_pawn2 = 0;
		fire_pawn3 = 0;
		game_start = 0;
		LCDClear();
		break;

		case end_game:
		PORTB = 0x00;
		LCDClear();
		if(win == 1){
			gotoXY(0,0);
			unsigned char *vic = victory;
			LCD_Full_Image(vic);
			for(int i = 0; i<1000; i++);
		}

		else if(win == 0){
			gotoXY(0,0);
			unsigned char *def = defeat;
			LCD_Full_Image(def);
			for(int i = 0; i<1000; i++);
		}
		count++;
		
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
//							pawn Missile 1								   //
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

unsigned char s1_hit = 1;		// 0  when bullet is dead
//unsigned char p_hit1 = 1;			// 1 when player avoids
int SMTick6(int state) {

	switch (state) {
		case s6:
		if(fire_pawn1 == 1){
			state = travel1;
		}
		else{
			state = s6;
		}
		break;

		case travel1:
		if(s1_hit == 1){
			state = travel1;
		}
		else if(s1_hit == 0){
			state = s6;
		}
		break;
		
		default:
		state = s6;
		break;
	}
	
	switch(state) {
		case s6:
		Set_up_pawn_shot1();
		s1_hit = 1;
		break;
		
		case travel1:
		s1_hit = Move_pawn_shot1();
		break;

		default: break;
	}
	return state;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//							pawn Missile 2								   //
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

unsigned char s2_hit = 1;		// 0  when bullet is dead
//unsigned char p_hit2 = 1;			// 1 when player avoids
int SMTick7(int state) {

	switch (state) {
		case s7:
		if(fire_pawn2 == 1){
			state = travel2;
		}
		else{
			state = s7;
		}
		break;

		case travel2:
		if(s2_hit == 1){
			state = travel2;
		}
		else if(s2_hit == 0){
			state = s7;
		}
		break;
		
		default:
		state = s7;
		break;
	}
	
	switch(state) {
		case s7:
		Set_up_pawn_shot2();
		s2_hit = 1;
		break;
		
		case travel2:
		s2_hit = Move_pawn_shot2();
		break;

		default: break;
	}
	return state;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//							pawn Missile 3								   //
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

unsigned char s3_hit = 1;		// 0  when bullet is dead
//unsigned char p_hit3 = 1;			// 1 when player avoids
int SMTick8(int state) {

	switch (state) {
		case s8:
		if(fire_pawn3 == 1){
			state = travel3;
		}
		else{
			state = s8;
		}
		break;

		case travel3:
		if(s3_hit == 1){
			state = travel3;
		}
		else if(s3_hit == 0){
			state = s8;
		}
		break;
		
		default:
		state = s8;
		break;
	}
	
	switch(state) {
		case s8:
		Set_up_pawn_shot3();
		s3_hit = 1;
		break;
		
		case travel3:
		s3_hit = Move_pawn_shot3();
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
	unsigned long int SMTick5_calc = 400;		// boss missile move
	unsigned long int SMTick6_calc = 600;		// pawn missile move
	unsigned long int SMTick7_calc = 800;
	unsigned long int SMTick8_calc = 500;
	
	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
	tmpGCD = findGCD(tmpGCD, SMTick3_calc);
	tmpGCD = findGCD(tmpGCD, SMTick4_calc);
	tmpGCD = findGCD(tmpGCD, SMTick5_calc);
	tmpGCD = findGCD(tmpGCD, SMTick6_calc);
	tmpGCD = findGCD(tmpGCD, SMTick7_calc);
	tmpGCD = findGCD(tmpGCD, SMTick8_calc);
	
	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;
	
	//Recalculate GCD periods for scheduler
	unsigned long int SMTick1_period = SMTick1_calc/GCD;
	unsigned long int SMTick2_period = SMTick2_calc/GCD;
	unsigned long int SMTick3_period = SMTick3_calc/GCD;
	unsigned long int SMTick4_period = SMTick4_calc/GCD;
	unsigned long int SMTick5_period = SMTick5_calc/GCD;
	unsigned long int SMTick6_period = SMTick6_calc/GCD;
	unsigned long int SMTick7_period = SMTick7_calc/GCD;
	unsigned long int SMTick8_period = SMTick8_calc/GCD;
	
	//Declare an array of tasks
	static task task1, task2, task3, task4, task5, task6, task7, task8;
	task *tasks[] = { &task1, &task2, &task3, &task4, &task5, &task6, &task7, &task8};
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

	// Task 6
	task6.state = -1;//Task initial state.
	task6.period = SMTick6_period;//Task Period.
	task6.elapsedTime = SMTick6_period;//Task current elapsed time.
	task6.TickFct = &SMTick6;//Function pointer for the tick.

	// Task 7
	task7.state = -1;//Task initial state.
	task7.period = SMTick7_period;//Task Period.
	task7.elapsedTime = SMTick7_period;//Task current elapsed time.
	task7.TickFct = &SMTick7;//Function pointer for the tick.

	// Task 8
	task8.state = -1;//Task initial state.
	task8.period = SMTick8_period;//Task Period.
	task8.elapsedTime = SMTick8_period;//Task current elapsed time.
	task8.TickFct = &SMTick8;//Function pointer for the tick.

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
