/* USER CODE BEGIN Header */
/*
* main.c
* тестовый пример ведомого устройства с модулем NRF24L01.
* Проект "КомБО" (Открытые системы беспроводной коммуникации)
*
* Микроконтроллеры: stm32f030f4p6
*
* Подключение периферии:
* NRF24L01		STM32
* CE					PA3(9)					
* CSN					PA4(10)					
* SCK					PA5(11)					
* MOSI				PA7(13)					
* MISO				PA6(12)					
* IRQ					PA2(8)					
*
* USART
* TX					PA9(17)					
* RX					PA10(18)					
*
* LED					PB1(14)					
*
*   Краткое описание работы:
* При включении питания и сбросе в порт выдаются значения регистров радиомодуля
* В процессе работы устройство по кругу отправляет запросы на адреса, содержаниеся в массиве
* при получении ответа на запрос, в порт выдается адрес, с которого получен ответ 
* и байт данных, полученных от него.
*
* Примечания:
* - Проект сгенерирован в STM32CubeMX, основные настройки и инициализация периферии происходит автоматически
*   по настройкам проекта
* - Для МК stm32f030f4p6 - без перемычки на BOOT (1) - старт в режиме загрузчика, можно подключиться 
*   по USART и через SWD, с перемычкой - нормальная загрузка, можно подключиться только через SWD
*
* Автор: Мелехин В.Н. (MelexinVN)
*/
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
//объявляем переменные и массивы
char str[STRING_SIZE] = {0};						//буфер для вывода в порт
uint8_t slave_counter = 0;							//счетчик ведомых
uint8_t slave_addrs[] = ADRESS_LIST;		//массив адресов ведомых
//внешние переменные и массивы
extern volatile uint8_t f_rx, f_tx;			//флаги приема и передачи
extern uint8_t tx_buf[TX_PLOAD_WIDTH];	//буфер передачи
extern uint8_t rx_buf[TX_PLOAD_WIDTH];	//буфер приема
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_IWDG_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//Процедура вывода в порт данных из регистров радиомодуля
void nrf_info_print(void)
{
	uint8_t buf[TX_ADR_WIDTH] = {0};				//буфер для чтения адресов модуля
	uint8_t dt_reg = 0;											//переменная для чтения значения регистра
	
	dt_reg = nrf24_read_reg(CONFIG);							//читаем регистр CONFIG
	sprintf(str, "CONFIG: 0x%02X\r\n", dt_reg);		//
	usart_print(str);															//выводим данные в порт
	dt_reg = nrf24_read_reg(EN_AA);								//читаем регистр EN_AA
	sprintf(str, "EN_AA: 0x%02X\r\n", dt_reg);		//
	usart_print(str);															//выводим данные в порт
	dt_reg = nrf24_read_reg(EN_RXADDR);						//читаем регистр EN_RXADDR
	sprintf(str, "EN_RXADDR: 0x%02X\r\n", dt_reg);//
	usart_print(str);															//выводим данные в порт
	dt_reg = nrf24_read_reg(STATUS);							//читаем регистр STATUS
	sprintf(str, "STATUS: 0x%02X\r\n", dt_reg);		//
	usart_print(str);															//выводим данные в порт
	dt_reg = nrf24_read_reg(RF_SETUP);						//читаем регистр RF_SETUP
	sprintf(str, "RF_SETUP: 0x%02X\r\n", dt_reg);	//
	usart_print(str);															//выводим данные в порт
	nrf24_read_buf(TX_ADDR,buf,3);								//читаем буфер TX_ADDR
	sprintf(str, "TX_ADDR: 0x%02X, 0x%02X, 0x%02X\r\n", buf[0], buf[1], buf[2]);
	usart_print(str);															//выводим данные в порт
	nrf24_read_buf(RX_ADDR_P1,buf,3);							//читаем буфер RX_ADDR_P1
	sprintf(str, "RX_ADDR: 0x%02X, 0x%02X, 0x%02X\r\n", buf[0], buf[1], buf[2]);
	usart_print(str);															//выводим данные в порт
}

//Процедура моргания светодиодом
void blink_led(uint8_t blink_counter)
{
	while (blink_counter)		//пока счетчик не равен 0
	{
		LED_ON();							//включаем светодиод
		LL_mDelay(50);				//ждем
		LED_OFF();						//выключаем светодиод
		LL_mDelay(50);				//ждем
		blink_counter--;			//декрементируем счетчик
	}
}

