/*
* kombo_nrf24.c
* ���������� ������ � ������������ NRF24L01 
* ������ "�����" (�������� ������� ������������ ������������)
* 
* ����������������: ATmega8/48/88/168/328
* 
* �����: ������� �.�. MelexinVN
*/

#include "kombo_nrf24.h"				//��������� ������������ ����

#ifdef MASTER	//���� ������� ����������
uint8_t tx_addr_0[TX_ADR_WIDTH] = {0xa1,0xa2,0xa3};	//����� 0
uint8_t tx_addr_1[TX_ADR_WIDTH] = {0xa3,0xa2,0xa1};	//����� 1
#endif

#ifdef SLAVE	//���� ������� ����������
uint8_t tx_addr_0[TX_ADR_WIDTH] = {0xa3,0xa2,0xa1};	//����� 0
uint8_t tx_addr_1[TX_ADR_WIDTH] = {0xa1,0xa2,0xa3};	//����� 1
#endif

uint8_t rx_buf[TX_PLOAD_WIDTH] = {0};	//����� ������
uint8_t tx_buf[TX_PLOAD_WIDTH] = {0};	//����� ��������

volatile uint8_t f_rx = 0, f_tx = 0;	//����� ������ � ��������

//������� ������ �������� ������
uint8_t nrf24_read_reg(uint8_t addr)
{
	uint8_t dt = 0, cmd;				//���������� ������ � �������
	CSN_ON();							//��������� ���� CS � �����
	dt = spi_change_byte(addr);			//�������� ������ ��������, �����
	
	//���� ����� ����� ������ �������� ������� �� � ���������� ��� ���������	
	if (addr != STATUS)					//� ���� �� �����
	{
		cmd = 0xFF;						//������� NOP ��� ��������� ������
		dt = spi_change_byte(cmd);		//
	}
	CSN_OFF();							//��������� ���� CS
	return dt;							//������������ ��������
}

//��������� ������ �������� � ������
void nrf24_write_reg(uint8_t addr, uint8_t dt)		
{
	addr |= W_REGISTER;					//�������� ��� ������ � �����	
	CSN_ON();							//��������� ���� CS � �����
	spi_send_byte(addr);				//���������� �����
	spi_send_byte(dt);					//���������� ��������
	CSN_OFF();							//��������� ���� CS
}

//��������� ��������� �������������� ������
void nrf24_toggle_features(void)							
{	
	uint8_t dt = ACTIVATE;				//���������� � �������� ���������
	CSN_ON();							//��������� ���� CS � �����
	spi_send_byte(dt);					//���������� �������
	_delay_us(1);						//��������
	dt = 0x73;							//��������� �������
	spi_send_byte(dt);					//���������� �������
	CSN_OFF();							//��������� ���� CS
}

//��������� ������ ������
void nrf24_read_buf(uint8_t addr,uint8_t *p_buf,uint8_t bytes)
{
	CSN_ON();							//��������� ���� CS � �����
	spi_send_byte(addr);				//���������� �����
	//���� �� ������ ���������� ����
	for (uint8_t i = 0; i < bytes; i++) 
	{
		p_buf[i] = spi_change_byte(addr);//�������� ��������� ����
	}
	CSN_OFF();							//��������� ���� CS
}

//��������� ������ ������
void nrf24_write_buf(uint8_t addr,uint8_t *p_buf,uint8_t bytes)	
{
	addr |= W_REGISTER;					//�������� ��� ������ � �����
	CSN_ON();							//��������� ���� CS � �����
	spi_send_byte(addr);				//���������� �����
	_delay_us(1);						//��������
	//���� �� ������ ���������� ����
	for (uint8_t i = 0; i < bytes; i++) 
	{
		spi_send_byte(p_buf[i]);		//���������� ��������� ����
	}
	CSN_OFF();							//��������� ���� CS
}

