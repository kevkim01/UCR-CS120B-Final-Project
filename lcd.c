/*
 * Nokia5110.c
 *
 * Created: 3/4/2016 6:02:39 PM
 *  Author: Kevin
 */ 

#include <avr/io.h>
#include "pcd8544.h"
#include "pcd8544.c"


int main(void)
{
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

	LCDInit();
	LCDClear();
	while(1)
    {
        //TODO:: Please write your application code
		
		LCDBitmap(menu);
		for(int i = 0; i<1000; i++);

    }
}
