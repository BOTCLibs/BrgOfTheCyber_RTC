#include "BrgOfTheCyber_RTC.h"

BrgOfTheCyber_RTC::BrgOfTheCyber_RTC(RTCType type, uint8_t i2c_address) {
  _type = type;
  _i2c_address = i2c_address;
}

bool BrgOfTheCyber_RTC::begin() {
  Wire.begin();
  Wire.beginTransmission(_i2c_address);
  if (Wire.endTransmission() != 0) {
    return false;
  }
  
  _initRTC();
  return true;
}

void BrgOfTheCyber_RTC::setDateTime(uint16_t year, uint8_t month, uint8_t day,
                                   uint8_t hour, uint8_t minute, uint8_t second) {
  _time.year = year;
  _time.month = month;
  _time.day = day;
  _time.hour = hour;
  _time.minute = minute;
  _time.second = second;
  
  _writeRTC();
}

uint16_t BrgOfTheCyber_RTC::getYear() {
  _readRTC();
  return _time.year;
}

uint8_t BrgOfTheCyber_RTC::getMonth() {
  _readRTC();
  return _time.month;
}

uint8_t BrgOfTheCyber_RTC::getDay() {
  _readRTC();
  return _time.day;
}

uint8_t BrgOfTheCyber_RTC::getHour() {
  _readRTC();
  return _time.hour;
}

uint8_t BrgOfTheCyber_RTC::getMinute() {
  _readRTC();
  return _time.minute;
}

uint8_t BrgOfTheCyber_RTC::getSecond() {
  _readRTC();
  return _time.second;
}

String BrgOfTheCyber_RTC::getTime(bool tickingEffect) {
  if (tickingEffect) {
    _readRTC();
    
    char buffer[6];
    if (_time.second % 2 == 0) {
      sprintf(buffer, "%02d:%02d", _time.hour, _time.minute);
    } else {
      sprintf(buffer, "%02d %02d", _time.hour, _time.minute);
    }
    return String(buffer);
  } else {
    getHour(); getMinute();
    char buffer[6];
    sprintf(buffer, "%02d:%02d", _time.hour, _time.minute);
    return String(buffer);
  }
}

String BrgOfTheCyber_RTC::getToDay() {
  getDay(); getMonth();
  char buffer[6];
  sprintf(buffer, "%02d/%02d", _time.day, _time.month);
  return String(buffer);
}

String BrgOfTheCyber_RTC::getDate() {
  getDay(); getMonth(); getYear();
  char buffer[11];
  sprintf(buffer, "%02d/%02d/%04d", _time.day, _time.month, _time.year);
  return String(buffer);
}

float BrgOfTheCyber_RTC::getTemperature() {
  if (_type != RTC_DS3231) return -999.0;
  
  Wire.beginTransmission(_i2c_address);
  Wire.write(0x11);
  Wire.endTransmission();
  
  // Исправление: явно указываем тип параметров для requestFrom
  Wire.requestFrom((uint8_t)_i2c_address, (uint8_t)2);
  
  if (Wire.available() == 2) {
    int8_t temp_msb = Wire.read();
    uint8_t temp_lsb = Wire.read();
    return temp_msb + (temp_lsb >> 6) * 0.25;
  }
  return -999.0;
}

bool BrgOfTheCyber_RTC::lostPower() {
  if (_type == RTC_DS3231) {
    uint8_t status = _readRegister(0x0F);
    return (status & 0x80) != 0;
  }
  return false;
}

bool BrgOfTheCyber_RTC::isRunning() {
  Wire.beginTransmission(_i2c_address);
  return (Wire.endTransmission() == 0);
}

uint8_t BrgOfTheCyber_RTC::_decToBcd(uint8_t val) {
  return ((val / 10 * 16) + (val % 10));
}

uint8_t BrgOfTheCyber_RTC::_bcdToDec(uint8_t val) {
  return ((val / 16 * 10) + (val % 16));
}

void BrgOfTheCyber_RTC::_initRTC() {
  switch (_type) {
    case RTC_DS3231:
      _writeRegister(0x0E, 0x00);
      break;
    case RTC_DS1307:
      {
        uint8_t seconds = _readRegister(0x00);
        if (seconds & 0x80) {
          _writeRegister(0x00, seconds & 0x7F);
        }
      }
      break;
    case RTC_PCF8563:
      _writeRegister(0x00, 0x00);
      _writeRegister(0x01, 0x00);
      break;
  }
}

void BrgOfTheCyber_RTC::_readRTC() {
  Wire.beginTransmission(_i2c_address);
  Wire.write(0x00);
  Wire.endTransmission();
  
  // Исправление: явно указываем тип параметров
  Wire.requestFrom((uint8_t)_i2c_address, (uint8_t)7);
  
  if (Wire.available() >= 7) {
    _time.second = _bcdToDec(Wire.read() & 0x7F);
    _time.minute = _bcdToDec(Wire.read());
    _time.hour = _bcdToDec(Wire.read() & 0x3F);
    Wire.read(); // День недели
    _time.day = _bcdToDec(Wire.read());
    _time.month = _bcdToDec(Wire.read() & 0x1F);
    uint8_t year = _bcdToDec(Wire.read());
    _time.year = 2000 + year;
  }
}

void BrgOfTheCyber_RTC::_writeRTC() {
  Wire.beginTransmission(_i2c_address);
  Wire.write(0x00);
  
  Wire.write(_decToBcd(_time.second));
  Wire.write(_decToBcd(_time.minute));
  Wire.write(_decToBcd(_time.hour));
  Wire.write(0x01);
  Wire.write(_decToBcd(_time.day));
  Wire.write(_decToBcd(_time.month));
  Wire.write(_decToBcd(_time.year - 2000));
  
  Wire.endTransmission();
}

uint8_t BrgOfTheCyber_RTC::_readRegister(uint8_t reg) {
  Wire.beginTransmission(_i2c_address);
  Wire.write(reg);
  Wire.endTransmission();
  
  // Исправление: явно указываем тип параметров
  Wire.requestFrom((uint8_t)_i2c_address, (uint8_t)1);
  
  return Wire.available() ? Wire.read() : 0;
}

void BrgOfTheCyber_RTC::_writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(_i2c_address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}