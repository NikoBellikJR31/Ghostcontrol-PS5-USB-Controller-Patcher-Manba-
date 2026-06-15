#include "usb_helpers.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifdef __PROSPERO__
#include <ps5/klog.h>
#define LOG(...) klog_printf("[GC] " __VA_ARGS__)
#else
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#endif

int usb_send_out(int fd, struct usb_fs_endpoint *ep,
                 const uint8_t *data, uint32_t len, const char *tag) {
    void *bufs[1] = { (void *)data };
    uint32_t lens[1] = { len };
    struct usb_fs_start start;
    struct usb_fs_complete complete;

    ep->ppBuffer = bufs;
    ep->pLength  = lens;
    ep->nFrames  = 1;
    ep->timeout  = 150;
    ep->flags    = 0;
    ep->aFrames  = 0;
    ep->status   = 0;

    memset(&start, 0, sizeof(start));
    start.ep_index = 1;
    if (ioctl(fd, USB_FS_START, &start) != 0) {
        LOG("OUT %s START fail errno=%d\n", tag, errno);
        return -errno;
    }
    for (int w = 0; w < 20; w++) {
        memset(&complete, 0, sizeof(complete));
        complete.ep_index = 1;
        if (ioctl(fd, USB_FS_COMPLETE, &complete) == 0)
            return 0;
        if (errno != EBUSY) {
            LOG("OUT %s COMPLETE fail errno=%d\n", tag, errno);
            return -errno;
        }
        usleep(50000);
    }
    LOG("OUT %s timeout\n", tag);
    return -EBUSY;
}

int usb_send_cmd(int fd, struct usb_fs_endpoint *ep, uint8_t a, uint8_t b) {
    uint8_t buf[2] = { a, b };
    char tag[8];
    snprintf(tag, sizeof(tag), "%02x%02x", a, b);
    return usb_send_out(fd, ep, buf, 2, tag);
}
