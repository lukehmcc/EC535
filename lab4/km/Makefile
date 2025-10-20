ifneq ($(KERNELRELEASE),)
	obj-m := mytraffic.o
else
	KERNELDIR := $(EC535_KERNEL)
	PWD := $(shell pwd)
	ARCH := arm
	# Dynamically set CROSS based on available compiler
	ifndef CROSS
		ifeq ($(shell which arm-linux-gnueabihf-gcc >/dev/null 2>&1; echo $$?),0)
			CROSS := arm-linux-gnueabihf-
		else ifeq ($(shell which arm-linux-gnu-gcc >/dev/null 2>&1; echo $$?),0)
			CROSS := arm-linux-gnu-
		else
			$(error No compatible ARM cross-compiler found. Install arm-linux-gnueabihf-gcc or arm-linux-gnu-gcc.)
		endif
	endif

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=$(ARCH) clean

endif
