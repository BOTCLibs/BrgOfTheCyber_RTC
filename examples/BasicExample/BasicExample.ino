/*
  BrgOfTheCyber_RTC - BasicExample
  Демонстрация основных функций библиотеки для работы с RTC модулем DS3231
  
  Подключение модуля DS3231 к Arduino:
  VCC -> 5V
  GND -> GND
  SDA -> A4 (или SDA на других платах)
  SCL -> A5 (или SCL на других платах)
*/

#include <BrgOfTheCyber_RTC.h>

// Создаем объект RTC
BrgOfTheCyber_RTC rtc;

void setup() {
  Serial.begin(9600);
  while (!Serial); // Для плат с USB-подключением
  
  Serial.println("=== Библиотека BrgOfTheCyber_RTC ===");
  
  // Инициализация RTC
  if (!rtc.begin()) {
    Serial.println("Ошибка: RTC модуль не найден!");
    while (1);
  }
  Serial.println("RTC модуль инициализирован.");
  
  // Установка времени (раскомментируйте, чтобы установить время один раз)
  // После установки закомментируйте эту строкю и загрузите скетч снова,
  // чтобы время не сбрасывалось при каждом запуске.
  // rtc.setDateTime(2025, 1, 1, 12, 0, 0); // 1 января 2025, 12:00:00
  
  Serial.println("Текущее время установлено.");
}

void loop() {
  Serial.println("\n=== Информация с RTC ===");
  
  // 1. Вывод отдельных компонентов
  Serial.print("Год: ");
  Serial.println(rtc.getYear());
  
  Serial.print("Месяц: ");
  Serial.println(rtc.getMonth());
  
  Serial.print("День: ");
  Serial.println(rtc.getDay());
  
  Serial.print("Час: ");
  Serial.println(rtc.getHour());
  
  Serial.print("Минута: ");
  Serial.println(rtc.getMinute());
  
  Serial.print("Секунда: ");
  Serial.println(rtc.getSecond());
  
  // 2. Форматированный вывод
  Serial.print("Время (обычное): ");
  Serial.println(rtc.getTime());
  
  Serial.print("Время (тик-так): ");
  Serial.println(rtc.getTime(true)); // Эффект тикания
  
  Serial.print("Дата (день/месяц): ");
  Serial.println(rtc.getToDay());
  
  Serial.print("Полная дата: ");
  Serial.println(rtc.getDate());
  
  // 3. Пример создания собственного формата
  Serial.print("\nПользовательский формат: ");
  Serial.print(rtc.getDay());
  Serial.print(".");
  Serial.print(rtc.getMonth());
  Serial.print(".");
  Serial.print(rtc.getYear());
  Serial.print(" ");
  Serial.println(rtc.getTime());
  
  delay(5000); // Обновляем каждые 5 секунд
}