#include "buttons.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

// Статичні змінні для збереження поточного плавного значення курків (0.0 ... 255.0)
static float current_lt = 0.0f;
static float current_rt = 0.0f;

// 🔥 НАЛАШТУВАННЯ ШВИДКОСТІ НАТИСКАННЯ:
// Чим більша швидкість, тим швидше курок дотискається до максимуму.
// Значення 15.0f означає, що від 0 до 255 курок пройде приблизно за 70-80 мс.
#define TRIGGER_SPEED_PRESS   15.0f 
#define TRIGGER_SPEED_RELEASE 20.0f // Назад курок зазвичай повертається швидше (пружина)

// Список усіх використаних пінів для швидкої ініціалізації в циклі
static const uint8_t button_pins[] = {
    PIN_BTN_A, PIN_BTN_B, PIN_BTN_X, PIN_BTN_Y,
    PIN_BTN_LB, PIN_BTN_RB, PIN_BTN_BACK, PIN_BTN_START, PIN_BTN_GUIDE,
    PIN_BTN_LS, PIN_BTN_RS,
    PIN_DPAD_UP, PIN_DPAD_DOWN, PIN_DPAD_LEFT, PIN_DPAD_RIGHT,
    PIN_BTN_LT, PIN_BTN_RT
};

void gamepad_buttons_init(void) {
    uint8_t num_pins = sizeof(button_pins) / sizeof(button_pins[0]);
    for (uint8_t i = 0; i < num_pins; i++) {
        gpio_init(button_pins[i]);
        gpio_set_dir(button_pins[i], GPIO_IN);
        gpio_pull_up(button_pins[i]); // Підтяжка до 3.3V, кнопка замикає на GND
    }
}

void gamepad_buttons_read(XInputReport* report) {
    // Читання стандартних цифрових кнопок. !gpio_get поверне true (1), якщо пін замкнений на GND (натиснутий)
    report->button_a = !gpio_get(PIN_BTN_A);
    report->button_b = !gpio_get(PIN_BTN_B);
    report->button_x = !gpio_get(PIN_BTN_X);
    report->button_y = !gpio_get(PIN_BTN_Y);

    report->left_bumper  = !gpio_get(PIN_BTN_LB);
    report->right_bumper = !gpio_get(PIN_BTN_RB);

    report->back  = !gpio_get(PIN_BTN_BACK);
    report->start = !gpio_get(PIN_BTN_START);
    report->guide = !gpio_get(PIN_BTN_GUIDE);

    report->left_stick_click  = !gpio_get(PIN_BTN_LS);
    report->right_stick_click = !gpio_get(PIN_BTN_RS);

    report->dpad_up    = !gpio_get(PIN_DPAD_UP);
    report->dpad_down  = !gpio_get(PIN_DPAD_DOWN);
    report->dpad_left  = !gpio_get(PIN_DPAD_LEFT);
    report->dpad_right = !gpio_get(PIN_DPAD_RIGHT);

    // 🔥 АЛГОРИТМ ПЛАВНОГО ДОТИСКАННЯ КУРКІВ:
    
    // 1. Обробка Лівого Курка (LT)
    if (!gpio_get(PIN_BTN_LT)) { // Кнопка натиснута
        current_lt += TRIGGER_SPEED_PRESS;
        if (current_lt > 255.0f) current_lt = 255.0f;
    } else { // Кнопка відпущена
        current_lt -= TRIGGER_SPEED_RELEASE;
        if (current_lt < 0.0f) current_lt = 0.0f;
    }
    report->left_trigger = (uint8_t)current_lt;

    // 2. Обробка Правого Курка (RT)
    if (!gpio_get(PIN_BTN_RT)) { // Кнопка натиснута
        current_rt += TRIGGER_SPEED_PRESS;
        if (current_rt > 255.0f) current_rt = 255.0f;
    } else { // Кнопка відпущена
        current_rt -= TRIGGER_SPEED_RELEASE;
        if (current_rt < 0.0f) current_rt = 0.0f;
    }
    report->right_trigger = (uint8_t)current_rt;
}
