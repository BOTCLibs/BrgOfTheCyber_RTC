#include "BrgOfTheCyber_RTC.h"
#include <EEPROM.h>

// Адреса в EEPROM для сохранения времени
#define EEPROM_TIME_ADDR 0
#define EEPROM_DRIFT_ADDR (EEPROM_TIME_ADDR + sizeof(unsigned long))

// Конструкторы
BrgOfTheCyber_RTC::BrgOfTheCyber_RTC() {
  _mode = RTC_INTERNAL;
  _type = RTC_DS3231;
  _i2c_address = 0x68;
  _initialized = false;
  _driftFactor = 1.0;
  _millisOffset = 0;
}

BrgOfTheCyber_RTC::BrgOfTheCyber_RTC(RTCMode mode) {
  _mode = mode;
  _type = RTC_DS3231;
  _i2c_address = 0x68;
  _initialized = false;
  _driftFactor = 1.0;
  _millisOffset = 0;
}

BrgOfTheCyber_RTC::BrgOfTheCyber_RTC(RTCType type, uint8_t i2c_address) {
  _mode = RTC_EXTERNAL_I2C;
  _type = type;
  _i2c_address = i2c_address;
  _initialized = false;
  _driftFactor = 1.0;
  _millisOffset = 0;
}

BrgOfTheCyber_RTC::BrgOfTheCyber_RTC(RTCMode mode, RTCType type, uint8_t i2c_address) {
  _mode = mode;
  _type = type;
  _i2c_address = i2c_address;
  _initialized = false;
  _driftFactor = 1.0;
  _millisOffset = 0;
}

bool BrgOfTheCyber_RTC::begin() {
  if (_initialized) return true;
  
  switch (_mode) {
    case RTC_INTERNAL:
      // Загружаем сохраненное время из EEPROM
      EEPROM.get(EEPROM_TIME_ADDR, _millisOffset);
      
      // Загружаем коэффициент дрейфа
      EEPROM.get(EEPROM_DRIFT_ADDR, _driftFactor);
      if (isnan(_driftFactor) || _driftFactor < 0.9 || _driftFactor > 1.1) {
        _driftFactor = 1.0; // Значение по умолчанию
      }
      
      _lastMillis = millis();
      
      // Устанавливаем время компиляции по умолчанию
      setDateTimeFromCompileTime();
      break;
      
    case RTC_EXTERNAL_I2C:
      Wire.begin();
      
      // Проверка наличия устройства на шине I2C
      Wire.beginTransmission(_i2c_address);
      if (Wire.endTransmission() != 0) {
        return false; // Устройство не отвечает
      }
      
      // Инициализация конкретного типа RTC
      switch (_type) {
        case RTC_DS3231: _initDS3231(); break;
        case RTC_DS1307: _initDS1307(); break;
        case RTC_PCF8563: _initPCF8563(); break;
      }
      break;
  }
  
  _initialized = true;
  return true;
}

void BrgOfTheCyber_RTC::setDateTime(uint16_t year, uint8_t month, uint8_t day,
                                   uint8_t hour, uint8_t minute, uint8_t second) {
  _currentTime.year = year;
  _currentTime.month = month;
  _currentTime.day = day;
  _currentTime.hour = hour;
  _currentTime.minute = minute;
  _currentTime.second = second;
  
  _calculateWeekday();
  _normalizeTime();
  
  switch (_mode) {
    case RTC_INTERNAL:
      _saveToInternal();
      _lastMillis = millis();
      break;
      
    case RTC_EXTERNAL_I2C:
      _saveToExternal();
      break;
  }
}

void BrgOfTheCyber_RTC::setDateTimeFromCompileTime() {
  // Используем макросы времени компиляции
  setDateTime(
    (__DATE__[9] - '0') * 1000 + (__DATE__[10] - '0') * 100 + 
    (__DATE__[11] - '0') * 10 + (__DATE__[12] - '0'),
    // Месяц
    (strstr(__DATE__, "Jan") ? 1 : strstr(__DATE__, "Feb") ? 2 :
     strstr(__DATE__, "Mar") ? 3 : strstr(__DATE__, "Apr") ? 4 :
     strstr(__DATE__, "May") ? 5 : strstr(__DATE__, "Jun") ? 6 :
     strstr(__DATE__, "Jul") ? 7 : strstr(__DATE__, "Aug") ? 8 :
     strstr(__DATE__, "Sep") ? 9 : strstr(__DATE__, "Oct") ? 10 :
     strstr(__DATE__, "Nov") ? 11 : 12),
    // День
    (__DATE__[4] == ' ' ? 0 : __DATE__[4] - '0') * 10 + (__DATE__[5] - '0'),
    // Время из __TIME__
    (__TIME__[0] - '0') * 10 + (__TIME__[1] - '0'),
    (__TIME__[3] - '0') * 10 + (__TIME__[4] - '0'),
    (__TIME__[6] - '0') * 10 + (__TIME__[7] - '0')
  );
}

