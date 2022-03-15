#include <opencv2/opencv.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#ifndef VID_WIDTH
#define VID_WIDTH 1280
#endif

#ifndef VID_HEIGHT
#define VID_HEIGHT 720
#endif

static void usage(const char *argv0)
{
    fprintf(stderr, "Usage: %s input_device output_1 output_2 .... \n", argv0);
}

int main(int argc, char* argv[]) {

    if (argc < 3) {
        usage(argv[0]);
        return -1;
    }

    char* input_device = argv[1];
    
    int n_outputs = argc - 2;
    int* output_devices_fd = (int*)malloc(n_outputs * sizeof(int));
    if (output_devices_fd == NULL) {
        fprintf(stderr, "Failed to allocate memory for output devices\n");
        return -1;
    }

    // open and configure input camera (/dev/video0)
    cv::VideoCapture cam(input_device);
    if (not cam.isOpened()) {
        std::cerr << "ERROR: could not open camera!\n";
        return -1;
    }
    cam.set(cv::CAP_PROP_FRAME_WIDTH, VID_WIDTH);
    cam.set(cv::CAP_PROP_FRAME_HEIGHT, VID_HEIGHT);

    size_t framesize = VID_WIDTH * VID_HEIGHT * 3 * 3; // 3 bytes for each channel RGB

    // open output device
    for (int i = 0; i < n_outputs; i++)
    {
        output_devices_fd[i] = open(argv[i+2], O_RDWR);
        if(output_devices_fd[i] < 0) {
            fprintf(stderr, "ERROR: could not open output device %s!\n", argv[i+2]);
            return -2;
        }

        // configure params for output device
        struct v4l2_format vid_format;
        memset(&vid_format, 0, sizeof(vid_format));
        vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

        if (ioctl(output_devices_fd[i], VIDIOC_G_FMT, &vid_format) < 0) {
            fprintf(stderr, "ERROR: unable to get video format!\n");
            return -1;
        }

        vid_format.fmt.pix.width = cam.get(cv::CAP_PROP_FRAME_WIDTH);
        vid_format.fmt.pix.height = cam.get(cv::CAP_PROP_FRAME_HEIGHT);

        // NOTE: change this according to below filters...
        // Chose one from the supported formats on Chrome:
        // - V4L2_PIX_FMT_YUV420,
        // - V4L2_PIX_FMT_Y16,
        // - V4L2_PIX_FMT_Z16,
        // - V4L2_PIX_FMT_INVZ,
        // - V4L2_PIX_FMT_YUYV,
        // - V4L2_PIX_FMT_RGB24,
        // - V4L2_PIX_FMT_MJPEG,
        // - V4L2_PIX_FMT_JPEG
        vid_format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;

        vid_format.fmt.pix.sizeimage = framesize;
        vid_format.fmt.pix.field = V4L2_FIELD_NONE;

        if (ioctl(output_devices_fd[i], VIDIOC_S_FMT, &vid_format) < 0) {
            fprintf(stderr, "ERROR: unable to set video format!\n");
            return -1;
        }
    }

    // loop over these actions:
    while (true) {

        // grab frame
        cv::Mat frame;
        if (not cam.grab()) {
            std::cerr << "ERROR: could not read from camera!\n";
            break;
        }
        cam.retrieve(frame);

        // apply simple filter (NOTE: result should be as defined PIXEL FORMAT)
        // convert twice because we need RGB24
        cv::Mat result;
        cv::cvtColor(frame, result, cv::COLOR_RGB2GRAY);
        cv::cvtColor(result, result, cv::COLOR_GRAY2RGB);

        // show frame
        // cv::imshow("input", frame);

	std::vector<uchar> outbuffer;
        cv::imencode(".jpg", frame, outbuffer);

        for (int i = 0; i < n_outputs; i++)
        {   
            // write frame to output device
            size_t written = write(output_devices_fd[i], outbuffer.data(), framesize);
            if (written < 0) {
                std::cerr << "ERROR: could not write to output device!\n";
                close(output_devices_fd[i]);
                break;
            }
        }

        // wait for user to finish program pressing ESC
        if (cv::waitKey(10) == 27)
            break;
    }

    std::cout << "\n\nFinish, bye!\n";
}

