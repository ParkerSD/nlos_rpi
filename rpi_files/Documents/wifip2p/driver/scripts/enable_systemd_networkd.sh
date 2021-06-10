#!/bin/bash
if [ "$EUID" -ne 0 ]; then
    echo "ERROR: Script must be ran as root (sudo)"
    exit
fi

#Uninstall classic networking -----------------------
#- Remove and mark hold classic networking, remove folders.
apt --autoremove purge ifupdown dhcpcd5 isc-dhcp-client isc-dhcp-common rsyslog

apt-mark hold ifupdown dhcpcd5 isc-dhcp-client isc-dhcp-common rsyslog raspberrypi-net-mods openresolv

rm -r /etc/network /etc/dhcp

#Setup/enable systemd-resolved and systemd-networkd
apt --autoremove purge avahi-daemon
apt-mark hold avahi-daemon libnss-mdns
apt install libnss-resolve
ln -sf /run/systemd/resolve/stub-resolv.conf /etc/resolv.conf
systemctl enable systemd-networkd.service systemd-resolved.service