// ===== ФУНКЦИИ ЗАПРОСА =====

uint16_t BrgOfTheCyber_RTC::getYear() {
  if (_mode == RTC_INTERNAL) _updateFromInternal();
  else _updateFromExternal();
  return _currentTime.year;
}

uint8_t BrgOfTheCyber_RTC::getMonth() {
  if (_mode == RTC_INTERNAL) _updateFromInternal();
  else _updateFromExternal();
  return _currentTime.month;
}

uint8_t BrgOfTheCyber_RTC::getDay() {
  if (_mode == RTC_INTERNAL) _updateFromInternal();
  else _updateFromExternal();
  return _currentTime.day;
}

uint8_t BrgOfTheCyber_RTC::getHour() {
  if (_mode == RTC_INTERNAL) _updateFromInternal();
  else _updateFromExternal();
  return _currentTime.hour;
}

uint8_t BrgOfTheCyber_RTC::getMinute() {
  if (_mode == RTC_INTERNAL) _updateFromInternal();
  else _updateFromExternal();
  return _currentTime.minute;
}

uint8_t BrgOfTheCyber_RTC::getSecond() {
  if (_mode == RTC_INTERNAL) _updateFromInternal();
  else _updateFromExternal();
  return _currentTime.second;
}

uint8_t BrgOfTheCyber_RTC::getWeekday() {
  if (_mode == RTC_INTERNAL) _updateFromInternal();
  else _updateFromExternal();
  return _currentTime.weekday;
}

String BrgOfTheCyber_RTC::getTime(bool tickingEffect) {
  getHour(); getMinute(); // Обновляем время
  char buffer[6];
  if (tickingEffect) {
    sprintf(buffer, "%02d %02d", _currentTime.hour, _currentTime.minute);
  } else {
    sprintf(buffer, "%02d:%02d", _currentTime.hour, _currentTime.minute);
  }
  return String(buffer);
}

String BrgOfTheCyber_RTC::getToDay() {
  getDay(); getMonth(); // Обновляем дату
  char buffer[6];
  sprintf(buffer, "%02d/%02d", _currentTime.day, _currentTime.month);
  return String(buffer);
}

String BrgOfTheCyber_RTC::getDate() {
  getDay(); getMonth(); getYear(); // Обновляем дату
  char buffer[11];
  sprintf(buffer, "%02d/%02d/%04d", _currentTime.day, _currentTime.month, _currentTime.year);
  return String(buffer);
}

String BrgOfTheCyber_RTC::getDateTime() {
  // Обновляем все значения
  if (_mode == RTC_INTERNAL) _updateFromInternal();
  else _updateFromExternal();
  
  char buffer[20];
  sprintf(buffer, "%02d/%02d/%04d %02d:%02d:%02d",
          _currentTime.day, _currentTime.month, _currentTime.year,
          _currentTime.hour, _currentTime.minute, _currentTime.second);
  return String(buffer);
}

String BrgOfTheCyber_RTC::getWeekdayString() {
  static const char* days[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
  };
  uint8_t wd = getWeekday();
  return (wd >= 1 && wd <= 7) ? String(days[wd-1]) : String("Unknown");
}

// ===== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ =====

uint8_t BrgOfTheCyber_RTC::_decToBcd(uint8_t val) {
  return ((val / 10 * 16) + (val % 10));
}

uint8_t BrgOfTheCyber_RTC::_bcdToDec(uint8_t val) {
  return ((val / 16 * 10) + (val % 16));
}

