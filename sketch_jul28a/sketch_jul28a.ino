#include <SPI.h>

// These three pins are defined by SPI.h, can't be changed
// const int SDO = 12; // MISO in SPI
// const int SDI = 11; // MOSI in SPI
// const int SCK = 13; // SCK in SPI

const int CSB = 10; // SS in SPI

void writeRegister(byte registerAddress, byte value){
  // Set Chip Select pin low to signal the beginning of an SPI packet.
  digitalWrite(CSB, LOW);
  // Transfer the register address over SPI.
  SPI.transfer(registerAddress);
  // Transfer the desired register value over SPI.
  SPI.transfer(value);
  // Set the Chip Select pin high to signal the end of an SPI packet.
  digitalWrite(CSB, HIGH);
}

unsigned int readRegister(byte thisRegister, int bytesToRead) {
  byte lsb = 0;           // least significant byte
  unsigned int msb = 0;   // most significant byte
  
  // read process requires setting the first bit to 1
  thisRegister = 0x80 | thisRegister;

  Serial.println(thisRegister, BIN);
  digitalWrite(CSB, LOW);
  delay(100);

  SPI.transfer(thisRegister);

  // read least significant byte returned
  lsb = SPI.transfer(0x00);
  msb = SPI.transfer(0x00);
  Serial.println(lsb, BIN);
  Serial.println(msb, BIN);
  Serial.println();

  delay(100);
  digitalWrite(CSB, HIGH);
  return msb;
}

byte PMU_Range = 0x0F;
byte PMU_BW = 0x10;
byte PMU_SELF_TEST = 0x32;

void setup() {
  // CSB chip selection pin
  pinMode(CSB, OUTPUT);
  digitalWrite(CSB, HIGH);
  
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPI.begin();


  // (page 98) MBA280 supports SPI_MODE0 (00 mode)
  //SPI.setBitOrder(LSBFIRST);
  //SPI.setDataMode(SPI_MODE0);   // set SPI mode to '00'
  //SPI.setClockDivider(84);

  SPI.beginTransaction(SPISettings(14000000, LSBFIRST, SPI_MODE0));
  
  //writeRegister(PMU_Range, 0x03);   // set ge-range to +/- 2G
  //writeRegister(PMU_BW, 0x0E);   // set bandwidth to 500Hz
  //writeRegister(PMU_SELF_TEST, 0x00);   // disable self-test

  
  // wait the sensor to setup
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:

  unsigned int result = readRegister(0x00, 2);
    
  
  delay(1000);
}
