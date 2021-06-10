#!/bin/bash
if [ "$EUID" -ne 0 ]; then
    echo "ERROR: Script must be ran as root (sudo)"
    exit
fi

cat > /etc/systemd/network/12-p2p-wlan0.network <<EOF
[Match]
Name=p2p-wlan0-*
[Network]
Address=192.168.4.2/24
DHCPServer=yes
EOF
