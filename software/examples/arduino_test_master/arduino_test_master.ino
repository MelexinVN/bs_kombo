/*
* main.c
* �������� ������ �������� ���������� � ������� NRF24L01. 
* ������ "�����" (�������� ������� ������������ ������������)
*
* ����������� ���������:
* NRF24L01		Arduino nano/UNO    Arduino Mega 
* CE					D9                  D9
* CSN					D10                 D10
* SCK					D13                 D52
* MOSI				D12                 D51
* MISO				D11                 D50
* IRQ					D2                  D2
*
*   ������� �������� ������:
* ��� ��������� ������� � ������ � ���� �������� �������� ��������� �����������
* � �������� ������ ���������� �� ����� ���������� ������� �� ������, ������������ � �������
* ��� ��������� ������ �� ������, � ���� �������� �����, � �������� ������� ����� 
* � ���� ������, ���������� �� ����.
* 
* �����: ������� �.�. (MelexinVN)
*/


#include "main.h"  //���������� �������� ������������ ����

//��������� ���������� � �������
char str[STRING_SIZE];                //����� ��� ������ � ����
uint8_t slave_counter = 0;            //������� �������
uint8_t slave_addrs[] = ADRESS_LIST;  //������ ������� �������
//������� ���������� � �������
extern volatile uint8_t f_rx, f_tx;     //����� ������ � ��������
extern uint8_t tx_buf[TX_PLOAD_WIDTH];  //����� ��������
extern uint8_t rx_buf[TX_PLOAD_WIDTH];  //����� ������


#ifdef MASTER                                            //���� ������� ����������
uint8_t tx_addr_0[TX_ADR_WIDTH] = { 0xa1, 0xa2, 0xa3 };  //����� 0
uint8_t tx_addr_1[TX_ADR_WIDTH] = { 0xa3, 0xa2, 0xa1 };  //����� 1
#endif

#ifdef SLAVE                                             //���� ������� ����������
uint8_t tx_addr_0[TX_ADR_WIDTH] = { 0xa3, 0xa2, 0xa1 };  //����� 0
uint8_t tx_addr_1[TX_ADR_WIDTH] = { 0xa1, 0xa2, 0xa3 };  //����� 1
#endif

uint8_t rx_buf[TX_PLOAD_WIDTH] = { 0 };  //����� ������
uint8_t tx_buf[TX_PLOAD_WIDTH] = { 0 };  //����� ��������

volatile uint8_t f_rx = 0, f_tx = 0;  //����� ������ � ��������

//������ ���������
void setup() {
  interrupt_init();  //�������������� ����������
  gpio_init();       //�������������� ����� �����-������
  SPI.begin();
  nrf24_init();             //�������������� �����������
  Serial.begin(9600);       //�������������� USART 9600 ���
  Serial.println("start");  //�������� ��������� ������ � ����
  nrf_info_print();         //������� �������� ��������� � ����
  blink_led(5);             //������� �����������
}

//�������� ����
void loop() {
  //���������� ������ �� ���������� ������
  tx_buf[0] = slave_addrs[slave_counter];  //���������� � ����� ��������� �����

  nrf24_send(tx_buf);  //���������� ������� � ����
  //������ �������� ������ ����� ����������, �� ����� ��������� ������ ���������� �������� �� 32 ����

  slave_counter++;                     //��������� � ���������� ������
  if (slave_counter == NUM_OF_SLAVES)  //���� ����� �� ���������� ��������
  {
    slave_counter = 0;  //�������� �������
  }

  _delay_ms(20);  //��������

  //�������� ����������, ����� ������� ���������� ������ �������� ������, ��������� �����,
  //� ������ ������� ���������� ������ �������� � ���������� ���� �� ������, ������ ���
  //���������� � �������� ����������

  //��� ���������� ������������� �������������� �������� �������� ���������� ��������� ����������
  //���������������� � ������ �������, �������������� �� ��������� �������� ����������������

  nrf24l01_receive();  //������������ ����� �����������
}


//��������� � �������:
//��������� ������������� ������
void gpio_init(void) {

  pinMode(LED_BUILTIN, OUTPUT);
}

//��������� ������������� ����������
void interrupt_init(void) {
  //�������� ������� ���������� �� �����
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), irq_callback, FALLING);
}

