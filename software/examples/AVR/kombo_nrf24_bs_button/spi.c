/*
* spi.c
* ���������� ���������� SPI
* ����������������: ATmega8/48/88/168/328
*										STM32 (���������� ������� ����� ����������)
*/

#include "spi.h"				//��������� ������������ ����

//��������� ������������� SPI
void spi_init(void)
{
	#ifdef ATMEGA8
	DDRB |= ((1<<PORTB2)|(1<<PORTB3)|(1<<PORTB5));		//����� SPI �� �����
	PORTB &= ~((1<<PORTB2)|(1<<PORTB3)|(1<<PORTB5));	//������ �������
	SPCR = (0<<SPIE) | (1<<SPE) | (0<<DORD) | (1<<MSTR) | (0<<CPOL) | (0<<CPHA) | (0<<SPR1) | (0<<SPR0);
	#endif
	
	#ifdef ATMEGA88
	DDRB |= ((1<<PB2)|(1<<PB3)|(1<<PB5));				//����� SPI �� �����
	PORTB &= ~((1<<PB2)|(1<<PB3)|(1<<PB5));				//������ �������
	SPCR = (0<<SPIE) | (1<<SPE) | (0<<DORD) | (1<<MSTR) | (0<<CPOL) | (0<<CPHA) | (0<<SPR1) | (0<<SPR0);
	#endif
	
	#ifdef STM32_LL
	LL_SPI_Enable(SPI1);										//�������� spi, �������� ������������� ������������ �������������
	#endif
}

//��������� �������� ����� 
void spi_send_byte(uint8_t byte)
{
	#ifdef ATMEGA8
	SPDR = byte;				//���������� ���� � �������
	while(!(SPSR & (1<<SPIF)));	//�������� ���� ������ �����������
	#endif
	
	#ifdef ATMEGA88
	SPDR = byte;				//���������� ���� � �������
	while(!(SPSR & (1<<SPIF)));	//�������� ���� ������ �����������
	#endif
	
	#ifdef STM32_LL
	while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {}
	LL_SPI_TransmitData8 (SPI1, byte);				//���������� ����� � �������� ������
	while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {}
	(void) SPI1->DR;													//������ ������� DR, �������� ������, ������� ����������� ���������� ������������ � �������
	#endif
}

//������� ������/�������� �����
uint8_t spi_change_byte(uint8_t byte)
{
	#ifdef ATMEGA8
	SPDR = byte;				//���������� ���� � �������
	while(!(SPSR & (1<<SPIF)));	//�������� ���� ������ ����������� (����������)
	return SPDR;				//���������� �������� ��������
	#endif
	
	#ifdef ATMEGA88
	SPDR = byte;				//���������� ���� � �������
	while(!(SPSR & (1<<SPIF)));	//�������� ���� ������ ����������� (����������)
	return SPDR;				//���������� �������� ��������
	#endif
	
	#ifdef STM32_LL
	uint8_t dt = 0;
	while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {}	//���� ���� �� ���������� ���� txe (���������� � ��������)
  LL_SPI_TransmitData8 (SPI1, byte);				//�������� ���� ������ �� spi
	while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {} //���� ���� �� ���������� ���� rxne (���� ������ �� �����)
	dt = LL_SPI_ReceiveData8(SPI1);						//��������� ������ �� spi
	return dt;
	#endif
}