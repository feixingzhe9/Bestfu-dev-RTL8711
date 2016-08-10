#ifndef UVC_V4L2_H
#define UVC_V4L2_H

#define UVC_V4L2_EN 1

#if UVC_V4L2_EN



int uvc_v4l2_open(void);
int uvc_v4l2_release(void);
long uvc_v4l2_ioctl(unsigned int cmd, unsigned long arg);
int uvc_v4l2_mmap(struct v4l2_buffer * buf);

#else
int v4l2_open(void);
int v4l2_release(void);
long v4l2_ioctl(unsigned int cmd, unsigned long arg);
int v4l2_mmap(void);
#endif

#endif