//������� ������ �������� ������
uint8_t nrf24_read_reg(uint8_t addr) {
  uint8_t dt = 0, cmd;      //���������� ������ � �������
  CSN_ON();                 //��������� ���� CS � �����
  dt = SPI.transfer(addr);  //�������� ������ ��������, �����

  //���� ����� ����� ������ �������� ������� �� � ���������� ��� ���������
  if (addr != STATUS)  //� ���� �� �����
  {
    cmd = 0xFF;              //������� NOP ��� ��������� ������
    dt = SPI.transfer(cmd);  //
  }
  CSN_OFF();  //��������� ���� CS
  return dt;  //������������ ��������
}

//��������� ������ �������� � ������
void nrf24_write_reg(uint8_t addr, uint8_t dt) {
  addr |= W_REGISTER;  //�������� ��� ������ � �����
  CSN_ON();            //��������� ���� CS � �����
  SPI.transfer(addr);  //���������� �����
  SPI.transfer(dt);    //���������� ��������
  CSN_OFF();           //��������� ���� CS
}

//��������� ��������� �������������� ������
void nrf24_toggle_features(void) {
  uint8_t dt = ACTIVATE;  //���������� � �������� ���������
  CSN_ON();               //��������� ���� CS � �����
  SPI.transfer(dt);       //���������� �������
  _delay_us(1);           //��������
  dt = 0x73;              //��������� �������
  SPI.transfer(dt);       //���������� �������
  CSN_OFF();              //��������� ���� CS
}

//��������� ������ ������
void nrf24_read_buf(uint8_t addr, uint8_t *p_buf, uint8_t bytes) {
  CSN_ON();            //��������� ���� CS � �����
  SPI.transfer(addr);  //���������� �����
  //���� �� ������ ���������� ����
  for (uint8_t i = 0; i < bytes; i++) {
    p_buf[i] = SPI.transfer(addr);  //�������� ��������� ����
  }
  CSN_OFF();  //��������� ���� CS
}

//��������� ������ ������
void nrf24_write_buf(uint8_t addr, uint8_t *p_buf, uint8_t bytes) {
  addr |= W_REGISTER;  //�������� ��� ������ � �����
  CSN_ON();            //��������� ���� CS � �����
  SPI.transfer(addr);  //���������� �����
  _delay_us(1);        //��������
  //���� �� ������ ���������� ����
  for (uint8_t i = 0; i < bytes; i++) {
    SPI.transfer(p_buf[i]);  //���������� ��������� ����
  }
  CSN_OFF();  //��������� ���� CS
}

//��������� ������� ������ ������
void nrf24_flush_rx(void) {
  uint8_t dt = FLUSH_RX;  //���������� � �������� �������
  CSN_ON();               //��������� ���� CS � �����
  SPI.transfer(dt);       //�������� �������
  _delay_us(1);           //��������
  CSN_OFF();              //��������� ���� CS
}

//��������� ������� ������ ��������
void nrf24_flush_tx(void) {
  uint8_t dt = FLUSH_TX;  //���������� � �������� �������
  CSN_ON();               //��������� ���� CS � �����
  SPI.transfer(dt);       //�������� �������
  _delay_us(1);           //��������
  CSN_OFF();              //��������� ���� CS
}

//��������� ��������� ������ ���������
void nrf24_rx_mode(void) {
  uint8_t regval = 0x00;            //���������� ��� �������� ��������
  regval = nrf24_read_reg(CONFIG);  //��������� �������� �������� ������������
  //�������� ������ � �������� ��� � ����� ��������, ������� ���� PWR_UP � PRIM_RX
  regval |= (1 << PWR_UP) | (1 << PRIM_RX);
  nrf24_write_reg(CONFIG, regval);  //���������� �������� �������� �������
  //����������  ����� �����������
  nrf24_write_buf(TX_ADDR, tx_addr_1, TX_ADR_WIDTH);
  //���������� ����� ���������
  nrf24_write_buf(RX_ADDR_P0, tx_addr_1, TX_ADR_WIDTH);
  CE_SET();        //��������� ���� CE
  _delay_us(150);  //�������� ������� 130 ���
  //������� �������
  nrf24_flush_rx();
  nrf24_flush_tx();
}

//��������� ��������� ������ �����������
void nrf24_tx_mode(void) {
  //���������� ����� �����������
  nrf24_write_buf(TX_ADDR, tx_addr_0, TX_ADR_WIDTH);
  //���������� ����� ���������
  nrf24_write_buf(RX_ADDR_P0, tx_addr_0, TX_ADR_WIDTH);
  CE_RESET();  //�������� ���� CE
  //������� ��� ������
  nrf24_flush_rx();
  nrf24_flush_tx();
}