//Процедура приема радиомодуля
void nrf24l01_receive(void)
{
	if(f_rx)	//если флаг приема поднят (флаг поднимается по внешнему прерыванию от радиомодуля)
	{
		/*ДЕЙСТВИЯ ПО ПОЛУЧЕНИЮ ОТВЕТА ОТ ВЕДОМОГО УСТРОЙСТВА*/
		
		//тут можно прописать действия ведущего устройства при получении
		//ответа от одного из ведомых мы определяем с какого конкретно адреса пришел ответ,
		//формируется и отправляется ответ на запрос и сообщаем об этом в порт
		
		//цикл по всем ведомым устройствам
		for (uint8_t i = 0; i < NUM_OF_SLAVES; i++)
		{	//если найден принятый адрес
			if (rx_buf[0] == slave_addrs[i]) 	
			{//формируем строку для вывода в порт
				sprintf(str,"Adress 0x%02X\t data: %d\r\n",rx_buf[0],rx_buf[1]);			
				usart_print(str);	//отправляем строку в порт
			}
		}
		f_rx = 0;					//опускаем флаг приема
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */

  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* System interrupt init*/
  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, 3);

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_IWDG_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

	//Включаем SPI
	LL_SPI_Enable(SPI1);
	//Включаем внешнее прерывание
	LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_2);
	LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_2);

  usart_init(0);					//инициализируем USART
	usart_println("start");	//отправляем стартовую строку в порт
	nrf24_init();						//инициализируем радиомодуль
	nrf_info_print();
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		//отправляем запрос по очередному адресу
		tx_buf[0] = slave_addrs[slave_counter];	//записываем в буфер очередной адрес
		
		nrf24_send(tx_buf);						//отправляем посылку в эфир
		//запрос содержит только адрес устройства, но может содержать другую информацию размером до 32 байт
		
		slave_counter++;						//переходим к следующему адресу
		if (slave_counter == NUM_OF_SLAVES)		//если дошли до последнего элемента
		{
			slave_counter = 0;					//обнуляем счетчик
		}
		
		LL_mDelay(2);							//задержка
		
		//задержка необходима, чтобы ведомое устройство успело получить запрос, отправить ответ, 
		//а данное ведущее устройство успело получить и обработать отет на запрос, прежде чем 
		//переходить к отправке следующего
		
		//для достижения максимального быстродействия величину задержки необходимо подбирать наименьшей
		//экспериментально с учетом времени, затрачиваемого на остальные действия микроконтроллера
		
		nrf24l01_receive();		//обрабатываем прием радиомодуля

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		LL_IWDG_ReloadCounter(IWDG);//сбрасываем сторожевой таймер
		
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
  while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1)
  {
  }
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_LSI_Enable();

   /* Wait till LSI is ready */
  while(LL_RCC_LSI_IsReady() != 1)
  {

  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI_DIV_2, LL_RCC_PLL_MUL_12);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_Init1msTick(48000000);
  LL_SetSystemCoreClock(48000000);
  LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);
}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  LL_IWDG_Enable(IWDG);
  LL_IWDG_EnableWriteAccess(IWDG);
  LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_32);
  LL_IWDG_SetReloadCounter(IWDG, 4095);
  while (LL_IWDG_IsReady(IWDG) != 1)
  {
  }

  LL_IWDG_ReloadCounter(IWDG);
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  LL_SPI_InitTypeDef SPI_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SPI1);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**SPI1 GPIO Configuration
  PA5   ------> SPI1_SCK
  PA6   ------> SPI1_MISO
  PA7   ------> SPI1_MOSI
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV16;
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
  SPI_InitStruct.CRCPoly = 7;
  LL_SPI_Init(SPI1, &SPI_InitStruct);
  LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);
  LL_SPI_EnableNSSPulseMgt(SPI1);
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM1);

  /* TIM1 interrupt Init */
  NVIC_SetPriority(TIM1_CC_IRQn, 0);
  NVIC_EnableIRQ(TIM1_CC_IRQn);

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  TIM_InitStruct.Prescaler = 0;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 48;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  TIM_InitStruct.RepetitionCounter = 0;
  LL_TIM_Init(TIM1, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM1);
  LL_TIM_SetClockSource(TIM1, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM1);
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**USART1 GPIO Configuration
  PA9   ------> USART1_TX
  PA10   ------> USART1_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  USART_InitStruct.BaudRate = 9600;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);
  LL_USART_DisableIT_CTS(USART1);
  LL_USART_ConfigAsyncMode(USART1);
  LL_USART_Enable(USART1);
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

  /**/
  LL_GPIO_ResetOutputPin(CE_GPIO_Port, CE_Pin);

  /**/
  LL_GPIO_ResetOutputPin(CSN_GPIO_Port, CSN_Pin);

  /**/
  LL_GPIO_ResetOutputPin(LED_GPIO_Port, LED_Pin);

  /**/
  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE2);

  /**/
  LL_GPIO_SetPinPull(IRQ_GPIO_Port, IRQ_Pin, LL_GPIO_PULL_NO);

  /**/
  LL_GPIO_SetPinMode(IRQ_GPIO_Port, IRQ_Pin, LL_GPIO_MODE_INPUT);

  /**/
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_2;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
  LL_EXTI_Init(&EXTI_InitStruct);

  /**/
  GPIO_InitStruct.Pin = CE_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(CE_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = CSN_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(CSN_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  NVIC_SetPriority(EXTI2_3_IRQn, 0);
  NVIC_EnableIRQ(EXTI2_3_IRQn);

}

/* USER CODE BEGIN 4 */
void TIM1_Callback(void)
{
	if(LL_TIM_IsActiveFlag_UPDATE(TIM1))			//если флаг прерывания таймера поднят
	{
		LL_TIM_ClearFlag_UPDATE(TIM1);					//опускаем флаг прерывания
	}
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

