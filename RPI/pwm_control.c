#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pigpio.h>

#define PWM_1 13
#define PWM_2 18
#define FREQ 4000

int main(int argc, char *argv[])
{
   int i, g;

   if (gpioInitialise() < 0) #Initialize pigpio 
	   return -1;
   
   if (gpioSetPWMfrequency(PWM_1, FREQ) < 0) #Set PWM_1 pin to FREQ Hz
	   return -1;

   if (gpioSetPWMfrequency(PWM_2, FREQ) < 0) #Set PWM_2 pin to FREQ Hz
	   return -1;
	   
	   
