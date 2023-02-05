/*
* kombo_nrf24.c
* Библиотека работы с радиомодулем NRF24L01 
* Проект "КомБО" (Открытые системы беспроводной коммуникации)
* 
* Микроконтроллеры: ATmega8/48/88/168/328
* 									STM32 (LL)
* 
* Автор: Мелехин В.Н. MelexinVN
*/

#include "kombo_nrf24.h"				//добавляем заголовочный файл

#ifdef MASTER	//если ведущее устройство
uint8_t tx_addr_0[TX_ADR_WIDTH] = {0xa1,0xa2,0xa3};	//адрес 0
uint8_t tx_addr_1[TX_ADR_WIDTH] = {0xa3,0xa2,0xa1};	//адрес 1
#endif

#ifdef SLAVE	//если ведомое устройство
uint8_t tx_addr_0[TX_ADR_WIDTH] = {0xa3,0xa2,0xa1};	//адрес 0
uint8_t tx_addr_1[TX_ADR_WIDTH] = {0xa1,0xa2,0xa3};	//адрес 1
#endif

uint8_t rx_buf[TX_PLOAD_WIDTH] = {0};	//буфер приема
uint8_t tx_buf[TX_PLOAD_WIDTH] = {0};	//буфер передачи

volatile uint8_t f_rx = 0, f_tx = 0;	//флаги приема и передачи

#ifdef STM32_LL
//самодельная функция микросекундной задержки
__STATIC_INLINE void delay_us(__IO uint32_t micros)
{
  micros *= (SystemCoreClock / 1000000) / 9;	//делитель (9) получен экспериментально
  /* Wait till done */
  while (micros--) ;
}
#endif

//Функция чтения регистра модуля
uint8_t nrf24_read_reg(uint8_t addr)
{
	uint8_t dt = 0, cmd;				//переменные данных и команды
	CSN_ON();										//прижимаем ногу CS к земле
	dt = spi_change_byte(addr);	//отправка адреса регистра, прием
	
	//если адрес равен адресу регистра статуса то и возварщаем его состояние	
	if (addr != STATUS)					//а если не равен
	{
		cmd = 0xFF;								//команда NOP для получения данных
		dt = spi_change_byte(cmd);//
	}
	CSN_OFF();									//поднимаем ногу CS
	return dt;									//возвращаемое значение
}

//Процедура записи регистра в модуль
void nrf24_write_reg(uint8_t addr, uint8_t dt)		
{
	addr |= W_REGISTER;					//включаем бит записи в адрес	
	CSN_ON();										//прижимаем ногу CS к земле
	spi_send_byte(addr);				//отправляем адрес
	spi_send_byte(dt);					//отправляем значение
	CSN_OFF();									//поднимаем ногу CS
}

//Процедура активации дополнительных команд
void nrf24_toggle_features(void)							
{	
	uint8_t dt = ACTIVATE;				//переменная с командой активации
	CSN_ON();											//прижимаем ногу CS к земле
	spi_send_byte(dt);						//отправляем команду
	
	#ifdef ATMEGA8
	_delay_us(1);						//задержка
	#endif
	
	#ifdef ATMEGA88
	_delay_us(1);						//задержка
	#endif
	
	#ifdef STM32_LL
	delay_us(1);
	#endif
	
	dt = 0x73;							//следующая команда
	spi_send_byte(dt);			//отправляем команду
	CSN_OFF();							//поднимаем ногу CS
}

//Процедура чтения буфера
void nrf24_read_buf(uint8_t addr,uint8_t *p_buf,uint8_t bytes)
{
	CSN_ON();										//прижимаем ногу CS к земле
	spi_send_byte(addr);				//отправляем адрес
	//цикл на нужное количество байт
	for (uint8_t i = 0; i < bytes; i++) 
	{
		p_buf[i] = spi_change_byte(addr);//получаем очередной байт
	}
	CSN_OFF();									//поднимаем ногу CS
}

