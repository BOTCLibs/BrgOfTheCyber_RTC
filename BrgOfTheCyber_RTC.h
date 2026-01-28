#ifndef BrgOfTheCyber_RTC_h
#define BrgOfTheCyber_RTC_h

#include <Arduino.h>
#include <Wire.h> // Для работы с I2C (большинство RTC модулей)

class BrgOfTheCyber_RTC {
  public:
    // Конструктор. Можно указать адрес I2C (по умолчанию 0x68 для DS3231)
    BrgOfTheCyber_RTC(uint8_t i2c_addr = 0x68);
    
    // ===== 1. ИНИЦИАЛИЗАЦИЯ =====
    // Инициализация библиотеки и проверка связи с модулем.
    // Возвращает true, если модуль найден.
    bool begin();

    // ===== 2. УСТАНОВКА ВРЕМЕНИ =====
    // Установка текущей даты и времени.
    void setDateTime(uint16_t year, uint8_t month, uint8_t day,
                     uint8_t hour, uint8_t minute, uint8_t second);

    // ===== 3. ФУНКЦИИ ЗАПРОСА =====
    // 3.1 - 3.6: Получение отдельных компонентов
    uint16_t getYear();   // Пример: 2025
    uint8_t getMonth();   // Пример: 1
    uint8_t getDay();     // Пример: 1
    uint8_t getHour();    // Пример: 1
    uint8_t getMinute();  // Пример: 1
    uint8_t getSecond();  // Пример: 1

    // 3.7 - 3.8: Получение времени в формате строки
    String getTime(bool tickingEffect = false); // Пример: "00:01" или "00 01"
    
    // 3.9: Получение даты (день/месяц)
    String getToDay(); // Пример: "01/01"
    
    // 3.10: Получение полной даты
    String getDate(); // Пример: "01/01/2025"
    
    // ===== ДОПОЛНИТЕЛЬНЫЕ ФУНКЦИИ =====
    // Получение всей даты и времени одной командой (оптимизация для DS3231)
    void refreshDateTime();
    
    // Проверка, является ли год високосным (внутренняя функция)
    bool isLeapYear(uint16_t year);

  private:
    uint8_t _i2c_addr;
    // Переменные для хранения последних прочитанных данных
    uint16_t _year;
    uint8_t _month, _day, _hour, _minute, _second;
    
    // Функция для преобразования чисел в BCD формат (для отправки в RTC)
    uint8_t _decToBcd(uint8_t val);
    
    // Функция для преобразования из BCD в обычное число (для чтения из RTC)
    uint8_t _bcdToDec(uint8_t val);
};

#endif