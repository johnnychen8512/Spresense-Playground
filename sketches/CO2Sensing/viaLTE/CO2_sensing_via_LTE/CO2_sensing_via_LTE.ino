/*
 *  CO2_sensing.ino - CO2 Sensing for Emviroment to upload the Ambient via LTE.
 *  Copyright 2020 Sony Semiconductor Solutions Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <Arduino.h>
#include <Wire.h>
#include <File.h>
#include <Flash.h>

/***   Select Mode   ***/
#define USE_OLED
#define UPLOAD_AMBIENT

#ifdef UPLOAD_AMBIENT
#include <Arduino.h>
#include "Ambient_SpresenseLTEM.h"
#endif

#ifdef USE_OLED
#include "SparkFun_SCD30_Arduino_Library.h" 
#include "U8g2lib.h"
U8G2_SSD1327_EA_W128128_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#endif


/* Change the settings according to your sim card */
#define APN_NAME "XXXXXX"
#define APN_USRNAME "XXXXXX"
#define APN_PASSWD "XXXXXX"

SCD30 airSensor;

const char* apn_file  = "apn.txt";
const char* name_file = "name.txt";
const char* pass_file = "pass.txt";
const char* id_file   = "id.txt";
const char* key_file  = "key.txt";
const char* sens_file = "sens.txt";
const char* up_file   = "up.txt";

static String apnName = "xxxxxxxx";
static String usrName = "xxxxxxxx";
static String apnPass = "xxxxxxxx";

static uint16_t channelId = 00000;
static String writeKey  = "xxxxxxxxxxxxx";

static int setting_mode_time = 500; /* 5000ms */

static uint16_t sensing_interval = 5; /* 5s */
static uint16_t upload_interval  = 6; /* 1/n */

#define STRING_BUFFER_SIZE 128
char StringBuffer[STRING_BUFFER_SIZE];

bool settingUint16(const char* name, uint16_t& data)
{
  Serial.print(data);
  Serial.println(") : ");
  while(!Serial.available()); 
  String tmp = Serial.readString();
  if(tmp.length() > 1){
    Flash.remove(name);
    File settingFile = Flash.open(name, FILE_WRITE);
    if(settingFile==0){    
     return false;
    }
    data = tmp.toInt();
    settingFile.write(tmp.c_str());
    settingFile.close();
  }
  Serial.println(data);
  return true;
}

bool settingString(const char* name, String& data)
{
  Serial.print(data);
  Serial.println(") : ");
  while(!Serial.available()); 
  String tmp = Serial.readString();
  if(tmp.length() > 1){
    Flash.remove(name);
    File settingFile = Flash.open(name, FILE_WRITE);
    if(settingFile==0){    
     return false;
    }
    tmp.trim();
    data = tmp;
    settingFile.write(data.c_str());
    settingFile.close();
  }
  Serial.println(data);
  return true;
}

void setting()
{
  Serial.print("Please input Apn Name(");
  if(!settingString(apn_file, apnName)){ goto ERROR_EXIT; }

  Serial.print("Please input User Name(");
  if(!settingString(name_file, usrName)){ goto ERROR_EXIT; }
  
  Serial.print("Please input Apn Passphase(");
  if(!settingString(pass_file, apnPass)){ goto ERROR_EXIT; }
  
  Serial.print("Please input Ambient Channel ID(");
  if(!settingUint16(id_file, channelId)){ goto ERROR_EXIT; }
  
  Serial.print("Please input Ambient writeKey(");
  if(!settingString(key_file, writeKey)){ goto ERROR_EXIT; }

  Serial.print("Please input Sensing Interval(sec) (");
  if(!settingUint16(sens_file, sensing_interval)){ goto ERROR_EXIT; }

  Serial.print("Please input Sensing Interval(1/n) (");
  if(!settingUint16(up_file, upload_interval)){ goto ERROR_EXIT; }

  return;

ERROR_EXIT:
  Serial.println("Setting Error!");
  exit(1);
}

bool readUint16(const char* name, uint16_t& data)
{
  Serial.println(name);
  File settingFile = Flash.open(name, FILE_READ);
  if(settingFile.available()>0) {
    data = settingFile.readString().toInt();
    Serial.println(data);
  }else{
    return false;
  }
  settingFile.close();
  return true;
}

