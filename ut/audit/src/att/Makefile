# $Id: Makefile 70973 2015-05-26 07:30:32Z bethy $
#

include ../../../utEnv.mk


ATTXTLIB = $(LIBPRE)$(PRODUCT_PREFIX)atTxt.$(LIBEXT)

ATTXTSHLIB = $(LIBPRE)$(PRODUCT_PREFIX)atTxt.$(SOEXT)

INCLUDES += $(foreach i, ../include, $(IDROPT)$(i))

LIB_OPT += $(foreach i, $(PRODUCT_PREFIX)atDb $(UTIL_LIB)_sl $(ODBCCLI_LIB)_sl, $(LIBOPT)$(i)$(LIBAFT)) $(LIBS)

LFLAGS += $(SKIP_ERR_SYMBOL) -L$(ALTI_HOME)/lib

SRCS = $(DEV_DIR)/ut/audit/src/att/utTxtField.cpp \
       $(DEV_DIR)/ut/audit/src/att/utTxtRow.cpp \
       $(DEV_DIR)/ut/audit/src/att/utTxtQuery.cpp \
       $(DEV_DIR)/ut/audit/src/att/utTxtConnection.cpp \
       $(DEV_DIR)/ut/audit/src/att/utTxt.cpp

OBJS=$(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))

SHOBJS = $(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%_shobj.$(OBJEXT))

.PHONY: compile all lib shlib clean cleanall check

compile: shlib

all: shlib lib

lib: $(ATTXTLIB)

shlib: $(ATTXTSHLIB)

$(ATTXTLIB): $(OBJS)

$(ATTXTSHLIB): $(SHOBJS)
ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
	@echo  SO $@
else
	@echo " SO $@"
endif
	${Q} $(SOLINK.cc) $(LFLAGS) $(LIBDIRS) $(SHOBJS) $(LIB_OPT) $(SOOUT)$@
	$(COPY) $(ATTXTSHLIB) $(ALTI_HOME)/lib

clean:
	$(RM) .shobj SunWS_cache core* $(TARGET_DIR)/ut/audit/src/att $(ATTXTLIB) $(ATTXTSHLIB)

cleanall: clean

check:
	-echo $(EXTRA_CXXOPT)

-include $(OBJS:.$(OBJEXT)=.d)
