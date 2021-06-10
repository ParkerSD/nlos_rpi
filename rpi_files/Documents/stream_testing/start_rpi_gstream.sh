#!bin/bash
raspivid -t 0 -h 720 -w 1080 -fps 30 -hf -b 2000000 -o - -n | gst-launch-1.0 -vvv fdsrc ! h264parse ! rtph264pay config-interval=10 pt=96 ! udpsink host=192.168.4.1 port=8998
