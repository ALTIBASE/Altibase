# $Id: Makefile 71185 2015-06-08 05:24:18Z bethy $
#

include ../../utEnv.mk
include ../ut_util_objs.mk


ifeq "$(OS_TARGET)" "IBM_AIX"
	LIBDIRS += $(foreach i, $(ALTI_HOME)/lib $(ID_DIR)/src/lib $(PD_DIR)/src/lib, $(LDROPT)$(i))
endif
ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
	LIBDIRS += $(foreach i, $(ALTI_HOME)/lib $(ID_DIR)/src/lib $(PD_DIR)/src/lib, $(LDROPT)$(i))
endif

UT_UTIL_OBJS = $(UT_UTE_OBJS) $(UT_UTT_OBJS) $(GENOBJS) $(ANTLROBJS)

UT_UTIL_SHOBJS = $(UT_UTE_SHOBJS) $(UT_UTT_SHOBJS) $(GENOBJS) $(ANTLROBJS)

UTILLIB = $(LIBPRE)$(UTIL_LIB).$(LIBEXT)

UTILSHLIB = $(LIBPRE)$(UTIL_LIB)_sl.$(SOEXT)

ifeq "$(OS_TARGET)" "IBM_AIX"
	UTILSHLIB_DEP_LIBS = $(foreach i, $(ODBCCLI_LIB) id pd mtc4ut, $(LIBOPT)$(i)$(LIBAFT))
endif
ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
	UTILSHLIB_DEP_LIBS = $(foreach i, $(ODBCCLI_LIB) id pd mtc4ut netapi32 iphlpapi advapi32 alticore_pic_static, $(LIBOPT)$(i)$(LIBAFT))
endif


.PHONY: all compile shobjs archive install alint clean dep

all: archive

compile:

shobjs:

ifeq "$(OS_TARGET)" "MIPS64_LINUX"
    ARCHIVES=$(UTILLIB)
else
    ARCHIVES=$(UTILLIB) $(UTILSHLIB)
endif

archive: install

$(UTILLIB): $(UT_UTIL_OBJS)

$(UTILSHLIB): $(UT_UTIL_SHOBJS)
ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
	$(Q) echo  SO $@
else
	$(Q) echo " SO $@"
endif
ifeq "$(NEED_INDIRECTION_BUILD)" "yes" # now only windows use indirection build
	$(Q) $(SOLINK.cc) $(LDFLAGS) $(LIBDIRS) $(SOOUT)$@ $^ $(UTILSHLIB_DEP_LIBS) $(LIB_OPT) /DEF:$(UTIL_LIB)_sl.def
else
 ifeq "$(OS_TARGET)" "X86_64_DARWIN"
	$(Q) $(SOLINK.cc) $(LDFLAGS) $(LIBDIRS) $(SOOUT)$@ $^ $(UTILSHLIB_DEP_LIBS) $(LIB_OPT) -lid_pic -lpd -lalticore
 else
	$(Q) $(SOLINK.cc) $(LDFLAGS) $(LIBDIRS) $(SOOUT)$@ $^ $(UTILSHLIB_DEP_LIBS) $(LIB_OPT)
 endif
endif

install: $(ARCHIVES)
	$(COPY) $(UTILLIB) $(ALTI_HOME)/lib
ifneq "$(OS_TARGET)" "MIPS64_LINUX"
	$(COPY) $(UTILSHLIB) $(ALTI_HOME)/lib
endif
ifeq "$(NEED_INDIRECTION_BUILD)" "yes"
	$(COPY) $(LIBPRE)$(UTIL_LIB)_sl.exp $(ALTI_HOME)/lib
	$(COPY) $(LIBPRE)$(UTIL_LIB)_sl.$(LIBEXT) $(ALTI_HOME)/lib
#	$(COPY) $(LIBPRE)$(UTIL_LIB)_sl.pdb $(ALTI_HOME)/lib
endif

alint:

clean:
	$(RM) core *.$(OBJEXT) *.d SunWS_cache .shobj *.$(LIBEXT) *.$(SOEXT) *.exp *.pdb

-include $(UT_UTIL_OBJS:.$(OBJEXT)=.d)

-include $(UT_UTIL_SHOBJS:.$(OBJEXT)=.d)
