
  
PLATFORM := "am335x-evm"  
MACHINE_NAME := "am335x"  
  
# If CROSS_COMPILE is not set by Rules.make then set a sane default  
CC = arm-linux-gnueabihf-gcc 
export CC
#obj-m 表示编译成模块
obj-m := gpio_drv.o

gpio_drv-objs  :=  gpio_irq_test.o

KERNEL_DIR := /root/Desktop/01-code/5Gluce/335_interrupt/linux3.2-cm335h
MAKE_ENV = ARCH=arm  
  
PWD := $(shell pwd)  
all:  
	$(MAKE) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" -C $(KERNEL_DIR) $(MAKE_ENV)  M=$(PWD) modules  
  
clean:  
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions *.symvers  


