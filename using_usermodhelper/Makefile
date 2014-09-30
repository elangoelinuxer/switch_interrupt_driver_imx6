obj-m += switch_interrupt.o

all:
	make -C /usr/src/linux-headers-2.6.32-53-generic/  M=$(PWD) modules
clean:
	make -C /usr/src/linux-headers-2.6.32-53-generic/  M=$(PWD) clean
