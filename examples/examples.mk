BUILDDIR = build

ifeq ($(strip $(EXAMPLE)),)
$(error EXAMPLE not set, e.g. make -f fse_pb_bsp/examples/examples.mk EXAMPLE=adc)
endif

BINARY = fse_pb_bsp/examples/build/$(EXAMPLE)

OBJS += $(BUILDDIR)/fse_pb_bsp/examples/$(EXAMPLE).o

TGT_CPPFLAGS += -Iusr/inc

LDLIBS += -lm

all: $(BINARY).elf

include fse_pb_bsp/Makefile.rtos
include fse_pb_bsp/Makefile

OBJS += $(LIB_OBJS)

include fse_pb_bsp/tools/Makefile.include
include fse_pb_bsp/tools/rules.mk
