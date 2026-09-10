#ifndef RUMBLE_PWM_H
#define RUMBLE_PWM_H

#include <stdint.h>
#include "config.h"

void gamepad_pwm_init(void);
void gamepad_pwm_set_rumble(uint8_t left, uint8_t right);

#endif // RUMBLE_PWM_H
