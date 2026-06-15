#pragma once
#include <stdint.h>
#include <dev/usb/usb.h>
#include <dev/usb/usb_ioctl.h>
#include "gc_types.h"

/*
 * Xbox One (VID=0x045E PID=0x02EA) — GIP protocol over USB
 *
 * Endpoints: IN=0x82 OUT=0x02 (interface 1, FS speed, maxpkt=64)
 *
 * GIP input report format (18 bytes total):
 *   [0]=0x20 cmd  [1]=seq  [2]=opts  [3]=0x0E (payload=14)
 *   [4..5] = buttons uint16 LE
 *   [6..7] = LT uint16 (0-1023)
 *   [8..9] = RT uint16 (0-1023)
 *   [10..11]= LX int16 (center=0)
 *   [12..13]= LY int16
 *   [14..15]= RX int16
 *   [16..17]= RY int16
 *
 * GIP wire button layout (from Linux xpad.c, xpad_process_packet_xboxone):
 *   b[4] GIP_BTN1: bit2=View(→Create) bit3=Menu(→Options)
 *                  bit4=A(→Cross) bit5=B(→Circle) bit6=X(→Square) bit7=Y(→Triangle)
 *   b[5] GIP_BTN2: bit0=DUp bit1=DDn bit2=DLt bit3=DRt
 *                  bit4=LB(→L1) bit5=RB(→R1) bit6=LS(→L3) bit7=RS(→R3)
 * NOTE: GIP wire bits differ from XInput wButtons constants (XInput driver remaps).
 * Guide arrives as separate GIP cmd=0x07 packet.
 */

#define XBOX_EP_IN   0x82
#define XBOX_EP_OUT  0x02

/* GIP command bytes */
#define GIP_CMD_ACK      0x01
#define GIP_CMD_ANNOUNCE 0x02
#define GIP_CMD_STATUS   0x03
#define GIP_CMD_INPUT    0x20

/* Parse GIP input report into ScePadData */
void xbox_parse_input(const uint8_t *buf, ScePadData *o);

/* GIP handshake: catch ANNOUNCE → ACK → POWER.
 * Call immediately after opening IN+OUT endpoints.
 * eps[0]=IN eps[1]=OUT. */
void xbox_gip_handshake(int fd, struct usb_fs_endpoint *eps);

/* Handle one IN packet. Returns 1 if pad updated, 0 to skip, -1 to reinit. */
int  xbox_handle_packet(int fd, struct usb_fs_endpoint *eps,
                        const uint8_t *buf, uint32_t len,
                        ScePadData *out_pad);
