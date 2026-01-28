/*
  BrgOfTheCyber_RTC - AdvancedExample
  Демонстрация часов реального времени с дисплеем и кнопками для настройки
  
  Оборудование:
  - DS3231 RTC модуль
  - LCD 16x2 дисплей (I2C)
  - 3 кнопки (Настройка, Плюс, Минус)
*/

#include <BrgOfTheCyber_RTC.h>
#include <LiquidCrystal_I2C.h>

BrgOfTheCyber_RTC rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Адрес LCD 0x27, 16x2

// Пины для кнопок
const int SET_BTN = 2;
const int UP_BTN = 3;
const int DOWN_BTN = 4;

// Режимы настройки
enum SetupMode { NONE, SET_HOUR, SET_MINUTE, SET_DAY, SET_MONTH, SET_YEAR };
SetupMode setupMode = NONE;

void setup() {
  Serial.begin(9600);
  
  // Инициализация LCD
  lcd.init();
  lcd.backlight();
  
  // Инициализация кнопок
  pinMode(SET_BTN, INPUT_PULLUP);
  pinMode(UP_BTN, INPUT_PULLUP);
  pinMode(DOWN_BTN, INPUT_PULLUP);
  
  // Инициализация RTC
  if (!rtc.begin()) {
    lcd.print("RTC Error!");
    while(1);
  }
  
  lcd.print("RTC Ready");
  delay(1000);
  lcd.clear();
}

void loop() {
  static unsigned long lastUpdate = 0;
  static bool blinkState = false;
  
  // Обработка кнопки настройки
  if (digitalRead(SET_BTN) == LOW) {
    delay(50); // Антидребезг
    if (digitalRead(SET_BTN) == LOW) {
      switch (setupMode) {
        case NONE: setupMode = SET_HOUR; break;
        case SET_HOUR: setupMode = SET_MINUTE; break;
        case SET_MINUTE: setupMode = SET_DAY; break;
        case SET_DAY: setupMode = SET_MONTH; break;
        case SET_MONTH: setupMode = SET_YEAR; break;
        case SET_YEAR:
          // Выход из режима настройки
          setupMode = NONE;
          // Применяем изменения
          rtc.setDateTime(
            rtc.getYear(), rtc.getMonth(), rtc.getDay(),
            rtc.getHour(), rtc.getMinute(), rtc.getSecond()
          );
          break;
      }
      while(digitalRead(SET_BTN) == LOW); // Ждем отпускания
    }
  }
  
  // Обновление дисплея каждые 500 мс
  if (millis() - lastUpdate >= 500) {
    lastUpdate = millis();
    blinkState = !blinkState;
    
    lcd.clear();
    lcd.setCursor(0, 0);
    
    if (setupMode == NONE) {
      // Обычный режим отображения
      lcd.print(rtc.getDate());
      lcd.setCursor(0, 1);
      lcd.print("Time: ");
      lcd.print(rtc.getTime());
    } else {
      // Режим настройки
      lcd.print("SET ");
      switch (setupMode) {
        case SET_HOUR:
          lcd.print("HOUR:");
          if (blinkState) lcd.print(rtc.getHour());
          break;
        case SET_MINUTE:
          lcd.print("MIN:");
          if (blinkState) lcd.print(rtc.getMinute());
          break;
        case SET_DAY:
          lcd.print("DAY:");
          if (blinkState) lcd.print(rtc.getDay());
          break;
        case SET_MONTH:
          lcd.print("MON:");
          if (blinkState) lcd.print(rtc.getMonth());
          break;
        case SET_YEAR:
          lcd.print("YEAR:");
          if (blinkState) lcd.print(rtc.getYear());
          break;
      }
    }
  }
  
  // Обработка кнопок +/-
  if (setupMode != NONE) {
    adjustValue();
  }
  
  delay(10);
}

void adjustValue() {
  static unsigned long lastAdjust = 0;
  
  if (millis() - lastAdjust < 200) return; // Защита от слишком быстрых нажатий
  
  uint16_t year = rtc.getYear();
  uint8_t month = rtc.getMonth();
  uint8_t day = rtc.getDay();
  uint8_t hour = rtc.getHour();
  uint8_t minute = rtc.getMinute();
  
  if (digitalRead(UP_BTN) == LOW) {
    switch (setupMode) {
      case SET_HOUR: hour = (hour + 1) % 24; break;
      case SET_MINUTE: minute = (minute + 1) % 60; break;
      case SET_DAY: day = max(1, min(31, day + 1)); break;
      case SET_MONTH: month = max(1, min(12, month + 1)); break;
      case SET_YEAR: year++; break;
    }
    lastAdjust = millis();
  }
  
  if (digitalRead(DOWN_BTN) == LOW) {
    switch (setupMode) {
      case SET_HOUR: hour = (hour + 23) % 24; break;
      case SET_MINUTE: minute = (minute + 59) % 60; break;
      case SET_DAY: day = max(1, min(31, day - 1)); break;
      case SET_MONTH: month = max(1, min(12, month - 1)); break;
      case SET_YEAR: year = max(2000, year - 1); break;
    }
    lastAdjust = millis();
  }
  
  // Временно обновляем время (финальное сохранение при выходе из режима)
  if (setupMode != NONE) {
    rtc.setDateTime(year, month, day, hour, minute, rtc.getSecond());
  }
}