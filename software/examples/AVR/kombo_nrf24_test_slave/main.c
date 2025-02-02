/*
* main.c
* �������� ������ �������� ���������� � ������� NRF24L01.
* ������ "�����" (�������� ������� ������������ ������������)
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
* LED			PD6(10)					D6
*
* USART
* TX			PD0(30)					D0
* RX			PD1(31)					D1
*
*   ������� �������� ������:
* ��� ��������� ������� � ������ � ���� �������� �������� ��������� �����������
* � �������� ������ ���������� ������� ������� �� �������� � ������� ��� ����������
* � ����� ���� ����� � �������� �������� �������� (�� 0 �� 255), � ����� ������������� 
* �� ���� ������� ������ ����������� � ������� ��������� � ���������������� ����.
*
* �����: ������� �.�. (MelexinVN)
*/

#include "main.h"						//���������� �������� ������������ ����

//��������� ���������� � �������
char str[STRING_SIZE] = {0};			//����� ��� ������ � ����
uint8_t counter = 0;					//������� �������� 0-255
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

//��������� ������ �����������
void nrf24l01_receive(void)
{
	if(f_rx)	//���� ���� ������ ������ (���� ����������� �� �������� ���������� �� �����������)
	{
		if (rx_buf[0] == OUR_ADDR)	//���� ������ �������� ���� ��������� � ������� ����������
		{
			/*�������� �� ������� �������� ����������*/
			
			//��� ����� ��������� �������� �������� ���������� ��� ��������� ������������� ��� �������
			//� ������ ������� �� ��������� � ���������� ����� �� ������, � ����� ������������� �� ���� �������
			//����� �������� ����� ���������� � ���� ������, �� ����� ��������� ������ ���������� �������� �� 32 ����
			
			tx_buf[0] = OUR_ADDR;	//���������� � ������ ���� ������ �����
			tx_buf[1] = counter;	//���������� �� ������ ���� ������ �������� ��������
			nrf24_send(tx_buf);		//���������� ������� � ����
			blink_led(2);			//������� �����������
			sprintf(str,"��������� ����� �� ������ �%d\r\n", counter);	//��������� ������ ��� ������ � ����
			usart_print(str);		//���������� ������ � ����
			counter++;				//�������������� ��������
		}
		f_rx = 0;					//�������� ���� ������
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
	irq_callback();	//������������ ���������� �� �����������
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

//�������� ���������
int main(void)
{
	spi_init();				//�������������� SPI
	//_delay_ms(1000);		//�������� ��� ����������� ��������� SPI
	//(� ��������� ������� ���������� �������� ��� ������������� �������� �� ������� ��������� � ����� � ����� ��� ��������� �������)
	interrupt_init();		//�������������� ����������
	gpio_init();			//�������������� ����� �����-������
	usart_init(103);		//�������������� USART 9600 ���
	//usart_init(8);		//�������������� USART 115200 ���
	usart_println("start");	//���������� ��������� ������ � ����
	nrf24_init();			//�������������� �����������
	blink_led(5);			//������� �����������
	nrf_info_print();		//������� �������� ��������� � ����
	
	wdt_enable(WDTO_500MS);	//�������� ���������� ������
	
	sei();					//���������� ���������� ����������	
	
    while (1) 
    {
		nrf24l01_receive();	//������������ ����� �����������
		//���������, �� ������ �� ������ �� ��������
		wdt_reset();		//���������� ���������� ������
    }
}