//Процедура записи буфера
void nrf24_write_buf(uint8_t addr,uint8_t *p_buf,uint8_t bytes)	
{
	addr |= W_REGISTER;					//включаем бит записи в адрес
	CSN_ON();										//прижимаем ногу CS к земле
	spi_send_byte(addr);				//отправляем адрес
	
	#ifdef ATMEGA8
	_delay_us(1);						//задержка
	#endif
	
	#ifdef ATMEGA88
	_delay_us(1);						//задержка
	#endif
	
	#ifdef STM32_LL
	delay_us(1);
	#endif
	
	//цикл на нужное количество байт
	for (uint8_t i = 0; i < bytes; i++) 
	{
		spi_send_byte(p_buf[i]);		//отправляем очередной байт
	}
	CSN_OFF();										//поднимаем ногу CS
}

//Процедура очистки буфера приема
void nrf24_flush_rx(void)
{
	uint8_t dt = FLUSH_RX;				//переменная с командой очистки
	CSN_ON();											//прижимаем ногу CS к земле
	spi_send_byte(dt);						//отправка команды
	
	#ifdef ATMEGA8
	_delay_us(1);						//задержка 
	#endif
	
	#ifdef ATMEGA88
	_delay_us(1);						//задержка
	#endif
	
	#ifdef STM32_LL
	delay_us(1);
	#endif
	
	CSN_OFF();										//поднимаем ногу CS
}

//Процедура очистки буфера передачи
void nrf24_flush_tx(void)
{
	uint8_t dt = FLUSH_TX;				//переменная с командой очистки
	CSN_ON();											//прижимаем ногу CS к земле
	spi_send_byte(dt);						//отправка команды
	
	#ifdef ATMEGA8
	_delay_us(1);						//задержка 
	#endif
	
	#ifdef ATMEGA88
	_delay_us(1);						//задержка
	#endif
	
	#ifdef STM32_LL
	delay_us(1);
	#endif
	
	CSN_OFF();										//поднимаем ногу CS
}

//Процедура включение режима приемника
void nrf24_rx_mode(void)
{
	uint8_t regval = 0x00;						//переменная для значения регистра
	regval = nrf24_read_reg(CONFIG);	//сохраняем значение регистра конфигурации
	//разбудим модуль и переведём его в режим приёмника, включив биты PWR_UP и PRIM_RX
	regval |= (1<<PWR_UP)|(1<<PRIM_RX);	
	nrf24_write_reg(CONFIG,regval);		//возвращаем значение регистра статуса
	//записываем  адрес передатчика
	nrf24_write_buf(TX_ADDR, tx_addr_1, TX_ADR_WIDTH);	
	//записываем адрес приемника
	nrf24_write_buf(RX_ADDR_P0, tx_addr_1, TX_ADR_WIDTH);	
	CE_SET();							//поднимаем ногу CE
	
	#ifdef ATMEGA8
	_delay_us(150);						//задержка минимум 130 мкс
	#endif
	
	#ifdef ATMEGA88
	_delay_us(150);						//задержка
	#endif
	
	#ifdef STM32_LL
	delay_us(150);
	#endif
	
	//очистка буферов
	nrf24_flush_rx();
	nrf24_flush_tx();
}

//Процедура включения режима передатчика
void nrf24_tx_mode(void)
{
	//записываем адрес передатчика
	nrf24_write_buf(TX_ADDR, tx_addr_0, TX_ADR_WIDTH);		
	//записываем адрес приемника
	nrf24_write_buf(RX_ADDR_P0, tx_addr_0, TX_ADR_WIDTH);	
	CE_RESET();							//опускаем ногу CE
	//очищаем оба буфера
	nrf24_flush_rx();
	nrf24_flush_tx();
}

//Процедура передачи данных в модуль
void nrf24_transmit(uint8_t addr,uint8_t *p_buf,uint8_t bytes)
{
	CE_RESET();						//опускаем ногу CE
	CSN_ON();							//прижимаем ногу CS к земле
	spi_send_byte(addr);	//отправляем адрес
	
	#ifdef ATMEGA8
	_delay_us(1);					//задержка
	#endif
	
	#ifdef ATMEGA88
	_delay_us(1);					//задержка
	#endif
	
	#ifdef STM32_LL
	delay_us(1);
	#endif
	
	//цикл на нужное количество байт
	for (uint8_t i = 0; i < bytes; i++) 
	{
		spi_send_byte(p_buf[i]);	//отправляем очередной байт
	}
	CSN_OFF();						//поднимаем ногу CS
	CE_SET();							//Поднимаем ногу CE
}

