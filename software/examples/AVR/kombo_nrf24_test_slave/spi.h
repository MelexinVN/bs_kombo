/*
* spi.h
* Заголовочный файл библиотеки интерфейса SPI
* Микроконтроллеры: ATmega8/48/88/168/328
* 									STM32 (LL)
*/

#ifndef SPI_H_
#define SPI_H_

#include "main.h"		//добавляем основной заголовочный файл проекта

//Процедура инициализации SPI
void spi_init(void);
//Процедура отправки байта 
void spi_send_byte(uint8_t byte);
//Функция приема/отправки байта
uint8_t spi_change_byte(uint8_t byte);

#endif /* SPI_H_ */