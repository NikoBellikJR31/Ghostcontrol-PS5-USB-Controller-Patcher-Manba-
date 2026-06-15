#pragma once
#include <stdint.h>
#include <dev/usb/usb.h>
#include <dev/usb/usb_ioctl.h>
#include "gc_types.h"

/* Handshake states */
#define HS_WAIT_81_01  0
#define HS_WAIT_81_02  1
#define HS_STREAMING   2

void nintendo_parse_0x30(const uint8_t *b, ScePadData *o);
void nintendo_parse_0x3f(const uint8_t *b, ScePadData *o);

/* Send Nintendo subcommand on OUT ep (eps[1]).
 * seq: rolling counter, incremented per call. */
int  nintendo_send_subcmd(int fd, struct usb_fs_endpoint *eps,
                          uint8_t *seq, uint8_t subcmd,
                          const uint8_t *data, uint32_t data_len);

/* Handle one IN packet in the Nintendo state machine.
 * Returns 1 if pad was updated and should be injected, 0 otherwise.
 * hs_state and seq are in/out: updated by the function. */
int  nintendo_handle_packet(int fd, struct usb_fs_endpoint *eps,
                            const uint8_t *buf, uint32_t len,
                            int *hs_state, uint8_t *seq,
                            ScePadData *out_pad);
