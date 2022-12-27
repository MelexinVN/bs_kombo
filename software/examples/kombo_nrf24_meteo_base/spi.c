/*
* spi.c
* ���������� ���������� SPI
* ����������������: ATmega8/48/88/168/328
*/

#include "spi.h"				//��������� ������������ ����

//��������� ������������� SPI
void spi_init(void)
{
	#ifdef ATMEGA8
	DDRB |= ((1<<PORTB2)|(1<<PORTB3)|(1<<PORTB5));		//����� SPI �� �����
	PORTB &= ~((1<<PORTB2)|(1<<PORTB3)|(1<<PORTB5));	//������ �������
	#endif
	
	#ifdef ATMEGA88
	DDRB |= ((1<<PB2)|(1<<PB3)|(1<<PB5));				//����� SPI �� �����
	PORTB &= ~((1<<PB2)|(1<<PB3)|(1<<PB5));				//������ �������
	#endif
	SPCR = (0<<SPIE) | (1<<SPE) | (0<<DORD) | (1<<MSTR) | (0<<CPOL) | (0<<CPHA) | (0<<SPR1) | (0<<SPR0);
}

//��������� �������� ����� 
void spi_send_byte(uint8_t byte)
{
	SPDR = byte;				//���������� ���� � �������
	while(!(SPSR & (1<<SPIF)));	//�������� ���� ������ �����������
}

//������� ������/�������� �����
uint8_t spi_change_byte(uint8_t byte)
{
	SPDR = byte;				//���������� ���� � �������
	while(!(SPSR & (1<<SPIF)));	//�������� ���� ������ ����������� (����������)
	return SPDR;				//���������� �������� ��������
}