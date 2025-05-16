BINARY = bp_test

OBJS += $(patsubst %.c, %.o, $(wildcard usr/src/*.c))

TGT_CPPFLAGS	+= -Iusr/inc

gdb:
	gdb-multiarch --tui -silent -x 'fse_pb_bsp/tools/gdbinit' -iex 'set auto-load safe-path /'


include ../Makefile
include ../tools/Makefile.include
include ../tools/rules.mk