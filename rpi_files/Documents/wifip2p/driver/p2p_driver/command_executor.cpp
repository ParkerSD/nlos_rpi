#include "command_executor.h"
#include <iostream>
#include <string>

static const std::string START_GSTREAMER_STREAMER = "raspivid -t 0 -h 720 -w 1080 -fps 30 -hf -b 2000000 -o - -n | gst-launch-1.0 -v fdsrc ! h264parse ! x264enc ! video/x-h264, stream-format=byte-stream ! rtpmp4vpay config-interval=1 ! udpsink host=192.168.4.1 port=8998";

static const std::string START_RPI_CAM_STREAM = "raspivid -o - -t 0 -n -w 600 -h 400 -fps 50 | cvlc -vv stream:///dev/stdin --sout '#rtp{sdp=rtsp://:8998/}' :demux=h264 --h264-fps=50 --network-caching=1000 ---rtsp-caching=1000 --no-drop-late-frames --no-skip-frames --sout-transcode-hurry-up --clock-jitter=500 --clock-syncro=0";

void flashlight_toggle(bool value) {
    if (value) {
        std::cout << "TODO: Turn on flashlight" << std::endl;
    } else {
        std::cout << "TODO: Turn off flashlight" << std::endl;
    }
}

void left_led_toggle(bool value) {
    if (value) {
        std::cout << "TODO: Turn on left LED" << std::endl;
    } else {
        std::cout << "TODO: Turn off left LED" << std::endl;
    }
}

void right_led_toggle(bool value) {
    if (value) {
        std::cout << "TODO: Turn on right LED" << std::endl;
    } else {
        std::cout << "TODO: Turn off right LED" << std::endl;
    }
}

void run_stream() {
    std::cout << "Starting Stream:" << std::endl;
}

void stop_stream() {
    std::cout << "TODO: Implement stop stream command." << std::endl;
}

void terminate_execution() {
    std::cout << "TODO: Implement termination command." << std::endl;
}