bool readString(const char* name, String& data)
{
  Serial.println(name);
  File settingFile = Flash.open(name, FILE_READ);
  if(settingFile.available()>0) {
    data = settingFile.readString();
    Serial.println(data);
  }else{
    return false;
  }
  settingFile.close();
  return true;
}

bool drawSettingMode()
{
#ifdef USE_OLED
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(20,40,"Device Booting... ");
  u8g2.sendBuffer();
#endif

  if(!readString(apn_file, apnName)){ goto ERROR_RETURN; }
  if(!readString(name_file, usrName)){ goto ERROR_RETURN; }
  if(!readString(pass_file, apnPass)){ goto ERROR_RETURN; }
  if(!readUint16(id_file, channelId)){ goto ERROR_RETURN; }
  if(!readString(key_file, writeKey)){ goto ERROR_RETURN; }
  if(!readUint16(sens_file, sensing_interval)){ goto ERROR_RETURN; }
  if(!readUint16(up_file, upload_interval)){ goto ERROR_RETURN; }

  return true;

ERROR_RETURN:
  Serial.println("Setting Error!");
  return false;
}

void menu()
{
  if(!drawSettingMode()) {

#ifdef USE_OLED
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(20,40,"Plase Setup via serial! ");
    u8g2.sendBuffer();
#endif
    setting();
  }

  Serial.println("Please push \'s\' to Setting menu.");
  for(int i=0;i<setting_mode_time;i++){
    if(Serial.available()){
      if(Serial.read()=='s'){
        Serial.read(); // remove '\n'
        setting();
        break;
      }
    }
    usleep(10000); // 10ms
  }
  Serial.println("Nomal mode start!.");
}


void setup()
{
  Wire.begin();

  Serial.begin(115200);

#ifdef USE_OLED
  u8g2.begin();
#endif

  menu();

  airSensor.begin();

  Serial.println("Device Initilizized.");

#ifdef UPLOAD_AMBIENT
  /* LTE Module Initialize */
  bool ret = theAmbient.begin(APN_NAME ,APN_USRNAME ,APN_PASSWD);
  if (ret == false) {
    Serial.println("theAmbient begin failed");
    theAmbient.end(); while(1);
  }
  theAmbient.setupChannel(channelId, writeKey);
#endif

  sleep(1);
}

#ifdef USE_OLED
void drawBackgraund(uint16_t co2)
{
  u8g2.setFont(u8g2_font_open_iconic_weather_2x_t);
  if(co2<3000){
    u8g2.drawGlyph(10,20,69);
  }else if(co2<8000){
    u8g2.drawGlyph(10,20,64);
  }else{
    u8g2.drawGlyph(10,20,67);
  }
}

void drawParameter(uint16_t co2, float temp, float humidity)
{
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(20,40,"CO2(ppm):");
  u8g2.drawStr(85,40,String(co2).c_str());
  u8g2.drawStr(24,56,"Temp(��C):");
  u8g2.drawStr(85,56,String(temp).c_str());
  u8g2.drawStr(24,72,"Humi(%):");
  u8g2.drawStr(85,72,String(humidity).c_str());
}
#endif

void loop()
{
  static int counter = 0;

  if (airSensor.dataAvailable())
  {
    Serial.print("co2(ppm):");
    Serial.print(airSensor.getCO2());

    Serial.print(" temp(C):");
    Serial.print(airSensor.getTemperature(), 1);

    Serial.print(" humidity(%):");
    Serial.print(airSensor.getHumidity(), 1);

    Serial.println();
  } else {
    Serial.println("No data");
  }
  
#ifdef USE_OLED
  u8g2.clearBuffer();
  drawBackgraund(airSensor.getCO2());
  drawParameter(airSensor.getCO2(),airSensor.getTemperature(),airSensor.getHumidity());
  u8g2.sendBuffer();
#endif

  if(counter >= upload_interval){
#ifdef UPLOAD_AMBIENT
    theAmbient.set(1, (String(airSensor.getCO2()).c_str()));
    theAmbient.set(2, (String(airSensor.getTemperature()).c_str()));
    theAmbient.set(3, (String(airSensor.getHumidity()).c_str()));
    int ret = theAmbient.send();

    if (ret == 0) {
      Serial.println("*** ERROR! RESET Wifi! ***\n");
      exit(1);
    }else{
      Serial.println("*** Send comleted! ***\n");
    }
#endif

    counter = 0;

  }else{
    counter++;
  }

  sleep(sensing_interval);

}
