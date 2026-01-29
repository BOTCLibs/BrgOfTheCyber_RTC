/*
  BrgOfTheCyber_RTC - AdvancedFeatures
  Демонстрация расширенных функций
*/

#include <BrgOfTheCyber_RTC.h>
#include <LiquidCrystal_I2C.h>

BrgOfTheCyber_RTC rtc(RTC_INTERNAL);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Пины для кнопок
const uint8_t BTN_SET = 2;
const uint8_t BTN_UP = 3;
const uint8_t BTN_DOWN = 4;
const uint8_t BTN_MODE = 5;

// Переменные для меню
enum Menu { CLOCK, DATE, TIMER, ALARM, SETTINGS };
Menu currentMenu = CLOCK;
bool settingMode = false;

void setup() {
  Serial.begin(9600);
  
  // Инициализация LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  // Инициализация кнопок
  pinMode(BTN_SET, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_MODE, INPUT_PULLUP);
  
  // Инициализация RTC
  if (!rtc.begin()) {
    lcd.print("RTC Error!");
    while(1);
  }
  
  // Установка времени компиляции
  rtc.setDateTimeFromCompileTime();
  
  lcd.print("BrgOfTheCyber RTC");
  lcd.setCursor(0, 1);
  lcd.print("v1.0 Initialized");
  delay(2000);
  lcd.clear();
}

void loop() {
  static unsigned long lastUpdate = 0;
  static bool colonVisible = true;
  
  // Обновление дисплея каждые 500 мс
  if (millis() - lastUpdate >= 500) {
    lastUpdate = millis();
    colonVisible = !colonVisible;
    
    updateDisplay(colonVisible);
  }
  
  // Обработка кнопок
  handleButtons();
  
  // Таймер сна (если нужен)
  checkSleepTimer();
}

void updateDisplay(bool colon) {
  lcd.clear();
  
  switch (currentMenu) {
    case CLOCK:
      lcd.setCursor(4, 0);
      lcd.print("ВРЕМЯ");
      lcd.setCursor(3, 1);
      lcd.print(rtc.getTime(!colon)); // Эффект тикания
      break;
      
    case DATE:
      lcd.setCursor(5, 0);
      lcd.print("ДАТА");
      lcd.setCursor(0, 1);
      lcd.print(rtc.getDate());
      lcd.setCursor(11, 1);
      lcd.print(rtc.getWeekdayString().substring(0, 3));
      break;
      
    case TIMER:
      lcd.setCursor(5, 0);
      lcd.print("ТАЙМЕР");
      lcd.setCursor(4, 1);
      lcd.print("00:00:00");
      break;
      
    case ALARM:
      lcd.setCursor(5, 0);
      lcd.print("БУДИЛЬНИК");
      lcd.setCursor(4, 1);
      lcd.print("07:30 ON");
      break;
      
    case SETTINGS:
      lcd.setCursor(3, 0);
      lcd.print("НАСТРОЙКИ");
      lcd.setCursor(0, 1);
      lcd.print("RTC:");
      lcd.print(rtc.getMode() == RTC_INTERNAL ? "Internal" : "External");
      break;
  }
  
  // Индикация режима
  lcd.setCursor(15, 0);
  lcd.print(rtc.getMode() == RTC_INTERNAL ? "I" : "E");
}

void handleButtons() {
  // Кнопка MODE - переключение меню
  if (digitalRead(BTN_MODE) == LOW) {
    delay(50);
    if (digitalRead(BTN_MODE) == LOW) {
      currentMenu = (Menu)((currentMenu + 1) % 5);
      while(digitalRead(BTN_MODE) == LOW);
    }
  }
  
  // Кнопка SET - вход/выход из режима настройки
  if (digitalRead(BTN_SET) == LOW) {
    delay(50);
    if (digitalRead(BTN_SET) == LOW) {
      settingMode = !settingMode;
      while(digitalRead(BTN_SET) == LOW);
    }
  }
  
  // Кнопки UP/DOWN - настройка времени в режиме настройки
  if (settingMode && currentMenu == CLOCK) {
    if (digitalRead(BTN_UP) == LOW) {
      rtc.adjustInternalClock(60000); // +1 минута
      delay(200);
    }
    if (digitalRead(BTN_DOWN) == LOW) {
      rtc.adjustInternalClock(-60000); // -1 минута
      delay(200);
    }
  }
}

void checkSleepTimer() {
  // Пример: выключение дисплея ночью
  uint8_t hour = rtc.getHour();
  if (hour >= 23 || hour < 7) {
    lcd.noBacklight();
  } else {
    lcd.backlight();
  }
  
  // Пример будильника на 7:30
  if (hour == 7 && rtc.getMinute() == 30 && rtc.getSecond() == 0) {
    triggerAlarm();
  }
}

void triggerAlarm() {
  // Здесь может быть код для включения звука или светодиода
  for (int i = 0; i < 3; i++) {
    lcd.setBacklight(255);
    delay(200);
    lcd.setBacklight(0);
    delay(200);
  }
  lcd.setBacklight(255);
}