#include "turbo_mode.h"
#include "config.h"
#include "pico/stdlib.h"
#include "rgb_led.h"

// Флаги активності режиму Турбо для кожної конкретної кнопки
static bool turbo_a = false;
static bool turbo_b = false;
static bool turbo_x = false;
static bool turbo_y = false;
static bool turbo_lb = false;
static bool turbo_rb = false;

static bool combo_ready = true;

// Таймери для реалізації затримки видачі Back/Start в гру
static uint64_t back_press_start_us = 0;
static uint64_t start_press_start_us = 0;
static uint64_t combo_release_guard_us = 0;

void gamepad_turbo_process(XInputReport* report, bool* override_led, uint32_t* led_out) {
    uint64_t now_us = time_us_64();

    // 1. ЗАХИСТ ВІДПУСКАННЯ КОМБІНАЦІЇ
    if (combo_release_guard_us != 0) {
        if (now_us - combo_release_guard_us < COMBO_DELAY_US) {
            // Повністю пригнічуємо кнопки, поки пальці розтискають комбінацію
            report->back = 0;
            report->start = 0;
        } else {
            combo_release_guard_us = 0; // Час післякомбінаційного захисту вийшов
        }
    }

    // Відстежуємо фізичні натискання кнопок для таймерів
    if (report->back && back_press_start_us == 0)   back_press_start_us = now_us;
    if (!report->back)                              back_press_start_us = 0;
    if (report->start && start_press_start_us == 0) start_press_start_us = now_us;
    if (!report->start)                             start_press_start_us = 0;

    // Перевіряємо, чи натиснуті ОБИДВІ кнопки одночасно
    bool is_combo = report->back && report->start;

    if (is_combo) {
        // Якщо затиснута комбінація, блокуємо їх видачу на ПК
        report->back = 0;
        report->start = 0;
        combo_release_guard_us = now_us; // Ставимо мітку захисту відпускання

        if (combo_ready) {
            // Перевіряємо, яку саме кнопку дії натиснув користувач разом із комбінацією, і інвертуємо її режим
            if (report->button_a)     { turbo_a = !turbo_a;   combo_ready = false; }
            if (report->button_b)     { turbo_b = !turbo_b;   combo_ready = false; }
            if (report->button_x)     { turbo_x = !turbo_x;   combo_ready = false; }
            if (report->button_y)     { turbo_y = !turbo_y;   combo_ready = false; }
            if (report->left_bumper)  { turbo_lb = !turbo_lb; combo_ready = false; }
            if (report->right_bumper) { turbo_rb = !turbo_rb; combo_ready = false; }

            if (!combo_ready) {
                // Спалах для підтвердження налаштування
                rgb_led_set_color(60, 60, 60);
                sleep_ms(150);
            }
        }
    } else {
        // Якщо жодна кнопка дії не натиснута, скидаємо блокування комбінації
        if (!report->button_a && !report->button_b && !report->button_x && 
            !report->button_y && !report->left_bumper && !report->right_bumper) {
            combo_ready = true;
        }

        // --- ЛОГІКА ЗВИЧАЙНИХ НАТИСКАНЬ ЗАТРИМКИ ---
        if (combo_release_guard_us == 0) {
            // Якщо натиснуто тільки Back, і COMBO_DELAY_US мс ще НЕ пройшло — ховаємо її від ПК (чекаємо Start)
            if (report->back && !report->start) {
                if (now_us - back_press_start_us < COMBO_DELAY_US) {
                    report->back = 0;
                }
            }
            // Якщо натиснуто тільки Start, і COMBO_DELAY_US мс ще НЕ пройшло — ховаємо її від ПК (чекаємо Back)
            if (report->start && !report->back) {
                if (now_us - start_press_start_us < COMBO_DELAY_US) {
                    report->start = 0;
                }
            }
        }
    }

    // --- ГЕНЕРАЦІЯ КЛІКІВ ТА ПЛАВНОЇ ІНДИКАЦІЇ ---
    uint64_t click_clock = now_us / 33000;
    bool turbo_pulse = click_clock & 1;
    bool led_pulse = (click_clock / 3) & 1; // Частота блимання світлодіода (~5 Гц)

    *override_led = false;

    // Перевіряємо утримання активованих Турбо-кнопок і застосовуємо автоклік
    if (report->button_a && turbo_a) { report->button_a = turbo_pulse; *override_led = true; }
    if (report->button_b && turbo_b) { report->button_b = turbo_pulse; *override_led = true; }
    if (report->button_x && turbo_x) { report->button_x = turbo_pulse; *override_led = true; }
    if (report->button_y && turbo_y) { report->button_y = turbo_pulse; *override_led = true; }
    if (report->left_bumper  && turbo_lb) { report->left_bumper  = turbo_pulse; *override_led = true; }
    if (report->right_bumper && turbo_rb) { report->right_bumper = turbo_pulse; *override_led = true; }

    // Якщо в цей момент користувач утримує будь-яку активну Турбо-кнопку
    if (*override_led) {
        *led_out = led_pulse ? rgb_led_color_pack(15, 15, 15) : 0;
    }
}
