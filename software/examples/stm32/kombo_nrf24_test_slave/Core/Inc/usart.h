/*
* usart.h
* Заголовочный файл последовательного порта USART
* Микроконтроллеры: ATmega8/48/88/168/328
* 									STM32 (LL)
*/

#ifndef USART_H_
#define USART_H_

#include "main.h"	//добавляем основной заголовочный файл проекта

//Процедура инициализации USART
void usart_init(unsigned int ubrr);
//Процедура отправки одного байта
void usart_transmit(unsigned char data);
//Процедура отправки массива
void usart_print(char *str);
//Процедура отправки массива с переходом в начало новой строки
void usart_println(char *str);

#endif /* USART_H_ */