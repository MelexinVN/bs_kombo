/*
* spi.h
* ������������ ���� ���������� ���������� SPI
* ����������������: ATmega8/48/88/168/328
* 									STM32 (LL)
*/

#ifndef SPI_H_
#define SPI_H_

#include "main.h"		//��������� �������� ������������ ���� �������

//��������� ������������� SPI
void spi_init(void);
//��������� �������� ����� 
void spi_send_byte(uint8_t byte);
//������� ������/�������� �����
uint8_t spi_change_byte(uint8_t byte);

#endif /* SPI_H_ */