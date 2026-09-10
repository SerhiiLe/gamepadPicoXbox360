#include "sticks_adc.h"
#include "sticks_filter.h"
#include "hardware/adc.h"

void gamepad_adc_init(void) {
    adc_init();

    // Вмикаємо режим АЦП (аналоговий вхід) для потрібних GPIO
    adc_gpio_init(26); // ADC 0
    adc_gpio_init(27); // ADC 1
    adc_gpio_init(28); // ADC 2
    adc_gpio_init(29); // ADC 3
}

//  Read ADC with EMA Filtering and Process Radial Deadzone
void gamepad_adc_read(GamepadAxes* axes) {

    adc_select_input(ADC_STICK_LX);
    int16_t raw_lx = adc_read();

    adc_select_input(ADC_STICK_LY);
    int16_t raw_ly = adc_read();

    adc_select_input(ADC_STICK_RX);
    int16_t raw_rx = adc_read();

    adc_select_input(ADC_STICK_RY);
    int16_t raw_ry = adc_read();

    int16_t lx = 0, ly = 0, rx = 0, ry = 0;
    process_stick_radial(raw_lx, raw_ly, ADC_STICK_LX, ADC_STICK_LY, &lx, &ly);
    process_stick_radial(raw_rx, raw_ry, ADC_STICK_RX, ADC_STICK_RY, &rx, &ry);

#if INVERT_DIRECTION_LX
    lx = -lx;
#endif

#if INVERT_DIRECTION_LY
    ly = -ly;
#endif

#if INVERT_DIRECTION_RX
    rx = -rx;
#endif

#if INVERT_DIRECTION_RY
    ry = -ry;
#endif

    axes->lx = lx;
    axes->ly = ly;
    axes->rx = rx;
    axes->ry = ry;
}
