// Зчитування кнопок та імітація аналогових курків

#include <hardware/gpio.h>
#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#include "buttons.h"

// Статичні змінні для збереження поточного плавного значення курків (0.0 ... 255.0)
static float current_lt = 0.0f;
static float current_rt = 0.0f;

// Список усіх використаних пінів для швидкої ініціалізації в циклі
static const uint8_t button_pins[] = {
    PIN_BTN_A, PIN_BTN_B, PIN_BTN_X, PIN_BTN_Y,
    PIN_BTN_LB, PIN_BTN_RB, PIN_BTN_BACK, PIN_BTN_START, PIN_BTN_GUIDE,
    PIN_BTN_LS, PIN_BTN_RS,
    PIN_DPAD_UP, PIN_DPAD_DOWN, PIN_DPAD_LEFT, PIN_DPAD_RIGHT,
    PIN_BTN_LT, PIN_BTN_RT
};

// Total Buttons
#define NUM_BUTTONS (sizeof(button_pins) / sizeof(button_pins[0]))
#define MAX_BUTTONS 30 // у rp2040 всего 30 ножек

static bool debounced_state[MAX_BUTTONS];
static int8_t lockout_counter[NUM_BUTTONS];

void gamepad_buttons_init(void) {
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        uint8_t pin = button_pins[i];
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin); // Підтяжка до 3.3V, кнопка замикає на GND
        debounced_state[pin] = false;
        lockout_counter[i] = 0;
    }
}

// Чтение всех кнопок и заполнение отчёта
void gamepad_buttons_read(XInputReport* report) {

    // чтение всех кнопок и отсеивание дребезга за один проход
    for (uint8_t i=0; i < NUM_BUTTONS; i++) {
        uint8_t pin = button_pins[i];
        if (lockout_counter[i] > 0) {
            lockout_counter[i]--; // идет слепой период
        } else {
            // Читання стандартних цифрових кнопок. !gpio_get поверне true (1), якщо пін замкнений на GND (натиснутий)
            bool raw_state = !gpio_get(pin);
            if (raw_state != debounced_state[pin]) {
                debounced_state[pin] = raw_state; // Мгновенное срабатывание
                lockout_counter[i] = DEBOUNCE_LOCKOUT_TICKS; // Блокируем на 6 тиков (6 мс)
            }
        }
    }
    
    // заполнение отчёта. Это структура одна кнопка - один бит, хотя выглядит как обычное присвоение

    report->button_a = debounced_state[PIN_BTN_A];
    report->button_b = debounced_state[PIN_BTN_B];
    report->button_x = debounced_state[PIN_BTN_X];
    report->button_y = debounced_state[PIN_BTN_Y];

    report->left_bumper  = debounced_state[PIN_BTN_LB];
    report->right_bumper = debounced_state[PIN_BTN_RB];

    report->back  = debounced_state[PIN_BTN_BACK];
    report->start = debounced_state[PIN_BTN_START];
    report->guide = debounced_state[PIN_BTN_GUIDE];

    report->left_stick_click  = debounced_state[PIN_BTN_LS];
    report->right_stick_click = debounced_state[PIN_BTN_RS];

    report->dpad_up    = debounced_state[PIN_DPAD_UP];
    report->dpad_down  = debounced_state[PIN_DPAD_DOWN];
    report->dpad_left  = debounced_state[PIN_DPAD_LEFT];
    report->dpad_right = debounced_state[PIN_DPAD_RIGHT];

    // 🔥 АЛГОРИТМ ПЛАВНОГО ДОТИСКАННЯ КУРКІВ:
    
    // 1. Обробка Лівого Курка (LT)
    if (debounced_state[PIN_BTN_LT]) { // Кнопка натиснута
        current_lt += TRIGGER_SPEED_PRESS;
        if (current_lt > 255.0f) current_lt = 255.0f;
    } else { // Кнопка відпущена
        current_lt -= TRIGGER_SPEED_RELEASE;
        if (current_lt < 0.0f) current_lt = 0.0f;
    }
    report->left_trigger = (uint8_t)current_lt;

    // 2. Обробка Правого Курка (RT)
    if (debounced_state[PIN_BTN_RT]) { // Кнопка натиснута
        current_rt += TRIGGER_SPEED_PRESS;
        if (current_rt > 255.0f) current_rt = 255.0f;
    } else { // Кнопка відпущена
        current_rt -= TRIGGER_SPEED_RELEASE;
        if (current_rt < 0.0f) current_rt = 0.0f;
    }
    report->right_trigger = (uint8_t)current_rt;
}
