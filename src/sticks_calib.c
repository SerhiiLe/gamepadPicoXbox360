#include "sticks_calib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/adc.h"
// #include "pico/stdlib.h"
#include "rgb_led.h"
#include <string.h>

#define FLASH_TARGET_OFFSET ((2 * 1024 * 1024) - 4096) // Останній сектор 2MB флеш-пам'яті
#define CALIB_MAGIC_NUMBER  0xDEADBEEF

CalibrationData calib_ram;

// Дефолтні налаштування, якщо флеш порожній (12-бітний АЦП: центр 2047, мін 0, макс 4095)
static const CalibrationData default_calib = {
    .magic = CALIB_MAGIC_NUMBER,
    .center = {ADC_DEFAULT_CENTER, ADC_DEFAULT_CENTER, ADC_DEFAULT_CENTER, ADC_DEFAULT_CENTER},
    .min_val = {ADC_MIN_VALUE, ADC_MIN_VALUE, ADC_MIN_VALUE, ADC_MIN_VALUE},
    .max_val = {ADC_MAX_VALUE, ADC_MAX_VALUE, ADC_MAX_VALUE, ADC_MAX_VALUE}
};

void gamepad_calib_init(void) {
    // Читаємо дані прямо з адреси флеш-пам'яті у просторі XIP (eXecute In Place)
    const CalibrationData* flash_data = (const CalibrationData*)(XIP_BASE + FLASH_TARGET_OFFSET);
    
    if (flash_data->magic == CALIB_MAGIC_NUMBER) {
        memcpy(&calib_ram, flash_data, sizeof(CalibrationData));
    } else {
        // Якщо плата ні разу не калібрувалася — копіюємо дефолтні значення
        memcpy(&calib_ram, &default_calib, sizeof(CalibrationData));
    }
}

// Функція збереження структури у флеш-пам'ять
static void save_calib_to_flash(void) {
    uint8_t write_buf[256];
    memset(write_buf, 0, sizeof(write_buf));
    memcpy(write_buf, &calib_ram, sizeof(CalibrationData));

    // Важливо: під час операцій з флешем потрібно вимикати переривання
    uint32_t ints = save_and_disable_interrupts();
    
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, write_buf, sizeof(write_buf));
    
    restore_interrupts(ints);
}

void gamepad_run_calibration(void) {
    // 1. ЕТАП: 1 секунда — миготіння світлодіода (Підготовка)
    for (int i = 0; i < 5; i++) {
        rgb_led_set_color(30, 30, 30); // Білий
        sleep_ms(100);
        rgb_led_set_color(0, 0, 0);    // Вимкнено
        sleep_ms(100);
    }

    // 2. ЕТАП: 2 секунди — встановлення нуля (індикація жовтим)
    rgb_led_set_color(40, 40, 0); // Жовтий
    
    uint32_t center_sums[4] = {0, 0, 0, 0};
    uint32_t samples = 0;
    absolute_time_t end_time = make_timeout_time_ms(2000);

    while (get_absolute_time() < end_time) {
        for (uint8_t ch = 0; ch < 4; ch++) {
            adc_select_input(ch);
            center_sums[ch] += adc_read();
        }
        samples++;
        sleep_ms(2);
    }
    
    // Розраховуємо середнє значення для центрів
    for (uint8_t ch = 0; ch < 4; ch++) {
        calib_ram.center[ch] = center_sums[ch] / samples;
        // Тимчасово ініціалізуємо мінімуми/максимуми поточною точкою спокою
        calib_ram.min_val[ch] = calib_ram.center[ch];
        calib_ram.max_val[ch] = calib_ram.center[ch];
    }

    // 3. ЕТАП: 5 секунд — обертання стиків (індикація фіолетовим/пурпуровим)
    rgb_led_set_color(40, 0, 40); // Пурпуровий
    end_time = make_timeout_time_ms(5000);

    while (get_absolute_time() < end_time) {
        for (uint8_t ch = 0; ch < 4; ch++) {
            adc_select_input(ch);
            uint16_t val = adc_read();
            
            if (val < calib_ram.min_val[ch]) calib_ram.min_val[ch] = val;
            if (val > calib_ram.max_val[ch]) calib_ram.max_val[ch] = val;
        }
        sleep_ms(2);
    }

    // Захист від невалідних значень (якщо стіки не чіпали)
    for (uint8_t ch = 0; ch < 4; ch++) {
        if (calib_ram.max_val[ch] - calib_ram.min_val[ch] < 500) {
            calib_ram.min_val[ch] = ADC_MIN_VALUE;
            calib_ram.max_val[ch] = ADC_MAX_VALUE;
        }
    }

    // Зберігаємо отримані дані
    calib_ram.magic = CALIB_MAGIC_NUMBER;
    save_calib_to_flash();

    // 4. ЕТАП: Підтвердження успішного запису швидким зеленим миготінням
    for (int i = 0; i < 6; i++) {
        rgb_led_set_color(0, 50, 0); // Зелений
        sleep_ms(70);
        rgb_led_set_color(0, 0, 0);
        sleep_ms(70);
    }
}
