BUILDDIR = build

BINARY = $(BUILDDIR)/bp_test


# every object has to live under $(BUILDDIR), otherwise the pattern rule in
# fse_pb_bsp/tools/rules.mk does not match and make silently falls back to its
# builtin %.o: %.c rule - which compiles without include paths or ARCH_FLAGS
OBJS += $(BUILDDIR)/main.o
OBJS += $(patsubst %.c, $(BUILDDIR)/%.o, $(wildcard usr/src/*.c))

SIMULINK_DIRS = $(wildcard usr/simulink/*_ert_rtw)
# filter %/ keeps directories only: on windows the trailing slash does not stop
# wildcard from also returning fse_pb_bsp/simulink/ss_*.m, which then ends up as
# a bogus -I. Directories come back with the slash, files without.
SIMULINK_DIRS += $(patsubst %/,%,$(filter %/,$(wildcard fse_pb_bsp/simulink/ss_*/)))
OBJS += $(patsubst %.c, $(BUILDDIR)/%.o, $(wildcard usr/simulink/*_ert_rtw/*.c))

# usr/simulink/FSE_PB_BSP_TEST_ert_rtw -> FSE_PB_BSP_TEST
# main.c only compiles its model task when this is defined, so a fresh checkout
# still builds before ./ss matlab_build has ever generated code
SIMULINK_RTW = $(wildcard usr/simulink/*_ert_rtw)

ifneq ($(words $(SIMULINK_RTW)),0)
ifneq ($(words $(SIMULINK_RTW)),1)
$(error more than one generated model in usr/simulink: $(SIMULINK_RTW) - remove the stale one, or run ./ss matlab_clean)
endif
SIMULINK_MODEL = $(notdir $(patsubst %_ert_rtw,%,$(SIMULINK_RTW)))
TGT_CPPFLAGS	+= -DSS_SIMULINK_MODEL=$(SIMULINK_MODEL)
$(info simulink model: $(SIMULINK_MODEL) -> simulink_task compiled in)
else
$(info simulink model: none -> simulink_task skipped, run ./ss matlab_build)
endif



TGT_CPPFLAGS	+= -Iusr/inc
TGT_CPPFLAGS	+= $(addprefix -I,$(SIMULINK_DIRS))

LDLIBS		+= -lm

all: $(BINARY).elf

gdb:
	gdb-multiarch --tui -silent -x 'fse_pb_bsp/tools/gdbinit' -iex 'set auto-load safe-path /'

include fse_pb_bsp/Makefile.rtos
include fse_pb_bsp/Makefile

# after the two includes above, they are what defines LIB_OBJS
OBJS += $(LIB_OBJS)

include fse_pb_bsp/tools/Makefile.include
include fse_pb_bsp/tools/rules.mk
