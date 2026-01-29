/*
  BrgOfTheCyber_RTC - BasicExample
  Демонстрация работы с внутренними и внешними часами
*/

#include <BrgOfTheCyber_RTC.h>

// Создаем три разных экземпляра
BrgOfTheCyber_RTC internalRTC;                    // Внутренние часы
BrgOfTheCyber_RTC externalDS3231(RTC_DS3231);     // Внешний DS3231
BrgOfTheCyber_RTC externalDS1307(RTC_DS1307, 0x68); // Внешний DS1307

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  Serial.println("=== Тестирование библиотеки BrgOfTheCyber_RTC ===");
  
  // 1. Тестируем внутренние часы
  Serial.println("\n1. ВНУТРЕННИЕ ЧАСЫ:");
  if (internalRTC.begin()) {
    internalRTC.setDateTimeFromCompileTime();
    
    Serial.print("Дата и время: ");
    Serial.println(internalRTC.getDateTime());
    
    Serial.print("День недели: ");
    Serial.println(internalRTC.getWeekdayString());
    
    Serial.print("Только время: ");
    Serial.println(internalRTC.getTime());
    
    Serial.print("Только дата: ");
    Serial.println(internalRTC.getDate());
    
    Serial.print("Время с эффектом тикания: ");
    Serial.println(internalRTC.getTime(true));
  }
  
  // 2. Тестируем внешний DS3231
  Serial.println("\n2. ВНЕШНИЙ DS3231:");
  if (externalDS3231.begin()) {
    // Устанавливаем время только если часы остановлены
    if (externalDS3231.lostPower()) {
      externalDS3231.setDateTimeFromCompileTime();
      Serial.println("Время установлено из времени компиляции");
    }
    
    Serial.print("Дата и время: ");
    Serial.println(externalDS3231.getDateTime());
    
    Serial.print("Температура DS3231: ");
    Serial.print(externalDS3231.getTemperature());
    Serial.println("°C");
  } else {
    Serial.println("DS3231 не найден!");
  }
}

void loop() {
  static unsigned long lastUpdate = 0;
  
  if (millis() - lastUpdate >= 1000) {
    lastUpdate = millis();
    
    Serial.print("\nВнутренние часы: ");
    Serial.print(internalRTC.getTime());
    Serial.print(" | Внешние часы (DS3231): ");
    Serial.print(externalDS3231.getTime());
    
    Serial.print(" | Разница: ");
    int diff = abs(internalRTC.getHour() * 60 + internalRTC.getMinute() -
                   externalDS3231.getHour() * 60 - externalDS3231.getMinute());
    Serial.print(diff);
    Serial.println(" минут");
    
    // Корректировка внутренних часов, если разница больше 1 минуты
    if (diff > 1 && externalDS3231.isRunning()) {
      Serial.println("Синхронизируем внутренние часы с внешними...");
      internalRTC.setDateTime(
        externalDS3231.getYear(), externalDS3231.getMonth(), externalDS3231.getDay(),
        externalDS3231.getHour(), externalDS3231.getMinute(), externalDS3231.getSecond()
      );
    }
  }
}