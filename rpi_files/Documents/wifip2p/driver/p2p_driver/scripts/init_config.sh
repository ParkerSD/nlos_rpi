#!/bin/bash
if [ "$EUID" -ne 0 ]; then
    echo "ERROR: Script must be ran as root (sudo)"
    exit
fi

if [ -z "$1" ]; then
    deviceName="NLOS_DEVICE"
    echo "no device name provided - using default of \"NLOS_DEVICE\""
else
    deviceName="$1";
fi

rm /etc/wpa_supplicant/wpa_supplicant-wlan0.conf
cat > /etc/wpa_supplicant/wpa_supplicant-wlan0.conf <<EOF
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=US
device_name=DIRECT-$deviceName
p2p_go_intent=15
device_type=6-0050F204-1
p2p_go_ht40=1
driver_param=p2p_device=6
config_methods=virtual_push_button
EOF
