#pragma once
#include <stdint.h>
#include <dev/usb/usb.h>
#include <dev/usb/usb_ioctl.h>
#include "gc_types.h"

/*
 * Manba V2 NBJr support.
 *
 * PC cable mode exposes a classic Xbox 360/XInput USB shape:
 *   VID:PID              045e:028e
 *   receiver idle mode   1a34:f517 (ignored while the controller is off)
 *   interface subclass   0x5d
 *   interface protocol   0x01
 *   IN endpoint          0x81, 20-byte reports
 *   OUT endpoint         0x02 or 0x01
 *
 * Switch USB/dongle mode usually exposes Nintendo Switch Pro VID:PID:
 *   VID:PID              057e:2009
 *   IN endpoint          0x81, 64-byte reports
 */

#define MAMBA_XINPUT_VID        0x045eu
#define MAMBA_XINPUT_PID        0x028eu
#define MAMBA_DONGLE_VID        0x1a34u
#define MAMBA_DONGLE_PID        0xf517u
#define MAMBA_XINPUT_SUBCLASS   0x5du
#define MAMBA_XINPUT_PROTOCOL   0x01u

#define MAMBA_SWITCH_VID        0x057eu
#define MAMBA_SWITCH_PID        0x2009u

#define MAMBA_XINPUT_EP_IN      0x81u
#define MAMBA_XINPUT_EP_OUT     0x02u
#define MAMBA_XINPUT_EP_OUT_ALT 0x01u

int mamba_is_xinput_vidpid(uint16_t vid, uint16_t pid);
int mamba_is_switch_vidpid(uint16_t vid, uint16_t pid);
int mamba_is_supported_vidpid(uint16_t vid, uint16_t pid);
int mamba_is_xinput_interface(uint8_t subclass, uint8_t protocol);

const char *mamba_name(uint16_t vid, uint16_t pid);

void mamba_log_switch_packet(const uint8_t *buf, uint32_t len);

void mamba_xinput_parse_input(const uint8_t *buf, ScePadData *out_pad);
void mamba_xinput_send_enable(int fd, struct usb_fs_endpoint *eps);

int mamba_xinput_handle_packet(int fd, struct usb_fs_endpoint *eps,
                               const uint8_t *buf, uint32_t len,
                               ScePadData *out_pad);
