/* controller_ds4.c — DualShock 4 / DS4-compatible (XIM4 etc.) for Ghost-Control
 *
 * DS4 USB wire format is the public Sony report (ID 0x01, 64 bytes).
 * No init or handshake required: open the IN endpoint, reports stream at ~250 Hz.
 */

#include "controller_ds4.h"
#include <string.h>

/* Hat lookup: index 0..8 → (up, right, down, left) bits */
static const uint8_t HAT_DPAD[9] = {
    /* 0 N  */ SCE_PAD_BUTTON_UP,
    /* 1 NE */ SCE_PAD_BUTTON_UP   | SCE_PAD_BUTTON_RIGHT,
    /* 2 E  */ SCE_PAD_BUTTON_RIGHT,
    /* 3 SE */ SCE_PAD_BUTTON_DOWN | SCE_PAD_BUTTON_RIGHT,
    /* 4 S  */ SCE_PAD_BUTTON_DOWN,
    /* 5 SW */ SCE_PAD_BUTTON_DOWN | SCE_PAD_BUTTON_LEFT,
    /* 6 W  */ SCE_PAD_BUTTON_LEFT,
    /* 7 NW */ SCE_PAD_BUTTON_UP   | SCE_PAD_BUTTON_LEFT,
    /* 8 -- */ 0u,
};

void ds4_parse_input(const uint8_t *b, ScePadData *o) {
    o->leftStick.x      = b[1];
    o->leftStick.y      = b[2];
    o->rightStick.x     = b[3];
    o->rightStick.y     = b[4];
    o->analogButtons.l2 = b[8];
    o->analogButtons.r2 = b[9];

    uint32_t btn = 0;

    /* b[5]: dpad (low nibble, hat 0..8) + face buttons (high nibble) */
    uint8_t hat = b[5] & 0x0Fu;
    if (hat <= 8) btn |= HAT_DPAD[hat];
    if (b[5] & 0x10u) btn |= SCE_PAD_BUTTON_SQUARE;
    if (b[5] & 0x20u) btn |= SCE_PAD_BUTTON_CROSS;
    if (b[5] & 0x40u) btn |= SCE_PAD_BUTTON_CIRCLE;
    if (b[5] & 0x80u) btn |= SCE_PAD_BUTTON_TRIANGLE;

    /* b[6]: shoulders + select/start + stick clicks */
    if (b[6] & 0x01u) btn |= SCE_PAD_BUTTON_L1;
    if (b[6] & 0x02u) btn |= SCE_PAD_BUTTON_R1;
    if (b[6] & 0x04u) btn |= SCE_PAD_BUTTON_L2;
    if (b[6] & 0x08u) btn |= SCE_PAD_BUTTON_R2;
    if (b[6] & 0x10u) btn |= SCE_PAD_BUTTON_SHARE;    /* Share → Create */
    if (b[6] & 0x20u) btn |= SCE_PAD_BUTTON_OPTIONS;
    if (b[6] & 0x40u) btn |= SCE_PAD_BUTTON_L3;
    if (b[6] & 0x80u) btn |= SCE_PAD_BUTTON_R3;

    /* b[7]: PS + touchpad-click (low 2 bits) */
    if (b[7] & 0x01u) btn |= SCE_PAD_BUTTON_PS;
    if (b[7] & 0x02u) btn |= SCE_PAD_BUTTON_TOUCH_PAD;

    o->buttons   = btn;
    o->connected = 1;
    o->quat.w    = 1.0f;
}

int ds4_handle_packet(int fd, struct usb_fs_endpoint *eps,
                      const uint8_t *buf, uint32_t len,
                      ScePadData *out_pad) {
    (void)fd; (void)eps;

    /* DS4 USB uses report ID 0x01. Real DS4 sends 64 bytes; HORI third-party
     * pads (and XIM4) often truncate to ~27 bytes — bytes [1..9] are identical
     * so anything ≥ 10 bytes with [0]==0x01 is parseable. */
    if (len >= 10 && buf[0] == 0x01) {
        ds4_parse_input(buf, out_pad);
        return 1;
    }
    return 0;
}
