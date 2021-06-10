#!/bin/bash
#remove all current groups we are owners of
echo "Removing all existing p2p groups (may fail if no groups exist)"
wpa_cli -i p2p-dev-wlan0 p2p_group_remove $(ip -br link | grep -Po 'p2p-wlan0-\d+')
#flush all p2p tables
echo "flushing p2p tables"
wpa_cli -i p2p-dev-wlan0 p2p_flush
#advertise as p2p owner
echo "starting p2p_find" 
wpa_cli -i p2p-dev-wlan0 p2p_find
