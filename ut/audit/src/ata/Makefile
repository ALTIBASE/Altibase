# $Id: Makefile 70973 2015-05-26 07:30:32Z bethy $
#

include ../../../utEnv.mk
include $(PD_DIR)/lib/pd_objs.mk
include $(ID_DIR)/lib/id_objs.mk

ATATBLIB = $(LIBPRE)$(PRODUCT_PREFIX)atAtb.$(LIBEXT)

ATATBSHLIB = $(LIBPRE)$(PRODUCT_PREFIX)atAtb.$(SOEXT)

INCLUDES += $(foreach i, ../include $(ID_DIR)/idk, $(IDROPT)$(i))

LIB_OPT += $(foreach i, $(PRODUCT_PREFIX)atDb $(UTIL_LIB)_sl $(ODBCCLI_LIB)_sl alticore_pic, $(LIBOPT)$(i)$(LIBAFT))

LFLAGS += $(SKIP_ERR_SYMBOL) -L$(ALTI_HOME)/lib

SRCS = $(DEV_DIR)/ut/audit/src/ata/utAtbField.cpp \
       $(DEV_DIR)/ut/audit/src/ata/utAtbRow.cpp \
       $(DEV_DIR)/ut/audit/src/ata/utAtbQuery.cpp \
       $(DEV_DIR)/ut/audit/src/ata/utAtbConnection.cpp \
       $(DEV_DIR)/ut/audit/src/ata/utAtb.cpp

SHSRCS = $(DEV_DIR)/ut/audit/src/ata/utAtbField.cpp \
         $(DEV_DIR)/ut/audit/src/ata/utAtbRow.cpp \
         $(DEV_DIR)/ut/audit/src/ata/utAtbQuery.cpp \
         $(DEV_DIR)/ut/audit/src/ata/utAtbConnection.cpp \
         $(DEV_DIR)/ut/audit/src/ata/utAtb.cpp \
         $(PD_CLIENT_SRCS) \
         $(ID_CLIENT_SRCS)

OBJS=$(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))

SHOBJS = $(patsubst $(DEV_DIR)/%,$(TARGET_DIR)/%_shobj.$(OBJEXT) ,$(basename $(SHSRCS)))

.PHONY: all lib shlib compile clean cleanall check

all: shlib lib

lib: $(ATATBLIB)

shlib: $(ATATBSHLIB)

compile: $(ATATBSHLIB)

$(ATATBLIB): $(OBJS)

$(ATATBSHLIB): $(SHOBJS)
ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
	@echo  SO $@
else
	@echo " SO $@"
endif
	@echo "$(LIB_OPT)"
	$(SOLINK.cc) $(LFLAGS) $(LIBDIRS) $(SHOBJS) $(LIB_OPT) $(SOOUT)$@
	$(COPY) $(ATATBSHLIB) $(ALTI_HOME)/lib

clean:
	$(RM) SunWS_cache core* $(TARGET_DIR)/ut/audit/src/ata $(ATATBLIB) $(ATATBSHLIB)

cleanall: clean

check:
	-echo $(EXTRA_CXXOPT)

-include $(OBJS:.$(OBJEXT)=.d)
