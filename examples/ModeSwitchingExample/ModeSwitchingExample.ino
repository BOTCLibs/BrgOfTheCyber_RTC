/*
  BrgOfTheCyber_RTC - ModeSwitchingExample
  Демонстрация переключения между внутренними и внешними часами
*/

#include <BrgOfTheCyber_RTC.h>

BrgOfTheCyber_RTC rtc;
bool useExternalRTC = true;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  Serial.println("=== Режимы работы RTC ===");
  
  // Пытаемся использовать внешний RTC
  if (rtc.begin()) {
    Serial.println("Внешний RTC найден");
    rtc.setDateTimeFromCompileTime();
    useExternalRTC = true;
  } else {
    Serial.println("Внешний RTC не найден, переключаемся на внутренние часы");
    rtc.syncToExternal(RTC_INTERNAL);
    rtc.setDateTimeFromCompileTime();
    useExternalRTC = false;
  }
  
  printCurrentMode();
}

void loop() {
  static unsigned long lastUpdate = 0;
  static bool blink = false;
  
  if (millis() - lastUpdate >= 1000) {
    lastUpdate = millis();
    blink = !blink;
    
    Serial.print("\r"); // Возврат каретки
    
    if (blink) {
      Serial.print("Время: ");
      Serial.print(rtc.getDateTime());
    } else {
      Serial.print("Время: ");
      Serial.print(rtc.getDate());
      Serial.print(" ");
      Serial.print(rtc.getTime(true)); // С эффектом тикания
    }
    
    Serial.print(" [");
    Serial.print(rtc.getMode() == RTC_INTERNAL ? "Внутр." : "Внешн.");
    Serial.print("]");
    
    // Каждые 10 секунд переключаем режим (для демонстрации)
    static unsigned long lastSwitch = 0;
    if (millis() - lastSwitch >= 10000) {
      lastSwitch = millis();
      
      if (useExternalRTC) {
        Serial.print("\nПереключаемся на внутренние часы...");
        rtc.syncToExternal(RTC_INTERNAL);
        useExternalRTC = false;
      } else {
        Serial.print("\nПытаемся переключиться на внешний RTC...");
        if (rtc.syncToExternal(RTC_EXTERNAL_I2C)) {
          useExternalRTC = true;
          Serial.print("Успешно!");
        } else {
          Serial.print("Не удалось!");
        }
      }
      
      printCurrentMode();
    }
  }
}

void printCurrentMode() {
  Serial.print("\nТекущий режим: ");
  if (rtc.getMode() == RTC_INTERNAL) {
    Serial.println("ВНУТРЕННИЕ ЧАСЫ");
    Serial.println("Примечание: время может дрейфовать!");
  } else {
    Serial.println("ВНЕШНИЙ RTC МОДУЛЬ");
    if (rtc.lostPower()) {
      Serial.println("Внимание: RTC терял питание!");
    }
  }
  Serial.println("-------------------");
}