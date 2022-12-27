/*
* main.c
* ������ �������� ���������� � ������� NRF24L01.
* ������ "�����" (�������� ������� ������������ ������������)
* ���� ������� ����� �����������
*
* ����������������: ATmega8/48/88/168/328
*
* ����� ATmega8:				High: 0xD8, Low: 0xE4
* ����� ATmega48/88/168/328:	High: 0xDF, Low: 0xE2
*
* ����������� ���������:
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
*   ������� �������� ������:
* ��� ��������� ������� � ������ � ���� �������� �������� ��������� �����������
* � �������� ������ ���������� �� ����� ���������� ������� �� ������, ������������ � �������
* ��� ��������� ������ �� ������, � ���� �������� �����, � �������� ������� �����,
* ��� ������� � ��� ���������.
*
* �����: ������� �.�. (MelexinVN)
*/

#include "main.h"						//���������� �������� ������������ ����

//��������� ���������� � �������
char str[STRING_SIZE] = {0};			//����� ��� ������ � ����
uint8_t slave_counter = 0;				//������� �������
uint8_t slave_addrs[] = ADRESS_LIST;	//������ ������� �������
//������� ���������� � �������
extern volatile uint8_t f_rx, f_tx;		//����� ������ � ��������
extern uint8_t tx_buf[TX_PLOAD_WIDTH];	//����� ��������
extern uint8_t rx_buf[TX_PLOAD_WIDTH];	//����� ������

//��������� ������������� ������
void gpio_init(void)
{
	LED_DDR |= 1<<LED_DD;				//��� ���������� �� �����
	LED_PORT |= 0<<LED_PIN;				//������ ������� �� ������ ����������
}

