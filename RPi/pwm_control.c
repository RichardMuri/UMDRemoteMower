#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <pigpio.h>

#define PWM_1 12
#define PWM_1D 16
#define PWM_2 13
#define PWM_2D 6
#define FREQ 4000
#define RELAY 26

#define SERTTY "/dev/ttyAMA0"
#define BAUD_RATE 115200
#define BUF_SIZE 100
/* 
	Author: Richard Muri (rmuri@umassd.edu)
	This program is written for the Raspberry Pi 2 to control 2 dc motors,
	read an ADXL345 accelerometer, communicate with an ESP8266, and control a
	relay attached to a lawnmower. All pinouts are listed in BCM format
	(see https://pinout.xyz). The pigpio library is used
	(See abyz.co.uk/rpi/pigpio/cif.html).
	
	This intended to be used in conjunction with an init script installed in 
	/etc/init.d

*/

// Relay controls power of mower, pwd2d is pwm channel 2 direction, pwm2 is pwm channel 2
bool relay, pwm2d, pwm1d;
int pwm2, pwm1;
char buf[BUF_SIZE]; // String buffer that holds input from serial pins

/*
Test sequence of &g:?@ sets PWM1D, PWM2D, and RELAY to true and PWM1 to 63, PWM2 58
    buf[0] = 0x26;
    buf[1] = 0x67;
    buf[2] = 0x3A;
    buf[3] = 0X3F;
    buf[4] = 0x40;
    buf[5] = '\0';
*/

// Function to parse serial information. Serial commands are a sequence of 5 characters
// The first char is an &, the second uses the rightmost bit to represent pwm1d, next bit
// to represent pwd2d, third bit relay status. The third char is PWM2 speed from 0-255, 
// fourth char PWM1 speed, and fifth char is an @. The global buffer is parsed and broken
// into global control variables.
void parseBuf(void)
{
   if(buf[0] != '&' || buf[4] != '@')
   {
	   printf("Error: message header and footer not found in buffer\n");
   }
   
	pwm1 = buf[3] & 0xFF;
	pwm2 = buf[2] & 0xFF;
	pwm1d = buf[1] & 0x01;
	pwm2d = buf[1] & 0x02;
	relay = buf[1] & 0x03;
	
}

int main(int argc, char *argv[])
{
   int i, g, shand; // shand is serial handle
   unsigned test = 80;
      
   // Prevents the operating system from using the serial port
   if(system("sudo systemctl stop serial-getty@ttyAMA0.service") < 0)
   {
	   printf("Error: Unable to close serial interface\n");
	   return -1;
   }
      
   // Initialize pigpio library
   if (gpioInitialise() < 0)
   {	   
	   printf("Error: Unable to initialize pigpio\n");
	   return -1;
   }
   
   // Set the PWM frequency on PWM_1 to a predefined value (The intended motor 
   // drivers allow up to 20kHz)
   if (gpioSetPWMfrequency(PWM_1, FREQ) < 0) 
   {
	   printf("Error: Unable to set PWM_1\n");
	   return -1;
   }
   // Set the PWM frequency on PWM_2 to a predefined value (The intended motor 
   // drivers allow up to 20kHz)
   if (gpioSetPWMfrequency(PWM_2, FREQ) < 0)
   {
	   printf("Error: Unable to set PWM_2\n");
	   return -1;
   }
   
   // Opens serial port and assigns handle to shand. Return if open failed
   shand = serOpen(SERTTY, BAUD_RATE, 0);
   if(shand < 0)
   {
	   printf("Error code %d: Unable to assign serial handle\n", shand);
	   return -1;
   }
   
   // Begin main loop
   while(1)
   {
	   printf("Reading serial buffer:\n");
	   // Check number of bytes available on serial port and read if >10
	   // Output is stored in buf
	   if(serDataAvailable(shand) >= 1)
	   {
		   serRead(shand, &buf[0], BUF_SIZE);
		   parseBuf();
	   }
	   
	   // Debugging information
	   //serWriteByte(shand, test);
	   printf("The serial buffer reads: %s\n", buf);
	   printf("PWM1: %d, PWM2: %d, PWM1d: %d, PWM2d: %d, RELAY: %d", 
			pwm1, pwm2, pwm1d, pwm2d, relay);
		
	   // Set PWM directions
	   
	   // Set PWM channel speeds
	   gpioPWM(PWM_1, pwm1);
	   gpioPWM(PWM_2, pwm2);
	   
	   buf[0] = '\0'; //clear the buffer
	   sleep(5);	   
		   
   }
}   

	   
