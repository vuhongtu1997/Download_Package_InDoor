#!/bin/sh

read MAC </sys/class/net/eth0/address
uci set wireless.ap.ssid="RD_HC_$MAC"
uci set system.@system[0].hostname="RD_HC_$MAC"
uci set system.@system[0].zonename='Asia/Ho Chi Minh'
uci set system.@system[0].timezone='<+07>-7'
uci commit
reboot

