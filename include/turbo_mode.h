#ifndef TURBO_MODE_H
#define TURBO_MODE_H

#include "xinput_device.h"
#include <stdbool.h>

// Функція обробки автокліків та перехоплення керування єдиним RGB-діодом
void gamepad_turbo_process(XInputReport* report, bool* override_led, uint32_t* led_out);

#endif // TURBO_MODE_H
