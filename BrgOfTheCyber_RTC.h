#ifndef BrgOfTheCyber_RTC_h
#define BrgOfTheCyber_RTC_h

#include <Arduino.h>
#include <Wire.h>

// Режимы работы библиотеки
enum RTCMode {
  RTC_INTERNAL,      // Использовать внутренние часы микроконтроллера
  RTC_EXTERNAL_I2C   // Использовать внешний RTC модуль по I2C
};

// Типы поддерживаемых внешних RTC
enum RTCType {
  RTC_DS3231,        // Модуль DS3231 (по умолчанию)
  RTC_DS1307,        // Модуль DS1307
  RTC_PCF8563        // Модуль PCF8563
};

class BrgOfTheCyber_RTC {
  public:
    // ===== КОНСТРУКТОРЫ =====
    // Для внутренних часов (без параметров или с режимом)
    BrgOfTheCyber_RTC();
    BrgOfTheCyber_RTC(RTCMode mode);
    
    // Для внешних RTC модулей
    BrgOfTheCyber_RTC(RTCType type, uint8_t i2c_address = 0x68);
    BrgOfTheCyber_RTC(RTCMode mode, RTCType type, uint8_t i2c_address = 0x68);
    
    // ===== ИНИЦИАЛИЗАЦИЯ =====
    bool begin(); // Инициализация библиотеки
    
    // ===== УСТАНОВКА ВРЕМЕНИ =====
    void setDateTime(uint16_t year, uint8_t month, uint8_t day,
                     uint8_t hour, uint8_t minute, uint8_t second);
    void setDateTimeFromCompileTime(); // Установить время из времени компиляции
    
    // ===== ФУНКЦИИ ЗАПРОСА (полный список) =====
    uint16_t getYear();
    uint8_t getMonth();
    uint8_t getDay();
    uint8_t getHour();
    uint8_t getMinute();
    uint8_t getSecond();
    uint8_t getWeekday(); // День недели (1-7, 1=воскресенье)
    
    // Форматированный вывод
    String getTime(bool tickingEffect = false);
    String getToDay();
    String getDate();
    String getDateTime(); // Полная дата и время
    String getWeekdayString(); // Название дня недели
    
    // ===== ДОПОЛНИТЕЛЬНЫЕ ФУНКЦИИ =====
    // Только для внутренних часов
    void adjustInternalClock(long adjustment_ms); // Корректировка внутренних часов
    float getInternalDrift(); // Получить дрейф внутренних часов (мс/час)
    
    // Только для внешних RTC
    float getTemperature(); // Получить температуру (только для DS3231)
    bool lostPower(); // Проверка потери питания (только для внешних RTC)
    
    // Общие функции
    RTCMode getMode(); // Получить текущий режим работы
    bool isRunning(); // Проверка, работают ли часы
    void syncToExternal(RTCMode targetMode); // Синхронизация между режимами
    
  private:
    // Переменные состояния
    RTCMode _mode;
    RTCType _type;
    uint8_t _i2c_address;
    bool _initialized;
    
    // Время в удобном формате
    struct DateTime {
      uint16_t year;
      uint8_t month;
      uint8_t day;
      uint8_t hour;
      uint8_t minute;
      uint8_t second;
      uint8_t weekday;
    } _currentTime;
    
    // Для внутренних часов
    unsigned long _lastMillis;
    unsigned long _millisOffset;
    float _driftFactor; // Коэффициент дрейфа
    
    // Для внешних RTC
    uint8_t _readRegister(uint8_t reg);
    void _writeRegister(uint8_t reg, uint8_t value);
    
    // Вспомогательные функции
    uint8_t _decToBcd(uint8_t val);
    uint8_t _bcdToDec(uint8_t val);
    bool _isLeapYear(uint16_t year);
    uint8_t _daysInMonth(uint16_t year, uint8_t month);
    void _calculateWeekday();
    void _updateFromInternal();
    void _updateFromExternal();
    void _saveToInternal();
    void _saveToExternal();
    void _normalizeTime();
    unsigned long _dateTimeToEpoch();
    void _epochToDateTime(unsigned long epoch);
    
    // Функции для разных типов RTC
    void _initDS3231();
    void _initDS1307();
    void _initPCF8563();
    void _readDS3231();
    void _readDS1307();
    void _readPCF8563();
    void _writeDS3231();
    void _writeDS1307();
    void _writePCF8563();
};

#endif