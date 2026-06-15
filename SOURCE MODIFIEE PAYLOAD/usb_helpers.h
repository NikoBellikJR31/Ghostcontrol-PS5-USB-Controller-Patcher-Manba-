#pragma once
#include <stdint.h>
#include <dev/usb/usb.h>
#include <dev/usb/usb_ioctl.h>

int  usb_send_out(int fd, struct usb_fs_endpoint *ep,
                  const uint8_t *data, uint32_t len, const char *tag);
int  usb_send_cmd(int fd, struct usb_fs_endpoint *ep, uint8_t a, uint8_t b);
