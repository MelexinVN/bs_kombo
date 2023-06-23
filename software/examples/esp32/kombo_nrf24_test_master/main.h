/*
* main.h
* заголовочный файл тестового примера ведущего устройства с модулем NRF24L01.
* Проект "КомБО" (Открытые системы беспроводной коммуникации)
*
* Платформа: ESP32
*
* Здесь необходимо сконфигурировать:
* пин и порт светодиода,
* число ведомых устройств, список адресов
*
* Автор: Мелехин В.Н. (MelexinVN)
*/

#ifndef MAIN_H_
#define MAIN_H_

#include <Ticker.h>
#include <SPI.h>          //библиотека интерфейса SPI
#include "kombo_nrf24_esp32.h"  //библиотека радиомодуля nrf24l01
#include <Arduino.h>
#include "BluetoothSerial.h"

//Настройки
#define NUM_OF_SLAVES 20  //количество ведомых устройств
//Список адресов устройств
#define ADRESS_LIST \
  { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13 }

#define LED 2       //пин светодиода

//Макросы управления светодиодом
#define LED_ON() digitalWrite(LED, HIGH);  //включение светодиода
#define LED_OFF() digitalWrite(LED, LOW);  //выключение светодиода

//Прочие настройки
#define STRING_SIZE 64  //размер строк

#endif /* MAIN_H_ */