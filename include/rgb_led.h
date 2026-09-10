#ifndef RGB_LED_H
#define RGB_LED_H

#include <stdint.h>

#define WS2812_PIN 16 // GPIO пін для RP2040-Zero

extern uint32_t last_color;

void rgb_led_init(void); // инициализация
void rgb_led_set_color(uint8_t r, uint8_t g, uint8_t b);
void rgb_led_set_color_one(uint32_t color);
uint32_t rgb_led_color_pack(uint8_t r, uint8_t g, uint8_t b);

#endif // RGB_LED_H
