/*
* ds18b20.h
* Заголовочный файл библиотеки для работы датчиком DS18B20
* Микроконтроллеры: ATmega8/88
*/

#ifndef DS18B20_H_
#define DS18B20_H_

#include "main.h"			//добавляем основной заголовочный файл проекта

//Макросы выводов микроконтроллера
#ifdef ATMEGA8
#define DS18B20_BIT		PORTC0	//пин, к которому подключен датчик
#endif

#ifdef ATMEGA88
#define DS18B20_BIT		PC0		//пин, к которому подключен датчик
#endif

#define DS18B20_PORT	PORTC	//порт датчика
#define DS18B20_DD		DDRC	//порт направления данных
#define DS18B20_PIN		PINC	//порт состояния пина датчика

#define DS18B20_OUT()	DS18B20_DD|=1<<DS18B20_BIT		//пин датчика на выход
#define DS18B20_IN()	DS18B20_DD&=~(1<<DS18B20_BIT)	//пин датчика на вход
#define DS_PIN_HIGH()	(DS18B20_PIN&(1<<DS18B20_BIT))	//проверка состояния пина датчика

//Макросы для работы с датчиком
#define NOID		0xCC	//пропустить идентификацию
#define T_CONVERT	0x44	//код измерения температуры
#define READ_DATA	0xBE	//передача байтов ведущему
#define WRITE_DATA	0x4E	//прием байтов
#define DS_CONFIG	0x3F	//разрешение 12 бит - 0x7F
							//			 11 бит - 0x5F
							//			 10 бит - 0x3F
							//			  9 бит - 0x1F
//Функция преобразования показаний датчика в температуру
int ds_check(void);					
//Функция выделения целой части значения температуры
char convert_temp_ed (unsigned int tt); 
//Функция выделения дробной части температуры
char convert_temp_drob (unsigned int tt);
//Функция выделения знака температуры
char temp_sign (unsigned int tt);

#endif /* DS18B20_H_ */