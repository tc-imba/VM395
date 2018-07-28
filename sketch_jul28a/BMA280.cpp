/* 06/16/2017 Copyright Tlera Corporation
 *  
 *  Created by Kris Winer
 *  
 *  The BMA280 is an inexpensive (~$1), three-axis, high-resolution (14-bit) acclerometer in a tiny 2 mm x 2 mm LGA12 package with 32-slot FIFO, 
 *  two multifunction interrupts and widely configurable sample rate (15 - 2000 Hz), full range (2 - 16 g), low power modes, 
 *  and interrupt detection behaviors. This accelerometer is nice choice for low-frequency sound and vibration analysis,
 *  tap detection and simple orientation estimation.
 *  
 *  Library may be used freely and without limit with attribution.
 *  
 */
#include "BMA280.h"

BMA280::BMA280(uint8_t intPin1, uint8_t intPin2)
{
  pinMode(intPin1, INPUT);
  _intPin1 = intPin1;
  pinMode(intPin2, INPUT);
  _intPin2 = intPin2;   
}


uint8_t BMA280::getChipID()
{
  uint8_t c = readByte(BMA280_ADDRESS, BMA280_BGW_CHIPID);
  return c;
}


uint8_t BMA280::getTapType()
{
  uint8_t c = readByte(BMA280_ADDRESS, BMA280_INT_STATUS_0);
  return c;
}


uint8_t BMA280::getTapStatus()
{
  uint8_t c = readByte(BMA280_ADDRESS, BMA280_INT_STATUS_2);
  return c;
}


float  BMA280::getAres(uint8_t Ascale) {
  switch (Ascale)
  {
   // Possible accelerometer scales (and their register bit settings) are:
   // 2 Gs , 4 Gs , 8 Gs , and 16 Gs . 
    case AFS_2G:
          _aRes = 2.0f/8192.0f;// per data sheet
          return _aRes;
          break;
    case AFS_4G:
          _aRes = 4.0f/8192.0f;
          return _aRes;
          break;
    case AFS_8G:
          _aRes = 8.0f/8192.0f;
          return _aRes;
          break;
    case AFS_16G:
          _aRes = 16.0f/8192.0f;
          return _aRes;
          break;
  }
}

void BMA280::initBMA280(uint8_t Ascale, uint8_t BW, uint8_t power_Mode, uint8_t sleep_dur)
{
   writeByte(BMA280_ADDRESS,BMA280_PMU_RANGE, Ascale);         // set full-scale range
   writeByte(BMA280_ADDRESS,BMA280_PMU_BW, BW);                // set bandwidth (and thereby sample rate) 
   writeByte(BMA280_ADDRESS,BMA280_PMU_LPW, power_Mode << 5 | sleep_dur << 1);  // set power mode and sleep duration
   
   writeByte(BMA280_ADDRESS,BMA280_INT_EN_1,  0x10);           // set data ready interrupt (bit 4) 
   writeByte(BMA280_ADDRESS,BMA280_INT_MAP_1, 0x01);           // map data ready interrupt to INT1 (bit 0) 
   writeByte(BMA280_ADDRESS,BMA280_INT_EN_0,  0x20 | 0x10);    // set single tap interrupt (bit 5) and double tap interrupt (bit 4)
   writeByte(BMA280_ADDRESS,BMA280_INT_MAP_2, 0x20 | 0x10);    // map single and double tap interrupts to INT2 (bits 4 and 5)
   writeByte(BMA280_ADDRESS,BMA280_INT_9, 0x0A);               // set tap threshold to 10 x 3.125% of full range
   writeByte(BMA280_ADDRESS,BMA280_INT_OUT_CTRL, 0x04 | 0x01); // interrupts push-pull, active HIGH (bits 0:3) 
}


