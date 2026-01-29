#ifndef BrgOfTheCyber_RTC_Sys_h
#define BrgOfTheCyber_RTC_Sys_h

#include <Arduino.h>

class BrgOfTheCyber_RTC_Sys {
  public:
    // Конструктор
    BrgOfTheCyber_RTC_Sys();
    
    // Инициализация
    void begin();
    
    // Установка времени
    void setDateTime(uint16_t year, uint8_t month, uint8_t day,
                     uint8_t hour, uint8_t minute, uint8_t second);
    void setDateTimeFromCompile();
    
    // Получение времени
    uint16_t getYear();
    uint8_t getMonth();
    uint8_t getDay();
    uint8_t getHour();
    uint8_t getMinute();
    uint8_t getSecond();
    
    // Форматированный вывод
    String getTime(bool tickingEffect = false);
    String getToDay();
    String getDate();
    String getDateTime();
    
    // Дополнительные функции
    void adjustTime(long adjustment_ms);
    
  private:
    // Структура для хранения времени
    struct DateTime {
      uint16_t year;
      uint8_t month;
      uint8_t day;
      uint8_t hour;
      uint8_t minute;
      uint8_t second;
    };
    
    DateTime _time;
    unsigned long _lastUpdate;
    
    // Вспомогательные функции
    bool _isLeapYear(uint16_t year);
    uint8_t _daysInMonth(uint16_t year, uint8_t month);
    void _normalizeTime();
    unsigned long _dateTimeToSeconds();
    void _secondsToDateTime(unsigned long seconds);
    void _updateTime();
};

#endif