obj-m := mychar.o
PWD := $(shell pwd)
PREFIX := /home/xinlin/sources
SRC := $(PREFIX)/linux-5.14.14
CC := gcc
CFLAGS := -Wall -Wextra -I$(PREFIX)/linux-5.14.14/include


# make is make all, this is default target
.PHONY: all
all: mychar others


.PHONY: mychar
mychar:
	$(MAKE) -C $(SRC) M=$(PWD) modules


OTHERS := showcmd try_ioctl try_write eat_mem
.PHONY: others
others: $(OTHERS)

# here use built-in pattern rules to save typing
# make -pn | less
showcmd.o \
try_ioctl.o \
try_write.o \
eat_mem.o: mychar.h
#showcmd: showcmd.c mychar.h
#	$(CC) $(CFLAGS) $< -o $@
#
#try_ioctl: try_ioctl.c mychar.h
#	$(CC) $(CFLAGS) $< -o $@
#
#try_write: try_write.c mychar.h
#	$(CC) $(CFLAGS) $< -o $@
#
#eat_mem: eat_mem.c mychar.h
#	$(CC) $(CFLAGS) $< -o $@


# char @ to make command invisible
# char - to ignore command error and continue
.PHONY: clean
clean:
	rm -f *.o *.ko *.cmd *.dwo *.mod *.mod.c Module.symvers modules.order
	rm -f .m* .M*
	rm -f $(OTHERS)
#@rm -f *.o *.ko *.cmd *.dwo *.mod *.mod.c Module.symvers modules.order
#@rm -f .m* .M*
#@rm -f $(OTHERS)
#-rm  *.o *.ko *.cmd *.dwo *.mod *.mod.c Module.symvers modules.order
#-rm  .m* .M*
#-rm  $(OTHERS)


