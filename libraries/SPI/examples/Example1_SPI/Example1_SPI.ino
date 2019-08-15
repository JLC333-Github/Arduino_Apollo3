/* Author: Owen Lyke
  Created: May 13 2019
  License: MIT. See SparkFun Arduino Apollo3 Project for more information

  This example demonstrates how to use Arduino SPI
*/

#include "SPI.h"

#define CS_PIN 2

#define SPI_SPEED 1000000
#define SPI_ORDER MSBFIRST
#define SPI_MODE SPI_MODE0

SPISettings mySettings(SPI_SPEED, SPI_ORDER, SPI_MODE);

const char *msg = "Hello world!";

void setup()
{

  Serial.begin(9600);
  while (!Serial)
  {
  }; //Wait for user to open terminal window

  Serial.println("SparkFun Arduino Apollo3 SPI Example");
  Serial.printf("Compiled on %s, %s\n\n", __DATE__, __TIME__);

  SPI.begin();

  pinMode(CS_PIN, OUTPUT);
}

void loop()
{
  digitalWrite(CS_PIN, LOW);
  SPI.beginTransaction(mySettings);
  SPI.transfer(0xAA);
  SPI.endTransaction();

  SPI.beginTransaction(mySettings);
  SPI.transferOut((void *)msg, strlen(msg));
  SPI.endTransaction();
  digitalWrite(CS_PIN, HIGH);
  delay(1000);
}
