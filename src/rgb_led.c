// Драйвер RGB WS2812

#include <hardware/pio.h>
#include "config.h"
#include "rgb_led.h"
#include "ws2812.pio.h" // Цей заголовок згенерується автоматично з нашого .pio файлу!


static PIO pio_block = pio0;
static uint statemachine_num = 0;
uint32_t last_color = 0;

// инициализация светодиода ws2812
void rgb_led_init(void) {
    // Знаходимо вільне місце в пам'яті PIO та завантажуємо туди програму
    uint offset = pio_add_program(pio_block, &ws2812_program);
    
    // Ініціалізуємо PIO-автомат на частоті 800 кГц (стандарт для WS2812)
    ws2812_program_init(pio_block, statemachine_num, offset, PIN_WS2812_LED, 800000, false);
}

// активация светодиода в формате 0xRRGGBB
void rgb_led_set_color_one(uint32_t color) {
    // Протокол WS2812 очікує дані у форматі GRB (Green-Red-Blue)
    last_color = color;
    
    // Зміна порядка бітів з RGB на GRB та вирівнювання вліво
    uint32_t grb_color = (color & 0xff0000) | ((color & 0xff00) << 16) | ((color & 0xff) << 8);
    // Записуємо 24-бітний колір у FIFO буфер автомата (він сам відправить його на світлодіод)
    pio_sm_put_blocking(pio_block, statemachine_num, grb_color);
}

// активація світлодіода у форматі R G B
void rgb_led_set_color(uint8_t r, uint8_t g, uint8_t b) {
    rgb_led_set_color_one(rgb_led_color_pack(r, g, b));
}

// упаковка трёх отдельных цветов в одно число в формате 0xRRGGBB
uint32_t rgb_led_color_pack(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b);
}

// Вспомогательная функция масштабирования базового цвета на текущую яркость (0..255)
uint32_t apply_brightness(uint32_t base, uint8_t brightness) {
    uint32_t R = (((base >> 16) & 0xff) * brightness) / 255;
    uint32_t G = (((base >> 8) & 0xff) * brightness) / 255;
    uint32_t B =  ((base & 0xff) * brightness) / 255;

    return ((R & 0xff) << 16) | ((G & 0xff) << 8) | (B & 0xff);
}

// Упаковка и преобразование
uint32_t apply_brightness_pack(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
    return apply_brightness(rgb_led_color_pack(r, g, b), brightness);
}
