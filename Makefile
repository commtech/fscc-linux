obj-m 	:= fscc.o
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)
IGNORE	:=
fscc-objs := src/main.o src/port.o src/card.o src/isr.o src/utils.o \
             src/frame.o src/sysfs.o src/descriptor.o src/debug.o src/stream.o

ifeq ($(DEBUG),1)
	EXTRA_CFLAGS += -DDEBUG
endif
             
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	
headers_install:
	mkdir -p /usr/local/include/fscc
	cp include/fscc.h /usr/local/include/fscc/

headers_remove:
	rm -rf /usr/local/include/fscc

clean:
	@find . $(IGNORE) \
	\( -name '*.[oas]' -o -name '*.ko' -o -name '.*.cmd' \
		-o -name '.*.d' -o -name '.*.tmp' -o -name '*.mod.c' \
		-o -name '*.markers' -o -name '*.symvers' -o -name '*.order' \
		-o -name '*.tmp_versions' \) \
		-type f -print | xargs rm -f

help:
	@echo 'Build targets:'
	@echo '  make clean - Remove most generated files'
	@echo '  make all - Build driver module'
	@echo '  make headers_install - Copy fscc header file to /usr/local/include/fscc'
	@echo '  make headers_remove - Remove fscc header file from /usr/loca/linclude/fscc'

