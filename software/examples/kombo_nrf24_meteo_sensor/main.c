/*
* main.c
* Пример ведущего устройства с модулем NRF24L01.
* Проект "КомБО" (Открытые системы беспроводной коммуникации)
* Модуль метеодатчиков
*
* Микроконтроллеры: ATmega8/48/88/168/328
* Датчики: ds18b20, dht11
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
* DS18B20/DHT11 PC0(23)					A0
*
* LED			PD6(10)					D6
*
* USART			ATmega8/48/88/168/328	Arduino nano(UNO)
* TX			PD0(30)					D0
* RX			PD1(31)					D1
*
*   Краткое описание работы:
* При включении питания и сбросе в порт выдаются значения регистров радиомодуля
* В процессе работы микроконтроллер опрашивает метеодатчики и ожидает запроса от ведущего. 
* Получив запрос оно отправляет в ответ показания датчиков температуры и влажности, 
* а также отправляет данные в порт.
*
* Автор: Мелехин В.Н. (MelexinVN)
*/

#include "main.h"						//подключаем основной заголовочный файл

//объявляем переменные и массивы
char str[STRING_SIZE] = {0};			//буфер для вывода в порт

volatile uint8_t f_read_temp = 0;		//флаг чтения температуры
volatile uint16_t ms_counter = 0;		//счетчик милисекунд

#ifdef DS18B20
uint8_t sign = 0;						//переменная знака температуры
uint8_t temp_ed = 0;					//переменная целой части температуры
uint8_t temp_drob = 0;					//переменная дробной части температуры
int ds_temp = 0;						//переменная показаний датчика ds18b20
#endif

#ifdef DHT11
unsigned int dht11_hum = 0;				//переменная влажности с dht11
unsigned int dht11_temp = 0;			//переменная температуры с dht11
#endif

//внешние переменные и массивы
extern volatile uint8_t f_rx, f_tx;		//флаги приема и передачи
extern uint8_t tx_buf[TX_PLOAD_WIDTH];	//буфер передачи
extern uint8_t rx_buf[TX_PLOAD_WIDTH];	//буфер приема

//Процедура инициализации портов ввода/вывода
void gpio_init(void)
{
	LED_DDR |= 1<<LED_DD;				//пин светодиода на выход
	LED_PORT |= 0<<LED_PIN;				//низкий уровень на выводе светодиода
}

// Процедура инициализации таймера 1
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
	//включаем внешнее прерывание INT0 по спаду
	//включаем прерывание по сравнению А таймера 1
	#ifdef ATMEGA8
	GICR|=(0<<INT1) | (1<<INT0);
	MCUCR=(0<<ISC11) | (0<<ISC10) | (1<<ISC01) | (0<<ISC00);
	GIFR=(0<<INTF1) | (1<<INTF0);
	
	TIMSK=(1<<OCIE2) | (0<<TOIE2) | (0<<TICIE1) | (1<<OCIE1A) | (0<<OCIE1B) | (0<<TOIE1) | (0<<TOIE0);
	#endif
	
	#ifdef ATMEGA88
 	EICRA=(0<<ISC11) | (0<<ISC10) | (1<<ISC01) | (0<<ISC00);
 	EIMSK=(0<<INT1) | (1<<INT0);
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
		if (rx_buf[0] == OUR_ADDR)	//если первый принятый байт совпадает с адресом устройства
		{
			tx_buf[0] = OUR_ADDR;	//записываем в первый байт буфера адрес
			#ifdef DS18B20
			tx_buf[1] = DS_TEMP;	//записываем команду отправки температуры ds18b20
			tx_buf[2] = sign;		//записываем знак в буфер
			tx_buf[3] = temp_ed;	//записываем целую часть температуры в буфер
			tx_buf[4] = temp_drob;	//записываем дробную часть температуры в буфер
			#endif	
			
			#ifdef DHT11
			tx_buf[1] = DHT11_DATA;	//записываем команду отправки данных dht11
			tx_buf[2] = dht11_temp;	//записываем температуру
			tx_buf[3] = dht11_hum;	//записываем влажность
			#endif
			
			nrf24_send(tx_buf);		//отправляем посылку в эфир
			blink_led(1);			//моргаем светодиодом
		}
		f_rx = 0;					//опускаем флаг приема
		wdt_reset();		//сбрасываем сторожевой таймер
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

//Процедура обработки прерывания по сравнению А таймера 1
void t1_compa_callback(void)
{
	ms_counter++;			//считаем мс
	if(ms_counter >= 1000)	//если прошло 2 секунды
	{
		f_read_temp = 1;	//поднимаем флаг чтения температуры
		ms_counter = 0;		//обнуляем счетчик
	}
	wdt_reset();		//сбрасываем сторожевой таймер
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

		if (f_read_temp)		//если поднят флаг чтения температуры (1 раз в секунду)
		{
			#ifdef DS18B20
			int temperature = ds_check();				//считываем показания датчика
			temp_ed = convert_temp_ed(temperature);		//получаем целую часть
			temp_drob = convert_temp_drob(temperature);	//получаем дробную часть
			sign = temp_sign(temperature);				//получаем знак
			#endif
			
			#ifdef DHT11
			get_data_dht11();
			#endif
			
			f_read_temp = 0;	//опускаем флаг

			//вывод показаний датчиков в порт, если нужно проверить датчики
			#ifdef DS18B20
			if (sign == 0)			//если температура положительная
			{	//выводим в порт строку
				sprintf(str, "t = %d.%d°C\r\n", temp_ed, temp_drob);
				usart_print(str);
			}
			else if (sign != 0)		//если температура отрицательная
			{	//выводим в порт строку с минусом и преобразованием значений
				sprintf(str, "t = -%d.%d°C\r\n", (255 - temp_ed), temp_drob);
				usart_print(str);
			}	
			#endif	
			
			#ifdef DHT11
			//выводим строку с данными температуры и влажности в порт
			sprintf(str, "dht11\tt = %d°C, H = %d%%\r\n", dht11_temp, dht11_hum);
			usart_print(str);
			#endif
		}

		wdt_reset();		//сбрасываем сторожевой таймер
	}
}

