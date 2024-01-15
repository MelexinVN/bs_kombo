/*
* spi.c
* Библиотека интерфейса SPI
* Микроконтроллеры: ATmega8/88,STM32 (необходимо указать номер spi)
*/

#include "spi.h"				//добавляем заголовочный файл

//Процедура инициализации SPI
void spi_init(void)
{
	#ifdef ATMEGA8
	DDRB |= ((1<<PORTB2)|(1<<PORTB3)|(1<<PORTB5));		//ножки SPI на выход
	PORTB &= ~((1<<PORTB2)|(1<<PORTB3)|(1<<PORTB5));	//низкий уровень
	SPCR = (0<<SPIE) | (1<<SPE) | (0<<DORD) | (1<<MSTR) | (0<<CPOL) | (0<<CPHA) | (0<<SPR1) | (0<<SPR0);
	#endif
	
	#ifdef ATMEGA88
	DDRB |= ((1<<PB2)|(1<<PB3)|(1<<PB5));				//ножки SPI на выход
	PORTB &= ~((1<<PB2)|(1<<PB3)|(1<<PB5));				//низкий уровень
	SPCR = (0<<SPIE) | (1<<SPE) | (0<<DORD) | (1<<MSTR) | (0<<CPOL) | (0<<CPHA) | (0<<SPR1) | (0<<SPR0);
	#endif
	
	#ifdef STM32_LL
	LL_SPI_Enable(SPI1);										//включаем spi, основная инициализация генерируется автоматически
	#endif
}

//Процедура отправки байта 
void spi_send_byte(uint8_t byte)
{
	#ifdef ATMEGA8
	SPDR = byte;				//записываем байт в регистр
	while(!(SPSR & (1<<SPIF)));	//подождем пока данные передадутся
	#endif
	
	#ifdef ATMEGA88
	SPDR = byte;				//записываем байт в регистр
	while(!(SPSR & (1<<SPIF)));	//подождем пока данные передадутся
	#endif
	
	#ifdef STM32_LL
	while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {}
	LL_SPI_TransmitData8 (SPI1, byte);				//записываем адрес с командой записи
	while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {}
	(void) SPI1->DR;													//читаем регистр DR, имитация приема, который обязательно происходит одновременно с записью
	#endif
}

//Функция приема/отправки байта
uint8_t spi_change_byte(uint8_t byte)
{
	#ifdef ATMEGA8
	SPDR = byte;				//записываем байт в регистр
	while(!(SPSR & (1<<SPIF)));	//подождем пока данные передадутся (обменяются)
	return SPDR;				//возвращаем принятое значение
	#endif
	
	#ifdef ATMEGA88
	SPDR = byte;				//записываем байт в регистр
	while(!(SPSR & (1<<SPIF)));	//подождем пока данные передадутся (обменяются)
	return SPDR;				//возвращаем принятое значение
	#endif
	
	#ifdef STM32_LL
	uint8_t dt = 0;
	while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {}	//ждем пока не поднимется флаг txe (готовность к передаче)
  LL_SPI_TransmitData8 (SPI1, byte);				//передаем байт адреса по spi
	while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {} //ждем пока не поднимется флаг rxne (есть данные на прием)
	dt = LL_SPI_ReceiveData8(SPI1);						//принимаем данные по spi
	return dt;
	#endif
}