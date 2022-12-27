/*
* main.c
* Тестовый пример ведущего устройства с модулем NRF24L01. 
* Проект "КомБО" (Открытые системы беспроводной коммуникации)
*
* Подключение периферии:
* NRF24L01		Arduino nano/UNO    Arduino Mega 
* CE					D9                  D9
* CSN					D10                 D10
* SCK					D13                 D52
* MOSI				D12                 D51
* MISO				D11                 D50
* IRQ					D2                  D2
*
*   Краткое описание работы:
* При включении питания и сбросе в порт выдаются значения регистров радиомодуля
* В процессе работы устройство по кругу отправляет запросы на адреса, содержаниеся в массиве
* при получении ответа на запрос, в порт выдается адрес, с которого получен ответ 
* и байт данных, полученных от него.
* 
* Автор: Мелехин В.Н. (MelexinVN)
*/


#include "main.h"  //подключаем основной заголовочный файл

//объявляем переменные и массивы
char str[STRING_SIZE];                //буфер для вывода в порт
uint8_t slave_counter = 0;            //счетчик ведомых
uint8_t slave_addrs[] = ADRESS_LIST;  //массив адресов ведомых
//внешние переменные и массивы
extern volatile uint8_t f_rx, f_tx;     //флаги приема и передачи
extern uint8_t tx_buf[TX_PLOAD_WIDTH];  //буфер передачи
extern uint8_t rx_buf[TX_PLOAD_WIDTH];  //буфер приема


#ifdef MASTER                                            //если ведущее устройство
uint8_t tx_addr_0[TX_ADR_WIDTH] = { 0xa1, 0xa2, 0xa3 };  //адрес 0
uint8_t tx_addr_1[TX_ADR_WIDTH] = { 0xa3, 0xa2, 0xa1 };  //адрес 1
#endif

#ifdef SLAVE                                             //если ведомое устройство
uint8_t tx_addr_0[TX_ADR_WIDTH] = { 0xa3, 0xa2, 0xa1 };  //адрес 0
uint8_t tx_addr_1[TX_ADR_WIDTH] = { 0xa1, 0xa2, 0xa3 };  //адрес 1
#endif

uint8_t rx_buf[TX_PLOAD_WIDTH] = { 0 };  //буфер приема
uint8_t tx_buf[TX_PLOAD_WIDTH] = { 0 };  //буфер передачи

volatile uint8_t f_rx = 0, f_tx = 0;  //флаги приема и передачи

//Начало программы
void setup() {
  interrupt_init();  //инициализируем прерывания
  gpio_init();       //инициализируем порты ввода-вывода
  SPI.begin();
  nrf24_init();             //инициализируем радиомодуль
  Serial.begin(9600);       //инициализируем USART 9600 бод
  Serial.println("start");  //отправка стартовой строки в порт
  nrf_info_print();         //выводим значения регистров в порт
  blink_led(5);             //моргаем светодиодом
}

//Основной цикл
void loop() {
  //отправляем запрос по очередному адресу
  tx_buf[0] = slave_addrs[slave_counter];  //записываем в буфер очередной адрес

  nrf24_send(tx_buf);  //отправляем посылку в эфир
  //запрос содержит только адрес устройства, но может содержать другую информацию размером до 32 байт

  slave_counter++;                     //переходим к следующему адресу
  if (slave_counter == NUM_OF_SLAVES)  //если дошли до последнего элемента
  {
    slave_counter = 0;  //обнуляем счетчик
  }

  _delay_ms(20);  //задержка

  //задержка необходима, чтобы ведомое устройство успело получить запрос, отправить ответ,
  //а данное ведущее устройство успело получить и обработать отет на запрос, прежде чем
  //переходить к отправке следующего

  //для достижения максимального быстродействия величину задержки необходимо подбирать наименьшей
  //экспериментально с учетом времени, затрачиваемого на остальные действия микроконтроллера

  nrf24l01_receive();  //обрабатываем прием радиомодуля
}


//Процедуры и функции:
//Процедура инициализации портов
void gpio_init(void) {

  pinMode(LED_BUILTIN, OUTPUT);
}

//Процедура инициализации прерываний
void interrupt_init(void) {
  //включаем внешнее прерывание по спаду
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), irq_callback, FALLING);
}

