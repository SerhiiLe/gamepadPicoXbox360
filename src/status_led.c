// Визначення кольору для індикації гравця або статуса

#include <hardware/timer.h>
#include "rgb_led.h"

// Вычисляет плавный множитель яркости (0..255) для эффекта "дыхания"
// period_ms — длительность одного полного цикла (вдох + выдох), например, 2000 мс
static uint8_t calc_breathing_brightness(uint64_t elapsed_ms, uint32_t period_ms) {
    if (period_ms == 0) return 255;

    uint32_t half_period = period_ms / 2;
    uint32_t pos = elapsed_ms % period_ms;

    // 1. Формируем треугольную волну от 0 до half_period и обратно к 0
    uint32_t linear = (pos < half_period) ? pos : (period_ms - pos);

    // 2. Нормализуем значение в диапазон 0..255
    uint32_t norm = (linear * 255) / half_period;

    // 3. Гамма-коррекция (квадратичная кривая: (norm * norm) / 255)
    // Делает затухание у нуля максимально плавным для человеческого глаза
    return (uint8_t)((norm * norm) / 255);
}

// Палитра 4 игроков (RGB, яркость ограничена до 20 (0x14), чтобы не слепило)
static const uint32_t player_colors[4] = {
    0x001200,   // 1-й гравець: Зелений
    0x140000,   // 2-й гравець: Червоний
    0x000013,   // 3-й гравець: Синій
    0x110D00    // 4-й гравець: Жовтий
};

// Определение нужного цвета на основе текущего статуса
uint32_t current_led_status_color(uint8_t cmd) {
    static uint8_t last_cmd = 0xFF;
    static uint8_t active_player = 0; // Индекс 0..3 (по умолчанию Игрок 1)
    static uint64_t cmd_start_us = 0;

    uint64_t now_us = time_us_64();

    // Фиксируем момент прихода новой команды от ПК
    if (cmd != last_cmd) {
        last_cmd = cmd;
        cmd_start_us = now_us;

        if (cmd >= 0x02 && cmd <= 0x05) {
            active_player = cmd - 0x02;
        } else if (cmd >= 0x06 && cmd <= 0x09) {
            active_player = cmd - 0x06;
        }
    }

    uint64_t elapsed_ms = (now_us - cmd_start_us) / 1000;
    uint32_t color = 0;

    switch (cmd) {
        case 0x00: // Все выключены
            color = 0;
            break;

        case 0x01: { // Ожидание / поиск (быстрое дыхание белым, цикл 800 мс)
            uint8_t br = calc_breathing_brightness(elapsed_ms, 800);
            color = apply_brightness_pack(30, 30, 30, br);
            break;
        }

        case 0x02: // Мигнуть 3 раза (1.5 сек) и зафиксировать Игрока 1..4
        case 0x03:
        case 0x04:
        case 0x05:
            if (elapsed_ms < 1500 && ((elapsed_ms / 250) % 2 == 1)) {
                color = 0; // Фаза гашения при стартовом мигании
            } else {
                color = player_colors[active_player];
            }
            break;

        case 0x06: // Постоянное свечение Игрока 1..4 без мигания
        case 0x07:
        case 0x08:
        case 0x09:
            color = player_colors[active_player];
            break;

        case 0x0A: { // Вращение по кругу: перебор 4 цветов игроков каждые 200 мс
            uint8_t step = (elapsed_ms / 200) % 4;
            color = player_colors[step];
            break;
        }

        case 0x0B: { // Быстрое дыхание текущим цветом игрока (цикл 1200 мс)
            uint8_t br = calc_breathing_brightness(elapsed_ms, 1200);
            color = apply_brightness(player_colors[active_player], br);
            break;
        }

        case 0x0C: { // Медленное дыхание белым (цикл 2000 мс)
            uint8_t br = calc_breathing_brightness(elapsed_ms, 2000);
            color = apply_brightness_pack(30, 30, 30, br);
            break;
        }

        case 0x0D: { // Попеременное мигание (чередование 1-го и 2-го цветов палитры)
            uint8_t pair = ((elapsed_ms / 300) % 2 == 0) ? 0 : 1;
            color = player_colors[pair];
            break;
        }

        default: { // До получения первой команды от ПК (спокойное дыхание, цикл 2500 мс)
            uint8_t br = calc_breathing_brightness(elapsed_ms, 3000);
            color = apply_brightness_pack(25, 25, 25, br);
            break;
        }

    }

    return color;
}
