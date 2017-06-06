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
struct Sprite user;					// single ship controlled by player
struct Sprite player_shot;			// shot fired by user
struct Sprite Boss;					// final boss


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//							shot fired 									//
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
		else if(y -1 < 0){		// out of bounds
			player_shot.life_pts = 0;
			return 0;
		}
		//else{
			
			//continue;
		//}
		
	}
	player_shot.y_pos = y - 1;
	Draw_shot();
	for(int i = 0; i<1000; i++);
	//player_shot.y_pos = y-1;
	return 1;
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
	user.life_pts = 5;
	user.sz = sizeof(player);
}

void Clear_player(){
	LCD_Clear_Sprite(user.x_pos, user.y_pos, user.bmp, user.sz);
	for(int i = 0; i<1000; i++);
	return;
}

void Draw_player(){
	
	LCD_Sprite(user.x_pos, user.y_pos, user.bmp, user.sz);
	for(int i = 0; i<1000; i++);
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
	Boss.x_pos = 21;
	Boss.y_pos = 0;
	Boss.bmp = boss;
	Boss.life_pts = 10;
	Boss.sz = sizeof(boss);
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
	LCD_Clear_Sprite(Boss.x_pos, Boss.y_pos, Boss.bmp, Boss.sz);
	for(int i = 0; i<1000; i++);
	return;
}

void Draw_boss(){
	if(Boss.life_pts > 0){		// dont draw dead enemies
		LCD_Sprite(Boss.x_pos, Boss.y_pos, Boss.bmp, Boss.sz);
		for(int i = 0; i<1000; i++);
	}
	else{
		LCD_Clear_Sprite(Boss.x_pos, Boss.y_pos, Boss.bmp, Boss.sz);
		for(int i = 0; i<1000; i++);
	}
	return;
}

unsigned char dir1 = 0;		// 0 for left, 1 for right

void Move_boss(){
	
	Clear_boss();
	unsigned char x;

	if(dir1 == 1){			// 1 =  move right
		if(Boss.x_pos + 1 < LCD_WIDTH-43){
			x = Boss.x_pos;
			Boss.x_pos = x + 1;
		}
		else{
			dir1 = 0;
			return;
		}
	}
	else if(dir1 == 0){	// 0 = move left
		if(Boss.x_pos - 1 > 0){
			x = Boss.x_pos;
			Boss.x_pos = x - 1;
		}
		else{
			dir1 = 1;
			return;
		}
	}
	return;
}
