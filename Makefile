# 根目录 Makefile
SUBDIRS = driver server tests
# 默认给交叉编译工具链一个路径，别人可以根据自己的环境覆盖它
CROSS_COMPILE ?=arm-buildroot-linux-gnueabihf-g
KERNELDIR ?=/home/book/100ask_imx6ull-sdk/Buildroot_2020.02.x/output/build/linux-origin_master

export CROSS_COMPILE KERNELDIR

all:
	@for dir in $(SUBDIRS); do $(MAKE) -C $$dir; done

clean:
	@for dir in $(SUBDIRS); do $(MAKE) -C $$dir clean; done

.PHONY: all clean