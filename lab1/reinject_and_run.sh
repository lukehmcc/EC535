#!/bin/bash

# Reinject the files into the image
mkdir -p /tmp/$USER-mnt
guestmount -a rootfs.img -m /dev/sda /tmp/$USER-mnt
cp -a --no-preserve=ownership $WORKSPACE/rootfs/* /tmp/$USER-mnt/
guestunmount /tmp/$USER-mnt/

# sleep so it doesn't freak out
sleep 1

# Run
qemu-system-arm -M virt -m 512 -kernel zImage -nographic \
  -drive file=rootfs.img,if=none,format=raw,id=rfs \
  -device virtio-blk-device,drive=rfs \
  -append "earlyprintk=serial root=/dev/vda"
