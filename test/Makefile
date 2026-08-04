BINARY = bp_test


OBJS += main.o
OBJS += $(patsubst %.c, %.o, $(wildcard usr/src/*.c))

SIMULINK_DIRS = $(wildcard usr/simulink/*_ert_rtw)
SIMULINK_DIRS += $(wildcard fse_pb_bsp/simulink/ss_*)
OBJS += $(patsubst %.c, %.o, $(wildcard usr/simulink/*_ert_rtw/*.c))



TGT_CPPFLAGS	+= -Iusr/inc
TGT_CPPFLAGS	+= $(addprefix -I,$(SIMULINK_DIRS))

all:

gdb:
	gdb-multiarch --tui -silent -x 'fse_pb_bsp/tools/gdbinit' -iex 'set auto-load safe-path /'

include fse_pb_bsp/Makefile.rtos
include fse_pb_bsp/Makefile
include fse_pb_bsp/tools/Makefile.include
include fse_pb_bsp/tools/rules.mk