//Функция чтения регистра модуля
uint8_t nrf24_read_reg(uint8_t addr) {
  uint8_t dt = 0, cmd;      //переменные данных и команды
  CSN_ON();                 //прижимаем ногу CS к земле
  dt = SPI.transfer(addr);  //отправка адреса регистра, прием

  //если адрес равен адресу регистра статуса то и возварщаем его состояние
  if (addr != STATUS)  //а если не равен
  {
    cmd = 0xFF;              //команда NOP для получения данных
    dt = SPI.transfer(cmd);  //
  }
  CSN_OFF();  //поднимаем ногу CS
  return dt;  //возвращаемое значение
}

//Процедура записи регистра в модуль
void nrf24_write_reg(uint8_t addr, uint8_t dt) {
  addr |= W_REGISTER;  //включаем бит записи в адрес
  CSN_ON();            //прижимаем ногу CS к земле
  SPI.transfer(addr);  //отправляем адрес
  SPI.transfer(dt);    //отправляем значение
  CSN_OFF();           //поднимаем ногу CS
}

//Процедура активации дополнительных команд
void nrf24_toggle_features(void) {
  uint8_t dt = ACTIVATE;  //переменная с командой активации
  CSN_ON();               //прижимаем ногу CS к земле
  SPI.transfer(dt);       //отправляем команду
  _delay_us(1);           //задержка
  dt = 0x73;              //следующая команда
  SPI.transfer(dt);       //отправляем команду
  CSN_OFF();              //поднимаем ногу CS
}

//Процедура чтения буфера
void nrf24_read_buf(uint8_t addr, uint8_t *p_buf, uint8_t bytes) {
  CSN_ON();            //прижимаем ногу CS к земле
  SPI.transfer(addr);  //отправляем адрес
  //цикл на нужное количество байт
  for (uint8_t i = 0; i < bytes; i++) {
    p_buf[i] = SPI.transfer(addr);  //получаем очередной байт
  }
  CSN_OFF();  //поднимаем ногу CS
}

//Процедура записи буфера
void nrf24_write_buf(uint8_t addr, uint8_t *p_buf, uint8_t bytes) {
  addr |= W_REGISTER;  //включаем бит записи в адрес
  CSN_ON();            //прижимаем ногу CS к земле
  SPI.transfer(addr);  //отправляем адрес
  _delay_us(1);        //задержка
  //цикл на нужное количество байт
  for (uint8_t i = 0; i < bytes; i++) {
    SPI.transfer(p_buf[i]);  //отправляем очередной байт
  }
  CSN_OFF();  //поднимаем ногу CS
}

//Процедура очистки буфера приема
void nrf24_flush_rx(void) {
  uint8_t dt = FLUSH_RX;  //переменная с командой очистки
  CSN_ON();               //прижимаем ногу CS к земле
  SPI.transfer(dt);       //отправка команды
  _delay_us(1);           //задержка
  CSN_OFF();              //поднимаем ногу CS
}

//Процедура очистки буфера передачи
void nrf24_flush_tx(void) {
  uint8_t dt = FLUSH_TX;  //переменная с командой очистки
  CSN_ON();               //прижимаем ногу CS к земле
  SPI.transfer(dt);       //отправка команды
  _delay_us(1);           //задержка
  CSN_OFF();              //поднимаем ногу CS
}

//Процедура включение режима приемника
void nrf24_rx_mode(void) {
  uint8_t regval = 0x00;            //переменная для значения регистра
  regval = nrf24_read_reg(CONFIG);  //сохраняем значение регистра конфигурации
  //разбудим модуль и переведём его в режим приёмника, включив биты PWR_UP и PRIM_RX
  regval |= (1 << PWR_UP) | (1 << PRIM_RX);
  nrf24_write_reg(CONFIG, regval);  //возвращаем значение регистра статуса
  //записываем  адрес передатчика
  nrf24_write_buf(TX_ADDR, tx_addr_1, TX_ADR_WIDTH);
  //записываем адрес приемника
  nrf24_write_buf(RX_ADDR_P0, tx_addr_1, TX_ADR_WIDTH);
  CE_SET();        //поднимаем ногу CE
  _delay_us(150);  //задержка минимум 130 мкс
  //очистка буферов
  nrf24_flush_rx();
  nrf24_flush_tx();
}

//Процедура включения режима передатчика
void nrf24_tx_mode(void) {
  //записываем адрес передатчика
  nrf24_write_buf(TX_ADDR, tx_addr_0, TX_ADR_WIDTH);
  //записываем адрес приемника
  nrf24_write_buf(RX_ADDR_P0, tx_addr_0, TX_ADR_WIDTH);
  CE_RESET();  //опускаем ногу CE
  //очищаем оба буфера
  nrf24_flush_rx();
  nrf24_flush_tx();
}

