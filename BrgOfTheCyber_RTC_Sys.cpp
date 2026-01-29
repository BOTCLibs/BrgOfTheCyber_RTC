#include "BrgOfTheCyber_RTC_Sys.h"
#include <EEPROM.h>

// Структура для сохранения в EEPROM
struct TimeData {
  unsigned long epoch;      // Секунды с 2000-01-01
  unsigned long lastMillis; // Последнее значение millis()
  bool initialized;
};

BrgOfTheCyber_RTC_Sys::BrgOfTheCyber_RTC_Sys() {
  _time.year = 2025;
  _time.month = 1;
  _time.day = 1;
  _time.hour = 0;
  _time.minute = 0;
  _time.second = 0;
  _lastUpdate = 0;
}

void BrgOfTheCyber_RTC_Sys::begin() {
  // Читаем сохраненное время из EEPROM
  TimeData savedTime;
  EEPROM.get(0, savedTime);
  
  if (savedTime.initialized) {
    // Восстанавливаем время из EEPROM
    _lastUpdate = millis();
    // Преобразуем epoch в дату и время
    _secondsToDateTime(savedTime.epoch + ((_lastUpdate - savedTime.lastMillis) / 1000));
  } else {
    // Первый запуск - устанавливаем время компиляции
    setDateTimeFromCompile();
  }
}

void BrgOfTheCyber_RTC_Sys::setDateTime(uint16_t year, uint8_t month, uint8_t day,
                                       uint8_t hour, uint8_t minute, uint8_t second) {
  _time.year = year;
  _time.month = month;
  _time.day = day;
  _time.hour = hour;
  _time.minute = minute;
  _time.second = second;
  
  _normalizeTime();
  _lastUpdate = millis();
  
  // Сохраняем в EEPROM
  TimeData toSave;
  toSave.epoch = _dateTimeToSeconds();
  toSave.lastMillis = _lastUpdate;
  toSave.initialized = true;
  
  EEPROM.put(0, toSave);
}

void BrgOfTheCyber_RTC_Sys::setDateTimeFromCompile() {
  // Временные переменные для парсинга
  char monthStr[4];
  int year, day, hour, minute, second;
  
  // Парсим __DATE__ в формате "MMM DD YYYY"
  sscanf(__DATE__, "%s %d %d", monthStr, &day, &year);
  
  // Парсим __TIME__ в формате "HH:MM:SS"
  sscanf(__TIME__, "%d:%d:%d", &hour, &minute, &second);
  
  // Преобразуем месяц из строки в число
  const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                         "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  uint8_t month = 1;
  for (int i = 0; i < 12; i++) {
    if (strcmp(monthStr, months[i]) == 0) {
      month = i + 1;
      break;
    }
  }
  
  setDateTime(year, month, day, hour, minute, second);
}

uint16_t BrgOfTheCyber_RTC_Sys::getYear() {
  _updateTime();
  return _time.year;
}

uint8_t BrgOfTheCyber_RTC_Sys::getMonth() {
  _updateTime();
  return _time.month;
}

uint8_t BrgOfTheCyber_RTC_Sys::getDay() {
  _updateTime();
  return _time.day;
}

uint8_t BrgOfTheCyber_RTC_Sys::getHour() {
  _updateTime();
  return _time.hour;
}

uint8_t BrgOfTheCyber_RTC_Sys::getMinute() {
  _updateTime();
  return _time.minute;
}

uint8_t BrgOfTheCyber_RTC_Sys::getSecond() {
  _updateTime();
  return _time.second;
}

String BrgOfTheCyber_RTC_Sys::getTime(bool tickingEffect) {
  _updateTime();
  
  char buffer[6];
  if (tickingEffect && (_time.second % 2 == 1)) {
    // Нечетная секунда - пробел вместо двоеточия
    sprintf(buffer, "%02d %02d", _time.hour, _time.minute);
  } else {
    // Четная секунда или обычный режим - двоеточие
    sprintf(buffer, "%02d:%02d", _time.hour, _time.minute);
  }
  return String(buffer);
}

String BrgOfTheCyber_RTC_Sys::getToDay() {
  _updateTime();
  char buffer[6];
  sprintf(buffer, "%02d/%02d", _time.day, _time.month);
  return String(buffer);
}

String BrgOfTheCyber_RTC_Sys::getDate() {
  _updateTime();
  char buffer[11];
  sprintf(buffer, "%02d/%02d/%04d", _time.day, _time.month, _time.year);
  return String(buffer);
}

