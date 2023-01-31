#!/bin/sh
gcc -o example-lcd example-lcd.c -lwiringPi -lwiringPiDev -lm -lpthread -lrt -lcrypt
gcc -o lcd_time lcd_time.c -lwiringPi -lwiringPiDev -lm -lpthread -lrt -lcrypt
gcc -o wiring_isr wiring_isr.c -lwiringPi -lwiringPiDev -lm -lpthread -lrt -lcrypt
