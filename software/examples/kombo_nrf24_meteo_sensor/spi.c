/*
* spi.c
* Библиотека интерфейса SPI
* Микроконтроллеры: ATmega8/48/88/168/328
*/

#include "spi.h"				//добавляем заголовочный файл

//Процедура инициализации SPI
void spi_init(void)
{
	#ifdef ATMEGA8
	DDRB |= ((1<<PORTB2)|(1<<PORTB3)|(1<<PORTB5));		//ножки SPI на выход
	PORTB &= ~((1<<PORTB2)|(1<<PORTB3)|(1<<PORTB5));	//низкий уровень
	#endif
	
	#ifdef ATMEGA88
	DDRB |= ((1<<PB2)|(1<<PB3)|(1<<PB5));				//ножки SPI на выход
	PORTB &= ~((1<<PB2)|(1<<PB3)|(1<<PB5));				//низкий уровень
	#endif
	SPCR = (0<<SPIE) | (1<<SPE) | (0<<DORD) | (1<<MSTR) | (0<<CPOL) | (0<<CPHA) | (0<<SPR1) | (0<<SPR0);
}

//Процедура отправки байта 
void spi_send_byte(uint8_t byte)
{
	SPDR = byte;				//записываем байт в регистр
	while(!(SPSR & (1<<SPIF)));	//подождем пока данные передадутся
}

//Функция приема/отправки байта
uint8_t spi_change_byte(uint8_t byte)
{
	SPDR = byte;				//записываем байт в регистр
	while(!(SPSR & (1<<SPIF)));	//подождем пока данные передадутся (обменяются)
	return SPDR;				//возвращаем принятое значение
}