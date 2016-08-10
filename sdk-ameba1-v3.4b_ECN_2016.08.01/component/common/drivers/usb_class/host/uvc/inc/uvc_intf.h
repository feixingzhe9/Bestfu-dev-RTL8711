#ifndef _UVC_INTF_H_
#define _UVC_INTF_H_

enum uvc_format_type{
      UVC_FORMAT_MJPEG = 1,
      UVC_FORMAT_UNKNOWN = -1,
};

typedef enum uvc_format_type uvc_fmt_t;

struct uvc_context
{
      uvc_fmt_t fmt_type; //video format type
      int width;//video frame width
      int height;//video frame height
      int frame_rate;//video frame rate
};

struct uvc_buf_context
{
      int index; //index of internal uvc buffer
      unsigned char *data; //address of uvc data
      int len; //length of uvc data
};

int uvc_stream_init(void); //entry function to start uvc
void uvc_stream_free(struct stream_context *stream_ctx); // free streaming resources
int uvc_stream_on(struct stream_context *stream_ctx); //enable camera streaming
void uvc_stream_off(struct stream_context *stream_ctx); //disable camera streaming
int uvc_set_param(struct stream_context *stream_ctx, uvc_fmt_t fmt_type, int width, int height, int frame_rate);//set camera streaming video parameters:video format, resolution and frame rate.
int uvc_buf_check(struct uvc_buf_context *b); //check if uvc_buf_context is legal (return 0 is legal otherwise -1)
int uvc_dqbuf(struct stream_context *stream_ctx, struct uvc_buf_context *b); //dequeue internal buffer & get internal buffer info
int uvc_qbuf(struct stream_context *stream_ctx, struct uvc_buf_context *b); //queue internal buffer
int is_pure_thru_on(struct stream_context *stream_ctx);  //return 1 if pure throughput test mode is on otherwise return 0
void uvc_pure_thru_on(struct stream_context *stream_ctx); //turn on pure uvc throughput test mode (i.e. no decoding is involved)
void uvc_dec_thru_on(struct stream_context *stream_ctx); //turn on uvc throughput test mode with uvc payload decoding
void uvc_thru_off(struct stream_context *stream_ctx);    //turn off uvc throughput log service

#endif
