//------------------------------------------------------------------------------------------------------------
//
// ODROID-XU4 printing CPU informations Test Application.
//
// Defined port number is wiringPi port number.
//
// Compile : gcc -o <create excute file name> <source file name> -lwiringPi -lwiringPiDev -lpthread
// Run : sudo ./<created excute file name>
//
//------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>

#include <unistd.h>
#include <string.h>
#include <time.h>

#include <ifaddrs.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <wiringSerial.h>
#include <lcd.h>

#include "lcd_16x2.h"

//------------------------------------------------------------------------------------------------------------
//
// Global handle Define
//
//------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------
//
// LCD:
//
//------------------------------------------------------------------------------------------------------------
#define LCD_UPDATE_PERIOD   300 // 300ms

static unsigned char lcdFb[LCD_ROW][LCD_COL] = {0, };

static int lcdHandle  = 0;

//
// DispMode
// 0 = date & time, 1 = ethrenet ip addr, 2 = cpu temperature
// 3 = freq governor. 4 = Big core frequency, 5 = Little core frequency
//
static int DispMode = 0;


//------------------------------------------------------------------------------------------------------------
//
// Button:
//
//------------------------------------------------------------------------------------------------------------
#define PORT_BUTTON1    5   // GPX1.7(#23)
#define PORT_BUTTON2    6   // GPX2.0(#24)
 
//------------------------------------------------------------------------------------------------------------
//
// LED:
//
//------------------------------------------------------------------------------------------------------------
static int ledPos = 0;

const int ledPorts[] = {
    21, // GPX2.4(#28)
    22, // GPX2.6(#30)
    23, // GPX2.7(#31)
    11, // GPX2.1(#25)
    26, // GPX2.5(#29)
    27, // GPX3.1(#33)
};

#define MAX_LED_CNT sizeof(ledPorts) / sizeof(ledPorts[0])

//------------------------------------------------------------------------------------------------------------
//
// DispMode
// 0 = date & time, 1 = ethrenet ip addr, 2 = cpu temperature
// 3 = freq governor. 4 = Big core frequency, 5 = Little core frequency
//
//------------------------------------------------------------------------------------------------------------
//
// Get little core freq(CPU4)
//
//------------------------------------------------------------------------------------------------------------
#define FD_LITTLECORE_FREQ "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"

static void get_littlecore_freq(void)
{
    int     n, fd, freq;
    char    buf[LCD_COL];

    memset(buf, ' ', sizeof(buf));

    if((fd = open(FD_LITTLECORE_FREQ, O_RDONLY)) < 0)   {
        fprintf(stderr, "%s : file open error!\n", __func__);
    } else {
        read(fd, buf, sizeof(buf));
        close(fd);
        freq = atoi(buf);
        n = sprintf(buf, "Little-Core Freq");
        strncpy(&lcdFb[0][0], buf, n);
        n = sprintf(buf, "%d Mhz", freq / 1000);
        strncpy(&lcdFb[1][4], buf, n);
    }
}

//------------------------------------------------------------------------------------------------------------
//
// Get big core freq(CPU4)
//
//------------------------------------------------------------------------------------------------------------
#define FD_BIGCORE_FREQ "/sys/devices/system/cpu/cpu4/cpufreq/scaling_cur_freq"

static void get_bigcore_freq(void)
{
    int     n, fd, freq;
    char    buf[LCD_COL];

    memset(buf, ' ', sizeof(buf));

    if((fd = open(FD_BIGCORE_FREQ, O_RDONLY)) < 0)   {
        fprintf(stderr, "%s : file open error!\n", __func__);
    } else {
        n = read(fd, buf, sizeof(buf));
        close(fd);
        freq = atoi(buf);
        n = sprintf(buf, "Big-Core Freq");
        strncpy(&lcdFb[0][0], buf, n);
        n = sprintf(buf, "%d Mhz", freq / 1000);
        strncpy(&lcdFb[1][4], buf, n);
    }
}

//------------------------------------------------------------------------------------------------------------
//
// Get system governor(CPU0)
//
//------------------------------------------------------------------------------------------------------------
#define FD_SYSTEM_GOVERNOR  "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"

static void get_system_governor(void)
{
    int     n, fd;
    char    buf[LCD_COL];

    memset(buf, ' ', sizeof(buf));

    if((fd = open(FD_SYSTEM_GOVERNOR, O_RDONLY)) < 0)   {
        fprintf(stderr, "%s : file open error!\n", __func__);
    } else {
        n = read(fd, buf, sizeof(buf));
        close(fd);
        strncpy(&lcdFb[1][2], buf, n -1);
        n = sprintf(buf, "SYSTEM Governor");
        strncpy(&lcdFb[0][0], buf, n);
    }
}

//------------------------------------------------------------------------------------------------------------
//
// Get ethernet ip addr
//
//------------------------------------------------------------------------------------------------------------
static void get_ethernet_ip(void)
{
    struct  ifaddrs *ifa;
    int     n;
    char    buf[LCD_COL];

    memset(buf, ' ', sizeof(buf));

    getifaddrs(&ifa);

    while(ifa)  {
        if(ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET)    {
            struct sockaddr_in *pAddr = (struct sockaddr_in *)ifa->ifa_addr;

            if(0==strncmp(ifa->ifa_name, "eth", 2)) {
                n = sprintf(buf, "My IP Addr(%s)", ifa->ifa_name);
                strncpy(&lcdFb[0][0], buf, n);

                n = sprintf(buf, "%s", inet_ntoa(pAddr->sin_addr));
                strncpy(&lcdFb[1][1], buf, n);
            }
        }
        ifa = ifa->ifa_next;
    }
    freeifaddrs(ifa);
}

