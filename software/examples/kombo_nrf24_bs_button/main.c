/*
* main.c
* Пример ведущего устройства с модулем NRF24L01.
* Проект "КомБО" (Открытые системы беспроводной коммуникации)
* Модуль метеодатчиков
*
* Микроконтроллеры: ATmega8/48/88/168/328
*
* фьюзы ATmega8:				High: 0xD8, Low: 0xE4
* фьюзы ATmega48/88/168/328:	High: 0xDF, Low: 0xE2
*
* Подключение периферии по умолчанию:
* NRF24L01		ATmega8/48/88/168/328	Arduino nano(UNO)
* CE			PB0(12)					D8
* CSN			PD7(11)					D7
* SCK			PB5(17)					D13
* MOSI			PB3(15)					D12
* MISO			PB4(16)					D11
* IRQ			PD2(32)					D2
*
* КНОПКА		PD3(1)					D3
*
* LED			PD6(10)					D6
*
* USART			ATmega8/48/88/168/328	Arduino nano(UNO)
* TX			PD0(30)					D0
* RX			PD1(31)					D1
*
*   Краткое описание работы:
* При включении питания и сбросе в порт выдаются значения регистров радиомодуля
* В процессе работы микроконтроллер отсчитывает время, прошедшее с последнего сброса, 
* или включения питания и ожидает запроса от ведущего. 
* Получив запрос, он отправляет в ответ состояние кнопки, или время прошедшее до нажатия,
* а также выполняет команду, содержащуюся в запросе - сбросить состояние, зажечь или погасить светодиод.
*	
* Автор: Мелехин В.Н. (MelexinVN)
*/

#include "main.h"						//подключаем основной заголовочный файл

//Объявляем переменные и массивы
char str[STRING_SIZE] = {0};			//буфер для вывода в порт
volatile uint8_t f_read_temp = 0;		//флаг чтения температуры
volatile uint32_t ms_counter = 0;		//счетчик милисекунд
volatile uint8_t f_pushed = 0;			//флаг нажатия
volatile uint32_t time_ms = 0;			//сохраненное время мс
//Внешние переменные и массивы
extern volatile uint8_t f_rx, f_tx;		//флаги приема и передачи
extern uint8_t tx_buf[TX_PLOAD_WIDTH];	//буфер передачи
extern uint8_t rx_buf[TX_PLOAD_WIDTH];	//буфер приема

//Процедура инициализации портов ввода/вывода
void gpio_init(void)
{
	LED_DDR |= 1<<LED_DD;				//пин светодиода на выход
	LED_PORT |= 0<<LED_PIN;				//низкий уровень на выводе светодиода
}

//Процедура инициализации таймера 1
void timer_init(void)
{
	//Частота тактирования: 125,000 kHz
	//Режим: CTC top=OCR1A (сброс по совпадению)
	//Выходы отключены
	//Период : 1 мс
	//Включено прерывание по совпадению
	#ifdef ATMEGA8
	TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
	TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (1<<WGM12) | (0<<CS12) | (1<<CS11) | (1<<CS10);
	TCNT1H=0x00;
	TCNT1L=0x00;
	ICR1H=0x00;
	ICR1L=0x00;
	OCR1AH=0x00;
	OCR1AL=0x7C;
	OCR1BH=0x00;
	OCR1BL=0x00;
	#endif
	
	#ifdef ATMEGA88
	TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
	TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (1<<WGM12) | (0<<CS12) | (1<<CS11) | (1<<CS10);
	TCNT1H=0x00;
	TCNT1L=0x00;
	ICR1H=0x00;
	ICR1L=0x00;
	OCR1AH=0x00;
	OCR1AL=0x7C;
	OCR1BH=0x00;
	OCR1BL=0x00;
	#endif
}

