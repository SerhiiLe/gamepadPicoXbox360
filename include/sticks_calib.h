#ifndef STICKS_CALIB_H
#define STICKS_CALIB_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

// Структура для збереження калібрувальних значень у флеш-пам'яті
typedef struct {
    uint32_t magic;       // Мітка валідності даних (наприклад, 0xDEADBEEF)
    uint16_t center[4];   // Центри для 4-х каналів АЦП (ADC 0..3)
    uint16_t min_val[4];  // Мінімальні відхилення
    uint16_t max_val[4];  // Максимальні відхилення
} CalibrationData;

extern CalibrationData calib_ram;

void gamepad_calib_init(void);
void gamepad_run_calibration(void);

#endif // STICKS_CALIB_H
