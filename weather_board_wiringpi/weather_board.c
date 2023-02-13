#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <wiringSerial.h>
#include <lcd.h>

#include "bme280-i2c.h"
#include "si1132.h"
#include "../lcd_16x2.h"

#define LCD_UPDATE_PERIOD   300 // 300ms

static unsigned char lcdFb[LCD_ROW][LCD_COL] = {0, };

static int lcdHandle  = 0;

#define PORT_BUTTON1    5   // GPX1.7(#23)
#define PORT_BUTTON2    6   // GPX2.0(#24)
//
// DispMode
// 0 = UV_index, Visible, IR
// 1 = temperature, humidity
// 2 = pressure, altitude
//
static int DispMode = 0;

static int pressure;
static int temperature;
static int humidity;
static float altitude;

float SEALEVELPRESSURE_HPA = 1024.25;

void getSi1132Data() {
	char buf[LCD_COL];
	int n;

	memset(buf, ' ', sizeof(buf));

	n = sprintf(buf, "UV index %.2f", Si1132_readUV()/100.0);
	strncpy(&lcdFb[0][0], buf, n);
	n = sprintf(buf, "Vis %.0f IR %.0f L", Si1132_readVisible(), Si1132_readIR());
	strncpy(&lcdFb[1][0], buf, n);
}

void getBme280data1() {
	char buf[LCD_COL];
	int n;

	memset(buf, ' ', sizeof(buf));

	bme280_read_pressure_temperature_humidity(
		&pressure, &temperature, &humidity);
	n = sprintf(buf, "temp %.2lf 'C", (double)temperature/100.0);
	strncpy(&lcdFb[0][0], buf, n);
	n = sprintf(buf, "humi %.2lf %%", (double)humidity/1024.0);
	strncpy(&lcdFb[1][0], buf, n);
}

void getBme280data2() {
	char buf[LCD_COL];
	int n;

	memset(buf, ' ', sizeof(buf));

	n = sprintf(buf, "pres %.2lf hPa", (double)pressure/100.0);
	strncpy(&lcdFb[0][0], buf, n);
	n = sprintf(buf, "alti %f m", bme280_readAltitude(pressure,
		SEALEVELPRESSURE_HPA));
	strncpy(&lcdFb[1][0], buf, n);
}

static void lcd_update (void)
{
	int i, j;

	// lcd fb clear
	memset((void *)&lcdFb, ' ', sizeof(lcdFb));

	// lcd fb update
	switch(DispMode)    {
		case 0:
			getSi1132Data();
		break;
		case 1:
			getBme280data1();
		break;
		case 2:
			getBme280data2();
		break;
		default:
			DispMode = 0;
	}

	for(i = 0; i < LCD_ROW; i++) {
		lcdPosition (lcdHandle, 0, i);
		for(j = 0; j < LCD_COL; j++)
			lcdPutchar(lcdHandle, lcdFb[i][j]);
	}
}
 
void boardDataUpdate(void)
{
	int i;

	// button status read
	if(!digitalRead (PORT_BUTTON1)) {
		if(DispMode)
			DispMode--;
		else
			DispMode = 5;
	}
	if(!digitalRead (PORT_BUTTON2)) {
		if(DispMode < 5)
			DispMode++;
		else
			DispMode = 0;
	}
}

int system_init(void)
{
	int i;

	// LCD Init   
	lcdHandle = lcdInit (LCD_ROW, LCD_COL, LCD_BUS,
						 PORT_LCD_RS, PORT_LCD_E,
						 PORT_LCD_D4, PORT_LCD_D5, PORT_LCD_D6, PORT_LCD_D7, 0, 0, 0, 0);

	if (lcdHandle < 0) {
		fprintf(stderr, "%s : lcdInit failed!\n", __func__);
		return -1;
	}

	// Button Pull Up Enable.
	pinMode (PORT_BUTTON1, INPUT);
	pullUpDnControl (PORT_BUTTON1, PUD_UP);
	pinMode (PORT_BUTTON2, INPUT);
	pullUpDnControl (PORT_BUTTON2, PUD_UP);

	return  0;
}

int main(int argc, char **argv)
{
	int status = 0;
	char *device = "/dev/i2c-1";
	int timer = 0 ;

	if (argc == 2) {
		device = argv[1];
	} else if (argc > 2) {
		printf("Usage :\n");
		printf("sudo ./weather_board [i2c node](default \"/dev/i2c-1\")\n");
		return -1;
	}

	si1132_begin(device);
	bme280_begin(device);

	wiringPiSetup();

	if (system_init() < 0) {
		fprintf (stderr, "%s: System Init failed\n", __func__);
		return -1;
	}

	while(1) {

		if (millis () < timer)  {
			usleep(100000);    // 100ms sleep state
			continue ;
		}

		timer = millis () + LCD_UPDATE_PERIOD;

		// All Data update
		boardDataUpdate();

		// lcd update
		lcd_update();
	}

	return 0;
}
