// Original work Copyright (c) 2015-2022 Pololu Corporation (www.pololu.com)
// Modified work Copyright 2023 Moritz Lang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <Wire.h>
#include "ProfizumoImu.h"

#define TEST_REG_ERROR -1

#define LSM303D_WHO_ID  0x49
#define L3GD20H_WHO_ID  0xD7
#define LSM6DS33_WHO_ID 0x69
#define LIS3MDL_WHO_ID  0x3D

namespace profizumo
{
bool ProfizumoImu::Init()
{
  if (TestReg(LSM303D_ADDR, LSM303D_REG_WHO_AM_I) == LSM303D_WHO_ID &&
      TestReg(L3GD20H_ADDR, L3GD20H_REG_WHO_AM_I) == L3GD20H_WHO_ID)
  {
    type = ProfizumoImuType::LSM303D_L3GD20H;
    return true;
  }
  else if (TestReg(LSM6DS33_ADDR, LSM6DS33_REG_WHO_AM_I) == LSM6DS33_WHO_ID &&
           TestReg( LIS3MDL_ADDR,  LIS3MDL_REG_WHO_AM_I) ==  LIS3MDL_WHO_ID)
  {
    type = ProfizumoImuType::LSM6DS33_LIS3MDL;
    return true;
  }
  else
  {
    return false;
  }
}

void ProfizumoImu::EnableDefault()
{
  switch (type)
  {
  case ProfizumoImuType::LSM303D_L3GD20H:

    // Accelerometer

    // 0x57 = 0b01010111
    // AODR = 0101 (50 Hz ODR); AZEN = AYEN = AXEN = 1 (all axes enabled)
    WriteReg(LSM303D_ADDR, LSM303D_REG_CTRL1, 0x57);
    if (lastError) { return; }

    // 0x00 = 0b00000000
    // AFS = 0 (+/- 2 g full scale)
    WriteReg(LSM303D_ADDR, LSM303D_REG_CTRL2, 0x00);
    if (lastError) { return; }

    // Magnetometer

    // 0x64 = 0b01100100
    // M_RES = 11 (high resolution mode); M_ODR = 001 (6.25 Hz ODR)
    WriteReg(LSM303D_ADDR, LSM303D_REG_CTRL5, 0x64);
    if (lastError) { return; }

    // 0x20 = 0b00100000
    // MFS = 01 (+/- 4 gauss full scale)
    WriteReg(LSM303D_ADDR, LSM303D_REG_CTRL6, 0x20);
    if (lastError) { return; }

    // 0x00 = 0b00000000
    // MD = 00 (continuous-conversion mode)
    WriteReg(LSM303D_ADDR, LSM303D_REG_CTRL7, 0x00);
    if (lastError) { return; }

    // Gyro

    // 0x7F = 0b01111111
    // DR = 01 (189.4 Hz ODR); BW = 11 (70 Hz bandwidth); PD = 1 (normal mode); Zen = Yen = Xen = 1 (all axes enabled)
    WriteReg(L3GD20H_ADDR, L3GD20H_REG_CTRL1, 0x7F);
    if (lastError) { return; }

    // 0x00 = 0b00000000
    // FS = 00 (+/- 245 dps full scale)
    WriteReg(L3GD20H_ADDR, L3GD20H_REG_CTRL4, 0x00);
    return;

  case ProfizumoImuType::LSM6DS33_LIS3MDL:

    // Accelerometer

    // 0x30 = 0b00110000
    // ODR = 0011 (52 Hz (high performance)); FS_XL = 00 (+/- 2 g full scale)
    WriteReg(LSM6DS33_ADDR, LSM6DS33_REG_CTRL1_XL, 0x30);
    if (lastError) { return; }

    // Gyro

    // 0x50 = 0b01010000
    // ODR = 0101 (208 Hz (high performance)); FS_G = 00 (+/- 245 dps full scale)
    WriteReg(LSM6DS33_ADDR, LSM6DS33_REG_CTRL2_G, 0x50);
    if (lastError) { return; }

    // Accelerometer + Gyro

    // 0x04 = 0b00000100
    // IF_INC = 1 (automatically increment register address)
    WriteReg(LSM6DS33_ADDR, LSM6DS33_REG_CTRL3_C, 0x04);
    if (lastError) { return; }

    // Magnetometer

    // 0x70 = 0b01110000
    // OM = 11 (ultra-high-performance mode for X and Y); DO = 100 (10 Hz ODR)
    WriteReg(LIS3MDL_ADDR, LIS3MDL_REG_CTRL_REG1, 0x70);
    if (lastError) { return; }

    // 0x00 = 0b00000000
    // FS = 00 (+/- 4 gauss full scale)
    WriteReg(LIS3MDL_ADDR, LIS3MDL_REG_CTRL_REG2, 0x00);
    if (lastError) { return; }

    // 0x00 = 0b00000000
    // MD = 00 (continuous-conversion mode)
    WriteReg(LIS3MDL_ADDR, LIS3MDL_REG_CTRL_REG3, 0x00);
    if (lastError) { return; }

    // 0x0C = 0b00001100
    // OMZ = 11 (ultra-high-performance mode for Z)
    WriteReg(LIS3MDL_ADDR, LIS3MDL_REG_CTRL_REG4, 0x0C);
    return;

  default:
    return;
  }
}

void ProfizumoImu::ConfigureForBalancing()
{
  switch (type)
  {
  case ProfizumoImuType::LSM303D_L3GD20H:

    // Accelerometer

    // 0x18 = 0b00011000
    // AFS = 0 (+/- 2 g full scale)
    WriteReg(LSM303D_ADDR, LSM303D_REG_CTRL2, 0x18);
    if (lastError) { return; }

    // Gyro

    // 0xFF = 0b11111111
    // DR = 11 (757.6 Hz ODR); BW = 11 (100 Hz bandwidth); PD = 1 (normal mode); Zen = Yen = Xen = 1 (all axes enabled)
    WriteReg(L3GD20H_ADDR, L3GD20H_REG_CTRL1, 0xFF);
    if (lastError) { return; }

    // 0x20 = 0b00100000
    // FS = 10 (+/- 2000 dps full scale)
    WriteReg(L3GD20H_ADDR, L3GD20H_REG_CTRL4, 0x20);
    return;

  case ProfizumoImuType::LSM6DS33_LIS3MDL:

    // Accelerometer

    // 0x3C = 0b00111100
    // ODR = 0011 (52 Hz (high performance)); FS_XL = 11 (+/- 8 g full scale)
    WriteReg(LSM6DS33_ADDR, LSM6DS33_REG_CTRL1_XL, 0x3C);
    if (lastError) { return; }

    // Gyro

    // 0x7C = 0b01111100
    // ODR = 0111 (833 Hz (high performance)); FS_G = 11 (+/- 2000 dps full scale)
    WriteReg(LSM6DS33_ADDR, LSM6DS33_REG_CTRL2_G, 0x7C);
    return;

  default:
    return;
  }
}

void ProfizumoImu::ConfigureForTurnSensing()
{
  switch (type)
  {
  case ProfizumoImuType::LSM303D_L3GD20H:

    // Gyro

    // 0xFF = 0b11111111
    // DR = 11 (757.6 Hz ODR); BW = 11 (100 Hz bandwidth); PD = 1 (normal mode); Zen = Yen = Xen = 1 (all axes enabled)
    WriteReg(L3GD20H_ADDR, L3GD20H_REG_CTRL1, 0xFF);
    if (lastError) { return; }

    // 0x20 = 0b00100000
    // FS = 10 (+/- 2000 dps full scale)
    WriteReg(L3GD20H_ADDR, L3GD20H_REG_CTRL4, 0x20);
    return;

  case ProfizumoImuType::LSM6DS33_LIS3MDL:

    // Gyro

    // 0x7C = 0b01111100
    // ODR = 0111 (833 Hz (high performance)); FS_G = 11 (+/- 2000 dps full scale)
    WriteReg(LSM6DS33_ADDR, LSM6DS33_REG_CTRL2_G, 0x7C);
    return;

  default:
    return;
  }
}

void ProfizumoImu::ConfigureForFaceUphill()
{
  switch (type)
  {
  case ProfizumoImuType::LSM303D_L3GD20H:

    // Accelerometer

    // 0x37 = 0b00110111
    // AODR = 0011 (12.5 Hz ODR); AZEN = AYEN = AXEN = 1 (all axes enabled)
    WriteReg(LSM303D_ADDR, LSM303D_REG_CTRL1, 0x37);
    return;

  case ProfizumoImuType::LSM6DS33_LIS3MDL:

    // Accelerometer

    // 0x10 = 0b00010000
    // ODR = 0001 (13 Hz (high performance)); FS_XL = 00 (+/- 2 g full scale)
    WriteReg(LSM6DS33_ADDR, LSM6DS33_REG_CTRL1_XL, 0x10);
    return;

  default:
    return;
  }
}

void ProfizumoImu::WriteReg(uint8_t addr, uint8_t reg, uint8_t value)
{
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(value);
  lastError = Wire.endTransmission();
}

uint8_t ProfizumoImu::ReadReg(uint8_t addr, uint8_t reg)
{
  Wire.beginTransmission(addr);
  Wire.write(reg);
  lastError = Wire.endTransmission();
  if (lastError) { return 0; }

  uint8_t byteCount = Wire.requestFrom(addr, (uint8_t)1);
  if (byteCount != 1)
  {
    lastError = 50;
    return 0;
  }
  return Wire.read();
}

// Reads the 3 accelerometer channels and stores them in vector a
void ProfizumoImu::ReadAcc(void)
{
  switch (type)
  {
  case ProfizumoImuType::LSM303D_L3GD20H:
    // set MSB of register address for auto-increment
    ReadAxes16Bit(LSM303D_ADDR, LSM303D_REG_OUT_X_L_A | (1 << 7), a);
    return;

  case ProfizumoImuType::LSM6DS33_LIS3MDL:
    // assumes register address auto-increment is enabled (IF_INC in CTRL3_C)
    ReadAxes16Bit(LSM6DS33_ADDR, LSM6DS33_REG_OUTX_L_XL, a);
    return;

  default:
    return;
  }
}

// Reads the 3 gyro channels and stores them in vector g
void ProfizumoImu::ReadGyro()
{
  switch (type)
  {
  case ProfizumoImuType::LSM303D_L3GD20H:
    // set MSB of register address for auto-increment
    ReadAxes16Bit(L3GD20H_ADDR, L3GD20H_REG_OUT_X_L | (1 << 7), g);
    return;

  case ProfizumoImuType::LSM6DS33_LIS3MDL:
    // assumes register address auto-increment is enabled (IF_INC in CTRL3_C)
    ReadAxes16Bit(LSM6DS33_ADDR, LSM6DS33_REG_OUTX_L_G, g);
    return;

  default:
    return;
  }
}

// Reads the 3 magnetometer channels and stores them in vector m
void ProfizumoImu::ReadMag()
{
  switch (type)
  {
  case ProfizumoImuType::LSM303D_L3GD20H:
    // set MSB of register address for auto-increment
    ReadAxes16Bit(LSM303D_ADDR, LSM303D_REG_OUT_X_L_M | (1 << 7), m);
    return;

  case ProfizumoImuType::LSM6DS33_LIS3MDL:
    // set MSB of register address for auto-increment
    ReadAxes16Bit(LIS3MDL_ADDR, LIS3MDL_REG_OUT_X_L | (1 << 7), m);
    return;

  default:
    return;
  }
}

// Reads all 9 accelerometer, gyro, and magnetometer channels and stores them
// in the respective vectors
void ProfizumoImu::Read()
{
  ReadAcc();
  if (lastError) { return; }
  ReadGyro();
  if (lastError) { return; }
  ReadMag();
}

bool ProfizumoImu::AccDataReady()
{
  switch (type)
  {
  case ProfizumoImuType::LSM303D_L3GD20H:
    return ReadReg(LSM303D_ADDR, LSM303D_REG_STATUS_A) & 0x08;

  case ProfizumoImuType::LSM6DS33_LIS3MDL:
    return ReadReg(LSM6DS33_ADDR, LSM6DS33_REG_STATUS_REG) & 0x01;

  default:
    return false;
  }
}

bool ProfizumoImu::GyroDataReady()
{
  switch (type)
  {
  case ProfizumoImuType::LSM303D_L3GD20H:
    return ReadReg(L3GD20H_ADDR, L3GD20H_REG_STATUS) & 0x08;

  case ProfizumoImuType::LSM6DS33_LIS3MDL:
    return ReadReg(LSM6DS33_ADDR, LSM6DS33_REG_STATUS_REG) & 0x02;

  default:
    return false;
  }
}

bool ProfizumoImu::MagDataReady()
{
  switch (type)
  {
  case ProfizumoImuType::LSM303D_L3GD20H:
    return ReadReg(LSM303D_ADDR, LSM303D_REG_STATUS_M) & 0x08;

  case ProfizumoImuType::LSM6DS33_LIS3MDL:
    return ReadReg(LIS3MDL_ADDR, LIS3MDL_REG_STATUS_REG) & 0x08;

  default:
    return false;
  }
}

int16_t ProfizumoImu::TestReg(uint8_t addr, uint8_t reg)
{
  Wire.beginTransmission(addr);
  Wire.write(reg);
  if (Wire.endTransmission() != 0)
  {
    return TEST_REG_ERROR;
  }

  uint8_t byteCount = Wire.requestFrom(addr, (uint8_t)1);
  if (byteCount != 1)
  {
    return TEST_REG_ERROR;
  }
  return Wire.read();
}

void ProfizumoImu::ReadAxes16Bit(uint8_t addr, uint8_t firstReg, vector<int16_t> & v)
{
  Wire.beginTransmission(addr);
  Wire.write(firstReg);
  lastError = Wire.endTransmission();
  if (lastError) { return; }

  uint8_t byteCount = (Wire.requestFrom(addr, (uint8_t)6));
  if (byteCount != 6)
  {
    lastError = 50;
    return;
  }
  uint8_t xl = Wire.read();
  uint8_t xh = Wire.read();
  uint8_t yl = Wire.read();
  uint8_t yh = Wire.read();
  uint8_t zl = Wire.read();
  uint8_t zh = Wire.read();

  // combine high and low bytes
  v.x = (int16_t)(xh << 8 | xl);
  v.y = (int16_t)(yh << 8 | yl);
  v.z = (int16_t)(zh << 8 | zl);
}
}
