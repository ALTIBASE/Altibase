# $Id: Makefile 70397 2015-04-20 01:43:07Z heesuk.shin $
#

include ../../utEnv.mk
include ../src/libedit_objs.mk


BINS = $(ALTI_HOME)/bin/sample$(BINEXT)

INCLUDES += $(foreach i, ../include, $(IDROPT)$(i))

OBJS = $(TARGET_DIR)/ut/libedit/sample/sample.$(OBJEXT) $(LIBEDIT_OBJS)

SAMPLE_DEP_LIBS = $(LIBOPT)pd$(LIBAFT)
ifeq "$(USE_NCURSES)" "1"
	SAMPLE_DEP_LIBS += $(LIBOPT)ncurses$(LIBAFT)
else
	SAMPLE_DEP_LIBS += $(LIBOPT)curses$(LIBAFT)
endif
SAMPLE_DEP_LIBS += $(LIBS)


.PHONY: all compile clean

all: compile

compile: $(BINS)

$(BINS): $(OBJS)
ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
	@echo  LD $@
else
	@echo " LD $@"
endif
	$(Q) $(LD) $(LFLAGS) $(LIBDIRS) $(LDOUT)$@ $^ $(SAMPLE_DEP_LIBS)

clean:
	$(RM) $(OBJS) sample$(BINEXT)

-include $(OBJS:.$(OBJEXT)=.d)
