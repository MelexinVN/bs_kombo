/*
* main.h
* заголовочный файл тестового примера ведущего устройства с модулем NRF24L01.
* Проект "КомБО" (Открытые системы беспроводной коммуникации)
*
* Платформа: Arduino
*
* Здесь необходимо сконфигурировать:
* пин и порт светодиода,
* число ведомых устройств, список адресов
*
* Автор: Мелехин В.Н. (MelexinVN)
*/

#ifndef MAIN_H_
#define MAIN_H_

#include <SPI.h>          //библиотека интерфейса SPI
#include "kombo_nrf24_arduino.h"  //библиотека радиомодуля nrf24l01

//Настройки
#define OUR_ADDR 0x0B  //адрес устройства (должен быть в списке адресов ведущего)

//Макросы управления светодиодом
#define LED_ON() digitalWrite(LED_BUILTIN, HIGH);  //включение светодиода
#define LED_OFF() digitalWrite(LED_BUILTIN, LOW);  //выключение светодиода

//Прочие настройки
#define STRING_SIZE 64  //размер строк


#endif /* MAIN_H_ */