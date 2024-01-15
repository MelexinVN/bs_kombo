/*
* kombo_nrf24.h
* Заголовочный файл библиотеки работы с радиомодулем NRF24L01. 
* Версия 1.1
* Проект "КомБО"(Открытые системы беспроводной коммуникации)
*
* Микроконтроллеры: ATmega8/88,	STM32 (LL)
*
* Здесь необходимо сконфигурировать:
* - частоту работы модуля,
* - уникальные адреса приема и передачи,
* - устройство как ведущее/ведомое,
* - номера пинов и порты МК, к которым будет подключен радиомодуль (для AVR).
*
* Автор: Мелехин В.Н. (MelexinVN)
*/

#ifndef NRF24_H_
#define NRF24_H_

#include "main.h"		//добавляем основной заголовочный файл проекта
//Настройки устройства
#define MASTER			//выбор устройства: MASTER - ведущий, SLAVE - ведомый
#define UNIQUE_ADDRESS_0	{0xa5,0xa6,0xa7}	//уникальный адрес 0
#define UNIQUE_ADDRESS_1	{0xa7,0xa6,0xa5}	//уникальный адрес 1
#define CHANNEL 	81	//частота 2481 MHz
//Мощность при скорости передачи 1Mbps
#define MAX_POWER 0x06
#define MID_POWER 0x04
#define LOW_POWER 0x02
#define MIN_POWER 0x00

//Макросы выводов микроконтроллера
#ifdef ATMEGA8
#define IRQ_PIN 	PORTD2	//пин прерывания радиомодуля
#define CE_PIN		PORTB0	//пин CE
#define CSN_PIN 	PORTD7	//пин CSN

#define IRQ_PORT	PORTD		//порт прерывания радиомодуля
#define IRQ_DD		DDD2		//бит направления данных прерывания радиомодуля
#define IRQ_DDR		DDRD		//порт направления данных прерывания радиомодуля

#define CE_PORT		PORTB		//порт CE
#define CE_DD			DDB0		//бит направления данных CE
#define CE_DDR		DDRB		//порт направления данных CE

#define CSN_PORT	PORTD		//порт CSN
#define CSN_DD		DDD7		//бит направления данных CSN
#define CSN_DDR		DDRD		//порт направления данных CSN	

//Макросы манипуляции пинами 
#define CSN_ON()		CSN_PORT&=~(1<<CSN_PIN)	//прижимание пина CSN к земле
#define CSN_OFF()		CSN_PORT|=(1<<CSN_PIN)	//поднятие пина CSN
#define CE_RESET()	CE_PORT&=~(1<<CE_PIN)		//опускание ноги CE
#define CE_SET()		CE_PORT|=(1<<CE_PIN)		//поднятие ноги CE
#endif

#ifdef ATMEGA88
#define IRQ_PIN PD2			//пин прерывания радиомодуля
#define CE_PIN	PB0			//пин CE
#define CSN_PIN PD7			//пин CSN

#define IRQ_PORT	PORTD	//порт прерывания радиомодуля
#define IRQ_DD 		DDD2	//бит направления данных прерывания радиомодуля
#define IRQ_DDR		DDRD	//порт направления данных прерывания радиомодуля

#define CE_PORT		PORTB	//порт CE
#define CE_DD			DDB0	//бит направления данных CE
#define CE_DDR		DDRB	//порт направления данных CE

#define CSN_PORT	PORTD	//порт CSN
#define CSN_DD		DDD7	//бит направления данных CSN
#define CSN_DDR		DDRD	//порт направления данных CSN	

//Макросы манипуляции пинами 
#define CSN_ON()		CSN_PORT&=~(1<<CSN_PIN)	//прижимание пина CSN к земле
#define CSN_OFF()		CSN_PORT|=(1<<CSN_PIN)	//поднятие пина CSN
#define CE_RESET()	CE_PORT&=~(1<<CE_PIN)		//опускание ноги CE
#define CE_SET()		CE_PORT|=(1<<CE_PIN)		//поднятие ноги CE
#endif

#ifdef STM32_LL
//Макросы манипуляции пинами 
#define CSN_ON()		LL_GPIO_ResetOutputPin(CSN_GPIO_Port, CSN_Pin)	//прижимание пина CSN к земле
#define CSN_OFF()		LL_GPIO_SetOutputPin(CSN_GPIO_Port, CSN_Pin)		//поднятие пина CSN
#define CE_RESET()	LL_GPIO_ResetOutputPin(CE_GPIO_Port, CE_Pin)		//опускание ноги CE
#define CE_SET()		LL_GPIO_SetOutputPin(CE_GPIO_Port, CE_Pin)			//поднятие ноги CE
#endif

//Макросы команд радиомодуля
#define ACTIVATE 		0x50 //активация доп. функций
#define RD_RX_PLOAD	0x61 //прием данных
#define WR_TX_PLOAD	0xA0 //передача данных
#define FLUSH_TX 		0xE1 //очистка буфера передачи
#define FLUSH_RX 		0xE2 //очистка буфера приема
//Адреса регистров радиомодуля
#define CONFIG 			0x00 //адрес регистра конфигурации
#define EN_AA 			0x01 //адрес регистра включения автоподтверждения
#define EN_RXADDR		0x02 //адрес регистра разрешения pipe0
#define SETUP_AW 		0x03 //адрес регистра установки размера адреса
#define SETUP_RETR	0x04 //адрес регистра установки параметров ретрансляции
#define RF_CH 			0x05 //адрес регистра установки частоты
#define RF_SETUP 		0x06 //адрес регистра параметров передачи
#define STATUS 			0x07 //адрес регистра статуса
#define OBSERVE_TX	0x08 //адрес регистра OBSERVE_TX
#define RX_ADDR_P0	0x0A //адрес регистра адреса pipe0
#define RX_ADDR_P1	0x0B //адрес регистра адреса pipe1
#define TX_ADDR 		0x10 //адрес регистра передачи
#define RX_PW_P0 		0x11 //адрес регистра размера полезной нагрузки pipe0
#define RX_PW_P1 		0x12 //адрес регистра размера полезной нагрузки pipe1
#define FIFO_STATUS	0x17 //адрес регистра FIFO
#define DYNPD 			0x1C //адрес регистра управления типа размера полезной нагрузки
#define FEATURE 		0x1D //адрес регистра FEATURE
//Биты и флаги модуля
#define PRIM_RX 		0x00 //бит переключения режимов приема/передачи (1: RX, 0: TX)
#define PWR_UP 			0x01 //бит включения/выключения 1: POWER UP, 0:POWER DOWN
#define RX_DR 			0x40 //флаг готовности принятых данных в FIFO
#define TX_DS 			0x20 //флаг отправления данных в FIFO
#define MAX_RT 			0x10 //флаг превышения количества попыток отправки
#define W_REGISTER	0x20 //бит записи
//Макросы параметров модуля
#define TX_ADR_WIDTH		3		//размер адреса передачи
#define TX_PLOAD_WIDTH	32	//размер полезной нагрузки

//Процедуры и функции:
//Процедура инициализации модуля
void nrf24_init(void);
//Функция чтения регистра модуля
uint8_t nrf24_read_reg(uint8_t addr);
//Процедура чтения буфера
void nrf24_read_buf(uint8_t addr,uint8_t *p_buf,uint8_t bytes);
//Процедура отправки данных в эфир
void nrf24_send(uint8_t *p_buf);
//Процедура обработки прерывания
void irq_callback(void);

#endif /* NRF24_H_ */
