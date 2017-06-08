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
unsigned char Check_player();
void Clear_player();
void Draw_player();
void Move_player(unsigned char direction);

void Set_up_boss();
void Clear_boss();
void Draw_boss();
void Move_boss();

void Set_up_boss_shot();
unsigned char Check_boss();
void Clear_boss_shot();
void Draw_boss_shot();
unsigned char Move_boss_shot();

void Set_up_pawn_shot1();
void Set_up_pawn_shot2();
void Set_up_pawn_shot3();

void Clear_pawn_shot1();
void Clear_pawn_shot2();
void Clear_pawn_shot3();

void Draw_pawn_shot1();
void Draw_pawn_shot2();
void Draw_pawn_shot3();

unsigned char Move_pawn_shot1();
unsigned char Move_pawn_shot2();
unsigned char Move_pawn_shot3();

unsigned char Check_for_fire1();
unsigned char Check_for_fire2();
unsigned char Check_for_fire3();
#endif
