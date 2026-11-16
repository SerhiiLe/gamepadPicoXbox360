// Імітація Gamepad Xbox 360 для PC та Android. З реальними консолями працювати не буде тому, що немає сертификата.

#include <hardware/gpio.h>
#include <pico/time.h>
#include <string.h>

#include "xinput_device.h" // драйвер usb xinput
#include "rgb_led.h" // Підключаємо наш RGB драйвер
#include "sticks_adc.h" // Підключаємо наш АЦП-модуль
#include "buttons.h" // Підключаємо модуль кнопок
#include "rumble_pwm.h" // Обробка вібрації rumble
#include "sticks_calib.h" // Калібрування
#include "turbo_mode.h" // Турбо
#include "status_led.h" // Визначення кольорів статусу


int main() {
    // Ініціалізація RGB світлодіода
    rgb_led_init();

    // Запуск АЦП
    gamepad_adc_init();

    // Запуск кнопок
    gamepad_buttons_init();

    // Запуск апаратного ШІМ для моторів
    gamepad_pwm_init();

    // Ініціалізуємо структури збереженого калібрування з флеш-пам'яті
    gamepad_calib_init();

    // Перевірка входу в режим калібрування при старті живлення
    // Якщо центральна кнопка Guide (GP8) затиснута (дає 0 / LOW)
    if (gpio_get(PIN_BTN_GUIDE) == 0) {
        gamepad_run_calibration(); // Запускаємо 7-секундний процес
    }

    uint32_t current_color = 0x111111;
    uint32_t active_color = current_color;
    // Стартовий колір при ввімкненні (Білий)
    rgb_led_set_color_one(current_color); // Значення від 0 до 255 (краще не ставити 255, щоб не сліпило)

    // Ініціалізуємо структуру репорту XInput
    XInputReport report_data;
    memset(&report_data, 0, sizeof(XInputReport));
    report_data.report_id = 0x00;
    report_data.report_size = sizeof(XInputReport); // 20 байт

    // Запускаємо USB XInput
    xinput_init();

    absolute_time_t last_blink_time = get_absolute_time();
    absolute_time_t last_report_time = get_absolute_time();
    bool button_state = false;
    
    while (1) {
        // Обслуговування USB завдань (і отримання вібрації)
        xinput_tasks();

        active_color = current_color;

        // Зчитуємо поточні значення вібрації, які TinyUSB отримав від ПК
        uint8_t vibe_left = 0, vibe_right = 0;
        xinput_get_rumble(&vibe_left, &vibe_right);

        // Передаємо їх на апаратний ШІМ моторів GP17/GP18
        gamepad_pwm_set_rumble(vibe_left, vibe_right);

        absolute_time_t now = get_absolute_time();

        // Відправна ігрового звіту на ПК кожні 4 мс (Частота 250 Гц)
        if (absolute_time_diff_us(last_report_time, now) >= 4000) {
            last_report_time = now;

            // Структура для збереження поточних координат стиків
            GamepadAxes current_axes;
    
            // Зчитуємо реальні фізичні осі з потенціометрів. Одразу значення фільтруються.
            gamepad_adc_read(&current_axes);

            // Переносимо дані в USB-репорт геймпада
            report_data.thumb_lx = current_axes.lx;
            report_data.thumb_ly = current_axes.ly;
            report_data.thumb_rx = current_axes.rx;
            report_data.thumb_ry = current_axes.ry;

            // Читання фізичних кнопок та імітація курків LT/RT
            gamepad_buttons_read(&report_data);

            // Змінні перехоплення індикації для режиму Турбо
            bool turbo_led_active = false;
            uint32_t turbo_led = 0;

            // Модифікуємо репорт кнопками Турбо
            gamepad_turbo_process(&report_data, &turbo_led_active, &turbo_led);

            xinput_send_report(&report_data);

            // Оновлюємо колір світлодіода за статусом від ПК
            // Перевіряємо статус гравця від ПК
            uint8_t current_led_status = xinput_get_led_status();
            current_color = current_led_status_color(current_led_status);

            // Логіка світлодіода (Турбо має вищий пріоритет за колір номера гравця)
            if (turbo_led_active) {
                // turbo имеет максимальный приоритет
                active_color = turbo_led; // Швидке блимання при автокліках
            } else if ( vibe_left || vibe_right ) {
                // rumble имеет приоритет ниже
                active_color = ((uint32_t)(vibe_left) << 16) | (uint32_t)(vibe_right);
            } else {
                // отобразить номер игрока
                active_color = current_color;
            }
            // поменять цвет светодиода, если надо
            if (last_color != active_color) rgb_led_set_color_one(active_color);
        }
    }

    return 0;
}
