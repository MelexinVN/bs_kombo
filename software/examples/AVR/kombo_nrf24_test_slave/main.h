/*
* main.h
* заголовочный файл тестового примера ведомого устройства с модулем NRF24L01.
* Проект "КомБО" (Открытые системы беспроводной коммуникации)
*
* Микроконтроллеры: ATmega8/48/88/168/328
*
* здесь необходимо сконфигурировать пин и порт светодиода, адрес устройства
*
* Автор: Мелехин В.Н. (MelexinVN)
*/

#ifndef MAIN_H_
#define MAIN_H_

#define F_CPU 8000000UL			//частота тактирования микроконтроллера

//Задаем используемый МК, строки с остальными МК должны быть закомментированными
#define ATMEGA8					//микроконтроллер Atmega8
//#define ATMEGA88				//микроконтроллер ATmega48/88/168/328 

//добавление библиотек
#include <avr/io.h>			
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//добавление пользовательских библиотек
#include "usart.h"				//библиотека последовательного порта
#include "spi.h"				//библиотека интерфейса SPI
#include "kombo_nrf24.h"		//библиотека радиомодуля nrf24l01

//Настройки устройства
#define OUR_ADDR	0x00		//адрес устройства (должен быть в списке адресов ведущего)

//Конфигурируем порты ввода-вывода
#ifdef ATMEGA8
#define LED_PIN		PORTD6		//пин светодиода
#endif

#ifdef ATMEGA88
#define LED_PIN		PD6			//пин светодиода
#endif

#define LED_PORT	PORTD		//порт светодиода
#define LED_DD		DDD6		//бит направления данных светодиода
#define LED_DDR		DDRD		//порт направления данных светодиода

//макросы управления светодиодом
#define LED_ON()	LED_PORT|=(1<<LED_PIN)	//включение светодиода
#define LED_OFF()	LED_PORT&=~(1<<LED_PIN)	//выключение светодиода
#define LED_TGL()	LED_PORT^=(1<<LED_PIN)	//смена состояния светодиода

//прочие настройки
#define STRING_SIZE		64		//размер строк

#endif /* MAIN_H_ */