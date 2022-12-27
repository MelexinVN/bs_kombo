/*
* kombo_nrf24.h
* ������������ ���� ���������� ������ � ������������ NRF24L01. 
* ������ "�����"(�������� ������� ������������ ������������)
*
* ����������������: ATmega8/48/88/168/328
*
* ��������� ��������� ��������� Arduino
*
* ����� ���������� ���������������� ���������� ��� �������/�������,
* ������ ����� � ����� ��, � ������� ����� ��������� ������
*
* �����: ������� �.�. (MelexinVN)
*/

#ifndef NRF24_H_
#define NRF24_H_

#include "main.h"  //��������� �������� ������������ ���� �������

#define MASTER  //����� ����������: MASTER - �������, SLAVE - �������

//������� ������� ����������������
#ifdef ATMEGA8
#define IRQ_PIN PORTD2  //��� ���������� �����������
#define CE_PIN PORTB0   //��� CE
#define CSN_PIN PORTD7  //��� CSN
#endif

#ifdef ATMEGA88
#define IRQ_PIN PD2  //��� ���������� �����������
#define CE_PIN PB0   //��� CE
#define CSN_PIN PD7  //��� CSN
#endif

#ifndef ARDUINO

#define IRQ_PORT PORTD  //���� ���������� �����������
#define IRQ_DD DDD2     //��� ����������� ������ ���������� �����������
#define IRQ_DDR DDRD    //���� ����������� ������ ���������� �����������

#define CE_PORT PORTB  //���� CE
#define CE_DD DDB0     //��� ����������� ������ CE
#define CE_DDR DDRB    //���� ����������� ������ CE

#define CSN_PORT PORTD  //���� CSN
#define CSN_DD DDD7     //��� ����������� ������ CSN
#define CSN_DDR DDRD    //���� ����������� ������ CSN

//������� ����������� ������
#define CSN_ON() CSN_PORT &= ~(1 << CSN_PIN)  //���������� ���� CSN � �����
#define CSN_OFF() CSN_PORT |= (1 << CSN_PIN)  //�������� ���� CSN
#define CE_RESET() CE_PORT &= ~(1 << CE_PIN)  //��������� ���� CE
#define CE_SET() CE_PORT |= (1 << CE_PIN)     //�������� ���� CE

#endif

#ifdef ARDUINO

#define IRQ_PIN 2   //��� ���������� �����������
#define CE_PIN 9    //��� CE
#define CSN_PIN 10  //��� CSN

//������� ����������� ������
#define CSN_ON() digitalWrite(CSN_PIN, LOW)    //���������� ���� CSN � �����
#define CSN_OFF() digitalWrite(CSN_PIN, HIGH)  //�������� ���� CSN
#define CE_RESET() digitalWrite(CE_PIN, LOW)   //��������� ���� CE
#define CE_SET() digitalWrite(CE_PIN, HIGH)    //�������� ���� CE

#endif

//������� ������ �����������
#define ACTIVATE 0x50     //��������� ���. �������
#define RD_RX_PLOAD 0x61  //����� ������
#define WR_TX_PLOAD 0xA0  //�������� ������
#define FLUSH_TX 0xE1     //������� ������ ��������
#define FLUSH_RX 0xE2     //������� ������ ������
//������ ��������� �����������
#define CONFIG 0x00       //����� �������� ������������
#define EN_AA 0x01        //����� �������� ��������� �����������������
#define EN_RXADDR 0x02    //����� �������� ���������� pipe0
#define SETUP_AW 0x03     //����� �������� ��������� ������� ������
#define SETUP_RETR 0x04   //����� �������� ��������� ���������� ������������
#define RF_CH 0x05        //����� �������� ��������� �������
#define RF_SETUP 0x06     //����� �������� ���������� ��������
#define STATUS 0x07       //����� �������� �������
#define OBSERVE_TX 0x08   //����� �������� OBSERVE_TX
#define RX_ADDR_P0 0x0A   //����� �������� ������ pipe0
#define RX_ADDR_P1 0x0B   //����� �������� ������ pipe1
#define TX_ADDR 0x10      //����� �������� ��������
#define RX_PW_P0 0x11     //����� �������� ������� �������� �������� pipe0
#define RX_PW_P1 0x12     //����� �������� ������� �������� �������� pipe1
#define FIFO_STATUS 0x17  //����� �������� FIFO
#define DYNPD 0x1C        //����� �������� ���������� ���� ������� �������� ��������
#define FEATURE 0x1D      //����� �������� FEATURE
//���� � ����� ������
#define PRIM_RX 0x00     //��� ������������ ������� ������/�������� (1: RX, 0: TX)
#define PWR_UP 0x01      //��� ���������/���������� 1: POWER UP, 0:POWER DOWN
#define RX_DR 0x40       //���� ���������� �������� ������ � FIFO
#define TX_DS 0x20       //���� ����������� ������ � FIFO
#define MAX_RT 0x10      //���� ���������� ���������� ������� ��������
#define W_REGISTER 0x20  //��� ������
//������� ���������� ������
#define TX_ADR_WIDTH 3     //������ ������ ��������
#define TX_PLOAD_WIDTH 32  //������ �������� ��������

//��������� � �������:
//��������� ������������� ������
void nrf24_init(void);
//������� ������ �������� ������
uint8_t nrf24_read_reg(uint8_t addr);
//��������� ������ ������
void nrf24_read_buf(uint8_t addr, uint8_t *p_buf, uint8_t bytes);
//��������� �������� ������ � ����
void nrf24_send(uint8_t *p_buf);
//��������� ��������� ����������
void irq_callback(void);
//------------------------------------------------
#endif /* NRF24_H_ */
