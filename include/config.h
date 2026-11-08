#ifndef CONFIG_H
#define CONFIG_H

// WS2812 RGB LED (RP2040-Zero onboard NeoPixel)
#define PIN_RGB_LED         16

// Rumble Motor PWM Output. Don`t change! Не міняти!
#define PIN_MOTOR_LEFT  17 // Сильний (низькочастотний) мотор
#define PIN_MOTOR_RIGHT 18 // Слабкий (високочастотний) мотор

// Мапінг кнопок на GPIO піни RP2040-Zero (крім зайнятих RGB=16 та ADC=26-29)
#define PIN_BTN_A      8
#define PIN_BTN_B      7
#define PIN_BTN_X      9
#define PIN_BTN_Y      6

#define PIN_BTN_LB     2
#define PIN_BTN_RB     3

#define PIN_BTN_BACK   15
#define PIN_BTN_START  5
#define PIN_BTN_GUIDE  10  // Центральна кнопка Xbox

#define PIN_BTN_LS     19  // Натискання лівого стіка
#define PIN_BTN_RS     4   // Натискання правого стіка

#define PIN_DPAD_UP    14
#define PIN_DPAD_DOWN  12
#define PIN_DPAD_LEFT  13
#define PIN_DPAD_RIGHT 11

// Цифрові кнопки, які стануть аналоговими курками LT / RT
#define PIN_BTN_LT     0
#define PIN_BTN_RT     1

// Analog Stick ADC Pins
#define ADC_STICK_LX        0 // 26  // ADC0
#define ADC_STICK_LY        1 // 27  // ADC1
#define ADC_STICK_RX        3 // 29  // ADC3
#define ADC_STICK_RY        2 // 28  // ADC2

#define INVERT_DIRECTION_LX     true
#define INVERT_DIRECTION_LY     true
#define INVERT_DIRECTION_RX     true
#define INVERT_DIRECTION_RY     true

// Exponential Moving Average (EMA) alpha factor for ADC noise filtering (0.0..1.0)
#define EMA_ALPHA           0.25f

// Analog Stick Deadzone percentage (0..100)
#define DEADZONE_PERCENT 2.5f

// ADC Resolution (RP2040 uses 12-bit ADC: 0..4095)
#define ADC_MIN_VALUE       18
#define ADC_MAX_VALUE       4095
#define ADC_DEFAULT_CENTER  2056

// Combo = Start + Back + target button
#define COMBO_DELAY_US 100000 // Затримка 100 мс

#endif // CONFIG_H
