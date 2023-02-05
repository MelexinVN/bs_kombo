/*
* main.c
* ������ �������� ���������� � ������� NRF24L01.
* ������ "�����" (�������� ������� ������������ ������������)
* ������ �������������
*
* ����������������: ATmega8/48/88/168/328
*
* ����� ATmega8:				High: 0xD8, Low: 0xE4
* ����� ATmega48/88/168/328:	High: 0xDF, Low: 0xE2
*
* ����������� ��������� �� ���������:
* NRF24L01		ATmega8/48/88/168/328	Arduino nano(UNO)
* CE			PB0(12)					D8
* CSN			PD7(11)					D7
* SCK			PB5(17)					D13
* MOSI			PB3(15)					D12
* MISO			PB4(16)					D11
* IRQ			PD2(32)					D2
*
* ������		PD3(1)					D3
*
* LED			PD6(10)					D6
*
* USART			ATmega8/48/88/168/328	Arduino nano(UNO)
* TX			PD0(30)					D0
* RX			PD1(31)					D1
*
*   ������� �������� ������:
* ��� ��������� ������� � ������ � ���� �������� �������� ��������� �����������
* � �������� ������ ��������������� ����������� �����, ��������� � ���������� ������, 
* ��� ��������� ������� � ������� ������� �� ��������. 
* ������� ������, �� ���������� � ����� ��������� ������, ��� ����� ��������� �� �������,
* � ����� ��������� �������, ������������ � ������� - �������� ���������, ������ ��� �������� ���������.
*	
* �����: ������� �.�. (MelexinVN)
*/

#include "main.h"						//���������� �������� ������������ ����

//��������� ���������� � �������
char str[STRING_SIZE] = {0};			//����� ��� ������ � ����
volatile uint8_t f_read_temp = 0;		//���� ������ �����������
volatile uint32_t ms_counter = 0;		//������� ����������
volatile uint8_t f_pushed = 0;			//���� �������
volatile uint32_t time_ms = 0;			//����������� ����� ��
//������� ���������� � �������
extern volatile uint8_t f_rx, f_tx;		//����� ������ � ��������
extern uint8_t tx_buf[TX_PLOAD_WIDTH];	//����� ��������
extern uint8_t rx_buf[TX_PLOAD_WIDTH];	//����� ������

//��������� ������������� ������ �����/������
void gpio_init(void)
{
	LED_DDR |= 1<<LED_DD;				//��� ���������� �� �����
	LED_PORT |= 0<<LED_PIN;				//������ ������� �� ������ ����������
}

//��������� ������������� ������� 1
void timer_init(void)
{
	//������� ������������: 125,000 kHz
	//�����: CTC top=OCR1A (����� �� ����������)
	//������ ���������
	//������ : 1 ��
	//�������� ���������� �� ����������
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

//��������� ������������� ����������
void interrupt_init(void)
{
	//�������� ������� ���������� INT0, INT1 �� �����
	//�������� ���������� �� ��������� � ������� 1
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

//��������� �������� �����������
void blink_led(uint8_t blink_counter)
{
	while (blink_counter)		//���� ������� �� ����� 0
	{
		LED_ON();				//�������� ���������
		_delay_ms(10);			//����
		LED_OFF();				//��������� ���������
		_delay_ms(50);			//����
		blink_counter--;		//�������������� �������
	}
}

//��������� ������ �����������
void nrf24l01_receive(void)
{
	if(f_rx)	//���� ���� ������ ������ (���� ����������� �� �������� ���������� �� �����������)
	{
		if (rx_buf[0] == SOFT_RESET)	//���� ������ ������� ������������ ������
		{
			f_pushed = 0;				//�������� ���� �������
			ms_counter = 0;				//�������� �������� �������
			usart_println("reset");
			blink_led(2);				//������������� ��������� ����������
		}
					
		if (rx_buf[0] == OUR_ADDR)		//���� ������ �������� ���� ��������� � ������� ������
		{
			if (f_pushed)				//���� ������ ���� �������
			{
				tx_buf[0] = OUR_ADDR;	//���������� � ������ ���� �����
				(*(unsigned long*)&tx_buf[1]) = time_ms;	//�� ������, ��������������� � ��� unsigned long, ���������� �������� �������
				nrf24_send(tx_buf);		//���������� ������� � ����
			}
			else						//���� ������� �� ����
			{
				tx_buf[0] = OUR_ADDR;	//���������� � ������ ���� �����
				(*(unsigned long*)&tx_buf[1]) = NOT_PUSHED;//�� ������, ��������������� � ��� unsigned long, ���������� �������� ���������� ���������
				nrf24_send(tx_buf);		//���������� ������� � ����
			}
			if (rx_buf[1] == 0x01)		//���� ��������� ������� ���������� �����������
			{	//������������� ���������� ���������� � ������������ � ��������� ��������
				if(rx_buf[2] == LED_STATE_ON) LED_ON();		
				if(rx_buf[2] == LED_STATE_OFF) LED_OFF();
			}
			if (rx_buf[1] == SOFT_RESET)//���� ������ ������� ������������ ������
			{
				f_pushed = 0;			//�������� ���� �������
				ms_counter = 0;			//�������� �������� �������
				blink_led(2);			//������������� ��������� ����������
			}
		}
		f_rx = 0;						//�������� ���� ������
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
	irq_callback();			//������������ ���������� �� �����������
}
//������ ���������� INT1
ISR(INT1_vect)
{
	if(!f_pushed)				//���� ������ ���� �������
	{
		f_pushed = 1;			//��������� ���� �������
		time_ms = ms_counter;	//��������� ���������� ��
	}
}

//��������� ��������� ���������� �� ��������� � ������� 1
void t1_compa_callback(void)
{
	ms_counter++;			//������� ��
}

//������ ���������� �� ��������� � ������� 1
ISR(TIMER1_COMPA_vect)
{
	t1_compa_callback();	//������������ ����������
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
	timer_init();			//�������������� ������
	usart_init(103);		//�������������� USART 9600 ���
	//usart_init(8);		//�������������� USART 115200 ���
	usart_println("start");	//�������� ��������� ������ � ����
	nrf24_init();			//�������������� �����������
	blink_led(5);			//������� �����������
	nrf_info_print();		//������� �������� ��������� � ����

	wdt_enable(WDTO_2S);	//�������� ���������� ������

	sei();					//���������� ���������� ����������
	
	while (1)
	{
		nrf24l01_receive();	//������������ ����� �����������
		//���������, �� ������ �� ������ �� ��������

		wdt_reset();		//���������� ���������� ������
	}
}

