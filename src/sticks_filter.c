#include "sticks_filter.h"
#include "sticks_calib.h"

#include <math.h>

static float normalize_axis_raw(int16_t raw_adc, int8_t adc_index) {
    static float s_ema_adc[4] = {-2.0f, -2.0f, -2.0f, -2.0f};
    if (adc_index >= 4) return 0;

    float norm = 0.0f;
    if (raw_adc < calib_ram.center[adc_index]) {
        int16_t span = calib_ram.center[adc_index] - calib_ram.min_val[adc_index];
        if (span < 1) span = 1;
        norm = (float)(raw_adc - calib_ram.center[adc_index]) / (float)span;
    } else {
        int16_t span = calib_ram.max_val[adc_index] - calib_ram.center[adc_index];
        if (span < 1) span = 1;
        norm = (float)(raw_adc - calib_ram.center[adc_index]) / (float)span;
    }

    if (s_ema_adc[adc_index] < -1.0f) {
        s_ema_adc[adc_index] = norm; // инициализация фильтра
    } else {
        s_ema_adc[adc_index] = s_ema_adc[adc_index] * (1.0f - EMA_ALPHA) + norm * EMA_ALPHA;
    }
  
    norm = s_ema_adc[adc_index];

    if (norm < -1.0f) norm = -1.0f;
    if (norm > 1.0f) norm = 1.0f;
    return norm;
}

void process_stick_radial(int16_t raw_x, int16_t raw_y, int8_t adcX, int8_t adcY, int16_t *out_x, int16_t *out_y) {
    // 1. Normalize raw ADC to square domain [-1.0, 1.0]
    float u = normalize_axis_raw(raw_x, adcX);
    float v = normalize_axis_raw(raw_y, adcY);

    // 2. Map Square domain [-1.0, 1.0] to Circular domain [-1.0, 1.0] (Elliptical Grid Mapping)
    float circle_x = u * sqrtf(1.0f - (v * v * 0.5f));
    float circle_y = v * sqrtf(1.0f - (u * u * 0.5f));

    // 3. Radial Deadzone in Circular Domain
    float magnitude = sqrtf(circle_x * circle_x + circle_y * circle_y);
    float deadzone = (float)(DEADZONE_PERCENT) / 100.0f;

    if (magnitude < deadzone) {
        *out_x = 0;
        *out_y = 0;
        return;
    }

    float range = 1.0f - deadzone;

    float scaled_magnitude = (magnitude - deadzone) / range;
    if (scaled_magnitude > 1.0f) scaled_magnitude = 1.0f;

    float norm_u = (circle_x / magnitude) * scaled_magnitude;
    float norm_v = (circle_y / magnitude) * scaled_magnitude;

    if (norm_u < -1.0f) norm_u = -1.0f;
    if (norm_u > 1.0f) norm_u = 1.0f;
    if (norm_v < -1.0f) norm_v = -1.0f;
    if (norm_v > 1.0f) norm_v = 1.0f;

    *out_x = (int16_t)(norm_u * 32767.0f);
    *out_y = (int16_t)(norm_v * 32767.0f);

}
