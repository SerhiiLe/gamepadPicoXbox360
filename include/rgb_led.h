#ifndef RGB_LED_H
#define RGB_LED_H

#include <stdint.h>

extern uint32_t last_color;

void rgb_led_init(void); // инициализация
void rgb_led_set_color(uint8_t r, uint8_t g, uint8_t b);
void rgb_led_set_color_one(uint32_t color);
uint32_t rgb_led_color_pack(uint8_t r, uint8_t g, uint8_t b);
uint32_t apply_brightness(uint32_t base, uint8_t brightness);
uint32_t apply_brightness_pack(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);

#endif // RGB_LED_H
