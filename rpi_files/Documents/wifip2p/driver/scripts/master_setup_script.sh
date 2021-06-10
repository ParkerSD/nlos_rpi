#!/bin/bash
if [ "$EUID" -ne 0 ]; then
    echo "ERROR: Script must be ran as root (sudo)"
    exit
fi

#Device name for WiFi P2P search. Can be supplied as first argument to script. 
if [ -z "$1" ]; then
    deviceName="NLOS_DEVICE"
    echo "no device name provided - using default of \"NLOS_DEVICE\""
else
    deviceName="$1";
fi

bash enable_systemd_networkd.sh
bash init_wpa_supplicant_config.sh $deviceName
bash enable_wpa_supplicant.sh
bash init_dhcp_server.sh
echo "Reboot device to apply changes." 