//��������� ������������� ����������
void interrupt_init(void)
{
	//�������� ������� ���������� INT0 �� �����
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

//��������� ������ �����������
void nrf24l01_receive(void)
{
	if(f_rx)	//���� ���� ������ ������ (���� ����������� �� �������� ���������� �� �����������)
	{
		//���� �� ���� ������� �����������
		for (uint8_t i = 0; i < NUM_OF_SLAVES; i++)
		{	//���� ������ �������� �����
			if (rx_buf[0] == slave_addrs[i])	//���� ����� ��������� � ����� �� ������
			{
				if (rx_buf[1] == DS_TEMP)		//���� ������ ����������� �� ds18b20
				{
					if (rx_buf[2] == 0)			//���� ����������� �������������
					{	//������� � ���� ������
						sprintf(str, "0x%02X\tds18b20\tt = %d.%d�C\r\n", rx_buf[0], rx_buf[3], rx_buf[4]);				
						usart_print(str);
					}	
					else if (rx_buf[2] != 0)	//���� ����������� �������������
					{	//������� � ���� ������ � ������� � ��������������� ��������
						sprintf(str, "0x%02X\tds18b20\tt = -%d.%d�C\r\n", rx_buf[0], (255 - rx_buf[3]),rx_buf[4]);			
						usart_print(str);
					}
				}
				else if (rx_buf[1] == DHT11_DATA)	//���� ������ ����������� �� dht11
				{	//������� ������ � ������� ����������� � ��������� � ����
					sprintf(str, "0x%02X\tdht11\tt = %d�C, H = %d%%\r\n", rx_buf[0], rx_buf[2], rx_buf[3]);				
					usart_print(str);
				}
			}
		}
		f_rx = 0;					//�������� ���� ������
	}
}

//��������� �������� �����������
void blink_led(uint8_t blink_counter)
{
	while (blink_counter)			//���� ������� �� ����� 0
	{
		LED_ON();					//�������� ���������
		_delay_ms(50);				//����
		LED_OFF();					//��������� ���������
		_delay_ms(50);				//����
		blink_counter--;			//�������������� �������
	}
}

//������ ���������� �� ����������������� �����
#ifdef ATMEGA88
ISR(USART_RX_vect)
{
	//����� ����� ��������� �������� �� ��������� ������ �� ����������������� �����
}
#endif

#ifdef ATMEGA8
ISR(USART_RXC_vect)
{
	//����� ����� ��������� �������� �� ��������� ������ �� ����������������� �����
}
#endif

//������ ���������� INT0
ISR(INT0_vect)
{
	irq_callback();					//������������ ���������� �� �����������
}

//��������� ������ � ���� ������ �� ��������� �����������
void nrf_info_print(void)
{
	uint8_t buf[TX_ADR_WIDTH] = {0};				//����� ��� ������ ������� ������
	uint8_t dt_reg = 0;								//���������� ��� ������ �������� ��������
	
	dt_reg = nrf24_read_reg(CONFIG);				//������ ������� CONFIG
	sprintf(str, "CONFIG: 0x%02X\r\n", dt_reg);		//
	usart_print(str);								//������� ������ � ����
	dt_reg = nrf24_read_reg(EN_AA);					//������ ������� EN_AA
	sprintf(str, "EN_AA: 0x%02X\r\n", dt_reg);		//
	usart_print(str);								//������� ������ � ����
	dt_reg = nrf24_read_reg(EN_RXADDR);				//������ ������� EN_RXADDR
	sprintf(str, "EN_RXADDR: 0x%02X\r\n", dt_reg);	//
	usart_print(str);								//������� ������ � ����
	dt_reg = nrf24_read_reg(STATUS);				//������ ������� STATUS
	sprintf(str, "STATUS: 0x%02X\r\n", dt_reg);		//
	usart_print(str);								//������� ������ � ����
	dt_reg = nrf24_read_reg(RF_SETUP);				//������ ������� RF_SETUP
	sprintf(str, "RF_SETUP: 0x%02X\r\n", dt_reg);	//
	usart_print(str);								//������� ������ � ����
	nrf24_read_buf(TX_ADDR,buf,3);					//������ ����� TX_ADDR
	sprintf(str, "TX_ADDR: 0x%02X, 0x%02X, 0x%02X\r\n", buf[0], buf[1], buf[2]);
	usart_print(str);								//������� ������ � ����
	nrf24_read_buf(RX_ADDR_P1,buf,3);				//������ ����� RX_ADDR_P1
	sprintf(str, "RX_ADDR: 0x%02X, 0x%02X, 0x%02X\r\n", buf[0], buf[1], buf[2]);
	usart_print(str);								//������� ������ � ����
}

int main(void)
{
	interrupt_init();		//�������������� ����������
	gpio_init();			//�������������� ����� �����-������
	spi_init();				//�������������� SPI
	nrf24_init();			//�������������� �����������
	usart_init(103);		//�������������� USART 9600 ���
	//usart_init(8);		//�������������� USART 115200 ���
	usart_println("start");	//�������� ��������� ������ � ����
	nrf_info_print();		//������� �������� ��������� � ����
	blink_led(5);			//������� �����������
	
	wdt_enable(WDTO_500MS);	//�������� ���������� ������
	
	sei();					//���������� ���������� ����������
	
	while (1)
	{
		//���������� ������ �� ���������� ������
		tx_buf[0] = slave_addrs[slave_counter];	//���������� � ����� ��������� �����
		
		nrf24_send(tx_buf);						//���������� ������� � ����
		//������ �������� ������ ����� ����������, �� ����� ��������� ������ ���������� �������� �� 32 ����
		
		slave_counter++;						//��������� � ���������� ������
		if (slave_counter == NUM_OF_SLAVES)		//���� ����� �� ���������� ��������
		{
			slave_counter = 0;					//�������� �������
		}
		
		_delay_ms(3);							//��������
		
		//�������� ����������, ����� ������� ���������� ������ �������� ������, ��������� �����,
		//� ������ ������� ���������� ������ �������� � ���������� ���� �� ������, ������ ���
		//���������� � �������� ����������
		
		//��� ���������� ������������� �������������� �������� �������� ���������� ��������� ����������
		//���������������� � ������ �������, �������������� �� ��������� �������� ����������������
		
		nrf24l01_receive();		//������������ ����� �����������
		
		wdt_reset();			//���������� ���������� ������
	}
}

