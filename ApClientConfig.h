/* 
 * wifi connection configuration Library
 * This library provided the simple utility class for Wifi connection configuration.
 * Written by Dr.Paween Khoenkaw
 * Copyright (C) 2020 All right reserved.
 */

#ifndef ApClientConfig_h
#define ApClientConfig_h

#include "Arduino.h"

class ApClientConfig{
  void (*Done)();
  public:
  ApClientConfig(char* Apssid,char* Appassword,char* UI_user,char* UI_password,char* ClientSSID,char* ClientPassword);
 void Begin();
 void setDoneCallback(void (*callback)());
 
  private:
  
  };

#endif
