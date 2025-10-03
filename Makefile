CROSS_COMPILE := /home/studentas/Projects/crosstool/x-tools/arm-rpi4-linux-musleabihf/bin/arm-rpi4-linux-musleabihf-
ARCH := arm
KDIR := ../../linux/linux-torvalds

obj-m += lcd-driver.o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
