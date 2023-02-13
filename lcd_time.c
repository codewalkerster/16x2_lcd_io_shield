//------------------------------------------------------------------------------------------------
//
// ODROID-C printing time Application.
//
// Defined port number is wiringPi port number.
//
// Compile : gcc -o <create excute file name> <source file name> -lwiringPi -lwiringPiDev -lpthread
// Run : sudo ./<created excute file name>
//
//-------------------------------------------------------------------------------------------------

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>
#include <string.h>
#include <time.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <wiringSerial.h>
#include <lcd.h>

#include "lcd_16x2.h"

#define LCD_UPDATE_PERIOD   1000 // 300ms

static unsigned char lcdFb[LCD_ROW][LCD_COL] = {0, };

static int lcdHandle  = 0;

static int ledPos = 0;

static void lcd_update (void)
{
	int i, j;
	time_t t;
	time(&t);
 
	memset((void *)&lcdFb, ' ', sizeof(lcdFb));
 
	sprintf(lcdFb[0], "Time %s", ctime(&t));
	lcdFb[0][strlen(lcdFb[0])-1] = ' ';
	lcdFb[0][strlen(lcdFb[0])] = ' ';
 
	for(i = 0; i < LCD_ROW; i++) {
		lcdPosition (lcdHandle, 0, i);
		for(j = 0; j < LCD_COL; j++)
			lcdPutchar(lcdHandle, lcdFb[i][j]);
	}
}
 
int system_init(void)
{
	int i, j;
 
	// LCD Init   
	lcdHandle = lcdInit(LCD_ROW, LCD_COL, LCD_BUS,
				PORT_LCD_RS, PORT_LCD_E,
				PORT_LCD_D4, PORT_LCD_D5,
				PORT_LCD_D6, PORT_LCD_D7, 0, 0, 0, 0);
 
	if(lcdHandle < 0) {
		fprintf(stderr, "%s : lcdInit failed!\n", __func__);
		return -1;
	}
	return  0;
}
 
int main (int argc, char *argv[])
{
	int timer = 0;
 
	wiringPiSetup();
 
	if (system_init() < 0) {
		fprintf (stderr, "%s: System Init failed\n", __func__);
		return -1;
	}
 
	for(;;) {
                usleep(100000);
		if (millis () < timer)
			continue;
		timer = millis () + LCD_UPDATE_PERIOD;
 
		// lcd update
		lcd_update();
	}
 
	return 0 ;
}