//��������� �������� ������ � ������
void nrf24_transmit(uint8_t addr, uint8_t *p_buf, uint8_t bytes) {
  CE_RESET();          //�������� ���� CE
  CSN_ON();            //��������� ���� CS � �����
  SPI.transfer(addr);  //���������� �����
  _delay_us(1);        //��������
  //���� �� ������ ���������� ����
  for (uint8_t i = 0; i < bytes; i++) {
    SPI.transfer(p_buf[i]);  //���������� ��������� ����
  }
  CSN_OFF();  //��������� ���� CS
  CE_SET();   //��������� ���� CE
}

//��������� �������� ������ � ����
void nrf24_send(uint8_t *p_buf) {
  noInterrupts();

  uint8_t regval = 0x00;            //���������� ��� �������� � ���������������� �������
  nrf24_tx_mode();                  //�������� ����� ��������
  regval = nrf24_read_reg(CONFIG);  //��������� �������� ����������������� �������
  //���� ������ ���� � ������ �����, �� �������� ���, ������� ��� PWR_UP � �������� PRIM_RX
  regval |= (1 << PWR_UP);
  regval &= ~(1 << PRIM_RX);
  nrf24_write_reg(CONFIG, regval);                     //���������� ����� �������� ����������������� ��������
  _delay_us(150);                                      //�������� ������� 130 ���
  nrf24_transmit(WR_TX_PLOAD, p_buf, TX_PLOAD_WIDTH);  //�������� ������
  CE_SET();                                            //��������� ���� CE
  _delay_us(15);                                       //�������� 10us
  CE_RESET();                                          //�������� ���� CE

  interrupts();
}

//��������� ������������� �����, ������������ � �����������
void nrf24_pins_init(void) {
  pinMode(CE_PIN, OUTPUT);     //CE �� �����
  digitalWrite(CE_PIN, HIGH);  //������� ������� �� CE

  pinMode(CSN_PIN, OUTPUT);     //CSN �� �����
  digitalWrite(CSN_PIN, HIGH);  //������� ������� �� CSN

  pinMode(IRQ_PIN, INPUT);  //IRQ �� ����
}

//��������� ������������� ������
void nrf24_init(void) {
  nrf24_pins_init();  //�������������� ����
  CE_RESET();         //�������� � ����� ����� CE
  _delay_us(5000);    //���� 5 ��
  //���������� ���������������� ����,
  //������������� ��� PWR_UP bit, �������� CRC(1 ����) &Prim_RX:0
  nrf24_write_reg(CONFIG, 0x0a);
  _delay_us(5000);                    //���� 5 ��
  nrf24_write_reg(EN_AA, 0x00);       //��������� �����������������
  nrf24_write_reg(EN_RXADDR, 0x01);   //��������� Pipe0
  nrf24_write_reg(SETUP_AW, 0x01);    //������������� ������ ������ 3 �����
  nrf24_write_reg(SETUP_RETR, 0x00);  //������������� ������ ���� ������������ 1500���, 15 �������
  nrf24_toggle_features();            //���������� �������������� �������
  nrf24_write_reg(FEATURE, 0x07);     //������������� ����������� �������� �������� FEATURE
  nrf24_write_reg(DYNPD, 0);          //��������� ������������ ������ �������� ��������
  nrf24_write_reg(STATUS, 0x70);      //�������� ���� ����������
  nrf24_write_reg(RF_CH, 76);         //������������� ������� 2476 MHz
  //��������� �������� 0dBm, �������� ��������: 1Mbps
  nrf24_write_reg(RF_SETUP, 0x06);                       //��� ��������� -6dBm: 0x04, -12dBm: 0x02, -18dBm: 0x00
  nrf24_write_buf(TX_ADDR, tx_addr_0, TX_ADR_WIDTH);     //������ ������ ��������
  nrf24_write_buf(RX_ADDR_P1, tx_addr_0, TX_ADR_WIDTH);  //������ ������ ������
  nrf24_write_reg(RX_PW_P0, TX_PLOAD_WIDTH);             //������������� ����� ���� �������� ��������
  nrf24_rx_mode();                                       //���� ������ � ����� ��������
}

