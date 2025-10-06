#!/bin/sh
# fasync_example demonstration
# Originally by Matthew Yee
# Updated for Linux 4.19 by Anthony Byrne

# Make the device node
mknod /dev/mytimer c 61 0
# Load the kernel module
insmod mytimer.ko
# Run the userland tester
./ktimer
