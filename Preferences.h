/* 
 * Preferences Utility Library
 * This library provided the simple utility class for Writing and Reading EEPROM data
 * Written by Dr.Paween Khoenkaw
 * Copyright (C) 2020 All right reserved.
 */
#ifndef Preferences_h
#define Preferences_h
#include "Arduino.h"


class Preferences{
  void (*empty)();
public:
    Preferences(byte* data,size_t data_size);    
    void setEmptyCallback(void (*callback)());
    void Write();
    void Read();
private:
byte* config_address;
size_t config_size;

};
#endif
