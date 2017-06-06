#include "pieces.h"


struct Sprite{
	unsigned char x_pos;			// current x position
	unsigned char y_pos;			// current y position
	const unsigned char *bmp;		// image 
	unsigned char life_pts;			// the life points of the sprite
	unsigned char sz;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//							CHARACTERS									//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

struct Sprite Enemy_Row1[4];		// row of small enemies
struct Sprite Enemy_Row2[4];		// 2nd row of small enemies

struct Sprite Boss_top;				// final boss top half
struct Sprite Boss_bot;				// final boss lower half

struct Sprite user;					// single ship controlled by player

struct Sprite player_shot;			// shot fired by user
struct Sprite boss_shot1;			// shot fired by the boss (left)
struct Sprite boss_shot2;			// shot fired by the boss (right)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//							player shot									//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

void Set_up_shot(){
	player_shot.x_pos = user.x_pos + 6;
	player_shot.y_pos = 4;
	player_shot.bmp = p_shot;
	player_shot.life_pts = 1;
	player_shot.sz = sizeof(p_shot);
}


void Clear_shot(){
	LCD_Clear_Sprite(player_shot.x_pos, player_shot.y_pos, player_shot.bmp, player_shot.sz);
	for(int i = 0; i<1000; i++);
	return;
}

void Draw_shot(){
	if(player_shot.life_pts > 0){
		LCD_Sprite(player_shot.x_pos, player_shot.y_pos, player_shot.bmp, player_shot.sz);
		for(int i = 0; i<1000; i++);
		//player_shot.life_pts -= 1;
	}
	else{
		LCD_Clear_Sprite(player_shot.x_pos, player_shot.y_pos, player_shot.bmp, player_shot.sz);
		for(int i = 0; i<1000; i++);
	}
	return;
}

unsigned char Move_shot(){

	Clear_shot();

	unsigned char x1;		// x position for Enemy row 1
	unsigned char x2;		// x position for enemy row 2
	unsigned char y = player_shot.y_pos;

	for(int i = 0; i < 4; ++i){
		x1 = Enemy_Row1[i].x_pos;
		x2 = Enemy_Row2[i].x_pos;

		if(player_shot.y_pos == Enemy_Row1[i].y_pos + 1 && player_shot.x_pos > x1 && player_shot.x_pos < (x1 + 16) && Enemy_Row1[i].life_pts > 0){
			Enemy_Row1[i].life_pts = 0;
			player_shot.life_pts = 0;
			return 0;
		}
		else if(player_shot.y_pos == Enemy_Row2[i].y_pos + 1 && player_shot.x_pos > x2 && player_shot.x_pos < (x2 + 16) && Enemy_Row2[i].life_pts > 0){
			Enemy_Row2[i].life_pts = 0;
			player_shot.life_pts = 0;
			return 0;
		}
		else if(player_shot.y_pos == Boss_bot.y_pos + 1 && player_shot.x_pos > Boss_bot.x_pos && player_shot.x_pos < (Boss_bot.x_pos +35) && Boss_bot.life_pts > 0){
			Boss_bot.life_pts -= 1;
			player_shot.life_pts = 0;
			return 0;
		}
		else if(y -1 < 0){		// out of bounds
			player_shot.life_pts = 0;
			return 0;
		}
	}
	player_shot.y_pos = y - 1;
	Draw_shot();
	for(int i = 0; i<1000; i++);
	return 1;
}


void Set_up_boss_shot(){
	// change p shot to e shot later
	boss_shot1.x_pos = Boss_bot.x_pos + 10;
	boss_shot1.y_pos = 2;
	boss_shot1.bmp = p_shot;
	boss_shot1.life_pts = 1;
	boss_shot1.sz = sizeof(p_shot);

	boss_shot2.x_pos = Boss_bot.x_pos + 31;
	boss_shot2.y_pos = 2;
	boss_shot2.bmp = p_shot;
	boss_shot2.life_pts = 1;
	boss_shot2.sz = sizeof(p_shot);
}

void Clear_boss_shot(){
	LCD_Clear_Sprite(boss_shot1.x_pos, boss_shot1.y_pos, boss_shot1.bmp, boss_shot1.sz);
	for(int i = 0; i<1000; i++);
	LCD_Clear_Sprite(boss_shot2.x_pos, boss_shot2.y_pos, boss_shot2.bmp, boss_shot2.sz);
	for(int i = 0; i<1000; i++);
	return;
}

void Draw_boss_shot(){
	if(boss_shot1.life_pts > 0){
		LCD_Sprite(boss_shot1.x_pos, boss_shot1.y_pos, boss_shot1.bmp, boss_shot1.sz);
		for(int i = 0; i<1000; i++);
	}
	else{
		LCD_Clear_Sprite(boss_shot1.x_pos, boss_shot1.y_pos, boss_shot1.bmp, boss_shot1.sz);
		for(int i = 0; i<1000; i++);
	}
	if(boss_shot2.life_pts > 0){
		LCD_Sprite(boss_shot2.x_pos, boss_shot2.y_pos, boss_shot2.bmp, boss_shot2.sz);
		for(int i = 0; i<1000; i++);
	}
	else{
		LCD_Clear_Sprite(boss_shot2.x_pos, boss_shot2.y_pos, boss_shot2.bmp, boss_shot2.sz);
		for(int i = 0; i<1000; i++);
	}
	return;
}

unsigned char Move_boss_shot(){

	Clear_boss_shot();

	if(boss_shot1.y_pos == user.y_pos - 1 && boss_shot1.x_pos > user.x_pos && boss_shot1.x_pos < (user.x_pos + 13) && user.life_pts > 0){
		user.life_pts -= 1;
		boss_shot1.life_pts = 0;
		return 0;		// return 0 if left shot dead
	}
	else if(boss_shot2.y_pos == user.y_pos - 1 && boss_shot2.x_pos > user.x_pos && boss_shot2.x_pos < (user.x_pos + 13) && user.life_pts > 0){
		user.life_pts -= 1;
		boss_shot2.life_pts = 0;
		return 1;		// return 1 if right shot dead
	}
	else if(boss_shot1.y_pos + 1 > 5 || boss_shot2.y_pos + 1 > 5){		// out of bounds
		boss_shot1.life_pts = 0;
		boss_shot2.life_pts = 0;
		return 2;		// return 2 if out of bounds
	}

	boss_shot1.y_pos += 1;
	boss_shot2.y_pos += 1;
	Draw_boss_shot();
	for(int i = 0; i<1000; i++);
	return 3;			// return 4 if both shots are alive
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//							small enemy									//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

void Set_up_enemy(){			//sets up a row of small enemies
	for(int i = 0; i < 4; ++i){
		Enemy_Row1[i].x_pos = (20*i)+ 4;
		Enemy_Row1[i].y_pos = 0;
		Enemy_Row1[i].bmp = small_enemy;
		Enemy_Row1[i].life_pts = 1;
		Enemy_Row1[i].sz = sizeof(small_enemy);

		
		Enemy_Row2[i].x_pos = (20*i)+ 4;
		Enemy_Row2[i].y_pos = 1;
		Enemy_Row2[i].bmp = small_enemy;
		Enemy_Row2[i].life_pts = 1;
		Enemy_Row2[i].sz = sizeof(small_enemy);
		
	}
}

unsigned char Check_enemies(){
	unsigned char count=0;
	for(int i = 0; i < 4; ++i){
		if(Enemy_Row1[i].life_pts == 0){
			count +=1;
		}
		if(Enemy_Row2[i].life_pts == 0){
			count +=1;
		}
	}
	return count;
}

void Clear_enemies(){
	for(int i = 0; i < 4; ++i){
		//if(Enemy_Row1[i].life_pts == 0){		// dont draw dead enemies
			LCD_Clear_Sprite(Enemy_Row1[i].x_pos, Enemy_Row1[i].y_pos, Enemy_Row1[i].bmp, Enemy_Row1[i].sz);
			for(int i = 0; i<1000; i++);
		//}
		//if(Enemy_Row2[i].life_pts == 0){		// dont draw dead enemies
			LCD_Clear_Sprite(Enemy_Row2[i].x_pos, Enemy_Row2[i].y_pos, Enemy_Row2[i].bmp, Enemy_Row2[i].sz);
			for(int i = 0; i<1000; i++);
		//}
	}
	return;
}

void Draw_enemy(){
	for(int i = 0; i < 4; ++i){
		if(Enemy_Row1[i].life_pts > 0){		// dont draw dead enemies
			LCD_Sprite(Enemy_Row1[i].x_pos, Enemy_Row1[i].y_pos, Enemy_Row1[i].bmp, Enemy_Row1[i].sz);
			for(int i = 0; i<1000; i++);
		}
		else{
			LCD_Clear_Sprite(Enemy_Row1[i].x_pos, Enemy_Row1[i].y_pos, Enemy_Row1[i].bmp, Enemy_Row1[i].sz);
			for(int i = 0; i<1000; i++);
		}
		
		if(Enemy_Row2[i].life_pts > 0){		// dont draw dead enemies
			LCD_Sprite(Enemy_Row2[i].x_pos, Enemy_Row2[i].y_pos, Enemy_Row2[i].bmp, Enemy_Row2[i].sz);
			for(int i = 0; i<1000; i++);
		}
		else{
			LCD_Clear_Sprite(Enemy_Row2[i].x_pos, Enemy_Row2[i].y_pos, Enemy_Row2[i].bmp, Enemy_Row2[i].sz);
			for(int i = 0; i<1000; i++);
		}
		
	}
	return;
}

unsigned char dir = 0;		// 0 for left, 1 for right
void Move_enemy(){
	Clear_enemies();
	unsigned char x;

	if(dir == 1){			// 1 =  move right
		if(Enemy_Row1[3].x_pos + 1 < LCD_WIDTH-16){

			for(int i = 0; i < 4; ++i){
				x = Enemy_Row1[i].x_pos;

				Enemy_Row1[i].x_pos = x + 1;
				//Enemy_Row2[i].x_pos = x + 1;
			}
		}
		if(Enemy_Row2[0].x_pos - 1 > 0){
			for(int i = 0; i < 4; ++i){
				x = Enemy_Row2[i].x_pos;

				//Enemy_Row1[i].x_pos = x - 1;
				Enemy_Row2[i].x_pos = x - 1;
			}
		}
		else{
			dir = 0;
			return;
		}
	}
	else if(dir == 0){	// 0 = move left
		if(Enemy_Row2[3].x_pos + 1 < LCD_WIDTH-16){

			for(int i = 0; i < 4; ++i){
				x = Enemy_Row2[i].x_pos;

				Enemy_Row2[i].x_pos = x + 1;
				//Enemy_Row2[i].x_pos = x + 1;
			}
		}
		if(Enemy_Row1[0].x_pos - 1 > 0){
			for(int i = 0; i < 4; ++i){
				x = Enemy_Row1[i].x_pos;

				Enemy_Row1[i].x_pos = x - 1;
				//Enemy_Row2[i].x_pos = x - 1;
			}
		}
		else{
			dir = 1;
			return;
		}
	}
	return;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//							player  									//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

void Set_up_player(){
	user.x_pos = 35;
	user.y_pos = 5;
	user.bmp = player;
	user.life_pts = 1;
	user.sz = sizeof(player);
}

void Clear_player(){
	LCD_Clear_Sprite(user.x_pos, user.y_pos, user.bmp, user.sz);
	for(int i = 0; i<1000; i++);
	return;
}

void Draw_player(){
	
	if(user.life_pts > 0){
		LCD_Sprite(user.x_pos, user.y_pos, user.bmp, user.sz);
		for(int i = 0; i<1000; i++);
	}
	else{
		LCD_Clear_Sprite(user.x_pos, user.y_pos, user.bmp, user.sz);
		for(int i = 0; i<1000; i++);
	}
	
	return;
}


void Move_player(unsigned char direction){
	Clear_player();
	//add movement when at end of the screen
	unsigned char x;
	unsigned char dist = 5;
	if(direction == 1){			// 1 =  move right
		if(user.x_pos + dist < LCD_WIDTH-13){
			x = user.x_pos;
			user.x_pos = x + dist;
		}
		else{return;}
	}
	else if(direction == 0){	// 0 = move left
		if(user.x_pos - dist > 0){
				x = user.x_pos;
				user.x_pos = x - dist;
		}
		else{return;}
	}
	return;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//							boss sprite 								//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

void Set_up_boss(){			//sets up a row of small enemies
	Boss_top.x_pos = 21;
	Boss_top.y_pos = 0;
	Boss_top.bmp = boss_top;
	Boss_top.life_pts = 10;
	Boss_top.sz = sizeof(boss_top);

	Boss_bot.x_pos = 21;
	Boss_bot.y_pos = 1;
	Boss_bot.bmp = boss_bot;
	Boss_bot.life_pts = 10;
	Boss_bot.sz = sizeof(boss_bot);
}

/*
unsigned char Check_enemies(){
	unsigned char count=0;
	for(int i = 0; i < 4; ++i){
		if(Enemy_Row1[i].life_pts == 0){
			count +=1;
		}
		if(Enemy_Row2[i].life_pts == 0){
			count +=1;
		}
	}
	return count;
}
*/

void Clear_boss(){
	LCD_Clear_Sprite(Boss_top.x_pos, Boss_top.y_pos, Boss_top.bmp, Boss_top.sz);
	for(int i = 0; i<1000; i++);
	LCD_Clear_Sprite(Boss_bot.x_pos, Boss_bot.y_pos, Boss_bot.bmp, Boss_bot.sz);
	for(int i = 0; i<1000; i++);
	return;
}

void Draw_boss(){
	if(Boss_bot.life_pts > 0){		// dont draw dead enemies
		LCD_Sprite(Boss_top.x_pos, Boss_top.y_pos, Boss_top.bmp, Boss_top.sz);
		for(int i = 0; i<1000; i++);
		LCD_Sprite(Boss_bot.x_pos, Boss_bot.y_pos, Boss_bot.bmp, Boss_bot.sz);
		for(int i = 0; i<1000; i++);
	}
	else{
		LCD_Clear_Sprite(Boss_top.x_pos, Boss_top.y_pos, Boss_top.bmp, Boss_top.sz);
		for(int i = 0; i<1000; i++);
		LCD_Clear_Sprite(Boss_bot.x_pos, Boss_bot.y_pos, Boss_bot.bmp, Boss_bot.sz);
		for(int i = 0; i<1000; i++);
	}
	return;
}

unsigned char dir1 = 0;		// 0 for left, 1 for right

void Move_boss(){
	
	Clear_boss();
	unsigned char x;

	if(dir1 == 1){			// 1 =  move right
		if(Boss_top.x_pos + 2 < LCD_WIDTH-43){
			x = Boss_top.x_pos;
			Boss_top.x_pos = x + 2;
			Boss_bot.x_pos = x + 2;
		}
		else{
			dir1 = 0;
			return;
		}
	}
	else if(dir1 == 0){	// 0 = move left
		if(Boss_top.x_pos - 2 > 0){
			x = Boss_top.x_pos;
			Boss_top.x_pos = x - 2;
			Boss_bot.x_pos = x - 2;
		}
		else{
			dir1 = 1;
			return;
		}
	}
	return;
}