bool BrgOfTheCyber_RTC::_isLeapYear(uint16_t year) {
  return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

uint8_t BrgOfTheCyber_RTC::_daysInMonth(uint16_t year, uint8_t month) {
  static const uint8_t days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (month == 2 && _isLeapYear(year)) return 29;
  return (month >= 1 && month <= 12) ? days[month-1] : 31;
}

void BrgOfTheCyber_RTC::_calculateWeekday() {
  // Алгоритм Зеллера
  uint16_t y = _currentTime.year;
  uint8_t m = _currentTime.month;
  uint8_t d = _currentTime.day;
  
  if (m < 3) {
    m += 12;
    y--;
  }
  
  uint16_t century = y / 100;
  y %= 100;
  
  uint8_t weekday = (d + (13 * (m + 1)) / 5 + y + y / 4 + century / 4 + 5 * century) % 7;
  
  // Преобразование: 0=Сб, 1=Вс, 2=Пн...6=Пт
  _currentTime.weekday = ((weekday + 5) % 7) + 1;
}

void BrgOfTheCyber_RTC::_updateFromInternal() {
  unsigned long currentMillis = millis();
  unsigned long elapsed = (unsigned long)((currentMillis - _lastMillis) * _driftFactor);
  
  // Добавляем прошедшие миллисекунды к смещению
  _millisOffset += elapsed;
  _lastMillis = currentMillis;
  
  // Преобразуем смещение в дату и время
  _epochToDateTime(_millisOffset / 1000);
}

void BrgOfTheCyber_RTC::_updateFromExternal() {
  switch (_type) {
    case RTC_DS3231: _readDS3231(); break;
    case RTC_DS1307: _readDS1307(); break;
    case RTC_PCF8563: _readPCF8563(); break;
  }
}

void BrgOfTheCyber_RTC::_saveToInternal() {
  // Сохраняем текущее смещение в EEPROM
  _millisOffset = _dateTimeToEpoch() * 1000;
  EEPROM.put(EEPROM_TIME_ADDR, _millisOffset);
  EEPROM.put(EEPROM_DRIFT_ADDR, _driftFactor);
}

void BrgOfTheCyber_RTC::_normalizeTime() {
  // Корректировка переполнений
  while (_currentTime.second >= 60) {
    _currentTime.second -= 60;
    _currentTime.minute++;
  }
  
  while (_currentTime.minute >= 60) {
    _currentTime.minute -= 60;
    _currentTime.hour++;
  }
  
  while (_currentTime.hour >= 24) {
    _currentTime.hour -= 24;
    _currentTime.day++;
    _currentTime.weekday = (_currentTime.weekday % 7) + 1;
  }
  
  while (_currentTime.day > _daysInMonth(_currentTime.year, _currentTime.month)) {
    _currentTime.day -= _daysInMonth(_currentTime.year, _currentTime.month);
    _currentTime.month++;
    
    if (_currentTime.month > 12) {
      _currentTime.month = 1;
      _currentTime.year++;
    }
  }
}

unsigned long BrgOfTheCyber_RTC::_dateTimeToEpoch() {
  // Простой расчет секунд с 2000-01-01
  unsigned long days = 0;
  
  // Добавляем дни за прошлые годы
  for (uint16_t y = 2000; y < _currentTime.year; y++) {
    days += _isLeapYear(y) ? 366 : 365;
  }
  
  // Добавляем дни за прошлые месяцы текущего года
  for (uint8_t m = 1; m < _currentTime.month; m++) {
    days += _daysInMonth(_currentTime.year, m);
  }
  
  // Добавляем дни
  days += (_currentTime.day - 1);
  
  // Переводим в секунды и добавляем время дня
  unsigned long seconds = days * 86400UL;
  seconds += _currentTime.hour * 3600UL;
  seconds += _currentTime.minute * 60UL;
  seconds += _currentTime.second;
  
  return seconds;
}

void BrgOfTheCyber_RTC::_epochToDateTime(unsigned long epoch) {
  unsigned long seconds = epoch;
  
  // Вычисляем годы
  _currentTime.year = 2000;
  while (seconds >= (_isLeapYear(_currentTime.year) ? 366UL : 365UL) * 86400UL) {
    seconds -= (_isLeapYear(_currentTime.year) ? 366UL : 365UL) * 86400UL;
    _currentTime.year++;
  }
  
  // Вычисляем месяцы
  _currentTime.month = 1;
  while (seconds >= _daysInMonth(_currentTime.year, _currentTime.month) * 86400UL) {
    seconds -= _daysInMonth(_currentTime.year, _currentTime.month) * 86400UL;
    _currentTime.month++;
  }
  
  // Вычисляем дни
  _currentTime.day = (seconds / 86400UL) + 1;
  seconds %= 86400UL;
  
  // Вычисляем время
  _currentTime.hour = seconds / 3600UL;
  seconds %= 3600UL;
  _currentTime.minute = seconds / 60UL;
  _currentTime.second = seconds % 60UL;
  
  _calculateWeekday();
}

// ===== ФУНКЦИИ ДЛЯ ВНЕШНИХ RTC =====

void BrgOfTheCyber_RTC::_initDS3231() {
  // Включаем осциллятор, если был выключен
  uint8_t status = _readRegister(0x0F);
  if (status & 0x80) { // Флаг остановки часов
    _writeRegister(0x0E, 0x00); // Сбрасываем флаг
  }
}

void BrgOfTheCyber_RTC::_initDS1307() {
  // Проверяем, не остановлены ли часы
  uint8_t seconds = _readRegister(0x00);
  if (seconds & 0x80) {
    _writeRegister(0x00, seconds & 0x7F); // Запускаем часы
  }
}

void BrgOfTheCyber_RTC::_initPCF8563() {
  // Сбрасываем флаги и включаем часы
  _writeRegister(0x00, 0x00); // Контроль 1: сброс
  _writeRegister(0x01, 0x00); // Контроль 2: очистка флагов
}

void BrgOfTheCyber_RTC::_readDS3231() {
  Wire.beginTransmission(_i2c_address);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(_i2c_address, 7);
  
  if (Wire.available() == 7) {
    _currentTime.second = _bcdToDec(Wire.read() & 0x7F);
    _currentTime.minute = _bcdToDec(Wire.read());
    _currentTime.hour = _bcdToDec(Wire.read() & 0x3F);
    _currentTime.weekday = _bcdToDec(Wire.read() & 0x07);
    _currentTime.day = _bcdToDec(Wire.read());
    _currentTime.month = _bcdToDec(Wire.read() & 0x1F);
    uint8_t year = _bcdToDec(Wire.read());
    _currentTime.year = 2000 + year;
  }
}

void BrgOfTheCyber_RTC::_readDS1307() {
  Wire.beginTransmission(_i2c_address);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(_i2c_address, 7);
  
  if (Wire.available() == 7) {
    _currentTime.second = _bcdToDec(Wire.read() & 0x7F);
    _currentTime.minute = _bcdToDec(Wire.read());
    _currentTime.hour = _bcdToDec(Wire.read() & 0x3F);
    _currentTime.weekday = _bcdToDec(Wire.read());
    _currentTime.day = _bcdToDec(Wire.read());
    _currentTime.month = _bcdToDec(Wire.read());
    uint8_t year = _bcdToDec(Wire.read());
    _currentTime.year = 2000 + year;
  }
}

void BrgOfTheCyber_RTC::_readPCF8563() {
  Wire.beginTransmission(_i2c_address);
  Wire.write(0x02); // Начинаем с регистра секунд
  Wire.endTransmission();
  Wire.requestFrom(_i2c_address, 7);
  
  if (Wire.available() == 7) {
    _currentTime.second = _bcdToDec(Wire.read() & 0x7F);
    _currentTime.minute = _bcdToDec(Wire.read() & 0x7F);
    _currentTime.hour = _bcdToDec(Wire.read() & 0x3F);
    _currentTime.day = _bcdToDec(Wire.read() & 0x3F);
    _currentTime.weekday = _bcdToDec(Wire.read() & 0x07);
    uint8_t century = Wire.read() & 0x80;
    _currentTime.month = _bcdToDec(Wire.read() & 0x1F);
    uint8_t year = _bcdToDec(Wire.read());
    _currentTime.year = (century ? 1900 : 2000) + year;
  }
}

void BrgOfTheCyber_RTC::_writeDS3231() {
  Wire.beginTransmission(_i2c_address);
  Wire.write(0x00);
  
  Wire.write(_decToBcd(_currentTime.second));
  Wire.write(_decToBcd(_currentTime.minute));
  Wire.write(_decToBcd(_currentTime.hour));
  Wire.write(_decToBcd(_currentTime.weekday));
  Wire.write(_decToBcd(_currentTime.day));
  Wire.write(_decToBcd(_currentTime.month));
  Wire.write(_decToBcd(_currentTime.year - 2000));
  
  Wire.endTransmission();
}

void BrgOfTheCyber_RTC::_writeDS1307() {
  Wire.beginTransmission(_i2c_address);
  Wire.write(0x00);
  
  Wire.write(_decToBcd(_currentTime.second));
  Wire.write(_decToBcd(_currentTime.minute));
  Wire.write(_decToBcd(_currentTime.hour));
  Wire.write(_decToBcd(_currentTime.weekday));
  Wire.write(_decToBcd(_currentTime.day));
  Wire.write(_decToBcd(_currentTime.month));
  Wire.write(_decToBcd(_currentTime.year - 2000));
  
  Wire.endTransmission();
}

void BrgOfTheCyber_RTC::_writePCF8563() {
  Wire.beginTransmission(_i2c_address);
  Wire.write(0x02);
  
  Wire.write(_decToBcd(_currentTime.second));
  Wire.write(_decToBcd(_currentTime.minute));
  Wire.write(_decToBcd(_currentTime.hour));
  Wire.write(_decToBcd(_currentTime.day));
  Wire.write(_decToBcd(_currentTime.weekday));
  Wire.write((_currentTime.year >= 2000) ? 0x00 : 0x80);
  Wire.write(_decToBcd(_currentTime.month));
  Wire.write(_decToBcd(_currentTime.year % 100));
  
  Wire.endTransmission();
}

void BrgOfTheCyber_RTC::_saveToExternal() {
  switch (_type) {
    case RTC_DS3231: _writeDS3231(); break;
    case RTC_DS1307: _writeDS1307(); break;
    case RTC_PCF8563: _writePCF8563(); break;
  }
}

uint8_t BrgOfTheCyber_RTC::_readRegister(uint8_t reg) {
  Wire.beginTransmission(_i2c_address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(_i2c_address, 1);
  return Wire.read();
}

void BrgOfTheCyber_RTC::_writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(_i2c_address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

// ===== ДОПОЛНИТЕЛЬНЫЕ ФУНКЦИИ =====

void BrgOfTheCyber_RTC::adjustInternalClock(long adjustment_ms) {
  if (_mode == RTC_INTERNAL) {
    _millisOffset += adjustment_ms;
    _lastMillis = millis();
    _saveToInternal();
  }
}

float BrgOfTheCyber_RTC::getInternalDrift() {
  return (_driftFactor - 1.0) * 3600000.0; // мс/час
}

float BrgOfTheCyber_RTC::getTemperature() {
  if (_mode == RTC_EXTERNAL_I2C && _type == RTC_DS3231) {
    Wire.beginTransmission(_i2c_address);
    Wire.write(0x11);
    Wire.endTransmission();
    Wire.requestFrom(_i2c_address, 2);
    
    if (Wire.available() == 2) {
      int8_t temp_msb = Wire.read();
      uint8_t temp_lsb = Wire.read();
      
      // Температура в градусах Цельсия
      return temp_msb + (temp_lsb >> 6) * 0.25;
    }
  }
  return -999; // Ошибка или не поддерживается
}

bool BrgOfTheCyber_RTC::lostPower() {
  if (_mode == RTC_EXTERNAL_I2C) {
    if (_type == RTC_DS3231) {
      uint8_t status = _readRegister(0x0F);
      return (status & 0x80) != 0;
    }
  }
  return false;
}

RTCMode BrgOfTheCyber_RTC::getMode() {
  return _mode;
}

bool BrgOfTheCyber_RTC::isRunning() {
  if (!_initialized) return false;
  
  if (_mode == RTC_EXTERNAL_I2C) {
    Wire.beginTransmission(_i2c_address);
    return (Wire.endTransmission() == 0);
  }
  
  return true; // Внутренние часы всегда "работают"
}

void BrgOfTheCyber_RTC::syncToExternal(RTCMode targetMode) {
  if (targetMode == _mode) return;
  
  DateTime tempTime = _currentTime;
  
  if (targetMode == RTC_EXTERNAL_I2C) {
    // Переключаемся на внешний RTC
    _mode = RTC_EXTERNAL_I2C;
    setDateTime(tempTime.year, tempTime.month, tempTime.day,
                tempTime.hour, tempTime.minute, tempTime.second);
  } else {
    // Переключаемся на внутренние часы
    _mode = RTC_INTERNAL;
    setDateTime(tempTime.year, tempTime.month, tempTime.day,
                tempTime.hour, tempTime.minute, tempTime.second);
  }
}