//Процедура передачи данных в модуль
void nrf24_transmit(uint8_t addr, uint8_t *p_buf, uint8_t bytes) {
  CE_RESET();          //опускаем ногу CE
  CSN_ON();            //прижимаем ногу CS к земле
  SPI.transfer(addr);  //отправляем адрес
  _delay_us(1);        //задержка
  //цикл на нужное количество байт
  for (uint8_t i = 0; i < bytes; i++) {
    SPI.transfer(p_buf[i]);  //отправляем очередной байт
  }
  CSN_OFF();  //поднимаем ногу CS
  CE_SET();   //Поднимаем ногу CE
}

//Процедура отправки данных в эфир
void nrf24_send(uint8_t *p_buf) {
  noInterrupts();

  uint8_t regval = 0x00;            //переменная для отправки в конфигурационный регистр
  nrf24_tx_mode();                  //включаем режим передачи
  regval = nrf24_read_reg(CONFIG);  //сохраняем значения конфигурационного региста
  //если модуль ушел в спящий режим, то разбудим его, включив бит PWR_UP и выключив PRIM_RX
  regval |= (1 << PWR_UP);
  regval &= ~(1 << PRIM_RX);
  nrf24_write_reg(CONFIG, regval);                     //записываем новое значение конфигурационного регистра
  _delay_us(150);                                      //задержка минимум 130 мкс
  nrf24_transmit(WR_TX_PLOAD, p_buf, TX_PLOAD_WIDTH);  //отправка данных
  CE_SET();                                            //поднимаем ногу CE
  _delay_us(15);                                       //задержка 10us
  CE_RESET();                                          //опускаем ногу CE

  interrupts();
}

//Процедура инициализации пинов, подключенных к радиомодулю
void nrf24_pins_init(void) {
  pinMode(CE_PIN, OUTPUT);     //CE на выход
  digitalWrite(CE_PIN, HIGH);  //высокий уровень на CE

  pinMode(CSN_PIN, OUTPUT);     //CSN на выход
  digitalWrite(CSN_PIN, HIGH);  //высокий уровень на CSN

  pinMode(IRQ_PIN, INPUT);  //IRQ на вход
}

//Процедура инициализации модуля
void nrf24_init(void) {
  nrf24_pins_init();  //инициализируем пины
  CE_RESET();         //опускаем к земле вывод CE
  _delay_us(5000);    //ждем 5 мс
  //записываем конфигурационный байт,
  //устанавливаем бит PWR_UP bit, включаем CRC(1 байт) &Prim_RX:0
  nrf24_write_reg(CONFIG, 0x0a);
  _delay_us(5000);                    //ждем 5 мс
  nrf24_write_reg(EN_AA, 0x00);       //отключаем автоподтверждение
  nrf24_write_reg(EN_RXADDR, 0x01);   //разрешаем Pipe0
  nrf24_write_reg(SETUP_AW, 0x01);    //устанавливаем размер адреса 3 байта
  nrf24_write_reg(SETUP_RETR, 0x00);  //устанавливаем период авто ретрансляции 1500мкс, 15 попыток
  nrf24_toggle_features();            //активируем дополнительные команды
  nrf24_write_reg(FEATURE, 0x07);     //устанавливаем стандартные значения регистра FEATURE
  nrf24_write_reg(DYNPD, 0);          //отключаем динамический размер полезной нагрузки
  nrf24_write_reg(STATUS, 0x70);      //опускаем флаг прерывания
  nrf24_write_reg(RF_CH, 76);         //устанавливаем частоту 2476 MHz
  //Выходноая мощность 0dBm, Скорость передачи: 1Mbps
  nrf24_write_reg(RF_SETUP, 0x06);                       //для установки -6dBm: 0x04, -12dBm: 0x02, -18dBm: 0x00
  nrf24_write_buf(TX_ADDR, tx_addr_0, TX_ADR_WIDTH);     //запись адреса передачи
  nrf24_write_buf(RX_ADDR_P1, tx_addr_0, TX_ADR_WIDTH);  //запись адреса приема
  nrf24_write_reg(RX_PW_P0, TX_PLOAD_WIDTH);             //устанавливаем число байт полезной нагрузки
  nrf24_rx_mode();                                       //пока уходим в режим приёмника
}

