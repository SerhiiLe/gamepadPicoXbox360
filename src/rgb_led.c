#include "rgb_led.h"
#include "hardware/pio.h"
#include "ws2812.pio.h" // Цей заголовок згенерується автоматично з нашого .pio файлу!

static PIO pio_block = pio0;
static uint statemachine_num = 0;
uint32_t last_color = 0;

// инициализация светодиода ws2812
void rgb_led_init(void) {
    // Знаходимо вільне місце в пам'яті PIO та завантажуємо туди програму
    uint offset = pio_add_program(pio_block, &ws2812_program);
    
    // Ініціалізуємо PIO-автомат на частоті 800 кГц (стандарт для WS2812)
    ws2812_program_init(pio_block, statemachine_num, offset, WS2812_PIN, 800000, false);
}

// активация светодиода
void rgb_led_set_color_one(uint32_t color) {
    // Протокол WS2812 очікує дані у форматі GRB (Green-Red-Blue)
    last_color = color;
    
    // Записуємо 24-бітний колір у FIFO буфер автомата (він сам відправить його на світлодіод)
    pio_sm_put_blocking(pio_block, statemachine_num, color << 8);
}

// активация светодиода в формате 0xRRGGBB
void rgb_led_set_color(uint8_t r, uint8_t g, uint8_t b) {
    rgb_led_set_color_one(rgb_led_color_pack(r, g, b));
}

// упаковка трёх отдельных цветов в одно число
uint32_t rgb_led_color_pack(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(g) << 16) | ((uint32_t)(r) << 8) | (uint32_t)(b);
}
