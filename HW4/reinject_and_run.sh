#!/bin/bash

# 1. Build steps
cd km && make && cd ..       # build kernel mod
cd ul && make && cd ..       # build user
cp km/mytimer.ko rootfs/root # copy that kernel mod
cp ul/ktimer rootfs/root     # copy that user mod
cp go.sh rootfs/root         # copy the autorun script

# 2. Safety cleanup before starting image manipulation
# This prevents the "guestunmount not mounted" error and clears old locks
if mountpoint -q /tmp/$USER-mnt; then
  guestunmount /tmp/$USER-mnt
fi
# Force kill any lingering guestmount processes causing locks
pkill guestmount
sleep 1

# 3. Create and Populate Image
dd if=/dev/zero of=rootfs.img bs=1M count=256
mkfs.ext4 -L rootfs rootfs.img # Answer 'y' if asked to proceed (can use -F to force)
mkdir -p /tmp/$USER-mnt

# Mount
guestmount -a rootfs.img -m /dev/sda /tmp/$USER-mnt

# Copy files
cp -a --no-preserve=ownership /home/lukehmcc/classes/EC535/HW4/rootfs/* /tmp/$USER-mnt/

# Unmount
guestunmount /tmp/$USER-mnt/

# 4. Critical Wait Step
# Wait for disk writes to finish and ensure the file lock is released
sync
sleep 2

# 5. Run QEMU
qemu-system-arm -M virt -m 512 -kernel stock-zImage -nographic \
  -drive file=rootfs.img,if=none,format=raw,id=rfs \
  -device virtio-blk-device,drive=rfs \
  -append "earlyprintk=serial root=/dev/vda"
