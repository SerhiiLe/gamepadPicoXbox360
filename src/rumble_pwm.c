#include "rumble_pwm.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"

static uint slice_left;
static uint slice_right;

void gamepad_pwm_init(void) {
    // Налаштовуємо GPIO на функцію PWM
    gpio_set_function(PIN_MOTOR_LEFT, GPIO_FUNC_PWM);
    gpio_set_function(PIN_MOTOR_RIGHT, GPIO_FUNC_PWM);

    // Визначаємо апаратні слайси для кожного піна
    slice_left  = pwm_gpio_to_slice_num(PIN_MOTOR_LEFT);
    slice_right = pwm_gpio_to_slice_num(PIN_MOTOR_RIGHT);

    // Обидва слайси будуть рахувати від 0 до 255 (це дасть роздільну здатність 8 біт під XInput)
    pwm_set_wrap(slice_left, 255);
    pwm_set_wrap(slice_right, 255);

    // Початково мотори вимкнені
    pwm_set_chan_level(slice_left, PWM_CHAN_B, 0);
    pwm_set_chan_level(slice_right, PWM_CHAN_A, 0);

    // Запускаємо генератори
    pwm_set_enabled(slice_left, true);
    pwm_set_enabled(slice_right, true);
}

void gamepad_pwm_set_rumble(uint8_t left, uint8_t right) {
    // Записуємо рівень заповнення ШІМ безпосередньо (0..255)
    pwm_set_chan_level(slice_left, PWM_CHAN_B, left);
    pwm_set_chan_level(slice_right, PWM_CHAN_A, right);
}
