/*
* dht11.c
* Заголовочный файл библиотеки работы с датчиком температуры и влажности DHT11
* Микроконтроллеры: ATmega8/48/88/168/328
*/

#ifndef DHT11_H_
#define DHT11_H_

#include "main.h"			//добавляем основной заголовочный файл проекта

//Макросы выводов микроконтроллера
#ifdef ATMEGA8
#define DHT11_BIT	PORTC1	//пин, к которому подключен датчик
#endif

#ifdef ATMEGA88
#define DHT11_BIT	PC1		//пин, к которому подключен датчик
#endif

#define DHT11_PORT	PORTC	//порт датчика
#define DHT11_DD	DDRC	//порт направления данных
#define DHT11_PIN	PINC	//порт состояния пина датчика

#define BITDHT_ON	DHT11_PORT|=(1<<DHT11_BIT)	//высокий уровень на пин датчика
#define BITDHT_OFF	DHT11_PORT&=~(1<<DHT11_BIT)	//низкий уровень на пин датчика

#define BITDHT_OUT	DHT11_DD|=(1<<DHT11_BIT)	//пин датчика на выход
#define BITDHT_IN	DHT11_DD&=~(1<<DHT11_BIT)	//пин датчика на вход

//Процедура чтения показаний датчика
void get_data_dht11(void);

#endif /* DHT11_H_ */