//Процедура обработки прерывания
void irq_callback(void) {
  noInterrupts();

  uint8_t status = 0x01;  //переменная статуса
  _delay_us(10);
  status = nrf24_read_reg(STATUS);  //читаем значения регистра статуса
  if (status & RX_DR)               //если есть данные на прием
  {
    nrf24_read_buf(RD_RX_PLOAD, rx_buf, TX_PLOAD_WIDTH);  //чтение буфера
    nrf24_write_reg(STATUS, 0x40);                        //запись в регистр статуса 1 в шестой бит, обнуление остальных
    f_rx = 1;                                             //поднимаем флаг приема
  }
  if (status & TX_DS)  //если данные успешно отправлены
  {
    nrf24_write_reg(STATUS, 0x20);  //очищаем все биты кроме пятого
    nrf24_rx_mode();                //переходим в режим приема
    f_tx = 1;                       //поднимаем флаг передачи
  } else if (status & MAX_RT)       //если превышение количества попыток отправки
  {
    nrf24_write_reg(STATUS, 0x10);  //однуление всех остальных битов, кроме 4го
    nrf24_flush_tx();               //очистка буфера отправки
    nrf24_rx_mode();                //переходим в режим приема
  }

  interrupts();
}

//Процедура моргания светодиодом
void blink_led(uint8_t blink_counter) {
  while (blink_counter)  //пока счетчик не равен 0
  {
    LED_ON();         //включаем светодиод
    _delay_ms(50);    //ждем
    LED_OFF();        //выключаем светодиод
    _delay_ms(50);    //ждем
    blink_counter--;  //декрементируем счетчик
  }
}

//Процедура приема радиомодуля
void nrf24l01_receive(void) {
  if (f_rx)  //если флаг приема поднят (флаг поднимается по внешнему прерыванию от радиомодуля)
  {
    /*ДЕЙСТВИЯ ПО ПОЛУЧЕНИЮ ОТВЕТА ОТ ВЕДОМОГО УСТРОЙСТВА*/

    //тут можно прописать действия ведущего устройства при получении
    //ответа от одного из ведомых мы определяем с какого конкретно адреса пришел ответ,
    //формируется и отправляется ответ на запрос и сообщаем об этом в порт

    //цикл по всем ведомым устройствам
    for (uint8_t i = 0; i < NUM_OF_SLAVES; i++) {  //если найден принятый адрес
      if (rx_buf[0] == slave_addrs[i]) {           //формируем строку для вывода в порт
        sprintf(str, "Ответ с адреса 0x%02X\t счетчик: %d\r\n", rx_buf[0], rx_buf[1]);
        Serial.print(str);  //отправляем строку в порт
      }
    }
    f_rx = 0;  //опускаем флаг приема
  }
}

//Процедура вывода в порт данных из регистров радиомодуля
void nrf_info_print(void) {
  uint8_t buf[TX_ADR_WIDTH] = { 0 };  //буфер для чтения адресов модуля
  uint8_t dt_reg = 0;                 //переменная для чтения значения регистра

  dt_reg = nrf24_read_reg(CONFIG);                //читаем регистр CONFIG
  sprintf(str, "CONFIG: 0x%02X\r\n", dt_reg);     //
  Serial.print(str);                              //выводим данные в порт
  dt_reg = nrf24_read_reg(EN_AA);                 //читаем регистр EN_AA
  sprintf(str, "EN_AA: 0x%02X\r\n", dt_reg);      //
  Serial.print(str);                              //выводим данные в порт
  dt_reg = nrf24_read_reg(EN_RXADDR);             //читаем регистр EN_RXADDR
  sprintf(str, "EN_RXADDR: 0x%02X\r\n", dt_reg);  //
  Serial.print(str);                              //выводим данные в порт
  dt_reg = nrf24_read_reg(STATUS);                //читаем регистр STATUS
  sprintf(str, "STATUS: 0x%02X\r\n", dt_reg);     //
  Serial.print(str);                              //выводим данные в порт
  dt_reg = nrf24_read_reg(RF_SETUP);              //читаем регистр RF_SETUP
  sprintf(str, "RF_SETUP: 0x%02X\r\n", dt_reg);   //
  Serial.print(str);                              //выводим данные в порт
  nrf24_read_buf(TX_ADDR, buf, 3);                //читаем буфер TX_ADDR
  sprintf(str, "TX_ADDR: 0x%02X, 0x%02X, 0x%02X\r\n", buf[0], buf[1], buf[2]);
  Serial.print(str);                   //выводим данные в порт
  nrf24_read_buf(RX_ADDR_P1, buf, 3);  //читаем буфер RX_ADDR_P1
  sprintf(str, "RX_ADDR: 0x%02X, 0x%02X, 0x%02X\r\n", buf[0], buf[1], buf[2]);
  Serial.print(str);  //выводим данные в порт
}
