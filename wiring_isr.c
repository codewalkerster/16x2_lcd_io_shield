#include <stdio.h>                                                              
#include <wiringPi.h>                                                           
 
int volatile btnCounter;                                                        
 
void myInterrupt(void)                                                          
{                                                                               
        btnCounter++;                                                           
}                                                                               
 
int main(void)                                                                  
{                                                                               
        if (wiringPiSetup() < 0) {                                              
                printf("wiringPiSetup error.\n");                               
                return 1;                                                       
        }                                                                       
        if (wiringPiISR(5, INT_EDGE_FALLING, &myInterrupt) < 0) {               
                printf("wiringPiISR error.\n");                                 
                return 1;                                                       
        }                                                                       
 
        while(1) {                                                              
                printf("%d\n", btnCounter);                                     
                delay(1000);                                                    
        }                                                                       
 
        return 0;                                                               
}
