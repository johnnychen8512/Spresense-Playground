/*
 *  CO2_sensing_via_WiFi.ino - CO2 Sensing for Emviroment via GS2200.
 *  Copyright 2020 Sresense Users
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
 
#include "AmbientGs2200.h"

#include <Arduino.h>
#include <Wire.h>
#include <File.h>
#include <Flash.h>

/***   Select Mode   ***/
#define USE_OLED
#define UPLOAD_AMBIENT

#include "SparkFun_SCD30_Arduino_Library.h" 

#ifdef UPLOAD_AMBIENT
#include <GS2200Hal.h>
#include <GS2200AtCmd.h>
#include "AppFunc.h"
#include "AmbientGs2200.h"
#endif

SCD30 airSensor;

#ifdef USE_OLED
#include "U8g2lib.h"
U8G2_SSD1327_EA_W128128_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#endif

const char* apn_file  = "apn.txt";
const char* pass_file = "pass.txt";
const char* id_file   = "id.txt";
const char* key_file  = "key.txt";
const char* sens_file = "sens.txt";
const char* up_file   = "up.txt";

static int setting_mode_time = 500; /* 5000ms */

static uint16_t sensing_interval = 5; /* s */
static uint16_t upload_interval  = 6; /* 1/n */

static String apSsid = "xxxxxxxx";
static String apPass = "xxxxxxxx";

static uint16_t channelId = 00000;
static String writeKey  = "xxxxxxxxxxxxx";

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
  bool ret = settingString(apn_file, apSsid);

  Serial.print("Please input Apn Passphase(");
  ret = settingString(pass_file, apPass);
  
  Serial.print("Please input Ambient Channel ID(");
  ret = settingUint16(id_file, channelId);
  
  Serial.print("Please input Ambient writeKey(");
  ret = settingString(key_file, writeKey);

  Serial.print("Please input Sensing Interval(sec) (");
  ret = settingUint16(sens_file, sensing_interval);

  Serial.print("Please input Sensing Interval(1/n) (");
  ret = settingUint16(up_file, upload_interval);

  if(!ret)
  {
    Serial.println("Setting Error!");
    exit(1);
  }
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

  bool ret = true;
  if(!readString(apn_file, apSsid)){ ret = false; }
  if(!readString(pass_file, apPass)){ ret = false; }
  if(!readUint16(id_file, channelId)){ ret = false; }
  if(!readString(key_file, writeKey)){ ret = false; }
  if(!readUint16(sens_file, sensing_interval)){ ret = false; }
  if(!readUint16(up_file, upload_interval)){ ret = false; }

  if(!ret) {
    Serial.println("Setting Error!");
    return false;
  }
  
  return true;
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

#ifdef UPLOAD_AMBIENT
  /* WiFi Module Initialize */
  Init_GS2200_SPI();
  AtCmd_Init();

  App_InitModule(apSsid,apPass);
  App_ConnectAP();

  AmbientGs2200.begin(channelId, writeKey.c_str());
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
  u8g2.drawStr(24,56,"Temp(C):");
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
    AmbientGs2200.set(1, (String(airSensor.getCO2()).c_str()));
    AmbientGs2200.set(2, (String(airSensor.getTemperature()).c_str()));
    AmbientGs2200.set(3, (String(airSensor.getHumidity()).c_str()));
    int ret = AmbientGs2200.send();

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
