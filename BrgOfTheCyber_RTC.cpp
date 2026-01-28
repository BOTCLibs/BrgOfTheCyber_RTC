#include "BrgOfTheCyber_RTC.h"

BrgOfTheCyber_RTC::BrgOfTheCyber_RTC(uint8_t i2c_addr) {
  _i2c_addr = i2c_addr;
}

bool BrgOfTheCyber_RTC::begin() {
  Wire.begin();
  Wire.beginTransmission(_i2c_addr);
  return (Wire.endTransmission() == 0); // 0 означает успешное подключение
}

void BrgOfTheCyber_RTC::setDateTime(uint16_t year, uint8_t month, uint8_t day,
                                    uint8_t hour, uint8_t minute, uint8_t second) {
  // Преобразуем год в двузначный формат (00-99) для DS3231
  uint8_t year_short = year % 100;
  
  Wire.beginTransmission(_i2c_addr);
  Wire.write(0x00); // Указываем начальный адрес регистра (секунды)
  
  Wire.write(_decToBcd(second)); // Секунды
  Wire.write(_decToBcd(minute)); // Минуты
  Wire.write(_decToBcd(hour));   // Часы (24-часовой формат)
  Wire.write(0x01);              // День недели (не используется, устанавливаем 1)
  Wire.write(_decToBcd(day));    // День месяца
  Wire.write(_decToBcd(month));  // Месяц
  Wire.write(_decToBcd(year_short)); // Год (две последние цифры)
  
  Wire.endTransmission();
  
  // Сохраняем значения в переменных класса
  _year = year;
  _month = month;
  _day = day;
  _hour = hour;
  _minute = minute;
  _second = second;
}

uint8_t BrgOfTheCyber_RTC::_decToBcd(uint8_t val) {
  return ((val / 10 * 16) + (val % 10));
}

uint8_t BrgOfTheCyber_RTC::_bcdToDec(uint8_t val) {
  return ((val / 16 * 10) + (val % 16));
}

void BrgOfTheCyber_RTC::refreshDateTime() {
  Wire.beginTransmission(_i2c_addr);
  Wire.write(0x00); // Указываем адрес регистра секунд
  Wire.endTransmission();
  
  Wire.requestFrom(_i2c_addr, 7); // Запрашиваем 7 регистров
  
  if (Wire.available() == 7) {
    _second = _bcdToDec(Wire.read() & 0x7F); // Игнорируем старший бит
    _minute = _bcdToDec(Wire.read());
    _hour = _bcdToDec(Wire.read() & 0x3F);   // 24-часовой формат
    Wire.read(); // День недели (пропускаем)
    _day = _bcdToDec(Wire.read());
    _month = _bcdToDec(Wire.read() & 0x1F);  // Игнорируем старшие биты
    uint8_t year_short = _bcdToDec(Wire.read());
    
    // Определяем полный год (предполагаем, что год 2000-2099)
    _year = 2000 + year_short;
  }
}

uint16_t BrgOfTheCyber_RTC::getYear() {
  refreshDateTime();
  return _year;
}

uint8_t BrgOfTheCyber_RTC::getMonth() {
  refreshDateTime();
  return _month;
}

uint8_t BrgOfTheCyber_RTC::getDay() {
  refreshDateTime();
  return _day;
}

uint8_t BrgOfTheCyber_RTC::getHour() {
  refreshDateTime();
  return _hour;
}

uint8_t BrgOfTheCyber_RTC::getMinute() {
  refreshDateTime();
  return _minute;
}

uint8_t BrgOfTheCyber_RTC::getSecond() {
  refreshDateTime();
  return _second;
}

String BrgOfTheCyber_RTC::getTime(bool tickingEffect) {
  refreshDateTime();
  char buffer[6];
  if (tickingEffect) {
    sprintf(buffer, "%02d %02d", _hour, _minute);
  } else {
    sprintf(buffer, "%02d:%02d", _hour, _minute);
  }
  return String(buffer);
}

String BrgOfTheCyber_RTC::getToDay() {
  refreshDateTime();
  char buffer[6];
  sprintf(buffer, "%02d/%02d", _day, _month);
  return String(buffer);
}

String BrgOfTheCyber_RTC::getDate() {
  refreshDateTime();
  char buffer[11];
  sprintf(buffer, "%02d/%02d/%04d", _day, _month, _year);
  return String(buffer);
}

bool BrgOfTheCyber_RTC::isLeapYear(uint16_t year) {
  return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}