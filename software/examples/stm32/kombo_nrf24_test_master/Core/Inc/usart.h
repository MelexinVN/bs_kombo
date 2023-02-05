/*
* usart.h
* ������������ ���� ����������������� ����� USART
* ����������������: ATmega8/48/88/168/328
* 									STM32 (LL)
*/

#ifndef USART_H_
#define USART_H_

#include "main.h"	//��������� �������� ������������ ���� �������

//��������� ������������� USART
void usart_init(unsigned int ubrr);
//��������� �������� ������ �����
void usart_transmit(unsigned char data);
//��������� �������� �������
void usart_print(char *str);
//��������� �������� ������� � ��������� � ������ ����� ������
void usart_println(char *str);

#endif /* USART_H_ */