//Процедура отправки данных в эфир
void nrf24_send(uint8_t *p_buf)
{
	#ifdef ATMEGA8
	char sreg_temp = SREG;				//сохраним значение регистра статуса
	cli();												//запрещаем прерывания
	#endif
	
	#ifdef ATMEGA88
	char sreg_temp = SREG;				//сохраним значение регистра статуса
	cli();												//запрещаем прерывания
	#endif
	
	#ifdef STM32_LL
	__disable_irq();									//запрещение всех прерываний
	#endif
	
	uint8_t regval = 0x00;						//переменная для отправки в конфигурационный регистр
	nrf24_tx_mode();									//включаем режим передачи
	regval = nrf24_read_reg(CONFIG);	//сохраняем значения конфигурационного региста
	//если модуль ушел в спящий режим, то разбудим его, включив бит PWR_UP и выключив PRIM_RX
	regval |= (1<<PWR_UP);					
	regval &= ~(1<<PRIM_RX);
	nrf24_write_reg(CONFIG, regval);	//записываем новое значение конфигурационного регистра
	
	#ifdef ATMEGA8
	_delay_us(150);						//задержка минимум 130 мкс
	#endif
	
	#ifdef ATMEGA88
	_delay_us(150);						//задержка минимум 130 мкс
	#endif
	
	#ifdef STM32_LL
	delay_us(150);
	#endif
	
	nrf24_transmit(WR_TX_PLOAD, p_buf, TX_PLOAD_WIDTH);//отправка данных
	CE_SET();									//поднимаем ногу CE
	
	#ifdef ATMEGA8
	_delay_us(15);						//задержка 10us
	#endif
	
	#ifdef ATMEGA88
	_delay_us(15);						//задержка 10us
	#endif
	
	#ifdef STM32_LL
	delay_us(15);
	#endif
	
	CE_RESET();								//опускаем ногу CE
	
	#ifdef ATMEGA8
	SREG = sreg_temp;					//вернем значение регистра статуса в исходное состояние
	#endif
	
	#ifdef ATMEGA88
	SREG = sreg_temp;					//вернем значение регистра статуса в исходное состояние
	#endif
	
	#ifdef STM32_LL
	__enable_irq();						//разрешение всех прерываний
	#endif
}

//Процедура инициализации пинов, подключенных к радиомодулю
void nrf24_pins_init(void)
{
	#ifdef ATMEGA8
	CE_DDR |= 1<<CE_DD;					//CE на выход
	CE_PORT |= 1<<CE_PIN;				//высокий уровень на CE
	
	CSN_DDR |= 1<<CSN_DD;				//CSN на выход
	CSN_PORT |= 1<<CSN_PIN;				//высокий уровень на CSN
	
	IRQ_DDR |= 0<<IRQ_DD;				//IRQ на вход
	IRQ_PORT |= 0<<IRQ_PIN;				//отключаем внутренний пуллап
	#endif
	
	#ifdef ATMEGA88
	CE_DDR |= 1<<CE_DD;					//CE на выход
	CE_PORT |= 1<<CE_PIN;				//высокий уровень на CE
	
	CSN_DDR |= 1<<CSN_DD;				//CSN на выход
	CSN_PORT |= 1<<CSN_PIN;				//высокий уровень на CSN
	
	IRQ_DDR |= 0<<IRQ_DD;				//IRQ на вход
	IRQ_PORT |= 0<<IRQ_PIN;				//отключаем внутренний пуллап
	#endif
}

