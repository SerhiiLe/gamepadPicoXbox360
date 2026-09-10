#include "xinput_device.h"
#include "tusb.h"
#include "device/usbd_pvt.h"
#include "pico/unique_id.h"
#include <string.h>

#define USBD_VID     0x045E 
#define USBD_PID     0x028E 

static char serial_buf[32];

static void xinput_init_serial_number(void) {
    pico_get_unique_board_id_string(serial_buf, sizeof(serial_buf));
}

CFG_TUSB_MEM_ALIGN static uint8_t xinput_out_buffer[32];
static uint8_t led_player_status = 0;
static uint8_t rumble_left = 0;
static uint8_t rumble_right = 0;

// 1. Дескриптор пристрою
tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200, 
    .bDeviceClass       = 0xFF, 
    .bDeviceSubClass    = 0xFF,
    .bDeviceProtocol    = 0xFF,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = USBD_VID,
    .idProduct          = USBD_PID,
    .bcdDevice          = 0x0114, 
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

// 2. ДЕСКРИПТОР КОНФІГУРАЦІЇ (Повний стандартний блок Xbox 360 - 49 байт)
uint8_t const desc_configuration[] __attribute__((packed)) = {
    // Configuration Descriptor (9 байт) -> wTotalLength = 0x0031 (49 байт)
    0x09, TUSB_DESC_CONFIGURATION, 0x31, 0x00, 0x01, 0x01, 0x00, 0xA0, 0xFA,

    // Interface Descriptor (9 байт) -> Interface 0, 2 Endpoints, Class 0xFF, SubClass 0x5D, Protocol 0x01
    0x09, TUSB_DESC_INTERFACE, 0x00, 0x00, 0x02, 0xFF, 0x5D, 0x01, 0x00,

    // XInput Class Sub-Descriptor (17 байт) -> Обов'язковий для Windows/Linux
    0x11, 0x21, 0x00, 0x01, 0x01, 0x25, 0x81, 0x14, 0x00, 0x00, 0x00, 0x00, 0x13, 0x02, 0x08, 0x00, 0x00,

    // Endpoint IN descriptor (7 байт) -> Адрес 0x81, bInterval = 1ms
    0x07, TUSB_DESC_ENDPOINT, EPNUM_XINPUT_IN, TUSB_XFER_INTERRUPT, 0x20, 0x00, 0x01,

    // Endpoint OUT descriptor (7 байт) -> Адрес 0x02, bInterval = 1ms
    0x07, TUSB_DESC_ENDPOINT, EPNUM_XINPUT_OUT, TUSB_XFER_INTERRUPT, 0x20, 0x00, 0x01
};

static const char* string_desc_arr[] = {
    (const char[]) { 0x09, 0x04 }, 
    "Microsoft",
    "Xbox 360 Controller",
    serial_buf
};
static uint16_t _desc_str[32]; // Масив для UTF-16

uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *) &desc_device;
}

uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
    (void) index;
    return desc_configuration;
}

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void) langid;
    uint8_t chr_count;

    if (index == 0) {
        _desc_str[0] = (TUSB_DESC_STRING << 8) | 4;
        _desc_str[1] = 0x0409; 
        return _desc_str;
    }

    if (index >= sizeof(string_desc_arr)/sizeof(string_desc_arr[0])) return NULL;

    const char* str = string_desc_arr[index];
    chr_count = strlen(str);
    if (chr_count > 30) chr_count = 30; 

    for(uint8_t i = 0; i < chr_count; i++) {
        _desc_str[1 + i] = str[i];
    }
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

    return _desc_str;
}

// ---------------- ДРАЙВЕР XINPUT ---------------- //

static void xinput_driver_init(void) {}
static void xinput_driver_reset(uint8_t rhport) {
    (void) rhport;
    led_player_status = 0;
    rumble_left = 0;
    rumble_right = 0;
}

