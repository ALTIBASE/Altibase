# $Id: Makefile 71214 2015-06-09 02:28:30Z bethy $
#

include ../../utEnv.mk
include ../../util/ut_util_objs.mk

ALTIBASE_PRODUCT_PROFILE=$(PRODUCT_PREFIX)altiProfile$(BINEXT)

BINS = $(ALTI_HOME)/bin/$(ALTIBASE_PRODUCT_PROFILE)

SRCS = $(DEV_DIR)/ut/altiProfile/src/utpProfile.cpp        \
       $(DEV_DIR)/ut/altiProfile/src/utpHash.cpp           \
       $(DEV_DIR)/ut/altiProfile/src/utpCSVWriter.cpp      \
       $(DEV_DIR)/ut/altiProfile/src/utpStatistics.cpp     \
       $(DEV_DIR)/ut/altiProfile/src/utpDump.cpp           \
       $(DEV_DIR)/ut/altiProfile/src/utpCommandOptions.cpp \
       $(DEV_DIR)/ut/altiProfile/src/utpMain.cpp

INCLUDES += $(foreach i, $(UT_DIR)/include $(CM_DIR)/include ../include ., $(IDROPT)$(i))
LIBS     := $(foreach i, cm id pd, $(LIBOPT)$(i)$(LIBAFT)) $(LIBS)
LIBDIRS  += $(foreach i, $(UT_DIR)/lib, $(LDROPT)$(i))

OBJS=$(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))
OBJS+=$(UT_UTE_OBJS)

ifeq ($(OS_TARGET),INTEL_WINDOWS)
       LIBS += $(ALTICORE_LIBDIR)/alticore_static.lib
else
       LIBS += $(ALTICORE_LIBDIR)/libalticore.a
endif

.PHONY: all compile archive install alint clean clean_cli cliclean sure

all: compile

compile: $(BINS)

$(BINS): $(OBJS)
	@echo " LD $@"
	$(Q) $(LD) $(LFLAGS) $(LDOUT)$@ $^ $(LIBDIRS) $(LIBS)

alint: $(SRCS:.cpp=.lint)

clean:
	$(RM) core $(TARGET_DIR)/ut/altiProfile/src/ *.d $(BINS) *.$(LIBEXT)

sure:

-include $(OBJS:.$(OBJEXT)=.d)
