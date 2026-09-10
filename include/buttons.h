#ifndef BUTTONS_H
#define BUTTONS_H

#include "xinput_device.h"
#include "config.h"

void gamepad_buttons_init(void);
void gamepad_buttons_read(XInputReport* report);

#endif // BUTTONS_H