void BMA280::fastCompensationBMA280()
{
     Serial.println("hold flat and motionless for bias calibration");
     delay(5000);
     
     uint8_t rawData[2];  // x/y/z accel register data stored here
     float FCres = 7.8125f; // fast compensation offset mg/LSB
     
     writeByte(BMA280_ADDRESS,BMA280_OFC_SETTING,0x20 | 0x01); // set target data to 0g, 0g, and +1 g, cutoff at 1% of bandwidth
     writeByte(BMA280_ADDRESS,BMA280_OFC_CTRL,0x20); // x-axis calibration
     while(!(0x10 & readByte(BMA280_ADDRESS,BMA280_OFC_CTRL))) { }; // wait for calibration completion
     writeByte(BMA280_ADDRESS,BMA280_OFC_CTRL,0x40); // y-axis calibration
     while(!(0x10 & readByte(BMA280_ADDRESS,BMA280_OFC_CTRL))) { }; // wait for calibration completion
     writeByte(BMA280_ADDRESS,BMA280_OFC_CTRL,0x60); // z-axis calibration
     while(!(0x10 & readByte(BMA280_ADDRESS,BMA280_OFC_CTRL))) { }; // wait for calibration completion

     readBytes(BMA280_ADDRESS, BMA280_OFC_OFFSET_X, 2, &rawData[0]);
     int16_t offsetX = ((int16_t)rawData[1] << 8) | 0x00 ;
     Serial.print("x-axis offset = "); Serial.print((float)(offsetX)*FCres/256.0f, 1); Serial.println("mg");
     readBytes(BMA280_ADDRESS, BMA280_OFC_OFFSET_Y, 2, &rawData[0]);
     int16_t offsetY = ((int16_t)rawData[1] << 8) | 0x00 ;
     Serial.print("y-axis offset = "); Serial.print((float)(offsetY)*FCres/256.0f, 1); Serial.println("mg");
     readBytes(BMA280_ADDRESS, BMA280_OFC_OFFSET_Z, 2, &rawData[0]);
     int16_t offsetZ = ((int16_t)rawData[1] << 8) | 0x00 ;
     Serial.print("z-axis offset = "); Serial.print((float)(offsetZ)*FCres/256.0f, 1); Serial.println("mg");
}

   void BMA280::resetBMA280()
   {
    writeByte(BMA280_ADDRESS,BMA280_BGW_SOFTRESET,0xB6); // software reset the BMA280
   }


   void BMA280::selfTestBMA280()
   {
   uint8_t rawData[2];  // x/y/z accel register data stored here

   writeByte(BMA280_ADDRESS,BMA280_PMU_RANGE, AFS_4G); // set full-scale range to 4G
   float STres = 4000.0f/8192.0f; // mg/LSB for 4 g full scale
   
   // x-axis test
   writeByte(BMA280_ADDRESS,BMA280_PMU_SELF_TEST, 0x10 | 0x04 | 0x01); // positive x-axis
   delay(100);
   readBytes(BMA280_ADDRESS, BMA280_ACCD_X_LSB, 2, &rawData[0]);
   int16_t posX = ((int16_t)rawData[1] << 8) | rawData[0];

   writeByte(BMA280_ADDRESS,BMA280_PMU_SELF_TEST, 0x10 | 0x00 | 0x01); // negative x-axis
   delay(100);
   readBytes(BMA280_ADDRESS, BMA280_ACCD_X_LSB, 2, &rawData[0]);
   int16_t negX = ((int16_t)rawData[1] << 8) | rawData[0];

   Serial.print("x-axis self test = "); Serial.print((float)(posX - negX)*STres/4.0f, 1); Serial.println("mg, should be > 800 mg");
 
   // y-axis test
   writeByte(BMA280_ADDRESS,BMA280_PMU_SELF_TEST, 0x10 | 0x04 | 0x02); // positive y-axis
   delay(100);
   readBytes(BMA280_ADDRESS, BMA280_ACCD_Y_LSB, 2, &rawData[0]);
   int16_t posY = ((int16_t)rawData[1] << 8) | rawData[0];

   writeByte(BMA280_ADDRESS,BMA280_PMU_SELF_TEST, 0x10 | 0x00 | 0x02); // negative y-axis
   delay(100);
   readBytes(BMA280_ADDRESS, BMA280_ACCD_Y_LSB, 2, &rawData[0]);
   int16_t negY = ((int16_t)rawData[1] << 8) | rawData[0];
 
   Serial.print("y-axis self test = "); Serial.print((float)(posY - negY)*STres/4.0f, 1); Serial.println("mg, should be > 800 mg");
  
   // z-axis test
   writeByte(BMA280_ADDRESS,BMA280_PMU_SELF_TEST, 0x10 | 0x04 | 0x03); // positive z-axis
   delay(100);
   readBytes(BMA280_ADDRESS, BMA280_ACCD_Z_LSB, 2, &rawData[0]);
   int16_t posZ = ((int16_t)rawData[1] << 8) | rawData[0];
   writeByte(BMA280_ADDRESS,BMA280_PMU_SELF_TEST, 0x10 | 0x00 | 0x03); // negative z-axis
   delay(100);
   readBytes(BMA280_ADDRESS, BMA280_ACCD_Z_LSB, 2, &rawData[0]);
   int16_t negZ = ((int16_t)rawData[1] << 8) | rawData[0];
 
   Serial.print("z-axis self test = "); Serial.print((float)(posZ - negZ)*STres/4.0f, 1); Serial.println("mg, should be > 400 mg");

   writeByte(BMA280_ADDRESS,BMA280_PMU_SELF_TEST, 0x00); // disable self test
/* end of self test*/
}


void BMA280::readBMA280AccelData(int16_t * destination)
{
  uint8_t rawData[6];  // x/y/z accel register data stored here
  readBytes(BMA280_ADDRESS, BMA280_ACCD_X_LSB, 6, &rawData[0]);  // Read the 6 raw data registers into data array
  destination[0] = ((int16_t)rawData[1] << 8) | rawData[0];      // Turn the MSB and LSB into a signed 14-bit value
  destination[1] = ((int16_t)rawData[3] << 8) | rawData[2];  
  destination[2] = ((int16_t)rawData[5] << 8) | rawData[4]; 
}


int16_t BMA280::readBMA280GyroTempData()
{
  uint8_t temp = readByte(BMA280_ADDRESS, BMA280_ACCD_TEMP);  // Read the raw data register  
  return ( ((int16_t)temp << 8) | 0x00) >> 8;  // Turn into signed 8-bit temperature value
}


// I2C scan function
void BMA280::I2Cscan()
{
// scan for i2c devices
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmission to see if
    // a device did acknowledge to the address.
//    Wire.beginTransmission(address);
//    error = Wire.endTransmission();
      error = Wire.transfer(address, NULL, 0, NULL, 0);
      
    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4) 
    {
      Serial.print("Unknown error at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
    
}

// I2C read/write functions for the BMA280

        void BMA280::writeByte(uint8_t address, uint8_t subAddress, uint8_t data) {
        uint8_t temp[2];
        temp[0] = subAddress;
        temp[1] = data;
        Wire.transfer(address, &temp[0], 2, NULL, 0); 
        }

        uint8_t BMA280::readByte(uint8_t address, uint8_t subAddress) {
        uint8_t temp[1];
        Wire.transfer(address, &subAddress, 1, &temp[0], 1);
        return temp[0];
        }

        void BMA280::readBytes(uint8_t address, uint8_t subAddress, uint8_t count, uint8_t * dest) {
        Wire.transfer(address, &subAddress, 1, dest, count); 
        }
        