//Процедура инициализации прерываний
void interrupt_init(void)
{
	//включаем внешнее прерывания INT0, INT1 по спаду
	//включаем прерывание по сравнению А таймера 1
	#ifdef ATMEGA8
	GICR|=(1<<INT1) | (1<<INT0);
	MCUCR=(0<<ISC11) | (0<<ISC10) | (1<<ISC01) | (0<<ISC00);
	GIFR=(0<<INTF1) | (1<<INTF0);
	
	TIMSK=(1<<OCIE2) | (0<<TOIE2) | (0<<TICIE1) | (1<<OCIE1A) | (0<<OCIE1B) | (0<<TOIE1) | (0<<TOIE0);
	#endif
	
	#ifdef ATMEGA88
 	EICRA=(0<<ISC11) | (0<<ISC10) | (1<<ISC01) | (0<<ISC00);
 	EIMSK=(1<<INT1) | (1<<INT0);
 	EIFR=(0<<INTF1) | (1<<INTF0);
 	PCICR=(0<<PCIE2) | (0<<PCIE1) | (0<<PCIE0);

	TIMSK1=(0<<ICIE1) | (0<<OCIE1B) | (1<<OCIE1A) | (0<<TOIE1);
	#endif
}

//Процедура моргания светодиодом
void blink_led(uint8_t blink_counter)
{
	while (blink_counter)		//пока счетчик не равен 0
	{
		LED_ON();				//включаем светодиод
		_delay_ms(10);			//ждем
		LED_OFF();				//выключаем светодиод
		_delay_ms(50);			//ждем
		blink_counter--;		//декрементируем счетчик
	}
}

//Процедура приема радиомодуля
void nrf24l01_receive(void)
{
	if(f_rx)	//если флаг приема поднят (флаг поднимается по внешнему прерыванию от радиомодуля)
	{
		if (rx_buf[0] == SOFT_RESET)	//если пришла команда программного сброса
		{
			f_pushed = 0;				//опускаем флаг нажатия
			ms_counter = 0;				//обнуляем значение времени
			usart_println("reset");
			blink_led(2);				//сигнализируем морганием светодиода
		}
					
		if (rx_buf[0] == OUR_ADDR)		//если первый принятый байт совпадает с адресом кнопки
		{
			if (f_pushed)				//если поднят флаг нажатия
			{
				tx_buf[0] = OUR_ADDR;	//записываем в первый байт адрес
				(*(unsigned long*)&tx_buf[1]) = time_ms;	//во второй, преобразованный в тип unsigned long, записываем значение времени
				nrf24_send(tx_buf);		//отправляем посылку в эфир
			}
			else						//если нажатия не бвло
			{
				tx_buf[0] = OUR_ADDR;	//записываем в первый байт адрес
				(*(unsigned long*)&tx_buf[1]) = NOT_PUSHED;//во второй, преобразованный в тип unsigned long, записываем значение ненажатого состояния
				nrf24_send(tx_buf);		//отправляем посылку в эфир
			}
			if (rx_buf[1] == 0x01)		//если поступила команда управления светодиодом
			{	//устанавливаем сосстояние светодиода в соответствии с пришедшей командой
				if(rx_buf[2] == LED_STATE_ON) LED_ON();		
				if(rx_buf[2] == LED_STATE_OFF) LED_OFF();
			}
			if (rx_buf[1] == SOFT_RESET)//если пришла команда программного сброса
			{
				f_pushed = 0;			//опускаем флаг нажатия
				ms_counter = 0;			//обнуляем значение времени
				blink_led(2);			//сигнализируем морганием светодиода
			}
		}
		f_rx = 0;						//опускаем флаг приема
	}
}

//Вектор прерывания от последовательного порта
#ifdef ATMEGA88
ISR(USART_RX_vect)
{
	//здесь можно прописать действия по получению данных по последовательному порту
}
#endif

#ifdef ATMEGA8
ISR(USART_RXC_vect)
{
	//здесь можно прописать действия по получению данных по последовательному порту
}
#endif

