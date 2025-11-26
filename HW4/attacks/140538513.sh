#!/bin/sh
# 140538513.sh – race-to-UAF
mknod /dev/mytimer c 61 0 2>/dev/null
cd /root/hw4_targets/140538513/
insmod mytimer.ko 2>/dev/null
for i in $(seq 10); do
  while true; do
    ./ktimer -s 0 "race" >/dev/null
    ./ktimer -r >/dev/null
  done &
done
wait
