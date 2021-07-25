/* 
 * Preferences Utility Library
 * This library provided the simple utility class for Writing and Reading EEPROM data
 * Written by Dr.Paween Khoenkaw
 * Copyright (C) 2020 All right reserved.
 */

#include "Preferences.h"
#include "EEPROM.h"
Preferences::Preferences(byte* data,size_t data_size){
 //   Serial.println(data_size);
  // Serial.println((char*)data);
   config_address=data;
   config_size=data_size;
   
}
void Preferences::setEmptyCallback(void(*callback)())
{
  empty=callback;
 // Serial.println("callback registered");
}

void Preferences::Write()
{ char Header[]="CONFIG_FORMAT";
#if defined(ARDUINO_ARCH_ESP32)
  EEPROM.begin(config_size+sizeof(Header));
#endif  
 for(int i=0;i<config_size;i++){
  EEPROM.write(i,*(config_address+i));
  }
  int j=0;
 for(int i=config_size;i<sizeof(Header)+config_size;i++){
  EEPROM.write(i,Header[j]);
  j++;
  }
#if defined(ARDUINO_ARCH_ESP32)  
  EEPROM.commit();
#endif
  Serial.println("Write to EEPROM");
  }

void Preferences::Read(){
  char Header[]="CONFIG_FORMAT";
  char TEMP[sizeof(Header)];
#if defined(ARDUINO_ARCH_ESP32)
  EEPROM.begin(config_size+sizeof(Header));
#endif
  byte* j=config_address;
 for(int i=0;i<config_size;i++){
  *j=EEPROM.read(i);
  j++;
  //Serial.println(TEMP[i]);
  }
 int k=0;
 for(int i=config_size;i<sizeof(TEMP)+config_size;i++){
 TEMP[k]=EEPROM.read(i);
  k++;
 }
if(strcmp(TEMP,Header))empty();
  
Serial.println("Read from EEProm");
  
  }