//��������� ������� ������ ������
void nrf24_flush_rx(void)
{
	uint8_t dt = FLUSH_RX;				//���������� � �������� �������
	CSN_ON();							//��������� ���� CS � �����
	spi_send_byte(dt);					//�������� �������
	_delay_us(1);						//�������� 
	CSN_OFF();							//��������� ���� CS
}

//��������� ������� ������ ��������
void nrf24_flush_tx(void)
{
	uint8_t dt = FLUSH_TX;				//���������� � �������� �������
	CSN_ON();							//��������� ���� CS � �����
	spi_send_byte(dt);					//�������� �������
	_delay_us(1);						//�������� 
	CSN_OFF();							//��������� ���� CS
}

//��������� ��������� ������ ���������
void nrf24_rx_mode(void)
{
	uint8_t regval = 0x00;				//���������� ��� �������� ��������
	regval = nrf24_read_reg(CONFIG);	//��������� �������� �������� ������������
	//�������� ������ � �������� ��� � ����� ��������, ������� ���� PWR_UP � PRIM_RX
	regval |= (1<<PWR_UP)|(1<<PRIM_RX);	
	nrf24_write_reg(CONFIG,regval);		//���������� �������� �������� �������
	//����������  ����� �����������
	nrf24_write_buf(TX_ADDR, tx_addr_1, TX_ADR_WIDTH);	
	//���������� ����� ���������
	nrf24_write_buf(RX_ADDR_P0, tx_addr_1, TX_ADR_WIDTH);	
	CE_SET();							//��������� ���� CE
	_delay_us(150);						//�������� ������� 130 ���
	//������� �������
	nrf24_flush_rx();
	nrf24_flush_tx();
}

//��������� ��������� ������ �����������
void nrf24_tx_mode(void)
{
	//���������� ����� �����������
	nrf24_write_buf(TX_ADDR, tx_addr_0, TX_ADR_WIDTH);		
	//���������� ����� ���������
	nrf24_write_buf(RX_ADDR_P0, tx_addr_0, TX_ADR_WIDTH);	
	CE_RESET();							//�������� ���� CE
	//������� ��� ������
	nrf24_flush_rx();
	nrf24_flush_tx();
}

//��������� �������� ������ � ������
void nrf24_transmit(uint8_t addr,uint8_t *p_buf,uint8_t bytes)
{
	CE_RESET();						//�������� ���� CE
	CSN_ON();						//��������� ���� CS � �����
	spi_send_byte(addr);			//���������� �����
	_delay_us(1);					//��������
	//���� �� ������ ���������� ����
	for (uint8_t i = 0; i < bytes; i++) 
	{
		spi_send_byte(p_buf[i]);	//���������� ��������� ����
	}
	CSN_OFF();						//��������� ���� CS
	CE_SET();						//��������� ���� CE
}

//��������� �������� ������ � ����
void nrf24_send(uint8_t *p_buf)
{
	char sreg_temp = SREG;				//�������� �������� �������� �������
	cli();								//��������� ����������
		
	uint8_t regval = 0x00;				//���������� ��� �������� � ���������������� �������
	nrf24_tx_mode();					//�������� ����� ��������
	regval = nrf24_read_reg(CONFIG);	//��������� �������� ����������������� �������
	//���� ������ ���� � ������ �����, �� �������� ���, ������� ��� PWR_UP � �������� PRIM_RX
	regval |= (1<<PWR_UP);					
	regval &= ~(1<<PRIM_RX);
	nrf24_write_reg(CONFIG, regval);	//���������� ����� �������� ����������������� ��������
	_delay_us(150);						//�������� ������� 130 ���
	nrf24_transmit(WR_TX_PLOAD, p_buf, TX_PLOAD_WIDTH);//�������� ������
	CE_SET();							//��������� ���� CE
	_delay_us(15);						//�������� 10us
	CE_RESET();							//�������� ���� CE
	
	SREG = sreg_temp;					//������ �������� �������� ������� � �������� ���������
}

