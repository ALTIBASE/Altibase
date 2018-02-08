#
# $Id: Makefile 71214 2015-06-09 02:28:30Z bethy $
#

include ../../utEnv.mk
include ../../util/ut_util_objs.mk

ALTIWRAP_BIN=$(ALTI_HOME)/bin/$(PRODUCT_PREFIX)altiwrap$(BINEXT)
BINS = $(ALTIWRAP_BIN)

INCLUDES += $(foreach i,$(ALTICORE_INCLUDES) $(ALTI_HOME)/lib $(UT_DIR)/lib ../include ., $(IDROPT)$(i))

ALTIWRAP_DEP_LIBS = $(LIBS)

ALTIWRAPARC = $(LIBPRE)altiWrapObj.$(LIBEXT)

LEX = $(FLEX)
LEXFLAGS = -Cfar

YACC = $(BISON)
YACCFLAGS=-d -t -v

LIBDIRS += $(foreach i, $(UT_DIR)/lib, $(LDROPT)$(i))

GENSRCS= $(UT_DIR)/altiWrap/src/altiWraplp.cpp $(UT_DIR)/altiWrap/src/altiWraply.cpp $(UT_DIR)/altiWrap/src/altiWrapll.cpp
GENOBJS=$(GENSRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))
GENSTUFFS=altiWraply.cpp.output altiWraply.cpp.h
GENHEADERS=$(UT_DIR)/altiWrap/src/altiWraply.hpp

SRCS= $(UT_DIR)/altiWrap/src/altiWrapEncrypt.cpp    \
      $(UT_DIR)/altiWrap/src/altiWrapParseMgr.cpp   \
      $(UT_DIR)/altiWrap/src/altiWrapFileMgr.cpp    \
      $(UT_DIR)/altiWrap/src/altiWrapi.cpp          \
      $(UT_DIR)/altiWrap/src/altiWrapMain.cpp
OBJS=$(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))
OBJS+=$(UT_UTE_OBJS)

.PHONY: all compile archive install alint clean cliclean sure

all: compile

compile: $(GENOBJS) $(OBJS) archive $(BINS) install

$(GENOBJS): $(GENSRCS)

$(TARGET_DIR)/ut/altiWrap/src/altiWrapParseMgr.$(OBJEXT): $(GENHEADERS)

%.cpp: %.l
	$(LEX)  $(LEXFLAGS)  -o$*.cpp $*.l
	$(SED) s,$(FLEX_REPLACE_BEFORE),$(FLEX_REPLACE_AFTER), < $*.cpp > $*.cpp.old
	$(RM) -f $*.cpp
	mv $*.cpp.old $*.cpp

%.hpp %.cpp: %.y
ifeq "$(BISON_ENV_NEEDED)" "yes"
	$(MAKE) $@ BISON_SIMPLE=$(BISON_SIMPLE_PATH) BISON_HAIRY=$(BISON_HAIRY_PATH) BISON_ENV_NEEDED=no
else
	$(YACC) $(YACCFLAGS) -p $(subst y.cpp,,$(notdir $@)) -o $*.cpp $*.y
endif

ifeq "$(OS_TARGET)" "HP_HPUX"
$(TARGET_DIR)/ut/altiWrap/altiWraply.o:
	mkdir -p $(dir $@)
	@$(CXX) $(subst +Onolimit,,$(CCFLAGS)) $(CPPFLAGS) $(PTDIRS) -c $(INCLUDES) $(CC_OUTPUT_FLAG)$@ $<
endif

ifeq "$(OS_TARGET)" "IBM_AIX"
$(TARGET_DIR)/ut/altiWrap/altiWraply.o:
	mkdir -p $(dir $@)
	@$(CXX) $(subst -O2,,$(CCFLAGS)) $(CPPFLAGS) $(PTDIRS) -c $(INCLUDES) $(CC_OUTPUT_FLAG)$@ $<
endif

archive: $(ALTIWRAPARC)

$(ALTIWRAPARC): $(GENOBJS) $(OBJS)

# Project library dependencies
$(ALTIWRAP_BIN): $(ALTICORE_STATIC_LIB) \
	$(ALTI_HOME)/lib/$(LIBPRE)pd.$(LIBEXT) \
	$(ALTI_HOME)/lib/$(LIBPRE)id.$(LIBEXT) \
	$(ALTI_HOME)/lib/$(LIBPRE)$(ODBCCLI_LIB).$(LIBEXT)
ifeq "$(ALTIBASE_PRODUCT)" "xdb"
$(ALTIWRAP_BIN): $(ALTI_HOME)/lib/$(LIBPRE)mm.$(LIBEXT) \
	$(ALTI_HOME)/lib/$(LIBPRE)qp.$(LIBEXT) \
	$(ALTI_HOME)/lib/$(LIBPRE)rp.$(LIBEXT) \
	$(ALTI_HOME)/lib/$(LIBPRE)sm.$(LIBEXT) \
	$(ALTI_HOME)/lib/$(LIBPRE)mt.$(LIBEXT) \
	$(ALTI_HOME)/lib/$(LIBPRE)cm.$(LIBEXT)
endif

$(ALTIWRAP_BIN): $(GENOBJS) $(OBJS)

ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
	echo  LD $@
else
	@echo " LD $@"
endif
	$(Q) $(LD) $(LFLAGS) $(LIBDIRS) $(LDOUT)$@ $^ $(ALTIWRAP_DEP_LIBS)

install: $(BINS)
	$(COPY) $(ALTIWRAPARC) $(ALTI_HOME)/lib

alint: $(SRCS:.cpp=.lint)

clean:
	$(RM) core $(TARGET_DIR)/ut/altiWrap/src *.d SunWS_cache $(GENSTUFFS) $(GENSRCS) $(ALTIWRAPARC) $(BINS)

cliclean:
	$(RM) filein.$(OBJEXT) filein

sure:
