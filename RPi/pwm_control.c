#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <pigpio.h>
#include "accel.h"

// Pi pin declarations
#define PWM_1 12
#define PWM_1D 16
#define PWM_2 13
#define PWM_2D 6
#define FREQ 4000
#define RELAY 26

// Serial communication declarations
#define SERTTY "/dev/ttyAMA0"
#define BAUD_RATE 115200
#define BUF_SIZE 100

#define DEBUG_MODE 0 // Set to 0 when using for real
/* 
	Author: Richard Muri (rmuri@umassd.edu)
	This program is written for the Raspberry Pi 2 to control 2 dc motors,
	read an ADXL345 accelerometer, communicate with an ESP8266, and control a
	relay attached to a lawnmower. All pinouts are listed in BCM format
	(see https://pinout.xyz). The pigpio library is used
	(See https://abyz.co.uk/rpi/pigpio/cif.html).
	
	This intended to be used in conjunction with an init script installed in 
	/etc/init.d
	Compile with the following command: gcc -Wall -pthread -o pwm_control pwm_control.c -lpigpio -lrt
*/

// Relay controls power of mower, pwd2d is pwm channel 2 direction, pwm2 is pwm channel 2
bool relay, pwm2d, pwm1d;
int pwm2, pwm1;
char buf[BUF_SIZE]; // String buffer that holds input from serial pins

// Sets all lawnmower control globals to off
void shutdown()
{
   relay = 0;
   pwm1d = 0;
   pwm2d = 0;
   pwm1 = 0;
   pwm2 = 0;
}

// Function to parse serial information. Serial commands are a sequence of 7 characters
// The 0th char is an &, the first is a hex value from 0-7 with the leftmost bit representing
// pwm2d, the middle bit pwm1d, and the rightmost bit representing relay. The second and third
// are hex 00-FF describing PWM2. The fourth and fifth chars are 0x00-FF representing PWM1. The sixth
// character is an & representing the end of message. The global buffer is parsed and broken
// into global control variables.
void parseBuf(int shand)
{
   if(buf[0] != '&' || buf[6] != '@')
   {
	   if(DEBUG_MODE)
	   {
		printf("Error: message header and footer not found in buffer\n");
	   }
	   // Buffer had garbage in it, likely we need to clear the rest of the buffer
	   tcflush(shand, TCIFLUSH);
	   //serRead(shand, &buf[0], serDataAvailable(shand));
	   shutdown();
	   return;
   }
   else
   {
	   // Split buf into temporary arrays
	   char tmp1[3], tmp2[3];
	   strncpy(tmp1, &buf[4], 2); 
	   strncpy(tmp2, &buf[2], 2);
	   
	   // Convert temporary array from hex char to integer
	   pwm2 = strtol(tmp1, '\0', 16); // chars 2+3 are PWM2
	   pwm1 = strtol(tmp2, '\0', 16); // chars 4+5 are PWM1
	   
	   // If either direction is out of range, reset it to 0
	   if(pwm2 > 255 || pwm2 < 0)
	   {
		   pwm2 = 0;
	   }
	   
	   if(pwm1 > 255 || pwm1 < 0)
	   {
		   pwm1 = 0;
	   }
	   
	   // Char 1 is a value from 0-7 describing 
	   switch(buf[1])
	   {
		   case '0': // 000
		   		   relay = 0;
				   pwm2d = 0;
		           pwm1d = 0;
		   break;
		   
		   case '1': // 001
		   		   relay = 0;
				   pwm2d = 0;
		           pwm1d = 1;
		   break;
		   
		   case '2': // 010
				   relay = 0;
				   pwm2d = 1;
		           pwm1d = 0;
		   break;
		   
		   case '3': // 011
		   		   relay = 0;
				   pwm2d = 1;
		           pwm1d = 1;
		   break;
		   
		   case '4': // 100
		   		   relay = 1;
				   pwm2d = 0;
		           pwm1d = 0;
		   break;
		   
		   case '5': // 101
		   		   relay = 1;
				   pwm2d = 0;
		           pwm1d = 1;
		   break;
		   
		   case '6': // 110
		   		   relay = 1;
				   pwm2d = 1;
		           pwm1d = 0;
		   break;
		   
		   case '7': // 111
		   		   relay = 1;
				   pwm2d = 1;
		           pwm1d = 1;
		   break;
		   
		   default: // Don't recognize the character, turn everything off
		   relay = 0;
		   pwm2d = 0;
		   pwm1d = 0;
	   
		}
   }
}

int main(int argc, char *argv[])
{
   // Start lawnmower control variables as off
   shutdown();
   
   // Serial port declarations
   int shand; // shand is serial handle
   
   // I2C declarations
   int fd; // file descriptor for accelerometer I2C port
   char buf2[16]; // Buffer that holds input from I2C pins
   float xa, ya, za; // X, Y, and Z angles
   
   // Open file descriptor for adxl345 on I2C
   // Also configure accelerometer options (see https://www.sparkfun.com/datasheets/Sensors/Accelerometer/ADXL345.pdf)
   fd = initAccelerometer();
   
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
   
   printf("Successfully started program \n");
   
   // Begin main loop
   while(1)
   {
	   
	   // Check number of bytes available on serial port and read if >7
	   // Output is stored in buf
	   if(serDataAvailable(shand) >= 7)
	   {
		   //printf("Had 7 bytes");
		   serRead(shand, &buf[0], BUF_SIZE);
		   parseBuf(shand);
	   }
	   
	   // Store accelerometer angles in xa, ya, and za
	   //printf("Reading accelerometer at file handle %d:\n", fd);
	   readAccelerometer(fd, buf2, &xa, &ya, &za);
	   
	   // Debugging information
	   if(DEBUG_MODE)
	   {
		   printf("The serial buffer reads: %s\n", buf);
		   printf("PWM1: %d, PWM2: %d, PWM1d: %d, PWM2d: %d, RELAY: %d\n", 
				pwm1, pwm2, pwm1d, pwm2d, relay);
		   printf("The accelerometer reads x: %lf, y: %lf, z: %lf\n\n", xa, ya, za);
	   }
	   
	   // Set PWM directions
	   gpioWrite(PWM_1D, pwm1d);
	   gpioWrite(PWM_2D, pwm2d);
	   
	   // Set blade mode
	   gpioWrite(RELAY, relay);
	   	   
	   // Set PWM channel speeds
	   gpioPWM(PWM_1, pwm1);
	   gpioPWM(PWM_2, pwm2);
	   
	   buf[0] = '\0'; //clear the buffer
	   
	   usleep(1000); // wait for 1 ms 
	   if(DEBUG_MODE)
	   {
		sleep(5); // brief pause for debugging
	   }
   }
}   

	   
