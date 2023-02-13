#include <stdio.h>
#include <wiringPi.h>

int volatile btnCounter;

void addInterrupt()
{
    btnCounter++;
}

void subInterrupt()
{
    btnCounter--;
}

int main()
{
    if (wiringPiSetup() < 0) {
        printf("wiringPiSetup error.\n");
        return 1;
    }
    if (wiringPiISR(5, INT_EDGE_FALLING, &addInterrupt) < 0) {
        printf("wiringPiISR error.\n");
        return 1;
    }
    if (wiringPiISR(6, INT_EDGE_FALLING, &subInterrupt) < 0) {
        printf("wiringPiISR error.\n");
        return 1;
    }

    while(1) {
        printf("%d\n", btnCounter);
        delay(1000);
    }

    return 0;
}
