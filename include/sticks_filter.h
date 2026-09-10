#ifndef STICKS_FILTER_H
#define STICKS_FILTER_H

#include <stdint.h>
#include <stdbool.h>

static float normalize_axis_raw(int16_t raw_adc, int8_t adc_index);
void process_stick_radial(int16_t raw_x, int16_t raw_y, int8_t adcX, int8_t adcY, int16_t *out_x, int16_t *out_y);

#endif // STICKS_FILTER_H
