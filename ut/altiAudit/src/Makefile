# $Id: Makefile 48416 2011-08-18 05:57:13Z djin $
#
include ../../utEnv.mk

BINS = $(ALTI_HOME)/bin/altiAudit$(BINEXT)
SRCS = $(DEV_DIR)/ut/altiAudit/src/altiAudit.cpp

INCLUDES += $(foreach i, $(UT_DIR)/include $(CM_DIR)/include ../include ., $(IDROPT)$(i))
LIBS     := $(foreach i, id pd, $(LIBOPT)$(i)$(LIBAFT)) $(LIBS)

OBJS=$(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))

ifeq ($(OS_TARGET),INTEL_WINDOWS)
       LIBS += $(ALTICORE_LIBDIR)/alticore_static.lib
else
       LIBS += $(ALTICORE_LIBDIR)/libalticore.a
endif

.PHONY: all clean 

all: $(BINS)

$(ALTI_HOME)/bin/altiAudit$(BINEXT):$(OBJS)
	@echo " LD $@"
	$(Q) $(LD) $(LFLAGS) $(LDOUT)$@ $^ $(LIBS)

clean:
	$(RM) core $(TARGET_DIR)/ut/altiAudit/src  *.d $(BINS) *.$(LIBEXT)

-include $(OBJS:.$(OBJEXT)=.d)
