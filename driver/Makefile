obj-m := ap3216c.o

BOARD_KERNELDIR := /lib/modules/$(shell uname -r)/build
VIRTUAL_KERNELDIR := /home/book/100ask_imx6ull-sdk/Buildroot_2020.02.x/output/build/linux-origin_master

KERNELDIR ?= $(if $(wildcard $(VIRTUAL_KERNELDIR)),$(VIRTUAL_KERNELDIR),$(BOARD_KERNELDIRR))

CURRENT_PATH := $(shell pwd)
all:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
