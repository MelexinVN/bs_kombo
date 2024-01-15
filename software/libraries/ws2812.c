/*
* ws2812.c
* Библиотека работы с адресными светодиодами ws2812
*/

#include "ws2812.h"		//подключаем заголовочный файл

//Переменные и массивы
uint16_t grb_offset = 0;		//смещение в массиве GRB при выводе светодиодов
uint8_t buffer[BUFFER_LEN];	//буфер 
uint32_t grb[LED_COUNT];		//массив состояния светодиодов

//Функция заполнения буфера
void led_fill_buffer(uint32_t offset, uint32_t length) 
{
	//для всех элементов с указанного на указанную длину
  for (uint32_t i = offset; i < length; i++) 
	{
		//объявляем и расчитываем значение индекса в буфере
    uint32_t grb_i = grb_offset + (i / LED_BPP);
		//если индекс вышел за пределы количества
    if (grb_i >= LED_COUNT) 
		{
      //Сбрасываем все что за пределами (скважность 0%).
      buffer[i] = DUTY_RESET;
    } 
		else 
		{
			uint32_t grb_value = grb[grb_i];										//получаем значение текущего элемента массива
      uint8_t grb_bit_offset = 23 - (i % 24);							//определяем смещение бита
      uint8_t grb_bit = (grb_value >> grb_bit_offset) & 1;//получаем значение бита
			
			//записываем в буфер скважность, соответствующее значению бита
      if (grb_bit == 1) 			
			{
        buffer[i] = DUTY_1;		
      } 
			else 
			{
        buffer[i] = DUTY_0;
      }
    }
  }
}

//Процедура отправки буфера светодиодам
void led_update(void) 
{
  grb_offset = 0;			//обнуляем смещение

  //Заполняем буфер
  led_fill_buffer(BUFFER_L_OFFSET, BUFFER_LEN);
  grb_offset = BUFFER_LED_COUNT;

  // Reset the DMA channel to point at the start of the buffer.
	// Сбрасываем канал DMA, чтобы он указывал на начало буфера.
  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
  LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)buffer);
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, BUFFER_LEN);
  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MODE_CIRCULAR);
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
	// Запускаем счетчик 3
  LL_TIM_EnableCounter(TIM3);
}

//Процедура моргания светодиодами
void blink_leds(void)
{
	//Заполняем массив значениями (белый цвет низкой яркости)
	for(uint8_t i = 0; i < LED_COUNT; i++)
	{
		grb[i] = new_color(10, 10, 10);
	}
	//Отправляем массив светодиодам
	led_update();
	
	LL_mDelay(50);	//Ждем
	
	//Заполняем массив нулевыми значениями (отключаем)
	for(uint8_t i = 0; i < LED_COUNT; i++)
	{
		grb[i] = new_color(0, 0, 0);
	}
	//Отправляем массив светодиодам
	led_update();
}

//Процедура формирования "радуги"
uint32_t led_wheel(uint8_t wheel_pos) 
{
  wheel_pos = 255 - wheel_pos;
  if(wheel_pos < 85) {
    return new_color(255 - wheel_pos*3, 0, wheel_pos*3);
  }
  if(wheel_pos < 170) {
    wheel_pos -= 85;
    return new_color(0, wheel_pos*3, 255 - wheel_pos*3);
  }
  wheel_pos -= 170;
  return new_color(wheel_pos*3, 255 - wheel_pos*3, 0);
}

//Процедура светоэффекта "радуга"
void rainbow(uint32_t effect_length)
{
	uint32_t i;
	static uint16_t step = 0;
	
	step++;	//инкрементируем шаг
	//если шаг достиг условного максимума
	if (step == 256*5)		step = 0;
	//заполняем массив
	for( i = 0; i < LED_COUNT; i++)
	{
		//определяем цвет каждого светодиода с учетом шага и длины эффекта
		uint32_t color = led_wheel(((i*256) / effect_length + step) & 0xFF);
		grb[i] = color;
	}
}

//Процедура светоэффекта "бегущий зеленый"
void green_run(void)
{
	static uint16_t step = 0;
	uint8_t green = 0;
	//инкрементируем шаг
	step++;
	//обнуляем шаг, если он больше количества светодиодов
	if (step == LED_COUNT) step = 0;		
	//заполняем массив
	for (uint8_t i = 0; i < LED_COUNT; i++)
	{
		//если текущий индекс светодиода совпадает с шагом,
		//включаем его на максимум
		if (i == step)  green = 255;
		//если нет, то делаем его тусклее, и чем дальше он от яркого, тем тусклее
		else green = 255/(abs(step-i));
		//записываем значение в массив
		grb[i] = green << 16;
	}
}

