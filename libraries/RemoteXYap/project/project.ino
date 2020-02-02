/*
   -- ForestLamp --
   
   This source code of graphical user interface 
   has been generated automatically by RemoteXY editor.
   To compile this code using RemoteXY library 2.4.3 or later version 
   download by link http://remotexy.com/en/library/
   To connect using RemoteXY mobile app by link http://remotexy.com/en/download/                   
     - for ANDROID 4.3.1 or later version;
     - for iOS 1.3.5 or later version;
    
   This source code is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.    
*/

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__ESP8266WIFI_LIB
#include <ESP8266WiFi.h>

#include <RemoteXY.h>

// RemoteXY connection settings 
#define REMOTEXY_WIFI_SSID "YOUR_SSID"
#define REMOTEXY_WIFI_PASSWORD "YOUR_PASS"
#define REMOTEXY_SERVER_PORT 6377


// RemoteXY configurate  
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =
  { 255,9,0,31,0,243,0,8,215,4,
  130,2,40,2,58,21,2,22,6,0,
  43,5,54,54,1,2,26,131,3,3,
  21,35,10,1,29,215,208,159,209,128,
  208,190,209,129,209,130,208,190,0,5,
  5,42,11,47,47,3,2,29,202,131,
  2,3,49,35,9,2,30,201,208,173,
  209,132,209,132,208,181,208,186,209,130,
  209,139,0,2,1,4,3,33,15,0,
  2,26,31,31,208,146,208,154,208,155,
  0,208,146,209,139,208,186,208,187,0,
  131,2,3,35,35,9,3,30,201,208,
  145,208,190,208,186,208,176,0,4,128,
  41,9,57,8,2,2,201,67,0,43,
  17,52,5,2,201,215,31,129,0,57,
  3,24,6,2,201,208,173,209,132,209,
  132,208,181,208,186,209,130,0,130,2,
  40,26,58,14,2,23,4,128,41,30,
  57,9,2,15,201,129,0,60,26,21,
  5,2,201,208,175,209,128,208,186,208,
  190,209,129,209,130,209,140,0,130,1,
  40,43,58,16,2,124,4,128,41,48,
  57,8,2,135,201,129,0,56,44,28,
  6,2,201,208,161,208,186,208,190,209,
  128,208,190,209,129,209,130,209,140,0 };
  
// this structure defines all the variables of your control interface 
struct {

    // input variable
  uint8_t color_r; // =0..255 Red color value 
  uint8_t color_g; // =0..255 Green color value 
  uint8_t color_b; // =0..255 Blue color value 
  int8_t direction_x; // =-100..100 x-coordinate joystick position 
  int8_t direction_y; // =-100..100 y-coordinate joystick position 
  uint8_t luz; // =1 if switch ON and =0 if OFF 
  int8_t effect; // =0..100 slider position 
  int8_t brigtness; // =0..100 slider position 
  int8_t scale; // =0..100 slider position 

    // output variable
  char effectName[31];  // string UTF8 end zero 

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0 

} RemoteXY;
#pragma pack(pop)

/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////



void setup() 
{
  RemoteXY_Init (); 
  
  
  // TODO you setup code
  
}

void loop() 
{ 
  RemoteXY_Handler ();
  
  
  // TODO you loop code
  // use the RemoteXY structure for data transfer


}