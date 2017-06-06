#ifndef __pieces_h__
#define __pieces_h__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include "pcd8544.h"

void Set_up_shot();
void Clear_shot();
void Draw_shot();
unsigned char Move_shot();

void Set_up_enemy();
unsigned char Check_enemies();
void Clear_enemies();
void Draw_enemy();
void Move_enemy();

void Set_up_player();
void Clear_player();
void Draw_player();
void Move_player(unsigned char direction);

void Set_up_boss();
void Clear_boss();
void Draw_boss();
void Move_boss();

void Set_up_boss_shot();
void Clear_boss_shot();
void Draw_boss_shot();
unsigned char Move_boss_shot();

#endif
