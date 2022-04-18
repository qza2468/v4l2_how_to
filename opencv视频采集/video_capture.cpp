//
// Created by qza2468 on 22-4-16.
//

#include <iostream>
#include "opencv4/opencv2/opencv.hpp"

using namespace cv;
using namespace std;

const int DEVICE_INDEX = 2;

// TODO: there is a big point, we can only set size that the device support, otherwise it will error.
// so how to know what resolution you camera is suitable, thanks to https://stackoverflow.com/questions/57660458/cant-set-frame-width-and-height-with-opencv-cv2-videocapture-set
// `uvcdynctrl -d /dev/video2 -f` is ok
// this device supports the following size
// Frame size: 1280x720
//    Frame rates: 60, 30
//  Frame size: 640x480
//    Frame rates: 60, 30
//  Frame size: 1920x1080
//    Frame rates: 60, 30
const Size set_size(1920, 1080);
Size actual_size;

const char *video_file = "aaa.avi";
const int output_video_fps = 10;

const int how_many_frames = 1000;

int main() {
    std::cout << "Hello, World!" << std::endl;

    // TODO: it's another big point, you should set CAP_V4L2
    // otherwise the opencv can't set fourcc for the camera. !!!!
    VideoCapture capture(2, CAP_V4L2);

    cout << "opening the camera" << endl;
    if (!capture.isOpened()) {
        cout << "can't open camera device " << DEVICE_INDEX << endl;
        return -1;
    }

    cout << "setting the width to " << set_size.width << ", "
         << "height to" << set_size.height << endl;






    actual_size.height = capture.get(CAP_PROP_FRAME_HEIGHT);
    actual_size.width = capture.get(CAP_PROP_FRAME_WIDTH);
    cout << "codec" << int(capture.get(CAP_PROP_FOURCC)) << endl;

    cout << "final camera width: " << capture.get(CAP_PROP_FRAME_HEIGHT) << ", "
         << "height: " << actual_size.height << endl;
    cout << "camera fps: " << capture.get(CAP_PROP_FPS);

    cout << "video preserve to " << video_file << endl;
    VideoWriter writer;
    writer.open(video_file, VideoWriter::fourcc('M', 'J', 'P', 'G'), 60, actual_size, true);

    namedWindow("output");

    Mat frame;
    int i = how_many_frames;
    while (capture.read(frame)) {
        cout << i << endl;
        imshow("output", frame);
        writer.write(frame);
        if (i-- == 0) {
            break;
        }
    }

    capture.release();
    return 0;

}
