# $Id: Makefile 70973 2015-05-26 07:30:32Z bethy $
#

include ../../../utEnv.mk


ATDBLIB = $(LIBPRE)$(PRODUCT_PREFIX)atDb.$(LIBEXT)

ATDBSHLIB = $(LIBPRE)$(PRODUCT_PREFIX)atDb.$(SOEXT)

INCLUDES += $(foreach i, ../include, $(IDROPT)$(i))

LFLAGS += -L$(ALTI_HOME)/lib

SRCS = $(DEV_DIR)/ut/audit/src/atc/Field.cpp \
       $(DEV_DIR)/ut/audit/src/atc/Row.cpp \
       $(DEV_DIR)/ut/audit/src/atc/Query.cpp \
       $(DEV_DIR)/ut/audit/src/atc/Connection.cpp \
       $(DEV_DIR)/ut/audit/src/atc/utdb.cpp

LFLAGS += -L$(ALTI_HOME)/lib $(SKIP_ERR_SYMBOL)

LIB_OPT += $(LIBOPT)id_pic$(LIBAFT)

OBJS=$(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))

SHOBJS = $(patsubst $(DEV_DIR)/%,$(TARGET_DIR)/%_soc.$(OBJEXT)     ,$(basename $(SRCS)))

.PHONY: all lib shlib compile clean cleanall

all: shlib lib

lib: $(ALTI_HOME)/lib/$(ATDBLIB)

shlib: $(ATDBSHLIB)

compile: $(ATDBSHLIB)

$(ALTI_HOME)/lib/$(ATDBLIB): $(OBJS)

$(ATDBSHLIB): $(SHOBJS)
ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
	@echo  SO $@
else
	@echo " SO $@"
endif
ifeq "$(OS_TARGET)" "X86_64_DARWIN"
	${Q} $(SOLINK.cc) $(LIBDIRS) $(SHOBJS) $(SOOUT)$(ATDBSHLIB) $(LIB_OPT) $(ID_LIBS)
else
	$(Q) $(SOLINK.cc) $(LIBDIRS) $(SHOBJS) $(LIB_OPT) $(SOOUT)$(ATDBSHLIB)
endif
	$(COPY) $(ATDBSHLIB) $(ALTI_HOME)/lib

clean:
	$(RM) SunWS_cache core* $(TARGET_DIR)/ut/audit/src/atc $(ATDBLIB) $(ATDBSHLIB)

cleanall: clean

$(TARGET_DIR)/%_soc.$(OBJEXT): $(DEV_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(COMPILE.cc) $(INCLUDES) $(PIC) $(CCOUT)$@ $<

$(TARGET_DIR)/%_soc.$(OBJEXT): $(DEV_DIR)/%.c
	mkdir -p $(dir $@)
	$(COMPILE.c) $(INCLUDES) $(PIC) $(CCOUT)$@ $<

env:
	echo $(OBJS)

-include $(OBJS:.$(OBJEXT)=.d)