//Вектор прерывания INT0
ISR(INT0_vect)
{
	irq_callback();			//обрабатываем прерывание от радиомодуля
}
//Вектор прерывания INT1
ISR(INT1_vect)
{
	if(!f_pushed)				//если опущен флаг нажатия
	{
		f_pushed = 1;			//поднимаем флаг нажатия
		time_ms = ms_counter;	//сохраняем количество мс
	}
}

//Процедура обработки прерывания по сравнению А таймера 1
void t1_compa_callback(void)
{
	ms_counter++;			//считаем мс
}

//Вектор прерывания по сравнению А таймера 1
ISR(TIMER1_COMPA_vect)
{
	t1_compa_callback();	//обрабатываем прерывание
}

//Процедура вывода в порт данных из регистров радиомодуля
void nrf_info_print(void)
{
	uint8_t buf[TX_ADR_WIDTH] = {0};				//буфер для чтения адресов модуля
	uint8_t dt_reg = 0;								//переменная для чтения значения регистра
	
	dt_reg = nrf24_read_reg(CONFIG);				//читаем регистр CONFIG
	sprintf(str, "CONFIG: 0x%02X\r\n", dt_reg);		//
	usart_print(str);								//выводим данные в порт
	dt_reg = nrf24_read_reg(EN_AA);					//читаем регистр EN_AA
	sprintf(str, "EN_AA: 0x%02X\r\n", dt_reg);		//
	usart_print(str);								//выводим данные в порт
	dt_reg = nrf24_read_reg(EN_RXADDR);				//читаем регистр EN_RXADDR
	sprintf(str, "EN_RXADDR: 0x%02X\r\n", dt_reg);	//
	usart_print(str);								//выводим данные в порт
	dt_reg = nrf24_read_reg(STATUS);				//читаем регистр STATUS
	sprintf(str, "STATUS: 0x%02X\r\n", dt_reg);		//
	usart_print(str);								//выводим данные в порт
	dt_reg = nrf24_read_reg(RF_SETUP);				//читаем регистр RF_SETUP
	sprintf(str, "RF_SETUP: 0x%02X\r\n", dt_reg);	//
	usart_print(str);								//выводим данные в порт
	nrf24_read_buf(TX_ADDR,buf,3);					//читаем буфер TX_ADDR
	sprintf(str, "TX_ADDR: 0x%02X, 0x%02X, 0x%02X\r\n", buf[0], buf[1], buf[2]);
	usart_print(str);								//выводим данные в порт
	nrf24_read_buf(RX_ADDR_P1,buf,3);				//читаем буфер RX_ADDR_P1
	sprintf(str, "RX_ADDR: 0x%02X, 0x%02X, 0x%02X\r\n", buf[0], buf[1], buf[2]);
	usart_print(str);								//выводим данные в порт
}

//Основная программа
int main(void)
{
	spi_init();				//инициализируем SPI
	//_delay_ms(1000);		//задержка для корректного включения SPI 
	//(в некоторых случаях отсутствие задержки или недостаточная задержка по времени приводила к уходу в ребут при включении питания)
	interrupt_init();		//инициализируем прерывания
	gpio_init();			//инициализируем порты ввода-вывода
	timer_init();			//инициализируем таймер
	usart_init(103);		//инициализируем USART 9600 бод
	//usart_init(8);		//инициализируем USART 115200 бод
	usart_println("start");	//отправка стартовой строки в порт
	nrf24_init();			//инициализируем радиомодуль
	blink_led(5);			//моргаем светодиодом
	nrf_info_print();		//выводим значения регистров в порт

	wdt_enable(WDTO_2S);	//включаем сторожевой таймер

	sei();					//глобальное разрешение прерываний
	
	while (1)
	{
		nrf24l01_receive();	//обрабатываем прием радиомодуля
		//проверяем, не пришел ли запрос от ведущего

		wdt_reset();		//сбрасываем сторожевой таймер
	}
}

