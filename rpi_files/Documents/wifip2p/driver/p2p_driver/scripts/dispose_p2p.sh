#!/bin/bash
echo "Ending all p2p actions." 
echo "Removing groups (may fail if no groups exist)" 
wpa_cli -i p2p-dev-wlan0 p2p_group_remove $(ip -br link | grep -Po 'p2p-wlan0-\d+')
echo "Flushing p2p tables"
wpa_cli -i p2p-dev-wlan0 p2p_flush
echo "Stopping p2p find"
wpa_cli -i p2p-dev-wlan0 p2p_stop_find
