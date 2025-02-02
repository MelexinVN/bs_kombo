﻿/*
* spi.h
* Библиотека для работы с одним датчиком ds18b20 
* Температура в градусах по Цельсию
* Микроконтроллеры: ATmega8/88
*/

#include "ds18b20.h"	//добавляем заголовочный файл

//Функция определения датчика на шине
char ds_test_device(void) //dt - digital termomether | определим, есть ли устройство на шине
{
	char sreg_temp = SREG;	//сохраним значение регистра статуса
	cli();					//запрещаем прерывания
	
	char dt;
	DS18B20_OUT();		//пин датчика на выход
	_delay_us(485);		//задержка минимум на 480 микросекунд
	DS18B20_IN();		//пин датчика на вход
	_delay_us(55);		//задержка максимум на 60 микросекунд
	//проверяем, ответит ли устройство
	if (DS_PIN_HIGH() == 0)
	{
		dt = 1;			//устройство есть
	} 
	else dt = 0;		//устройства нет

	SREG = sreg_temp;	//вернем значение регистра статуса в исходное состояние
	_delay_us(485);		//задержка как минимум на 480 микросекунд
	return dt;			//вернем результат
}

//Функция записи бита на устройство
void ds_send_bit(char bt)
{
	char sreg_temp = SREG;	//сохраним значение регистра статуса
	cli();					//запрещаем прерывания
		
	DS18B20_OUT();			//пин датчика на выход
	_delay_us(2);			//задержка минимум на 2 мкс
	
	if(bt) 	DS18B20_IN();	//пин датчика на вход
	
	_delay_us(65);			//задержка как минимум на 60 мкс
	
	DS18B20_IN();			//пин датчика на вход
	
	SREG = sreg_temp;	//вернем значение регистра статуса в исходное состояние
}

//Функция записи байта на устройство
void ds_send_byte(unsigned char bt)
{
	for(uint8_t i = 0; i < 8; i++)	//цикл на 8 бит
	{
		if((bt & (1<<i)) == 1<<i)	//если очередной бит - 1
			ds_send_bit(1);			//посылаем 1
		else						//иначе
			ds_send_bit(0);			//посылаем 0
	}	
}

//Функция чтения бита с устройства
char ds_read_bit(void)
{
	char sreg_temp = SREG;	//сохраним значение регистра статуса
	cli();					//запрещаем прерывания
	
	char bt;	
	DS18B20_OUT();			//пин датчика на выход
	_delay_us(2);			//задержка как минимум на 2 микросекунды
	DS18B20_IN();			//пин датчика на вход
	_delay_us(13);
	bt = (DS18B20_PIN & (1<<DS18B20_BIT))>>DS18B20_BIT; //читаем бит
	_delay_us(45);
	
	SREG = sreg_temp;		//вернем значение регистра статуса в исходное состояние
	return bt;				//вернем результат
}

//Функция чтения байта с устройства
unsigned char dt_read_byte(void)
{
	char c = 0;					
	for (uint8_t i = 0; i < 8; i++)	//цикл на 8 бит
		c |= ds_read_bit() << i;	//читаем очередной бит
	return c;						//возвращаем полученный байт
}

//Функция преобразования показаний датчика в температуру
int ds_check(void)
{
	unsigned char bt;			//переменная для считывания младшего байта
	unsigned int tt = 0;		//переменная считывания показаний датчика
	if (ds_test_device())		//если устройство нашлось
	{
		ds_send_byte(NOID);		//пропускаем идентификацию, тк у нас только одно устройство на шине
		ds_send_byte(T_CONVERT);//измеряем температуру
		_delay_ms(188);			//для разрешения 12 бит - 750   мс
								//				 11 бит - 375   мс
								//				 10 бит - 187,5 мс
								//				  9 бит - 93,75 мс
		ds_test_device();		//снова используем  те же манипуляции с шиной что и при проверке ее присутствия
		ds_send_byte(NOID);		//пропускаем идентификацию
		ds_send_byte(READ_DATA);//даем команду на чтение данных с устройства
		bt = dt_read_byte();	//читаем младший бит
		tt = dt_read_byte();	//читаем старший бит MS
		tt = (tt<<8)|bt;		//сдвигаем старший влево, младший пишем на его место, тем самым получаем общий результат
	}
	return tt;					//возвращаем результат
}

//Функция выделения целой части значения температуры
char convert_temp_ed (unsigned int tt)
{
	char t = tt>>4;				//сдвиг и отсечение части старшего байта
	return t;					//возвращаем значение
}

//Функция выделения дробной части температуры
char convert_temp_drob (unsigned int tt)
{
	float t;		
	t = (float)(tt & 0x000F) / 16.0f;	
	t *=10;
	return t;
}

//Функция выделения знака температуры
char temp_sign (unsigned int tt)
{
	char t = tt>>11;	//сдвиг на 10 разрядов и отсечение числового значения
	return t;			//возвращаем значение (если равно 0, температура положительная)
}