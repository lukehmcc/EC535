#!/bin/bash

# copy the timer files
cd km && make && cd ..         # build kernel mod
cp km/mytraffic.ko rootfs/root # copy that kernel mod
cp go.sh rootfs/root           # copy the autorun script

# Reinject the files into the image
# dd if=/dev/zero of=rootfs.img bs=1M count=32
# mkfs.ext4 -L rootfs rootfs.img # Answer 'y' if asked to proceed
# mkdir -p /tmp/$USER-mnt
# guestmount -a rootfs.img -m /dev/sda /tmp/$USER-mnt
# cp -a --no-preserve=ownership /home/covalent/devStuff/school/EC535/lab4/rootfs/* /tmp/$USER-mnt/
# guestunmount /tmp/$USER-mnt/

# 1. create the image
dd if=/dev/zero of=rootfs.img bs=1M count=32
mkfs.ext4 -F -L rootfs rootfs.img # -F avoids the interactive prompt

# 2. attach the image to the first free loop device
LOOP=$(sudo losetup -f --show -P rootfs.img) # e.g. /dev/loop0
# if the image had partitions, they would appear as ${LOOP}p1, ${LOOP}p2…
# here we have only a filesystem in the whole device, so mount ${LOOP}

# 3. mount
sudo mkdir -p /tmp/$USER-mnt
sudo mount ${LOOP} /tmp/$USER-mnt

# 4. copy files
sudo cp -a /home/covalent/devStuff/school/EC535/lab4/rootfs/* /tmp/$USER-mnt/

# 5. clean up
sudo umount /tmp/$USER-mnt
sudo losetup -d ${LOOP}

# sleep so it doesn't freak out
sleep 1

# Run
qemu-system-arm -M virt -m 512 -kernel stock-zImage -nographic \
  -drive file=rootfs.img,if=none,format=raw,id=rfs \
  -device virtio-blk-device,drive=rfs \
  -append "earlyprintk=serial root=/dev/vda"
