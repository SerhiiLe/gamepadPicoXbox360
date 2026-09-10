#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#define CFG_TUSB_RHPORT0_MODE       OPT_MODE_DEVICE
#define CFG_TUSB_OS                 OPT_OS_NONE
#define CFG_TUD_ENDPOINT0_SIZE      64

// Дозволяємо кастомний драйвер, вимикаємо автоматичний Vendor
#define CFG_TUD_EXTERN_APP_DRIVER   1

#define CFG_TUD_VENDOR              0
#define CFG_TUD_HID                 0
#define CFG_TUD_CDC                 0
#define CFG_TUD_MSC                 0
#define CFG_TUD_MIDI                0

#endif // _TUSB_CONFIG_H_
