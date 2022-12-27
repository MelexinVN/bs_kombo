/*
* main.h
* заголовочный файл примера ведущего устройства с модулем NRF24L01.
* Проект "КомБО" (Открытые системы беспроводной коммуникации)
* База системы сбора метеоданных
*
* Микроконтроллеры: ATmega8/48/88/168/328
*
* Здесь необходимо сконфигурировать:
* наименование микроконтроллера пин и порт светодиода, 
* число ведомых устройств, список адресов
*
* Автор: Мелехин В.Н. (MelexinVN)
*/

#ifndef MAIN_H_
#define MAIN_H_

#define F_CPU 8000000UL		//частота тактирования микроконтроллера

//Задаем используемый МК, строки с остальными МК должны быть закомментированными
#define ATMEGA8			//микроконтроллер Atmega8
//#define ATMEGA88			//микроконтроллер ATmega48/88/168/a328 

//Конфигурируем порты ввода-вывода
#ifdef ATMEGA8
#define LED_PIN		PORTD6			//пин светодиода
#endif

#ifdef ATMEGA88
#define LED_PIN		PD6				//пин светодиода
#endif

#define LED_PORT	PORTD			//порт светодиода
#define LED_DD		DDD6			//бит направления данных светодиода
#define LED_DDR		DDRD			//порт направления данных светодиода

//добавление библиотек
#include <avr/io.h>			
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//Добавление пользовательских библиотек
#include "usart.h"					//библиотека последовательного порта
#include "spi.h"					//библиотека интерфейса SPI
#include "kombo_nrf24.h"			//библиотека радиомодуля nrf24l01

//Настройки
#define NUM_OF_SLAVES 	20			//количество ведомых устройств
#define ADRESS_LIST		{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13}

//Макросы управления светодиодом
#define LED_ON()	LED_PORT|=(1<<LED_PIN)	//включение светодиода
#define LED_OFF()	LED_PORT&=~(1<<LED_PIN)	//выключение светодиода
#define LED_TGL()	LED_PORT^=(1<<LED_PIN)	//смена состояния светодиода

//Макросы данных датчиков
#define DS_TEMP			0xDD		//температура от ds18b20
#define DHT11_DATA		0x1D		//температура от dht11
#define LAST_SENSOR		0xFF		//признак последнего датчика в посылке

//Прочие настройки
#define STRING_SIZE		64			//размер строк

#endif /* MAIN_H_ */