obj-m 	:= fscc.o
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)
IGNORE	:=
fscc-objs := src/main.o src/port.o src/card.o src/isr.o src/utils.o \
             src/frame.o src/sysfs.o src/descriptor.o src/debug.o \
             src/flist.o

ifeq ($(DEBUG),1)
	EXTRA_CFLAGS += -DDEBUG
endif
             
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	
install:
	cp fscc.rules /etc/udev/rules.d/
	cp fscc.ko /lib/modules/`uname -r`/kernel/drivers/char/
	depmod
	mkdir -p /usr/local/include/fscc
	cp include/fscc.h /usr/local/include/fscc/
	cd lib/python/; python setup.py install
	cd cli/; python setup.py install
	
uninstall:
	rm /lib/modules/`uname -r`/kernel/drivers/char/fscc.ko
	depmod
	rm -rf /usr/local/include/fscc
	rm /etc/udev/rules.d/fscc.rules

clean:
	@find . $(IGNORE) \
	\( -name '*.[oas]' -o -name '*.ko' -o -name '.*.cmd' \
		-o -name '.*.d' -o -name '.*.tmp' -o -name '*.mod.c' \
		-o -name '*.markers' -o -name '*.symvers' -o -name '*.order' \
		-o -name '*.tmp_versions' \) \
		-type f -print | xargs rm -f
	rm -rf .tmp_versions

rules:
	cp fscc.rules /etc/udev/rules.d/

help:
	@echo
	@echo 'Build targets:'
	@echo '  make - Build driver module'
	@echo '  make clean - Remove most generated files'
	@echo '  make install - Copy fscc driver and header files to /lib/modules/`uname -r`/kernel/drivers/char/ and /usr/local/include/fscc respectively'
	@echo '  make uninstall - Remove fscc driver and header files from their installed directories'
	@echo '  make rules - Copy fscc.rules file to your /etc/udev/rules.d directory'
	@echo
