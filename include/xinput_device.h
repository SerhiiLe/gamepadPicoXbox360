#ifndef XINPUT_DEVICE_H
#define XINPUT_DEVICE_H

#include <stdint.h>
#include <stdbool.h>

#define EPNUM_XINPUT_IN   0x81
#define EPNUM_XINPUT_OUT  0x02

#pragma pack(push, 1)
typedef struct {
    uint8_t report_id;          // 0x00
    uint8_t report_size;        // 0x14 (20 байт)
    
    // Кнопки 1
    uint8_t dpad_up    : 1;
    uint8_t dpad_down  : 1;
    uint8_t dpad_left  : 1;
    uint8_t dpad_right : 1;
    uint8_t start      : 1;
    uint8_t back       : 1;
    uint8_t left_stick_click  : 1;
    uint8_t right_stick_click : 1;

    // Кнопки 2
    uint8_t left_bumper  : 1;
    uint8_t right_bumper : 1;
    uint8_t guide        : 1;
    uint8_t reserved     : 1;
    uint8_t button_a     : 1;
    uint8_t button_b     : 1;
    uint8_t button_x     : 1;
    uint8_t button_y     : 1;

    // Аналоговые триггеры
    uint8_t left_trigger;
    uint8_t right_trigger;

    // Стики
    int16_t thumb_lx;
    int16_t thumb_ly;
    int16_t thumb_rx;
    int16_t thumb_ry;

    uint8_t reserved_bytes[6];
} XInputReport;
#pragma pack(pop)

void xinput_init(void);
void xinput_tasks(void);
bool xinput_send_report(const XInputReport* report);
bool xinput_get_rumble(uint8_t* left_motor, uint8_t* right_motor);
// Повертає поточний LED-код від ПК (0x02 - 1-й гравець, 0x03 - 2-й і т.д.)
uint8_t xinput_get_led_status(void);

#endif // XINPUT_DEVICE_H
