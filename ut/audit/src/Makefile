# $Id: Makefile 76513 2016-08-03 06:26:02Z bethy $
#

include ../../utEnv.mk

AUDIT_DEP_LIBS = $(ID_LIBS)

AUDITARC = $(LIBPRE)auditobj.$(LIBEXT)

BINS = $(ALTI_HOME)/bin/$(PRODUCT_PREFIX)altiComp$(BINEXT)

INCLUDES += $(foreach i, ./include, $(IDROPT)$(i))

DBA_SRCS = $(DEV_DIR)/ut/audit/src/ata/utAtb.cpp \
       $(DEV_DIR)/ut/audit/src/ata/utAtbConnection.cpp \
       $(DEV_DIR)/ut/audit/src/ata/utAtbField.cpp \
       $(DEV_DIR)/ut/audit/src/ata/utAtbQuery.cpp \
       $(DEV_DIR)/ut/audit/src/ata/utAtbRow.cpp

DBC_SRCS = $(DEV_DIR)/ut/audit/src/atc/utdb.cpp \
       $(DEV_DIR)/ut/audit/src/atc/Connection.cpp \
       $(DEV_DIR)/ut/audit/src/atc/Field.cpp \
       $(DEV_DIR)/ut/audit/src/atc/Query.cpp \
       $(DEV_DIR)/ut/audit/src/atc/Row.cpp

# *** Audit itself *** #
SRCS = $(DBA_SRCS) $(DBC_SRCS) \
       $(DEV_DIR)/ut/audit/src/dmlQuery.cpp \
       $(DEV_DIR)/ut/audit/src/utProperties.cpp \
       $(DEV_DIR)/ut/audit/src/utScanner.cpp \
       $(DEV_DIR)/ut/audit/src/utTaskThread.cpp \
       $(DEV_DIR)/ut/audit/src/utTaskList.cpp \
       $(DEV_DIR)/ut/audit/src/process.cpp \
       $(DEV_DIR)/ut/audit/src/audit.cpp

OBJS=$(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))


.PHONY: all compile archive install check clean ctags

ifeq "$(OS_TARGET)" "INTEL_WINDOWS"

#BUGBUG: SKIP in Windows temporary

all: compile

compile:

else

all: compile

compile: archive $(BINS) install

endif

archive: $(AUDITARC)

$(AUDITARC): $(OBJS)

$(BINS): $(OBJS)
ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
	@echo  LD $@
else
	@echo " LD $@"
endif
	$(Q) $(LD) $(LFLAGS) $(LIBDIRS) $(LDOUT)$@ $(OBJS) $(AUDIT_DEP_LIBS)

install:
#	$(COPY) $(AUDITARC) $(ALTI_HOME)/lib

check:
	@echo EXTRA_CXXOPT= $(EXTRA_CXXOPT)

clean:
	$(RM) core* *.d SunWS_cache $(OBJS) $(TARGET_DIR)/ut/audit/src $(AUDITARC) $(BINS) ../log/* *.log

ctags:
	ctags -R . ./include

-include $(OBJS:.$(OBJEXT)=.d)
