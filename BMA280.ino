//-------------------------------------------------------------------------------
//  TinyCircuits Accelerometer TinyShield Example Sketch
//  Using Bosch BMA250 in I2C mode
//
//  Created 6/30/2013
//  by Ken Burns, TinyCircuits http://Tiny-Circuits.com
//
//  This example code is in the public domain.
//
//-------------------------------------------------------------------------------

#include <Wire.h>
#define BMA280_I2CADDR      0x18
#define BMA280_RANGE        0x08// 03 = 2g, 05 = 4g, 08 = 8g, 0C = 16g
#define BMA280_BW           0x08   // 7.81Hz (update time of 64ms) 

int AccelX;
int AccelY;
int AccelZ; 
float AccelTemperature;

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  BMA280Init();
 // output = createWriter( "data.txt" );
}


void loop()
{
  BMA280ReadAccel();
  
  // Print out the accelerometer data
 
  Serial.print("\t");
  Serial.print(AccelX);
  Serial.print(" \t");
  Serial.print(AccelY);
  Serial.print("\t ");
  Serial.print(AccelZ);
  Serial.print("\t");   
  Serial.print(AccelTemperature);
  Serial.println("degC");
  delay(10);
}


void BMA280Init()
{
  // Setup the range measurement setting
  Wire.beginTransmission(BMA280_I2CADDR);
  Wire.write(0x0F); //Range address
  Wire.write(BMA280_RANGE);
  Wire.endTransmission();

  // Setup the bandwidth
  Wire.beginTransmission(BMA280_I2CADDR);
  Wire.write(0x10); //Bandwidth address
  Wire.write(BMA280_BW);
  Wire.endTransmission();
}


int BMA280ReadAccel()
{
  uint8_t ReadBuff[8];

  // Read the 7 data bytes from the BMA280
  Wire.beginTransmission(BMA280_I2CADDR);
  Wire.write(0x02); 
  Wire.endTransmission();
  Wire.requestFrom(BMA280_I2CADDR,7); 

  for(int i = 0; i < 7;i++)
  {
    ReadBuff[i] = Wire.read(); 
  }

  AccelX = ReadBuff[1] << 8;
  AccelX |= ReadBuff[0];
  AccelX >>= 2;

  AccelY = ReadBuff[3] << 8;
  AccelY |= ReadBuff[2];
  AccelY >>= 2;

  AccelZ = ReadBuff[5] << 8;
  AccelZ |= ReadBuff[4];
  AccelZ >>= 2;  

 
  AccelTemperature = (ReadBuff[6] * 0.5) + 24.0;

//  accel.Write(AccelX,AccelX.GetLength());
}
 



