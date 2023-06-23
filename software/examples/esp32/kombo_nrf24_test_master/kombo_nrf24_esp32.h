/*
* kombo_nrf24.h
* Заголовочный файл библиотеки работы с радиомодулем NRF24L01. 
* Проект "КомБО"(Открытые системы беспроводной коммуникации)
*
* Специально для ESP32 WROOM (проверено для платы ESP32 Dev Module)
*
* Здесь необходимо сконфигурировать устройство как ведущее/ведомое,
* номера пинов и порты МК, к которым будет подключен модуль
*
* Автор: Мелехин В.Н. (MelexinVN)
*/

#include "main.h"  //добавляем основной заголовочный файл проекта

#define MASTER  //выбор устройства: MASTER - ведущий, SLAVE - ведомый

//пины VSPI
#define VSPI_MISO MISO
#define VSPI_MOSI MOSI
#define VSPI_SCLK SCK
#define VSPI_SS SS

//прочие пины
#define CE 15       //пин CE
#define IRQ_PIN 21  //пин прерывания радиомодуля

//Макросы манипуляции пинами
#define CSN_ON() digitalWrite(VSPI_SS, LOW)    //прижимание пина CSN к земле
#define CSN_OFF() digitalWrite(VSPI_SS, HIGH)  //поднятие пина CSN
#define CE_RESET() digitalWrite(CE, LOW)       //опускание ноги CE
#define CE_SET() digitalWrite(CE, HIGH)        //поднятие ноги CE

//Макросы команд радиомодуля
#define ACTIVATE 0x50     //активация доп. функций
#define RD_RX_PLOAD 0x61  //прием данных
#define WR_TX_PLOAD 0xA0  //передача данных
#define FLUSH_TX 0xE1     //очистка буфера передачи
#define FLUSH_RX 0xE2     //очистка буфера приема
//Адреса регистров радиомодуля
#define CONFIG 0x00       //адрес регистра конфигурации
#define EN_AA 0x01        //адрес регистра включения автоподтверждения
#define EN_RXADDR 0x02    //адрес регистра разрешения pipe0
#define SETUP_AW 0x03     //адрес регистра установки размера адреса
#define SETUP_RETR 0x04   //адрес регистра установки параметров ретрансляции
#define RF_CH 0x05        //адрес регистра установки частоты
#define RF_SETUP 0x06     //адрес регистра параметров передачи
#define STATUS 0x07       //адрес регистра статуса
#define OBSERVE_TX 0x08   //адрес регистра OBSERVE_TX
#define RX_ADDR_P0 0x0A   //адрес регистра адреса pipe0
#define RX_ADDR_P1 0x0B   //адрес регистра адреса pipe1
#define TX_ADDR 0x10      //адрес регистра передачи
#define RX_PW_P0 0x11     //адрес регистра размера полезной нагрузки pipe0
#define RX_PW_P1 0x12     //адрес регистра размера полезной нагрузки pipe1
#define FIFO_STATUS 0x17  //адрес регистра FIFO
#define DYNPD 0x1C        //адрес регистра управления типа размера полезной нагрузки
#define FEATURE 0x1D      //адрес регистра FEATURE
//Биты и флаги модуля
#define PRIM_RX 0x00     //бит переключения режимов приема/передачи (1: RX, 0: TX)
#define PWR_UP 0x01      //бит включения/выключения 1: POWER UP, 0:POWER DOWN
#define RX_DR 0x40       //флаг готовности принятых данных в FIFO
#define TX_DS 0x20       //флаг отправления данных в FIFO
#define MAX_RT 0x10      //флаг превышения количества попыток отправки
#define W_REGISTER 0x20  //бит записи
//Макросы параметров модуля
#define TX_ADR_WIDTH 3     //размер адреса передачи
#define TX_PLOAD_WIDTH 32  //размер полезной нагрузки
