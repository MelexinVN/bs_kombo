/*
* usart.c
* ���������� ����������������� ����� USART
* ����������������: ATmega8/48/88/168/328
*										STM32 (���������� ������� ����� �����)
*/

#include "usart.h"				//��������� ������������ ����

//��������� ������������� USART
void usart_init(unsigned int ubrr)
{
	#ifdef ATMEGA8
	//�������� ���� ������ � �������� �� ����
	DDRD |= (0<<DDD1) | (0<<DDD0);
	PORTD |= (0<<PORTD1) | (0<<PORTD0);
	//������� �������� ������ USART
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;
		
	UCSRB=(1<<RXEN)|( 1<<TXEN); //�������� ����� � �������� �� USART
	UCSRB |= (1<<RXCIE);		//��������� ���������� ��� ��������
	UCSRA |= (1<<U2X);			// ��� 8 ���
	UCSRC = (1<<URSEL)|(1<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);// ���������� ������ � �������� UCSRC (URSEL=1),
	//������������ ����� (UMSEL=0), ��� �������� �������� (UPM1=0 � UPM0=0),
	//1 ����-��� (USBS=0), 8-��� ������� (UCSZ1=1 � UCSZ0=1)
	#endif
		
	#ifdef ATMEGA88
	//�������� ���� ������ � �������� �� ����
	DDRD |= (0<<DDD1) | (0<<DDD0);
	PORTD |= (0<<PD1) | (0<<PD0);
	//������� �������� ������ USART
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
				
	UCSR0B = (1<<RXEN0)|(1<<TXEN0); //�������� ����� � �������� �� USART
	UCSR0B |= (1<<RXCIE0);			//��������� ���������� ��� ��������
	UCSR0A |= (1<<U2X0);			// ��� 8 ���
	UCSR0C = (0<<UMSEL01)|(0<<UMSEL00)|(1<<USBS0)|(1<<UCSZ01)|(1<<UCSZ00);// ���������� ������ � �������� UCSRC (URSEL=1),
	//������������ ����� (UMSEL=0), ��� �������� �������� (UPM1=0 � UPM0=0),
	//1 ����-��� (USBS=0), 8-��� ������� (UCSZ1=1 � UCSZ0=1)
	#endif
	
	#ifdef STM32_LL
	LL_USART_Enable(USART1);	//�������� ����, �������� ������������� ������������ �������������
	#endif
}

//��������� �������� ������ �����
void usart_transmit(unsigned char data) 
{
	#ifdef ATMEGA8
	while ( !(UCSRA & (1<<UDRE)) ); //���� ����������� ������ ������
	UDR = data;						//���������� ���� � �������
	#endif
	
	#ifdef ATMEGA88
	while ( !(UCSR0A & (1<<UDRE0)) ); //���� ����������� ������ ������
	UDR0 = data; //���������� ���� � �������
	#endif
	
	#ifdef STM32_LL
	while (!LL_USART_IsActiveFlag_TXE(USART1)) {}		//���� �������� ����������� �����
  LL_USART_TransmitData8(USART1,data);						//���������� ��������� ���� � ����
	#endif
}

//��������� �������� �������
void usart_print(char *str)
{	//���� �� ����� �������
	for (int i = 0; i < strlen(str); i++)
	{
		usart_transmit(str[i]); //���������� ��������� ����
	}
}

//��������� �������� ������� � ��������� � ������ ����� ������
void usart_println(char *str)
{	//���� �� ����� �������
	for (int i = 0; i < strlen(str); i++)
	{
		usart_transmit(str[i]);  //���������� ��������� ����
	}
	usart_transmit(0x0d);		//������� � ������ ������
	usart_transmit(0x0a);		//������� �� ����� ������
}