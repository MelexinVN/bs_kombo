/*
* usart.c
* библиотека последовательного порта USART
* Микроконтроллеры: ATmega8/48/88/168/328
*										STM32 (необходиом указать номер порта)
*/

#include "usart.h"				//добавляем заголовочный файл

//Процедура инициализации USART
void usart_init(unsigned int ubrr)
{
	#ifdef ATMEGA8
	//настроим пины приема и передачи на вход
	DDRD |= (0<<DDD1) | (0<<DDD0);
	PORTD |= (0<<PORTD1) | (0<<PORTD0);
	//зададим скорость работы USART
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;
		
	UCSRB=(1<<RXEN)|( 1<<TXEN); //включаем прием и передачу по USART
	UCSRB |= (1<<RXCIE);		//разрешаем прерывание при передаче
	UCSRA |= (1<<U2X);			// для 8 мгц
	UCSRC = (1<<URSEL)|(1<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);// обращаемся именно к регистру UCSRC (URSEL=1),
	//ассинхронный режим (UMSEL=0), без контроля четности (UPM1=0 и UPM0=0),
	//1 стоп-бит (USBS=0), 8-бит посылка (UCSZ1=1 и UCSZ0=1)
	#endif
		
	#ifdef ATMEGA88
	//настроим пины приема и передачи на вход
	DDRD |= (0<<DDD1) | (0<<DDD0);
	PORTD |= (0<<PD1) | (0<<PD0);
	//Зададим скорость работы USART
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
				
	UCSR0B = (1<<RXEN0)|(1<<TXEN0); //включаем прием и передачу по USART
	UCSR0B |= (1<<RXCIE0);			//разрешаем прерывание при передаче
	UCSR0A |= (1<<U2X0);			// для 8 мгц
	UCSR0C = (0<<UMSEL01)|(0<<UMSEL00)|(1<<USBS0)|(1<<UCSZ01)|(1<<UCSZ00);// обращаемся именно к регистру UCSRC (URSEL=1),
	//ассинхронный режим (UMSEL=0), без контроля четности (UPM1=0 и UPM0=0),
	//1 стоп-бит (USBS=0), 8-бит посылка (UCSZ1=1 и UCSZ0=1)
	#endif
	
	#ifdef STM32_LL
	LL_USART_Enable(USART1);	//включаем порт, основная инициализация генерируется автоматически
	#endif
}

//Процедура отправки одного байта
void usart_transmit(unsigned char data) 
{
	#ifdef ATMEGA8
	while ( !(UCSRA & (1<<UDRE)) ); //ждем опустошения буфера приема
	UDR = data;						//записываем байт в регистр
	#endif
	
	#ifdef ATMEGA88
	while ( !(UCSR0A & (1<<UDRE0)) ); //ждем опустошения буфера приема
	UDR0 = data; //записываем байт в регистр
	#endif
	
	#ifdef STM32_LL
	while (!LL_USART_IsActiveFlag_TXE(USART1)) {}		//ждем отправки предыдущего байта
  LL_USART_TransmitData8(USART1,data);						//отправляем очередной байт в порт
	#endif
}

//Процедура отправки массива
void usart_print(char *str)
{	//цикл по всему массиву
	for (int i = 0; i < strlen(str); i++)
	{
		usart_transmit(str[i]); //отправляем очередной байт
	}
}

//Процедура отправки массива с переходом в начало новой строки
void usart_println(char *str)
{	//цикл по всему массиву
	for (int i = 0; i < strlen(str); i++)
	{
		usart_transmit(str[i]);  //отправляем очередной байт
	}
	usart_transmit(0x0d);		//переход в начало строки
	usart_transmit(0x0a);		//переход на новую строку
}