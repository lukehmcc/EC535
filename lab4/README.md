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

## Running the code 
First run:
```bash
./reinject_and_run.sh
```

Then once in the machine you can run:
```bash
./go.sh
```

This will install the kernel module. Now that it is installed, you can 
simlulate the button presses by echoing:
```bash
echo 0 > /dev/mytraffic # simulates btn0
echo 1 > /dev/mytraffic # simulates btn1
```

> Note: These will be changed once the writeable character device is implmented

## Team

Members: Roger Brown, Luke McCarthy, Jackson Clary