static uint16_t xinput_driver_open(uint8_t rhport, tusb_desc_interface_t const * itf_desc, uint16_t max_len) {
    if (itf_desc->bInterfaceClass == 0xFF && itf_desc->bInterfaceSubClass == 0x5D && itf_desc->bInterfaceProtocol == 0x01) {
        
        uint16_t const drv_len = sizeof(desc_configuration) - 9;
        if (max_len < drv_len) return 0;

        uint8_t const * p_desc = (uint8_t const *) itf_desc + sizeof(tusb_desc_interface_t);
        uint16_t len = sizeof(tusb_desc_interface_t);

        while (len < drv_len) {
            if (p_desc[1] == TUSB_DESC_ENDPOINT) {
                tusb_desc_endpoint_t const * ep_desc = (tusb_desc_endpoint_t const *) p_desc;
                usbd_edpt_open(rhport, ep_desc);
            }
            len += p_desc[0];
            p_desc += p_desc[0];
        }

        // Одразу змикаємо буфер прийому OUT-підключення (0x02),
        // щоб RP2040 був готовий відповісти ACK на первинний magic packet xpad ({0x01, 0x03, 0x00})
        usbd_edpt_xfer(rhport, EPNUM_XINPUT_OUT, xinput_out_buffer, sizeof(xinput_out_buffer), false);

        return drv_len; 
    }
    return 0; 
}

static bool xinput_driver_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request) {
    if (stage != CONTROL_STAGE_SETUP) return true;

    // Обробка запиту дескриптора XInput (0x2100)
    if (request->wValue == 0x2100) {
        uint16_t len = tu_min16(request->wLength, 17);
        // Суб-дескриптор розташований з 18 байта desc_configuration
        return tud_control_xfer(rhport, request, (void*)(desc_configuration + 18), len);
    }

    // Підтверджуємо ВСІ невідомі контрольні запити хоста без STALL
    return tud_control_status(rhport, request);
}

static bool xinput_driver_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes) {
    (void) rhport;
    if (result == XFER_RESULT_SUCCESS && ep_addr == EPNUM_XINPUT_OUT) {
        
        // 1. Перевірка пакета світлодіодів (LED): довжина >= 3, починається з 0x01
        if (xferred_bytes >= 3 && xinput_out_buffer[0] == 0x01) {
            led_player_status = xinput_out_buffer[2]; 
        }
        // 2. Перевірка пакета вібрації (Rumble): довжина >= 5, починається з 0x00
        else if (xferred_bytes >= 5 && xinput_out_buffer[0] == 0x00) {
            rumble_left  = xinput_out_buffer[3]; 
            rumble_right = xinput_out_buffer[4]; 
        }

        // Перезапускаємо зчитування наступного OUT-пакета
        usbd_edpt_xfer(rhport, EPNUM_XINPUT_OUT, xinput_out_buffer, sizeof(xinput_out_buffer), false);
    }
    return true;
}

uint8_t xinput_get_led_status(void) {
    return led_player_status;
}

static const usbd_class_driver_t xinput_class_driver = {
#if CFG_TUSB_DEBUG >= 2
    .name = "XINPUT",
#endif
    .init             = xinput_driver_init,
    .reset            = xinput_driver_reset,
    .open             = xinput_driver_open,
    .control_xfer_cb  = xinput_driver_control_xfer_cb,
    .xfer_cb          = xinput_driver_xfer_cb,
    .sof              = NULL
};

usbd_class_driver_t const * usbd_app_driver_get_cb(uint8_t *driver_count) {
    *driver_count = 1;
    return &xinput_class_driver;
}

// ---------------- API ---------------- //

void xinput_init(void) {
    xinput_init_serial_number();
    tusb_init();
}

void xinput_tasks(void) {
    tud_task(); 
    if (tud_ready() && !usbd_edpt_busy(0, EPNUM_XINPUT_OUT)) {
        usbd_edpt_xfer(0, EPNUM_XINPUT_OUT, (uint8_t*)&xinput_out_buffer, sizeof(xinput_out_buffer), false);
    }
}

bool xinput_send_report(const XInputReport* report) {
    if (!tud_ready() || usbd_edpt_busy(0, EPNUM_XINPUT_IN)) {
        return false; 
    }
    return usbd_edpt_xfer(0, EPNUM_XINPUT_IN, (uint8_t*)report, sizeof(XInputReport), false);
}

bool xinput_get_rumble(uint8_t* left_motor, uint8_t* right_motor) {
    if (left_motor)  *left_motor = rumble_left;
    if (right_motor) *right_motor = rumble_right;
    return true;
}
