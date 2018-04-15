kroot := /lib/modules/$(shell uname -r)/build
obj-m += foxtrot.o

modules:
	make -C $(kroot) M=$(PWD) modules
clean:
	make -C $(kroot) M=$(PWD) clean

dryrun:
	sudo insmod foxtrot.ko
	sudo rmmod foxtrot
	dmesg | tail -n 5
