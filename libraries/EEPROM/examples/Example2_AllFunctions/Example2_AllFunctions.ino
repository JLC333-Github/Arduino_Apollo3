/*
  Reading and writing test of the EEPROM functions on the Artemis
  By: Nathan Seidle
  SparkFun Electronics
  Date: June 24th, 2019
  This example code is in the public domain.

  SparkFun labored with love to create this code. Feel like supporting open source hardware?
  Buy a board from SparkFun! https://www.sparkfun.com/products/15376

  Page erase takes 15ms
  Write byte takes 30ms - This is much longer than Arduino that takes 3.3ms
  Float write across two words takes 30ms
  Update (no write) takes 1ms
*/

#include <EEPROM.h>

void setup()
{
  Serial.begin(9600);
  Serial.println("EEPROM Examples");

  randomSeed(analogRead(A0));

  long startTime;
  long endTime;
  uint16_t randomLocation;

  //Test erase time
  startTime = millis();
  EEPROM.erase();
  endTime = millis();
  Serial.printf("Time to erase all EEPROM: %dms\n", endTime - startTime);

  //Byte sequential test
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  Serial.println("");
  Serial.println("8 bit tests");
  byte myValue1 = 200;
  byte myValue2 = 23;
  randomLocation = random(0, FLASH_EEPROM_SIZE);

  startTime = millis();
  EEPROM.write(randomLocation, myValue1); //(location, data)
  endTime = millis();
  EEPROM.put(randomLocation + 1, myValue2);

  Serial.printf("Write byte time: %dms\n", endTime - startTime);

  byte response1 = EEPROM.read(randomLocation);
  byte response2 = EEPROM.read(randomLocation + 1);
  Serial.printf("Location %d should be %d: %d\n\r", randomLocation, myValue1, response1);
  Serial.printf("Location %d should be %d: %d\n\r", randomLocation + 1, myValue2, response2);
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  Serial.println("");
  Serial.println("16 bit tests");

  //int16_t and uint16_t sequential test
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  uint16_t myValue3 = 3411;
  int16_t myValue4 = -366;
  randomLocation = random(0, FLASH_EEPROM_SIZE);

  EEPROM.put(randomLocation, myValue3);
  EEPROM.put(randomLocation + 2, myValue4);

  uint16_t response3;
  int16_t response4;
  EEPROM.get(randomLocation, response3);
  EEPROM.get(randomLocation + 2, response4);
  Serial.printf("Location %d should be %d: %d\n\r", randomLocation, myValue3, response3);
  Serial.printf("Location %d should be %d: %d\n\r", randomLocation + 2, myValue4, response4);
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  Serial.println("");
  Serial.println("32 bit tests");

  //int and unsigned int (32) sequential test
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  Serial.printf("Size of int: %d\n", sizeof(int));
  int myValue5 = -245000;
  unsigned int myValue6 = 400123;
  randomLocation = random(0, FLASH_EEPROM_SIZE);

  EEPROM.put(randomLocation, myValue5);
  EEPROM.put(randomLocation + 4, myValue6);

  int response5;
  unsigned int response6;
  EEPROM.get(randomLocation, response5);
  EEPROM.get(randomLocation + 4, response6);
  Serial.printf("Location %d should be %d: %d\n\r", randomLocation, myValue5, response5);
  Serial.printf("Location %d should be %d: %d\n\r", randomLocation + 4, myValue6, response6);
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  //int32_t and uint32_t sequential test
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  int32_t myValue7 = -341002;
  uint32_t myValue8 = 241544;
  randomLocation = random(0, FLASH_EEPROM_SIZE);

  EEPROM.update(randomLocation, myValue7);
  EEPROM.update(randomLocation + 4, myValue8);

  int32_t response7;
  uint32_t response8;
  EEPROM.get(randomLocation, response7);
  EEPROM.get(randomLocation + 4, response8);
  Serial.printf("Location %d should be %d: %d\n\r", randomLocation, myValue7, response7);
  Serial.printf("Location %d should be %d: %d\n\r", randomLocation + 4, myValue8, response8);
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  //float (32) sequential test
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  Serial.printf("Size of float: %d\n", sizeof(float));
  float myValue9 = -7.35;
  float myValue10 = 5.22;
  randomLocation = random(0, FLASH_EEPROM_SIZE);

  EEPROM.update(randomLocation, myValue9);
  EEPROM.update(randomLocation + 4, myValue10);

  float response9;
  float response10;
  EEPROM.get(randomLocation, response9);
  EEPROM.get(randomLocation + 4, response10);
  Serial.printf("Location %d should be %f: %f\n\r", randomLocation, myValue9, response9);
  Serial.printf("Location %d should be %f: %f\n\r", randomLocation + 4, myValue10, response10);
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  Serial.println("");
  Serial.println("64 bit tests");

  //double (64) sequential test
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  Serial.printf("Size of double: %d\n", sizeof(double));
  double myValue11 = -290.3485723409857;
  double myValue12 = 384.95734987;
  randomLocation = random(0, FLASH_EEPROM_SIZE);

  EEPROM.update(randomLocation, myValue11);
  EEPROM.update(randomLocation + 8, myValue12);

  double response11;
  double response12;
  EEPROM.get(randomLocation, response11);
  EEPROM.get(randomLocation + 8, response12);
  Serial.printf("Location %d should be %lf: %lf\n", randomLocation, myValue11, response11);
  Serial.printf("Location %d should be %lf: %lf\n", randomLocation + 8, myValue12, response12);
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  Serial.println();
  Serial.print("Flash Contents:");
  for (uint16_t x = 0; x < 8 * 4; x += 4)
  {
    if (x % 32 == 0)
      Serial.println();
    Serial.printf("0x%08X ", *(uint32_t *)(FLASH_EEPROM_START + x));
  }
  Serial.println();
}

void loop()
{
}