String BrgOfTheCyber_RTC_Sys::getDateTime() {
  _updateTime();
  char buffer[20];
  sprintf(buffer, "%02d/%02d/%04d %02d:%02d:%02d",
          _time.day, _time.month, _time.year,
          _time.hour, _time.minute, _time.second);
  return String(buffer);
}

void BrgOfTheCyber_RTC_Sys::adjustTime(long adjustment_ms) {
  // Просто сдвигаем отметку времени
  _lastUpdate += adjustment_ms;
  
  // Сохраняем в EEPROM
  TimeData toSave;
  toSave.epoch = _dateTimeToSeconds();
  toSave.lastMillis = _lastUpdate;
  toSave.initialized = true;
  
  EEPROM.put(0, toSave);
}

// Вспомогательные функции
bool BrgOfTheCyber_RTC_Sys::_isLeapYear(uint16_t year) {
  return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

uint8_t BrgOfTheCyber_RTC_Sys::_daysInMonth(uint16_t year, uint8_t month) {
  static const uint8_t days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (month == 2 && _isLeapYear(year)) return 29;
  return (month >= 1 && month <= 12) ? days[month-1] : 31;
}

void BrgOfTheCyber_RTC_Sys::_normalizeTime() {
  // Корректируем секунды
  while (_time.second >= 60) {
    _time.second -= 60;
    _time.minute++;
  }
  
  // Корректируем минуты
  while (_time.minute >= 60) {
    _time.minute -= 60;
    _time.hour++;
  }
  
  // Корректируем часы
  while (_time.hour >= 24) {
    _time.hour -= 24;
    _time.day++;
  }
  
  // Корректируем дни
  while (_time.day > _daysInMonth(_time.year, _time.month)) {
    _time.day -= _daysInMonth(_time.year, _time.month);
    _time.month++;
    
    if (_time.month > 12) {
      _time.month = 1;
      _time.year++;
    }
  }
}

unsigned long BrgOfTheCyber_RTC_Sys::_dateTimeToSeconds() {
  // Считаем секунды от 2000-01-01 00:00:00
  unsigned long totalDays = 0;
  
  // Дни за полные годы с 2000
  for (uint16_t y = 2000; y < _time.year; y++) {
    totalDays += _isLeapYear(y) ? 366 : 365;
  }
  
  // Дни за полные месяцы текущего года
  for (uint8_t m = 1; m < _time.month; m++) {
    totalDays += _daysInMonth(_time.year, m);
  }
  
  // Дни текущего месяца (минус 1, так как 1 января = 0 дней)
  totalDays += (_time.day - 1);
  
  // Переводим дни в секунды и добавляем время дня
  unsigned long totalSeconds = totalDays * 86400UL;
  totalSeconds += _time.hour * 3600UL;
  totalSeconds += _time.minute * 60UL;
  totalSeconds += _time.second;
  
  return totalSeconds;
}

void BrgOfTheCyber_RTC_Sys::_secondsToDateTime(unsigned long totalSeconds) {
  // Начинаем с 2000-01-01
  _time.year = 2000;
  
  // Вычисляем год
  while (totalSeconds >= (_isLeapYear(_time.year) ? 366UL : 365UL) * 86400UL) {
    totalSeconds -= (_isLeapYear(_time.year) ? 366UL : 365UL) * 86400UL;
    _time.year++;
  }
  
  // Вычисляем месяц
  _time.month = 1;
  while (totalSeconds >= _daysInMonth(_time.year, _time.month) * 86400UL) {
    totalSeconds -= _daysInMonth(_time.year, _time.month) * 86400UL;
    _time.month++;
  }
  
  // Вычисляем день (добавляем 1, так как день начинается с 1)
  _time.day = (totalSeconds / 86400UL) + 1;
  totalSeconds %= 86400UL;
  
  // Вычисляем часы, минуты, секунды
  _time.hour = totalSeconds / 3600UL;
  totalSeconds %= 3600UL;
  _time.minute = totalSeconds / 60UL;
  _time.second = totalSeconds % 60UL;
}

void BrgOfTheCyber_RTC_Sys::_updateTime() {
  unsigned long currentMillis = millis();
  unsigned long elapsedMillis = currentMillis - _lastUpdate;
  
  // Если прошло меньше секунды, не обновляем
  if (elapsedMillis < 1000) return;
  
  // Преобразуем текущее время в секунды
  unsigned long currentSeconds = _dateTimeToSeconds();
  
  // Добавляем прошедшие секунды
  currentSeconds += (elapsedMillis / 1000);
  
  // Преобразуем обратно в дату и время
  _secondsToDateTime(currentSeconds);
  
  // Обновляем _lastUpdate, сохраняя остаток миллисекунд
  _lastUpdate = currentMillis - (elapsedMillis % 1000);
}