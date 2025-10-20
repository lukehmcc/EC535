# Lab 4

## Goal

Build a "Traffic Light" controller board using the BeagleBone Black,
GPIO, and a kernel module.

## Build instructions

In order to build the kernel module, you need the kernel. This is NOT
included in this repo for space reasons (a build kernel takes up ~1GB).

The make file in `km/` will look for `$(EC535_KERNEL)`.
So to ensure it exists, put something like this in your `.bashrc/.zshenv`:

```bash
export EC535_KERNEL="/path/to/kernel/directory"
```

 > On the lab machines the correct path is:
 > `/ad/eng/courses/ec/ec535/bbb/stock/stock-linux-4.19.82-ti-rt-r33`

Then you can build:

```bash
cd km/
make
```

## Team

Members: Roger Brown, Luke McCarthy, Jackson Clary