//��������� ��������� ����������
void irq_callback(void) {
  noInterrupts();

  uint8_t status = 0x01;  //���������� �������
  _delay_us(10);
  status = nrf24_read_reg(STATUS);  //������ �������� �������� �������
  if (status & RX_DR)               //���� ���� ������ �� �����
  {
    nrf24_read_buf(RD_RX_PLOAD, rx_buf, TX_PLOAD_WIDTH);  //������ ������
    nrf24_write_reg(STATUS, 0x40);                        //������ � ������� ������� 1 � ������ ���, ��������� ���������
    f_rx = 1;                                             //��������� ���� ������
  }
  if (status & TX_DS)  //���� ������ ������� ����������
  {
    nrf24_write_reg(STATUS, 0x20);  //������� ��� ���� ����� ������
    nrf24_rx_mode();                //��������� � ����� ������
    f_tx = 1;                       //��������� ���� ��������
  } else if (status & MAX_RT)       //���� ���������� ���������� ������� ��������
  {
    nrf24_write_reg(STATUS, 0x10);  //��������� ���� ��������� �����, ����� 4��
    nrf24_flush_tx();               //������� ������ ��������
    nrf24_rx_mode();                //��������� � ����� ������
  }

  interrupts();
}

//��������� �������� �����������
void blink_led(uint8_t blink_counter) {
  while (blink_counter)  //���� ������� �� ����� 0
  {
    LED_ON();         //�������� ���������
    _delay_ms(50);    //����
    LED_OFF();        //��������� ���������
    _delay_ms(50);    //����
    blink_counter--;  //�������������� �������
  }
}

//��������� ������ �����������
void nrf24l01_receive(void) {
  if (f_rx)  //���� ���� ������ ������ (���� ����������� �� �������� ���������� �� �����������)
  {
    /*�������� �� ��������� ������ �� �������� ����������*/

    //��� ����� ��������� �������� �������� ���������� ��� ���������
    //������ �� ������ �� ������� �� ���������� � ������ ��������� ������ ������ �����,
    //����������� � ������������ ����� �� ������ � �������� �� ���� � ����

    //���� �� ���� ������� �����������
    for (uint8_t i = 0; i < NUM_OF_SLAVES; i++) {  //���� ������ �������� �����
      if (rx_buf[0] == slave_addrs[i]) {           //��������� ������ ��� ������ � ����
        sprintf(str, "����� � ������ 0x%02X\t �������: %d\r\n", rx_buf[0], rx_buf[1]);
        Serial.print(str);  //���������� ������ � ����
      }
    }
    f_rx = 0;  //�������� ���� ������
  }
}

//��������� ������ � ���� ������ �� ��������� �����������
void nrf_info_print(void) {
  uint8_t buf[TX_ADR_WIDTH] = { 0 };  //����� ��� ������ ������� ������
  uint8_t dt_reg = 0;                 //���������� ��� ������ �������� ��������

  dt_reg = nrf24_read_reg(CONFIG);                //������ ������� CONFIG
  sprintf(str, "CONFIG: 0x%02X\r\n", dt_reg);     //
  Serial.print(str);                              //������� ������ � ����
  dt_reg = nrf24_read_reg(EN_AA);                 //������ ������� EN_AA
  sprintf(str, "EN_AA: 0x%02X\r\n", dt_reg);      //
  Serial.print(str);                              //������� ������ � ����
  dt_reg = nrf24_read_reg(EN_RXADDR);             //������ ������� EN_RXADDR
  sprintf(str, "EN_RXADDR: 0x%02X\r\n", dt_reg);  //
  Serial.print(str);                              //������� ������ � ����
  dt_reg = nrf24_read_reg(STATUS);                //������ ������� STATUS
  sprintf(str, "STATUS: 0x%02X\r\n", dt_reg);     //
  Serial.print(str);                              //������� ������ � ����
  dt_reg = nrf24_read_reg(RF_SETUP);              //������ ������� RF_SETUP
  sprintf(str, "RF_SETUP: 0x%02X\r\n", dt_reg);   //
  Serial.print(str);                              //������� ������ � ����
  nrf24_read_buf(TX_ADDR, buf, 3);                //������ ����� TX_ADDR
  sprintf(str, "TX_ADDR: 0x%02X, 0x%02X, 0x%02X\r\n", buf[0], buf[1], buf[2]);
  Serial.print(str);                   //������� ������ � ����
  nrf24_read_buf(RX_ADDR_P1, buf, 3);  //������ ����� RX_ADDR_P1
  sprintf(str, "RX_ADDR: 0x%02X, 0x%02X, 0x%02X\r\n", buf[0], buf[1], buf[2]);
  Serial.print(str);  //������� ������ � ����
}
