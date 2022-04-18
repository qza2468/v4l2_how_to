# OpenCV控制摄像机要点

*烦死了，之前写的那篇不见了，只能在写一次了*

之前用`v4l2`来控制相机，可以用来拍照，但问题是没办法保存为视频。所以就尝试了一下用`opencv`试试，优势十分明显：

1. 使用方便，效率更高

2. 封装了底层接口，可跨平台使用。`v4l2`只能在`linux`下使用

3. 可以方便的对视频编码解码

##### 概述

主要分为3步

1. 打开相机并配置

2. 设置输出格式

3. 循环IO并处理数据



下面就具体讲讲各个环节使用`opencv` 时要注意的一些点

###### 打开相机并配置

```c++
VideoCapture capture(2, CAP_V4L2);
```

第一个参数是/dev/video后的那个数字，表示要打开的设备

第二个参数很重要，表示底层控制方式，默认是`CAP_V4L`，**必须要设置为`CAP_V4L2`，否则无法设置相机捕获数据的格式**

```c++
    capture.set(CAP_PROP_FOURCC, VideoWriter::fourcc('M', 'J', 'P', 'G'));
    capture.set(CAP_PROP_FRAME_WIDTH, set_size.width);
    capture.set(CAP_PROP_FRAME_HEIGHT, set_size.height);
    capture.set(CAP_PROP_FPS, 60);

    cout << capture.get(CAP_PROP_FOURCC) << endl;
```

分别设置了相机捕获数据的格式，宽度，高度，fps.

**需要注意的是这四个值只能为特定的组合**，不能为任意值，具体组合可以通过

```bash
uvcdynctrl -d /dev/video2 -f
```

获取，大致为

```bash
Listing available frame formats for device /dev/video2:
Pixel format: MJPG (Motion-JPEG; MIME type: image/jpeg)
  Frame size: 1280x720
    Frame rates: 30
  Frame size: 960x540
    Frame rates: 30, 15
  Frame size: 848x480
    Frame rates: 30, 15
  Frame size: 640x480
    Frame rates: 30, 15
  Frame size: 640x360
    Frame rates: 30, 15
Pixel format: YUYV (YUYV 4:2:2; MIME type: video/x-raw-yuv)
  Frame size: 1280x720
    Frame rates: 10
  Frame size: 640x360
    Frame rates: 30
  Frame size: 424x240
    Frame rates: 30
  Frame size: 320x240
    Frame rates: 30
  Frame size: 320x180
    Frame rates: 30
  Frame size: 160x120
    Frame rates: 30
  Frame size: 640x480
    Frame rates: 30

```

设置后要`get`这些值来确保没有问题，已经设置成功。



###### 设置输出格式

```c++
    writer.open(video_file, VideoWriter::fourcc('M', 'J', 'P', 'G'), 60, actual_size, true);
```

参数分别为输出视频文件名，输出视频文件格式，输出视频fps,输出视频尺寸。



###### 循环IO并处理数据

就是简单的循环来获取数据，并保存，没有什么太多特别的地方。

**主要是不知道获取数据是不是阻塞的。**



###### CMake 要加上这些

```cmake
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(untitled1 main.cpp)

target_link_libraries(untitled1 ${OpenCV_LIBS})
```


