#!/bin/bash
if [ "$EUID" -ne 0 ]; then
    echo "ERROR: Script must be ran as root (sudo)"
    exit
fi

apt-get install emacs codeblocks nmap gstreamer1.0-tools
