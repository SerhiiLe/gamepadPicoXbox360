#ifndef STICKS_ADC_H
#define STICKS_ADC_H

#include <stdint.h>
#include "config.h"

// Структура для збереження «сирих» або вже масштабованих осей
typedef struct {
    int16_t lx; // Лівий стик X (-32768 ... 32767)
    int16_t ly; // Лівий стик Y
    int16_t rx; // Правий стик X
    int16_t ry; // Правий стик Y
} GamepadAxes;

void gamepad_adc_init(void);
void gamepad_adc_read(GamepadAxes* axes);

#endif // STICKS_ADC_H