//Процедура инициализации модуля
void nrf24_init(void)
{
	nrf24_pins_init();				//инициализируем пины
	CE_RESET();								//опускаем к земле вывод CE
	
	#ifdef ATMEGA8
	_delay_us(5000);					//ждем 5 мс
	#endif
	
	#ifdef ATMEGA88
	_delay_us(5000);					//ждем 5 мс
	#endif
	
	#ifdef STM32_LL
	LL_mDelay(5);
	#endif
	
	//записываем конфигурационный байт, 
	//устанавливаем бит PWR_UP bit, включаем CRC(1 байт) &Prim_RX:0
	nrf24_write_reg(CONFIG, 0x0a);		
	
	#ifdef ATMEGA8
	_delay_us(5000);					//ждем 5 мс
	#endif
	
	#ifdef ATMEGA88
	_delay_us(5000);					//ждем 5 мс
	#endif
	
	#ifdef STM32_LL
	LL_mDelay(5);
	#endif
	
	nrf24_write_reg(EN_AA, 0x00);				//отключаем автоподтверждение
	nrf24_write_reg(EN_RXADDR, 0x01);		//разрешаем Pipe0
	nrf24_write_reg(SETUP_AW, 0x01);		//устанавливаем размер адреса 3 байта
	nrf24_write_reg(SETUP_RETR, 0x00);	//устанавливаем период авто ретрансляции 1500мкс, 15 попыток
	nrf24_toggle_features();						//активируем дополнительные команды
	nrf24_write_reg(FEATURE, 0x07);			//устанавливаем стандартные значения регистра FEATURE 
	nrf24_write_reg(DYNPD, 0);					//отключаем динамический размер полезной нагрузки
	nrf24_write_reg(STATUS, 0x70);			//опускаем флаг прерывания
	nrf24_write_reg(RF_CH, 76);					//устанавливаем частоту 2476 MHz
	//Выходноая мощность 0dBm, Скорость передачи: 1Mbps
	nrf24_write_reg(RF_SETUP, 0x06);		//для установки -6dBm: 0x04, -12dBm: 0x02, -18dBm: 0x00
	nrf24_write_buf(TX_ADDR, tx_addr_0, TX_ADR_WIDTH);		//запись адреса передачи
	nrf24_write_buf(RX_ADDR_P1, tx_addr_0, TX_ADR_WIDTH);	//запись адреса приема
	nrf24_write_reg(RX_PW_P0, TX_PLOAD_WIDTH); //устанавливаем число байт полезной нагрузки
	nrf24_rx_mode();					//пока уходим в режим приёмника
}

//Процедура обработки прерывания
void irq_callback(void)
{
	#ifdef ATMEGA8
	char sreg_temp = SREG;				//сохраним значение регистра статуса
	cli();												//запрещаем прерывания
	#endif
	
	#ifdef ATMEGA88
	char sreg_temp = SREG;				//сохраним значение регистра статуса
	cli();												//запрещаем прерывания
	#endif
	
	#ifdef STM32_LL
	__disable_irq();							//запрещение всех прерываний
	#endif
	
	uint8_t status = 0x01;				//переменная статуса
	
	#ifdef ATMEGA8
	_delay_us(10);			
	#endif
	
	#ifdef ATMEGA88
	_delay_us(10);			
	#endif
	
	#ifdef STM32_LL
	delay_us(10);
	#endif
	
	status = nrf24_read_reg(STATUS);	//читаем значения регистра статуса
	if (status & RX_DR)								//если есть данные на прием
	{
		nrf24_read_buf(RD_RX_PLOAD, rx_buf, TX_PLOAD_WIDTH);	//чтение буфера
		nrf24_write_reg(STATUS, 0x40);	//запись в регистр статуса 1 в шестой бит, обнуление остальных
		f_rx = 1;												//поднимаем флаг приема
	}
	if (status & TX_DS)								//если данные успешно отправлены
	{
		nrf24_write_reg(STATUS, 0x20);	//очищаем все биты кроме пятого
		nrf24_rx_mode();								//переходим в режим приема
		f_tx = 1;												//поднимаем флаг передачи
	}
	else if (status & MAX_RT)					//если превышение количества попыток отправки
	{
		nrf24_write_reg(STATUS, 0x10);	//однуление всех остальных битов, кроме 4го
		nrf24_flush_tx();								//очистка буфера отправки
		nrf24_rx_mode();								//переходим в режим приема
	}
	
	#ifdef ATMEGA8
	SREG = sreg_temp;					//вернем значение регистра статуса в исходное состояние
	#endif
	
	#ifdef ATMEGA88
	SREG = sreg_temp;					//вернем значение регистра статуса в исходное состояние
	#endif
	
	#ifdef STM32_LL
	__enable_irq();						//разрешение всех прерываний
	#endif
}
