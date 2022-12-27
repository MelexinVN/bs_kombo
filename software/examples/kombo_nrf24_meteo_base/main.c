/*
* main.c
* Пример ведущего устройства с модулем NRF24L01.
* Проект "КомБО" (Открытые системы беспроводной коммуникации)
* База системы сбора метеоданных
*
* Микроконтроллеры: ATmega8/48/88/168/328
*
* фьюзы ATmega8:				High: 0xD8, Low: 0xE4
* фьюзы ATmega48/88/168/328:	High: 0xDF, Low: 0xE2
*
* Подключение периферии:
* NRF24L01		ATmega8/48/88/168/328	Arduino nano(UNO)
* CE			PB0(12)					D8
* CSN			PD7(11)					D7
* SCK			PB5(17)					D13
* MOSI			PB3(15)					D12
* MISO			PB4(16)					D11
* IRQ			PD2(32)					D2
*
* LED			ATmega8/48/88/168/328	Arduino nano(UNO)
*				PD6(10)					D6
*
* USART			ATmega8/48/88/168/328	Arduino nano(UNO)
* TX			PD0(30)					D0
* RX			PD1(31)					D1
*
*   Краткое описание работы:
* При включении питания и сбросе в порт выдаются значения регистров радиомодуля
* В процессе работы устройство по кругу отправляет запросы на адреса, содержаниеся в массиве
* при получении ответа на запрос, в порт выдается адрес, с которого получен ответ,
* тип датчика и его показания.
*
* Автор: Мелехин В.Н. (MelexinVN)
*/

#include "main.h"						//подключаем основной заголовочный файл

//объявляем переменные и массивы
char str[STRING_SIZE] = {0};			//буфер для вывода в порт
uint8_t slave_counter = 0;				//счетчик ведомых
uint8_t slave_addrs[] = ADRESS_LIST;	//массив адресов ведомых
//внешние переменные и массивы
extern volatile uint8_t f_rx, f_tx;		//флаги приема и передачи
extern uint8_t tx_buf[TX_PLOAD_WIDTH];	//буфер передачи
extern uint8_t rx_buf[TX_PLOAD_WIDTH];	//буфер приема

//Процедура инициализации портов
void gpio_init(void)
{
	LED_DDR |= 1<<LED_DD;				//пин светодиода на выход
	LED_PORT |= 0<<LED_PIN;				//низкий уровень на выводе светодиода
}

//Процедура инициализации прерываний
void interrupt_init(void)
{
	//включаем внешнее прерывание INT0 по спаду
	#ifdef ATMEGA8
	GICR|=(0<<INT1) | (1<<INT0);
	MCUCR=(0<<ISC11) | (0<<ISC10) | (1<<ISC01) | (0<<ISC00);
	GIFR=(0<<INTF1) | (1<<INTF0);
	#endif
	
	#ifdef ATMEGA88
	EICRA=(0<<ISC11) | (0<<ISC10) | (1<<ISC01) | (0<<ISC00);
	EIMSK=(0<<INT1) | (1<<INT0);
	EIFR=(0<<INTF1) | (1<<INTF0);
	PCICR=(0<<PCIE2) | (0<<PCIE1) | (0<<PCIE0);
	#endif
}

//Процедура приема радиомодуля
void nrf24l01_receive(void)
{
	if(f_rx)	//если флаг приема поднят (флаг поднимается по внешнему прерыванию от радиомодуля)
	{
		//цикл по всем ведомым устройствам
		for (uint8_t i = 0; i < NUM_OF_SLAVES; i++)
		{	//если найден принятый адрес
			if (rx_buf[0] == slave_addrs[i])	//если адрес совпадает с одним из списка
			{
				if (rx_buf[1] == DS_TEMP)		//если пришла температура от ds18b20
				{
					if (rx_buf[2] == 0)			//если температура положительная
					{	//выводим в порт строку
						sprintf(str, "0x%02X\tds18b20\tt = %d.%d°C\r\n", rx_buf[0], rx_buf[3], rx_buf[4]);				
						usart_print(str);
					}	
					else if (rx_buf[2] != 0)	//если температура отрицательная
					{	//выводим в порт строку с минусом и преобразованием значений
						sprintf(str, "0x%02X\tds18b20\tt = -%d.%d°C\r\n", rx_buf[0], (255 - rx_buf[3]),rx_buf[4]);			
						usart_print(str);
					}
				}
				else if (rx_buf[1] == DHT11_DATA)	//если пришла температура от dht11
				{	//выводим строку с данными температуры и влажности в порт
					sprintf(str, "0x%02X\tdht11\tt = %d°C, H = %d%%\r\n", rx_buf[0], rx_buf[2], rx_buf[3]);				
					usart_print(str);
				}
			}
		}
		f_rx = 0;					//опускаем флаг приема
	}
}

//Процедура моргания светодиодом
void blink_led(uint8_t blink_counter)
{
	while (blink_counter)			//пока счетчик не равен 0
	{
		LED_ON();					//включаем светодиод
		_delay_ms(50);				//ждем
		LED_OFF();					//выключаем светодиод
		_delay_ms(50);				//ждем
		blink_counter--;			//декрементируем счетчик
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
	irq_callback();					//обрабатываем прерывание от радиомодуля
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

int main(void)
{
	interrupt_init();		//инициализируем прерывания
	gpio_init();			//инициализируем порты ввода-вывода
	spi_init();				//инициализируем SPI
	nrf24_init();			//инициализируем радиомодуль
	usart_init(103);		//инициализируем USART 9600 бод
	//usart_init(8);		//инициализируем USART 115200 бод
	usart_println("start");	//отправка стартовой строки в порт
	nrf_info_print();		//выводим значения регистров в порт
	blink_led(5);			//моргаем светодиодом
	
	wdt_enable(WDTO_500MS);	//включаем сторожевой таймер
	
	sei();					//глобальное разрешение прерываний
	
	while (1)
	{
		//отправляем запрос по очередному адресу
		tx_buf[0] = slave_addrs[slave_counter];	//записываем в буфер очередной адрес
		
		nrf24_send(tx_buf);						//отправляем посылку в эфир
		//запрос содержит только адрес устройства, но может содержать другую информацию размером до 32 байт
		
		slave_counter++;						//переходим к следующему адресу
		if (slave_counter == NUM_OF_SLAVES)		//если дошли до последнего элемента
		{
			slave_counter = 0;					//обнуляем счетчик
		}
		
		_delay_ms(3);							//задержка
		
		//задержка необходима, чтобы ведомое устройство успело получить запрос, отправить ответ,
		//а данное ведущее устройство успело получить и обработать отет на запрос, прежде чем
		//переходить к отправке следующего
		
		//для достижения максимального быстродействия величину задержки необходимо подбирать наименьшей
		//экспериментально с учетом времени, затрачиваемого на остальные действия микроконтроллера
		
		nrf24l01_receive();		//обрабатываем прием радиомодуля
		
		wdt_reset();			//сбрасываем сторожевой таймер
	}
}

