
#ifndef V4L2_DRIVER_H
#define V4L2_DRIVER_H											/*structure needed for video caputure*/

#include <platform/platform_stdlib.h>
#include "dlist.h"
#include "osdep_api.h"

typedef enum _streaming_state streaming_state;
enum _streaming_state {
    STREAMING_OFF = 0,
    STREAMING_ON = 1,
    STREAMING_PAUSED = 2,
};

/*ctrl_info used for ctrl exchange*/

typedef enum _ctrl_cmd ctrl_cmd;
enum _ctrl_cmd{
	CMD_START = 1,
	CMD_STOP,
	CMD_CHANGE,
	//MORE TO ADD
};

struct ctrl_info{
        ctrl_cmd cmd;
	int grabmethod; //only support streaming i/o(1) right now, not read i/o(0) nor userptr(2)
	int width;
	int height;
	int fps;
	int formatIn;
	int framesizeIn;
	streaming_state streamingState;
        _Mutex state_lock; //protect streamingState
};


struct vdCtrl {
	struct ctrl_info ctrlIn;
        void *streamCtx;        //streaming context 
        struct video_device *vdev; //video device context
	_Mutex ctrl_lock;   //for ctrlIn operation protection
	int quitsignal;
	struct list_head payload_entry;
	_Mutex entry_lock;   //for payload_entry protection
        _Sema sema;     //for queue processing protection
};


void stop_capturing (struct vdCtrl *videoCtrl);
int start_capturing (struct vdCtrl *videoCtrl);
void uninit_v4l2_device (struct vdCtrl *videoCtrl);
void init_read (unsigned int buffer_size);
void init_mmap (struct vdCtrl *videoCtrl, int n);
void init_userp (unsigned int buffer_size);
int init_v4l2_device (struct vdCtrl *videoCtrl);
int uvc_to_rtp_pt(u32 fmt_type);
int uvc_to_codec_id(u32 fmt_type);
void v4l2_probe(void *param);

#endif  //V4L2_DRIVER_H