MODULE=reveller

obj-m += reveller.o

compile:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
