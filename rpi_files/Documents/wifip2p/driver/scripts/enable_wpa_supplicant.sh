#!/bin/bash
if [ "$EUID" -ne 0 ]; then
    echo "ERROR: Script must be ran as root (sudo)"
    exit
fi

chmod 600 /etc/wpa_supplicant/wpa_supplicant-wlan0.conf
systemctl disable wpa_supplicant.service
systemctl enable wpa_supplicant@wlan0.service
rfkill unblock wlan
