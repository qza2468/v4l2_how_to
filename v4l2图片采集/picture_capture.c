//
// Created by qza2468 on 2022/4/13.
//


#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>


char *picture_name = "pic_capture_test%d.jpg";
char *video_name = "video_capture_test.jpg";
char *device_path = "/dev/video2";

typedef struct {
    void *start;
    size_t length;
} BufType;


BufType  *usr_buf;
static unsigned int n_buffer = 0;

int open_camera() {
    int fd = open(device_path, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "%s open err \n", device_path);
        exit(EXIT_FAILURE);
    }

    return fd;
}

int init_mmap(int fd) {
    struct v4l2_requestbuffers reqBuffers;
    reqBuffers.count = 4;
    reqBuffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqBuffers.memory = V4L2_MEMORY_MMAP;

    if ((ioctl(fd, VIDIOC_REQBUFS, &reqBuffers)) == -1) {
        fprintf(stderr, "Fail to ioctl `VIDIOC_REQBUFS`");
        exit(EXIT_FAILURE);
    }

    n_buffer = reqBuffers.count;
    printf("n_buffer = %d\n", n_buffer);

    usr_buf = calloc(reqBuffers.count, sizeof(BufType));
    if (usr_buf == NULL) {
        printf("Out of memory\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < reqBuffers.count; ++i) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
            perror("Fail to ioctl : VIDIOC_QUERYBUF");
            exit(EXIT_FAILURE);
        }

        usr_buf[i].length = buf.length;
        // TODO: there is the point, mmap to the device `fd` with offset. so the different addresses are point to the same content.
        usr_buf[i].start = (char *)mmap(NULL,buf.length,PROT_READ | PROT_WRITE,MAP_SHARED, fd,buf.m.offset);

        if (usr_buf[i].start == MAP_FAILED) {
            perror("Fail to mmap");
            exit(EXIT_FAILURE);
        }
    }
}

int init_camera(int fd) {
    struct v4l2_capability cap;
    struct v4l2_format fmt;
    struct v4l2_fmtdesc fmtdesc;
    struct v4l2_control ctrl;
    int res;

    memset(&fmtdesc, 0, sizeof(fmtdesc));
    fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if ((res = ioctl(fd, VIDIOC_QUERYCAP, &cap)) < 0) {
        fprintf(stderr, "fail to ioctl VIDEO_QUERYCAP \n");
        exit(EXIT_FAILURE);
    }

    printf("\ncamera driver name is : %s\n",cap.driver);
    printf("camera device name is : %s\n",cap.card);
    printf("camera bus information: %s\n",cap.bus_info);

    if (!(cap.capabilities & V4L2_BUF_TYPE_VIDEO_CAPTURE)) {
        fprintf(stderr, "The current device is not a video capture device \n");
    } else {
        printf("The current device is a video capture device \n");
    }

    if(!(cap.capabilities & V4L2_CAP_STREAMING))
    {
        printf("The Current device does not support streaming io\n");
    } else {
        printf("The Current device does support streaming io\n");
    }

    while(ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc)!=-1)
    {
        printf("support format %d.%s\n", fmtdesc.index+1, fmtdesc.description);
        fmtdesc.index++;
    }

    /*set the form of camera capture data*/
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;      /*v4l2_buf_typea,camera must use V4L2_BUF_TYPE_VIDEO_CAPTURE*/
    fmt.fmt.pix.width = 640;
    fmt.fmt.pix.height = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;	/*V4L2_PIX_FMT_YYUV*/
    fmt.fmt.pix.field = V4L2_FIELD_NONE;   		/*V4L2_FIELD_NONE*/
    if (ioctl(fd, VIDIOC_S_FMT, &fmt)< 0)
    {
        fprintf(stderr,"VIDIOC_S_FMT set err\n");
        exit(EXIT_FAILURE);
    }

    init_mmap(fd);
}

int start_capture(int fd) {
    enum v4l2_buf_type type;

    for (int i = 0; i < n_buffer; i++) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(fd, VIDIOC_QBUF, &buf)) {
            perror("Fail to ioctl `VIDIOC_QBUF`");
            exit(EXIT_FAILURE);
        }
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1) {
        perror("Fail to ioctl `VIDIOC_STREAMON`");
        exit(EXIT_FAILURE);
    }
}


int process_image(void *addr, size_t length)
{
    FILE *fp;

    static int num = 0;

    char image_name[32];
    sprintf(image_name, picture_name, num++);
    if((fp = fopen(image_name, "w")) == NULL)
    {
        perror("Fail to fopen");
        exit(EXIT_FAILURE);
    }
    fwrite(addr, length, 1, fp);
    fclose(fp);
    return 0;
}

int read_frame(int fd) {
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_DQBUF, &buf)) {
        perror("Fail to ioctl `VIDIOC_DQBUF");
        exit(EXIT_FAILURE);
    }

    process_image(usr_buf[buf.index].start, usr_buf[buf.index].length);
    if (-1 == ioctl(fd, VIDIOC_QBUF, &buf)) {
        perror("Fail to ioctl `VIDIOC_QBUF");
        exit(EXIT_FAILURE);
    }

    return 1;
}

int mainloop(int fd) {
    int count = 10;
    while (count-- > 0) {
        while(1) {
            fd_set fds;
            struct timeval tv;
            int res;

            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            tv.tv_sec = 2;
            tv.tv_usec = 0;
            res = select(fd + 1, &fds, NULL, NULL, &tv);
            if (res == -1) {
                if (EINTR == errno) {
                    continue;
                }

                perror("select error");
                exit(EXIT_FAILURE);
            } else if (res == 0) {
                fprintf(stderr, "select Timeout\n");
                exit(EXIT_FAILURE);
            }

            if (read_frame(fd)) {
                break;
            }
        }
    }
}

int main (void) {
    int fd;
    fd = open_camera();
    init_camera(fd);
    start_capture(fd);
    mainloop(fd);
}

