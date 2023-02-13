#!/bin/sh
rm -rf ./example-lcd ./lcd_time ./wiring_isr ./lcd_cpuinfo
gcc -o example-lcd example-lcd.c -lwiringPi -lwiringPiDev -lm -lpthread -lrt -lcrypt $(pkg-config --cflags --libs libwiringpi2)
gcc -o lcd_time lcd_time.c -lwiringPi -lwiringPiDev -lm -lpthread -lrt -lcrypt $(pkg-config --cflags --libs libwiringpi2)
gcc -o wiring_isr wiring_isr.c -lwiringPi -lwiringPiDev -lm -lpthread -lrt -lcrypt $(pkg-config --cflags --libs libwiringpi2)
gcc -o lcd_cpuinfo lcd_cpuinfo.c -lwiringPi -lwiringPiDev -lm -lpthread -lrt -lcrypt $(pkg-config --cflags --libs libwiringpi2)
