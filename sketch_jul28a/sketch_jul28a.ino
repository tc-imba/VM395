#include "BMA280.h"

// These three pins are defined by SPI.h, can't be changed
// const int SDO = 12; // MISO in SPI
// const int SDI = 11; // MOSI in SPI
// const int SCK = 13; // SCK in SPI

const int CSB = 10; // SS in SPI

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPI.begin();

  // CSB chip selection pin
  pinMode(CSB, OUTPUT);

  // (page 98) MBA280 supports SPI_MODE0 (00 mode)
  SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CSB, LOW);

  // wait the sensor to setup
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:


  unsigned int result = SPI.transfer((uint16_t) 1);
    
  Serial.println(result);
  delay(1000);
}
