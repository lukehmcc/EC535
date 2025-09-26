#!/bin/bash

# copy the timer files
cd km && make && cd ..       # build kernel mod
cd ul && make && cd ..       # build user
cp km/mytimer.ko rootfs/root # copy that kernel mod
cp ul/ktimer rootfs/root     # copy that user mod

# Reinject the files into the image
dd if=/dev/zero of=rootfs.img bs=1M count=32
mkfs.ext4 -L rootfs rootfs.img # Answer 'y' if asked to proceed
mkdir -p /tmp/$USER-mnt
guestmount -a rootfs.img -m /dev/sda /tmp/$USER-mnt
cp -a --no-preserve=ownership /home/lukehmcc/classes/EC535/lab2/rootfs/* /tmp/$USER-mnt/
guestunmount /tmp/$USER-mnt/

# sleep so it doesn't freak out
sleep 1

# Run
qemu-system-arm -M virt -m 512 -kernel stock-zImage -nographic \
  -drive file=rootfs.img,if=none,format=raw,id=rfs \
  -device virtio-blk-device,drive=rfs \
  -append "earlyprintk=serial root=/dev/vda"
