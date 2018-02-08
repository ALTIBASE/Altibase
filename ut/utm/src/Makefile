# $Id: Makefile 79348 2017-03-16 08:55:39Z hykim $
#

include ../../utEnv.mk
include ./ut_utm_objs.mk

AEXPORT_DEP_LIBS = $(ID_LIBS)

AEXPORTARC = $(LIBPRE)aexportobj.$(LIBEXT)

BINS = $(ALTI_HOME)/bin/aexport$(BINEXT)

INCLUDES += $(foreach i,$(ALTICORE_INCLUDES) $(RP_DIR)/include $(ALTI_HOME)/lib $(UT_DIR)/lib ../include, $(IDROPT)$(i))

OBJS = $(UT_UTM_OBJS)

SRCS = $(UT_UTM_SRCS)


.PHONY: all compile archive install sure alint clean

all: compile

compile: archive $(BINS) install

archive: $(AEXPORTARC)

$(AEXPORTARC): $(OBJS)

$(BINS): $(OBJS)
ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
	@sed 's/\.sh/\.bat/g' aexport.properties.org > aexport.properties.win
	@echo  LD $@
else
	@echo " LD $@"
endif
	$(Q) $(LD) $(LFLAGS) $(LIBDIRS) $(LDOUT)$@ $^ $(AEXPORT_DEP_LIBS)

install:
#	$(COPY) $(AEXPORTARC) $(ALTI_HOME)/lib

sure: clean all

alint: $(SRCS:.cpp=.lint)

clean:
	-$(RM) core $(TARGET_DIR)/ut/utm/src $(AEXPORTARC) $(BINS) aexport.properties.win

-include $(OBJS:.$(OBJEXT)=.d)
