//============================================================================
// Name: NixieServer.cpp
// Author: Richard Johnson (changed based on original DisplayNixie.cpp program by GRA&AFCH, see below)
// Copyright: Free
// Description: Simple Nixie Clock server, allowing Nixie display to be driven by a script
//============================================================================


// Original description of program:
//============================================================================
// Name        : DisplayNixie.cpp
// Author      : GRA&AFCH
// Version     : v1.3
// Copyright   : Free
// Description : Display digits on shields
//============================================================================

#include <iostream>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <ctime>
#include <string.h>
#include <wiringPiI2C.h>
#include <time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <softPwm.h>

#define I2CAddress 0x68
#define I2CFlush 0
#define SECOND_REGISTER 0x0
#define MINUTE_REGISTER 0x1
#define HOUR_REGISTER 0x2

#define RED_LIGHT_PIN 28
#define GREEN_LIGHT_PIN 27
#define BLUE_LIGHT_PIN 29
#define MAX_POWER 100

using namespace std;
int LEpin=3;
int I2CFileDesc;
uint16_t SymbolArray[10]={1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
int tubes_qty=6;
int ldot=0;
int rdot=0;

int decToBcd(int val) {
	return ((val / 10  * 16) + (val % 10));
}

void writeRTCDate(tm date) {
	wiringPiI2CWrite(I2CFileDesc, I2CFlush);
	wiringPiI2CWriteReg8(I2CFileDesc,HOUR_REGISTER,decToBcd(date.tm_hour));
	wiringPiI2CWriteReg8(I2CFileDesc,MINUTE_REGISTER,decToBcd(date.tm_min));
	wiringPiI2CWriteReg8(I2CFileDesc,SECOND_REGISTER,decToBcd(date.tm_sec));
	wiringPiI2CWrite(I2CFileDesc, I2CFlush);
}

#define UPPER_DOTS_MASK 0x80000000 // left dots
#define LOWER_DOTS_MASK 0x40000000 // right dots
uint32_t addDots(uint32_t var, int l, int r) {
    
  if (!l) {
            var &=~UPPER_DOTS_MASK;
	    var &=~LOWER_DOTS_MASK;
  } else {
            var |=UPPER_DOTS_MASK; // right dots
	    var |=LOWER_DOTS_MASK;
  }
            
  if (!r) {
            var &=~LOWER_DOTS_MASK;
	    var &=~UPPER_DOTS_MASK;
  } else {
            var |=LOWER_DOTS_MASK; // left dots
	    var |=UPPER_DOTS_MASK;
  }

	return var;
}

uint32_t charToVal(char c) {
  uint32_t val;
  
  if (c == ' ') val = 0;
  else val = (SymbolArray[c - 0x30]);
  
  return (val);
}

uint32_t showDigits(char _stringToDisplay[]) {
	uint32_t Var32;
        uint32_t Words[2];

	pinMode(LEpin, OUTPUT);


	/*
	 * Each word is created as:
	 * a = 1 << byte[5]
	 * b = 1 << byte[4]
	 * c = 1 << byte[3]
	 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * |           a           |         b         |         c         |
	 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * d = 1 << byte[2]
	 * e = 1 << byte[1]
	 * f = 1 << byte[0]
	 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * |           d           |         e         |         f         |
	 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

	 * Two 32 bit words are split between 8 characters, like so:
	 * 
	 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * |       0       |       1       |       2       |       3       |
	 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * |       4       |       5       |       6       |       7       |
	 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *
	 * This is really just passing the two 4-byte words as an array of chars
	 * and should really be done using a cast of an array of 2 32-bit words..
         *
         * (The words need to be in network byte order, of course.)
	 */

	/*
	 * This loop works first with elements 5,4,3
	 * then elements 2,1,0
	 * of the _stringToDisplay array of 6 values
	 */
	int word_index;
	for (int i=0 , word_index = 0; i<tubes_qty /* +1 */; i=i+3, word_index++)
	{
	  uint32_t val;
	  int index;
	  Var32 = 0;

	  index = tubes_qty-i-1;
	  Var32 |= charToVal(_stringToDisplay[index]) << 20;
	  
	  index = tubes_qty-i-2;
	  Var32 |= charToVal(_stringToDisplay[index]) << 10;
	  
	  index = tubes_qty-i-3;
	  Var32 |= charToVal(_stringToDisplay[index]);

	  //	  if (i == 0)
	  //	    Var32 = addDots(Var32, 0, rdot);
	  //	  else
	  //	    Var32 = addDots(Var32, ldot, 0);
	      
	  Var32 = addDots(Var32, ldot, rdot);

          /*
	  buff[bufferIndex]=Var32>>24; // high 4 bits
	  bufferIndex++;
	  buff[bufferIndex]=Var32>>16; // left-mid 4 bits
	  bufferIndex++;
	  buff[bufferIndex]=Var32>>8; // right-mid 4 bits
	  bufferIndex++;
	  buff[bufferIndex]=Var32; // low order 4 bits
	  bufferIndex++;
          */
          
          Words[word_index] = htonl(Var32);
	}

	digitalWrite(LEpin, LOW);
//	wiringPiSPIDataRW(0, buff, 4*tubes_qty/3);
	wiringPiSPIDataRW(0, (unsigned char *)Words, 4*tubes_qty/3);
	digitalWrite(LEpin, HIGH);
}


int main(int argc, char* argv[]) {
    char *command;
      FILE *input;
    
      printf("Display Server\n\r");

	if (argc < 2)
	{
		printf("Enter digits to display... or commands: \n now - show current time, \n "
				//"clock - loop the program and update time every second, \n "
				"[digits] - and six or nine digits, \n "
				"settime x - set time, where x time in format [hh:mm:ss], \n "
				"setsystime - set current time from OS, \n "
				//"ledson - turn on RGB LEDs,\n "
				//"ledsoff - turn off RGB LEDs, \n "
				//"setledscolor x - set color of LEDs where x is color in [RRR:GGG:BBB] format, \n "
				//"setledsbright x - [0...255], \n "
				"? - show this help.");
		return 0;
	}
	wiringPiSetup ();

	if (wiringPiSPISetupMode (0, 2000000, 2)) printf("SPI ok\n\r");
			else {printf("SPI NOT ok\n\r"); return 0;}

	I2CFileDesc = wiringPiI2CSetup(I2CAddress);

	char _stringToDisplay[10];

        /* Switch stdin to non-blocking reads */
        int flags = fcntl(0, F_GETFL, 0);
        fcntl(0, F_SETFL, flags | O_NONBLOCK);

        command = argv[1];

        /* Init the LED lights under the digits */
	softPwmCreate(RED_LIGHT_PIN, 0, MAX_POWER);
	softPwmCreate(GREEN_LIGHT_PIN, 0, MAX_POWER);
	softPwmCreate(BLUE_LIGHT_PIN, 0, MAX_POWER);

	input = fdopen(0, "r");

  do {
      char buffer[60];
      int i;
      
      if (fgets(buffer, sizeof(buffer), input) > 0) {
//      if ((n = read(0, buffer, sizeof(buffer))) > 0) {
          // Remove trailing newline
	//	buffer[n-1] = '\0';
	i = strlen(buffer)-1;
	if (buffer[i] == '\n')
	  buffer[i] = '\0';
	printf("Doing command: '%s'\n", buffer);
	command = buffer;
      }

      if (!strncmp(command, "leds", 3)) {
          int red, blue, green;
          
          sscanf(command, "leds %d %d %d", &red, &green, &blue);
          softPwmWrite(RED_LIGHT_PIN, red);
          softPwmWrite(BLUE_LIGHT_PIN, blue);
          softPwmWrite(GREEN_LIGHT_PIN, green);

          continue;
      }
      

      if (!strncmp(command, "dots", 4)) {

          sscanf(command, "dots %d %d", &ldot, &rdot);
          continue;
      }
      
      if (!strncmp(command,"clock", 5))
	{
		time_t seconds=time(NULL);
		tm* timeinfo=localtime(&seconds);
		char* format="%H%M%S";
		strftime(_stringToDisplay, 8, format, timeinfo);
	}

      else if (!strncmp(command,"settime", 7))
		{
		_stringToDisplay[0]=argv[2][0];
		_stringToDisplay[1]=argv[2][1];
		_stringToDisplay[2]=argv[2][3];
		_stringToDisplay[3]=argv[2][4];
		_stringToDisplay[4]=argv[2][6];
		_stringToDisplay[5]=argv[2][7];
		tm time;
		time.tm_hour=10*((int)_stringToDisplay[0]-48)+(int)_stringToDisplay[1]-48;
		time.tm_min=10*((int)_stringToDisplay[2]-48)+(int)_stringToDisplay[3]-48;
		time.tm_sec=10*((int)_stringToDisplay[4]-48)+(int)_stringToDisplay[5]-48;
		writeRTCDate(time);
		}

      else if (!strncmp(command, "roulette", 8))
      {
          // Loop for a total of 10 seconds
          for (int i = 100 ; i ; i--) {
              int interval=10;
              // Each loop is 100 ms
              
		ldot=1;rdot=1;
              showDigits("000000");
              delay(interval);
		ldot=0;rdot=0;
              showDigits("111111");
              delay(interval);
		ldot=1;rdot=1;
              showDigits("222222");
              delay(interval);
		ldot=0;rdot=0;
              showDigits("333333");
              delay(interval);
		ldot=1;rdot=1;
              showDigits("444444");
              delay(interval);
		ldot=0;rdot=0;
              showDigits("555555");
              delay(interval);
		ldot=1;rdot=1;
              showDigits("666666");
              delay(interval);
		ldot=0;rdot=0;
              showDigits("777777");
              delay(interval);
		ldot=1;rdot=1;
              showDigits("888888");
              delay(interval);
		ldot=0;rdot=0;
              showDigits("999999");
              delay(interval);
          }
      }
      else if (!strncmp(command,"setsystime", 10))
		{
		time_t seconds=time(NULL);
		tm* time=localtime(&seconds);
		writeRTCDate(*time);
		char* format="%H%M%S";
		strftime(_stringToDisplay, 8, format, time);
		}

	else
	{
		tubes_qty=strlen(command);
		if ((tubes_qty != 6) && (tubes_qty != 9))
		{
			puts("Wrong length: must be 6 or 9 digits. \n");
//			return 0;
			delay(1000);
			continue;
		}
		for (int i=0; i<tubes_qty+1; i++)
		{
			_stringToDisplay[i]=command[i];
		}
	}

      showDigits(_stringToDisplay);

	delay(1000); // milliseconds
  } while (1);

	puts("Exit...");
	return 0;
}