//��������� ������������� �����, ������������ � �����������
void nrf24_pins_init(void)
{
	CE_DDR |= 1<<CE_DD;					//CE �� �����
	CE_PORT |= 1<<CE_PIN;				//������� ������� �� CE
	
	CSN_DDR |= 1<<CSN_DD;				//CSN �� �����
	CSN_PORT |= 1<<CSN_PIN;				//������� ������� �� CSN
	
	IRQ_DDR |= 0<<IRQ_DD;				//IRQ �� ����
	IRQ_PORT |= 0<<IRQ_PIN;				//��������� ���������� ������
}

//��������� ������������� ������
void nrf24_init(void)
{
	nrf24_pins_init();					//�������������� ����
	CE_RESET();							//�������� � ����� ����� CE
	_delay_us(5000);					//���� 5 ��
	//���������� ���������������� ����, 
	//������������� ��� PWR_UP bit, �������� CRC(1 ����) &Prim_RX:0
	nrf24_write_reg(CONFIG, 0x0a);		
	_delay_us(5000);					//���� 5 ��
	nrf24_write_reg(EN_AA, 0x00);		//��������� �����������������
	nrf24_write_reg(EN_RXADDR, 0x01);	//��������� Pipe0
	nrf24_write_reg(SETUP_AW, 0x01);	//������������� ������ ������ 3 �����
	nrf24_write_reg(SETUP_RETR, 0x00);	//������������� ������ ���� ������������ 1500���, 15 �������
	nrf24_toggle_features();			//���������� �������������� �������
	nrf24_write_reg(FEATURE, 0x07);		//������������� ����������� �������� �������� FEATURE 
	nrf24_write_reg(DYNPD, 0);			//��������� ������������ ������ �������� ��������
	nrf24_write_reg(STATUS, 0x70);		//�������� ���� ����������
	nrf24_write_reg(RF_CH, 76);			//������������� ������� 2476 MHz
	//��������� �������� 0dBm, �������� ��������: 1Mbps
	nrf24_write_reg(RF_SETUP, 0x06);	//��� ��������� -6dBm: 0x04, -12dBm: 0x02, -18dBm: 0x00
	nrf24_write_buf(TX_ADDR, tx_addr_0, TX_ADR_WIDTH);		//������ ������ ��������
	nrf24_write_buf(RX_ADDR_P1, tx_addr_0, TX_ADR_WIDTH);	//������ ������ ������
	nrf24_write_reg(RX_PW_P0, TX_PLOAD_WIDTH); //������������� ����� ���� �������� ��������
	nrf24_rx_mode();					//���� ������ � ����� ��������
}

//��������� ��������� ����������
void irq_callback(void)
{
	char sreg_temp = SREG;				//�������� �������� �������� �������
	cli();								//��������� ����������

	uint8_t status = 0x01;				//���������� �������
	_delay_us(10);			
	status = nrf24_read_reg(STATUS);	//������ �������� �������� �������
	if (status & RX_DR)					//���� ���� ������ �� �����
	{
		nrf24_read_buf(RD_RX_PLOAD, rx_buf, TX_PLOAD_WIDTH);	//������ ������
		nrf24_write_reg(STATUS, 0x40);	//������ � ������� ������� 1 � ������ ���, ��������� ���������
		f_rx = 1;						//��������� ���� ������
	}
	if (status & TX_DS)					//���� ������ ������� ����������
	{
		nrf24_write_reg(STATUS, 0x20);	//������� ��� ���� ����� ������
		nrf24_rx_mode();				//��������� � ����� ������
		f_tx = 1;						//��������� ���� ��������
	}
	else if (status & MAX_RT)			//���� ���������� ���������� ������� ��������
	{
		nrf24_write_reg(STATUS, 0x10);	//��������� ���� ��������� �����, ����� 4��
		nrf24_flush_tx();				//������� ������ ��������
		nrf24_rx_mode();				//��������� � ����� ������
	}
	
	SREG = sreg_temp;					//������ �������� �������� ������� � �������� ���������
}
