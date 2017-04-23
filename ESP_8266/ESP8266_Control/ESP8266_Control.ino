#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
extern "C" {
#include "user_interface.h"
}
//////////////////////
// WiFi Definitions //
//////////////////////
const char WIFI_NAME[] = "LAWNMOWER";
const char WIFI_KEY[] = "tregtreg";
const int CHANNEL = 13; // Don't change this unless you know what you're doing

/////////////////////////
// Hardware Definition //
/////////////////////////
const int BAUD = 115200;
const int TIME = 2000; // Interrupt timer period in ms
boolean FLAG = false; // Flag set by interrupt service routine

WiFiUDP Udp; // UDP server
const unsigned int localUdpPort = 4210;  // local port to listen on

os_timer_t myTimer;
char reply[] = "ACK\r\n"; // Default response character
char buf[10]; // Buffer to store client requests. See pwm_control.c for actual commands
char accel_reading[1]; // Reading from accelerometer as communicated by Pi via serial

// Timer interrupt callback function
// This function ensures a client is still connected;
// if the client becomes disconnected, it will reestablish connection.
// The lawnmower is turned off on disconnect for safety reasons
void timerCallback(void *pArg) {

  //Serial.println(WiFi.softAPgetStationNum());
	if(!FLAG) // If flag is already set, service routine must already be in progress
	{
	  // Is there a device connected to our access point?
	  if(WiFi.softAPgetStationNum() >= 1) // Yes
	  {
		// Do nothing
		//Serial.println("Client is connected");
      return ;
	  }
	  else // No, set flag to trigger service routine in main loop
	  {
		FLAG = true;
	  }
	}
  else // Flag already set, do nothing more
  {
    return ;
  } 
}

// Creates timer object and managers interrupt settings attached to timer
void user_init(void) {
 /*
  os_timer_setfn - Define a function to be called when the timer fires

void os_timer_setfn(
      os_timer_t *pTimer,
      os_timer_func_t *pFunction,
      void *pArg)

Define the callback function that will be called when the timer reaches zero. The pTimer parameters is a pointer to the timer control structure.

The pFunction parameters is a pointer to the callback function.

The pArg parameter is a value that will be passed into the called back function. The callback function should have the signature:
void (*functionName)(void *pArg)

The pArg parameter is the value registered with the callback function.
*/

      os_timer_setfn(&myTimer, timerCallback, NULL);

/*
      os_timer_arm -  Enable a millisecond granularity timer.

void os_timer_arm(
      os_timer_t *pTimer,
      uint32_t milliseconds,
      bool repeat)

Arm a timer such that is starts ticking and fires when the clock reaches zero.

The pTimer parameter is a pointed to a timer control structure.
The milliseconds parameter is the duration of the timer measured in milliseconds. The repeat parameter is whether or not the timer will restart once it has reached zero.

*/

      os_timer_arm(&myTimer, TIME, true);

}

void setup() 
{
  // Establishes proper baud rate
  initHardware();
  
  // Turns on WiFi access point
  setupWiFi();

  // Launch UDP server
  Udp.begin(localUdpPort);
  //Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);

  // Start timer functions
  user_init();
}

void loop() 
{
  // If flag is set, client computer is disconnected and needs to be serviced
  if(FLAG)
  {
	  interruptService();
  }
  // If accelerometer output is added, it will be on the line below this comment
  //Serial.readBytes(accel_reading, sizeof(char));
  
  udpRecieveSend();
  delay(0);
}

// create WiFi object that can be accessed at 192.168.4.1
void setupWiFi()
{
  Serial.println("Starting access point");
  // Define WiFi type as Access point
  WiFi.mode(WIFI_AP);
  
  // Define WiFi object as a soft access point. See 
  // https://github.com/esp8266/Arduino/blob/master/doc/esp8266wifi/soft-access-point-class.md
  // for full documentation
  WiFi.softAP(WIFI_NAME, WIFI_KEY, CHANNEL, false);
}

// create serial object
void initHardware()
{
  Serial.begin(BAUD);
}

void udpRecieveSend()
{
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    // receive incoming UDP packets
    //Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(buf, 10);
    if (len > 0)
    {
      buf[len] = 0;
    }
    Serial.printf(buf);

    // send back a reply, to the IP address and port we got the packet from
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(reply);
    Udp.endPacket();
  }
}

// This routine is called when the client computer gets disconnected from ESP8266
void interruptService()
{
    // & used as a header
    // The second byte is 0 to turn off relay and reset wheel directions
    // Bytes 3 and 4 represent 0x00 to turn Wheel 2 off
    // Bytes 5 and 6 represent 0x00 to turn Wheel 1 off
    // @ used as an EOM
    buf[0] = '&';
    buf[1] = '0';
    buf[2] = '0';
    buf[3] = '0';
    buf[4] = '0';
    buf[5] = '0';
    buf[6] = '@';
    buf[7] = '\0';
    Serial.println(buf);

    // Restart the WiFi access point
    setupWiFi();

    while(WiFi.softAPgetStationNum() < 1)
    {
      delay(2000);
      //Serial.println("Attempting to reconnect to client");
    }
  Serial.println("Successfully reconnected");
	FLAG = false;
}
