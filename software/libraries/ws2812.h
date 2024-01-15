/*
* ws2812.c
* Заголовочный файл библиотеки для работы с адресными светодиодами ws2812
* здесь можно сконфигурировать:
* - число светодиодов,
* - параметры буферизации, если необходимо.
*/

#ifndef WS2812_H_
#define WS2812_H_

#include "main.h"		//добавляем заголовочный файл основной программы

//Число светодиодов
#define LED_COUNT 16

//Число режимов
#define MODES_COUNT 8

//Количество байт на светодиод
#define LED_BPP 24

//Настройки буферизации
#define BUFFER_LED_COUNT 16														//число светодиодов для буферизации
#define BUFFER_LED_COUNT_HALF (BUFFER_LED_COUNT / 2)	//
#define BUFFER_LEN (BUFFER_LED_COUNT * 24)						//длина буфера
#define BUFFER_LEN_HALF (BUFFER_LEN / 2)							//
#define BUFFER_L_OFFSET 0
#define BUFFER_H_OFFSET BUFFER_LEN_HALF

//Тайминги ШИМ
#define TIMER_AAR 59
#define DUTY_0 18 // (32% of 59)
#define DUTY_1 38 // (62% of 59)
#define DUTY_RESET 0

//Вспомогательный макрос для преобразование RGB в 32-х разрядное число GRB
#define new_color(r, g, b) (((uint32_t)(r) << 16) | ((uint32_t)(g) <<  8) | (b))

//Процедура отправки буфера светодиодам
void led_update(void);
//Процедура моргания светодиодами
void blink_leds(void);
//Процедура светоэффекта "радуга"
void rainbow(uint32_t effect_length);
//Процедура светоэффекта "бегущий зеленый"
void green_run(void);
//Процедура светоэффекта "бегущий красный"
void red_run(void);
//Процедура светоэффекта "бегущий синий"
void blue_run(void);
//Процедура светоэффекта "бегущие красный и синий"
void red_blue_run(void);
//Процедура светоэффекта "бегущие красный и зеленый"
void red_green_run(void);
//Процедура светоэффекта "бегущие синий и зеленый"
void blue_green_run(void);
//Процедура светоэффекта "случайные цвета"
void random_noise(void);

#endif /* WS2812_H_ */