/*
void green_run(void)
{
	static uint16_t x = 0;
	uint8_t green = 0;
	
	x++;
	if (x == LED_COUNT*50) x = 0;		
	
	for(uint16_t i = 0; i < LED_COUNT*50; i++)
	{
		green = grb[i/50]>>16 & 0xFF;
		
		if (i == x)  green = 255;
		else 
		{
			green = 0;
			//if (abs(x-i)/50 >= 255)  green = 0;
			//green = 255 - (abs(i-x)/50);
		}
		grb[i/50] = green << 16;
	}
}
*/
//Процедура светоэффекта "бегущий красный"
void red_run(void)
{
	static uint16_t step = 0;
	uint8_t red = 0;
	//алгоритм аналогичен бегущему зеленому
	step++;
	if (step == LED_COUNT) step = 0;		
	
	for (uint8_t i = 0; i < LED_COUNT; i++)
	{
		if (i == step)  red = 255;
		else red = 255/(5*abs(step-i));

		grb[i] = red << 8;
	}
}

//Процедура светоэффекта "бегущий синий"
void blue_run(void)
{
	static uint16_t step = 0;
	uint8_t blue = 0;
	//алгоритм аналогичен бегущему зеленому
	step++;
	if(step == LED_COUNT) step = 0;		
	
	for(uint8_t i = 0; i < LED_COUNT; i++)
	{
		blue = grb[i] & 0xFF;
		
		if (i == step)  blue = 255;
		else blue = 255/(5*abs(step-i));

		grb[i] = blue;
		
	}
}

//Процедура светоэффекта "бегущие красный и синий"
void red_blue_run(void)
{
	static uint16_t step_1 = 0;
	static uint16_t step_2 = 0;
	uint8_t blue = 0;
	uint8_t red = 0;
	//инкрементируем шаг
	step_1++;
	//обнуляем шаг, если он больше количества светодиодов
	if(step_1 == LED_COUNT) step_1 = 0;		
	
	//вычисляем значение второго шага, для круга он будет диаметрально противоположным
	if (step_1 < LED_COUNT/2) step_2 = step_1 + LED_COUNT/2;
	else if (step_1 == LED_COUNT/2) step_2 = LED_COUNT/2;
	else if (step_1 > LED_COUNT/2) step_2 = step_1 - LED_COUNT/2;
	//заполняем массив
	for(uint8_t i = 0; i < LED_COUNT; i++)
	{
		//устанавливаем значения цветов по аналогии с простым бегущим цветом
		if (i == step_1)  blue = 255;
		else blue = 255/(5*abs(step_1-i));
		if (i == step_2) red = 255;
		else red = 255/(5*abs(step_2-i));
		//записываем значение в массив
		grb[i] = (red << 8) | blue;
		
	}
}

//Процедура светоэффекта "бегущие красный и зеленый"
void red_green_run(void)
{
	static uint16_t step_1 = 0;
	static uint16_t step_2 = 0;
	uint8_t green = 0;
	uint8_t red = 0;
	//алгоритм аналогичен бегущим красному и синему
	step_1++;
	if(step_1 == LED_COUNT) step_1 = 0;		
	
	if (step_1 < LED_COUNT/2) step_2 = step_1 + LED_COUNT/2;
	else if (step_1 == LED_COUNT/2) step_2 = LED_COUNT/2;
	else if (step_1 > LED_COUNT/2) step_2 = step_1 - LED_COUNT/2;
	
	for(uint8_t i = 0; i < LED_COUNT; i++)
	{
		if (i == step_1)  green = 255;
		else green = 255/(5*abs(step_1-i));
		
		if (i == step_2) red = 255;
		else red = 255/(5*abs(step_2-i));

		grb[i] = (green << 16) | (red << 8);
	}
}

//Процедура светоэффекта "бегущие синий и зеленый"
void blue_green_run(void)
{
	static uint16_t step_1 = 0;
	static uint16_t step_2 = 0;
	uint8_t green = 0;
	uint8_t blue = 0;
	//алгоритм аналогичен бегущим красному и синему
	step_1++;
	if(step_1 == LED_COUNT) step_1 = 0;		
	
	if (step_1 < LED_COUNT/2) step_2 = step_1 + LED_COUNT/2;
	else if (step_1 == LED_COUNT/2) step_2 = LED_COUNT/2;
	else if (step_1 > LED_COUNT/2) step_2 = step_1 - LED_COUNT/2;
	
	for(uint8_t i = 0; i < LED_COUNT; i++)
	{
		if (i == step_1)  green = 255;
		else green = 255/(5*abs(step_1-i));
		
		if (i == step_2) blue = 255;
		else blue = 255/(5*abs(step_2-i));

		grb[i] = (green << 16) | blue;
	}
}

//Процедура светоэффекта "случайные цвета"
void random_noise(void) 
{
  uint8_t noise_r, noise_g, noise_b;
	//заполняем массив 
	for(uint8_t i = 0; i < LED_COUNT; i++) 
	{
		//получаем случайное значение для каждого из цветов
		noise_r = rand() % 255;
		noise_g = rand() % 255;
		noise_b = rand() % 255;
		//записываем значение в массив
		grb[i] = new_color(noise_r, noise_g, noise_b);
	}
}