//------------------------------------------------------------------------------------------------------------
//
// Get date & time
//
//------------------------------------------------------------------------------------------------------------
static void get_date_time(void)
{
    time_t      tm_time;
    struct tm   *st_time;
    char        buf[LCD_COL];
    int         n;
    
    memset(buf, ' ', sizeof(buf));

    time(&tm_time);     st_time = localtime( &tm_time);

    n = strftime(buf, LCD_COL, "%Y/%m/%d %a", st_time);
    strncpy(&lcdFb[0][0], buf, n);
    n = strftime(buf, LCD_COL, "%H:%M:%S %p", st_time);
    strncpy(&lcdFb[1][2], buf, n);
}

//------------------------------------------------------------------------------------------------------------
//
// Get CPU Temperature
//
//------------------------------------------------------------------------------------------------------------
#define FD_SYSTEM_TEMP  "/sys/class/thermal/thermal_zone0/temp"

static void get_cpu_temperature(void)
{
    int     fd, temp_C, temp_F, n;
    char    buf[LCD_COL];

    memset(buf, ' ', sizeof(buf));

    if((fd = open(FD_SYSTEM_TEMP, O_RDONLY)) < 0)    {
        fprintf(stderr, "%s : file open error!\n", __func__);
    } else {
        read(fd, buf, LCD_COL);
        close(fd);

        temp_C = atoi(buf) / 1000;
        temp_F = (temp_C * 18 + 320) / 10;

        n = sprintf(buf, "CPU Temperature");
        strncpy(&lcdFb[0][0], buf, n);
        n = sprintf(buf, "%3d *C, %3d.%1d *F", temp_C, temp_F, temp_F % 10);
        strncpy(&lcdFb[1][0], buf, n);
    }
}

//------------------------------------------------------------------------------------------------------------
//
// LCD Update Function:
//
//------------------------------------------------------------------------------------------------------------
static void lcd_update (void)
{
    int i, j;

    // lcd fb clear
    memset((void *)&lcdFb, ' ', sizeof(lcdFb));

    // lcd fb update
    switch(DispMode)    {
        default  :  DispMode = 0;
        case    0:  get_date_time();        break;
        case    1:  get_ethernet_ip();      break;
        case    2:  get_cpu_temperature();  break;
        case    3:  get_system_governor();  break;
        case    4:  get_bigcore_freq();     break;
        case    5:  get_littlecore_freq();  break;
    }

    for(i = 0; i < LCD_ROW; i++)    {

        lcdPosition (lcdHandle, 0, i);

        for(j = 0; j < LCD_COL; j++)    lcdPutchar(lcdHandle, lcdFb[i][j]);
    }
}
 
//------------------------------------------------------------------------------------------------------------
//
// system init
//
//------------------------------------------------------------------------------------------------------------
int system_init(void)
{
    int i;

    // LCD Init   
    lcdHandle = lcdInit (LCD_ROW, LCD_COL, LCD_BUS,
                         PORT_LCD_RS, PORT_LCD_E,
                         PORT_LCD_D4, PORT_LCD_D5, PORT_LCD_D6, PORT_LCD_D7, 0, 0, 0, 0);

    if(lcdHandle < 0)   {
        fprintf(stderr, "%s : lcdInit failed!\n", __func__);    return -1;
    }

    // GPIO Init(LED Port ALL Output)
    for(i = 0; i < MAX_LED_CNT; i++)    {
        pinMode (ledPorts[i], OUTPUT);  pullUpDnControl (PORT_BUTTON1, PUD_OFF);
    }

    // Button Pull Up Enable.
    pinMode (PORT_BUTTON1, INPUT);    pullUpDnControl (PORT_BUTTON1, PUD_UP);
    pinMode (PORT_BUTTON2, INPUT);    pullUpDnControl (PORT_BUTTON2, PUD_UP);

    return  0;
}

//------------------------------------------------------------------------------------------------------------
//
// board data update
//
//------------------------------------------------------------------------------------------------------------
void boardDataUpdate(void)
{
    int i;

    //  LED Control
    for(i = 0; i < MAX_LED_CNT; i++)    digitalWrite (ledPorts[i], 0); // LED All Clear

    // button status read
    if(!digitalRead (PORT_BUTTON1)) {
        if(DispMode)        DispMode--;
        else                DispMode = 5;
    }
    if(!digitalRead (PORT_BUTTON2)) {
        if(DispMode < 5)    DispMode++;
        else                DispMode = 0;
    }

    digitalWrite(ledPorts[DispMode], 1);
}

//------------------------------------------------------------------------------------------------------------
//
// Start Program
//
//------------------------------------------------------------------------------------------------------------
int main (int argc, char *argv[])
{
    int timer = 0 ;

    wiringPiSetup ();

    if (system_init() < 0)
    {
        fprintf (stderr, "%s: System Init failed\n", __func__);     return -1;
    }

    for(;;)    {

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

    return 0 ;
}

//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
