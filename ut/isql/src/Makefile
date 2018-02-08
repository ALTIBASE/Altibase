# $Id: Makefile 77746 2016-10-25 01:00:45Z bethy $
#

include ../../utEnv.mk
include ../../util/ut_util_objs.mk
include ../isql_objs.mk

ifeq "$(NEED_BUILD_LIBEDIT)" "yes"
include  ../../libedit/src/libedit_objs.mk
endif

PASSWD_FILENAME = altipasswd
ISQL_BAN = ISQL.ban

ALTIPASSWD_DEP_LIBS = $(foreach i, $(ODBCCLI_LIB) id pd mtc4ut, $(LIBOPT)$(i)$(LIBAFT)) \
	$(ALTICORE_STATIC_LIB) $(LIBS)

ALTIPASSWD_DEP_OBJS = $(TARGET_DIR)/ut/isql/src/$(PASSWD_FILENAME).$(OBJEXT) $(TARGET_DIR)/ut/isql/src/iSQLPassword.$(OBJEXT)

ISQL_BIN=$(ALTI_HOME)/bin/$(PRODUCT_PREFIX)isql$(BINEXT)
BINS = $(ISQL_BIN) $(ALTI_HOME)/bin/$(PASSWD_FILENAME)$(BINEXT)

ifeq "$(NEED_BUILD_LIBEDIT)" "yes"
	CCFLAGS += $(DEFOPT)USE_READLINE
endif

ifeq "$(NEED_BUILD_LIBEDIT)" "yes"
	INCLUDES += $(IDROPT)$(UT_DIR)/libedit/include
endif
INCLUDES += $(foreach i,$(ALTICORE_INCLUDES) $(ALTI_HOME)/lib $(UT_DIR)/lib ../include ., $(IDROPT)$(i))

ISQL_DEP_LIBS = $(foreach i, altiutil mtc4ut, $(LIBOPT)$(i)$(LIBAFT))

ifeq "$(NEED_BUILD_LIBEDIT)" "yes"
ifeq  "$(USE_NCURSES)" "1"
		ISQL_DEP_LIBS += $(LIBOPT)ncurses$(LIBAFT)
else
		ISQL_DEP_LIBS += $(LIBOPT)curses$(LIBAFT)
endif
endif
ISQL_DEP_LIBS += $(LIBS)

ISQLARC = $(LIBPRE)isqlobj.$(LIBEXT)

LEX = $(FLEX)

LEXFLAGS = -Cfar

LIBDIRS += $(foreach i, $(UT_DIR)/lib, $(LDROPT)$(i))

SRCS = $(ISQL_DBC_SRCS) \
       $(ISQL_TYPE_SRCS) \
       $(DEV_DIR)/ut/isql/src/iSQLPreLexer.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLScanLexer.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLParamLexer.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLParamParser.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLConnectLexer.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLConnectParser.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLLexer.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLParser.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLProperty.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLProgOption.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLCompiler.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLCommand.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLExecutor.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLCommandQueue.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLExecuteCommand.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLPassword.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLHelp.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLHostVarMgr.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLSpool.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLAdminCommand.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLPrepareCommand.cpp \
       $(DEV_DIR)/ut/isql/src/iSQLMain.cpp

OBJS=$(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))

YACC = $(BISON)

YACCFLAGS = -d -v


.PHONY: all compile archive install alint clean clean_cli cliclean sure

.l.cpp:
	$(LEX) $(LEXFLAGS) -o$*.cpp $*.l
	$(SED) s,$(FLEX_REPLACE_BEFORE),$(FLEX_REPLACE_AFTER), < $*.cpp > $*.cpp.old
	$(RM) -f $*.cpp
	$(COPY) $*.cpp.old $*.cpp
	$(RM) $*.cpp.old

all: compile

compile: $(DEV_DIR)/ut/isql/src/iSQLParser.cpp $(DEV_DIR)/ut/isql/src/iSQLConnectParser.cpp $(DEV_DIR)/ut/isql/src/iSQLParamParser.cpp $(TARGET_DIR)/ut/isql/src/iSQLPreLexer.$(OBJEXT) $(TARGET_DIR)/ut/isql/src/iSQLScanLexer.$(OBJEXT) $(TARGET_DIR)/ut/isql/src/iSQLLexer.$(OBJEXT) $(TARGET_DIR)/ut/isql/src/iSQLConnectLexer.$(OBJEXT) $(TARGET_DIR)/ut/isql/src/iSQLParamLexer.$(OBJEXT) archive $(BINS) install

$(DEV_DIR)/ut/isql/src/iSQLParser.cpp: iSQLParser.y
ifeq "$(BISON_ENV_NEEDED)" "yes"
	$(MAKE) $@ BISON_SIMPLE=$(BISON_SIMPLE_PATH) BISON_HAIRY=$(BISON_HAIRY_PATH) BISON_ENV_NEEDED=no
else
	$(YACC) $(YACCFLAGS) -p iSQLParser -o iSQLParser.cpp iSQLParser.y
endif

$(DEV_DIR)/ut/isql/src/iSQLConnectParser.cpp: iSQLConnectParser.y
ifeq "$(BISON_ENV_NEEDED)" "yes"
	$(MAKE) $@ BISON_SIMPLE=$(BISON_SIMPLE_PATH) BISON_HAIRY=$(BISON_HAIRY_PATH) BISON_ENV_NEEDED=no
else
	$(YACC) $(YACCFLAGS) -p iSQLConnectParser -o iSQLConnectParser.cpp iSQLConnectParser.y
endif

$(DEV_DIR)/ut/isql/src/iSQLParamParser.cpp: iSQLParamParser.y
ifeq "$(BISON_ENV_NEEDED)" "yes"
	$(MAKE) $@ BISON_SIMPLE=$(BISON_SIMPLE_PATH) BISON_HAIRY=$(BISON_HAIRY_PATH) BISON_ENV_NEEDED=no
else
	$(YACC) $(YACCFLAGS) -p iSQLParamParser -o iSQLParamParser.cpp iSQLParamParser.y
endif

archive: $(ISQLARC)

ifeq "$(NEED_BUILD_LIBEDIT)" "yes"
$(ISQLARC): $(OBJS) $(LIBEDIT_OBJS)
else
$(ISQLARC): $(OBJS)
endif

# Project library dependencies
$(ISQL_BIN): $(ALTICORE_STATIC_LIB) \
	$(ALTI_HOME)/lib/$(LIBPRE)pd.$(LIBEXT) \
	$(ALTI_HOME)/lib/$(LIBPRE)id.$(LIBEXT) \
	$(ALTI_HOME)/lib/$(LIBPRE)$(ODBCCLI_LIB).$(LIBEXT)

ifeq "$(NEED_BUILD_LIBEDIT)" "yes"
$(ISQL_BIN): $(OBJS) $(LIBEDIT_OBJS)
else
$(ISQL_BIN): $(OBJS)
endif
ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
	echo  LD $@
else
	@echo " LD $@"
endif
	$(Q) $(LD) $(LFLAGS) $(LIBDIRS) $(LDOUT)$@ $^ $(ISQL_DEP_LIBS)

$(ALTI_HOME)/bin/$(PASSWD_FILENAME)$(BINEXT): $(ALTIPASSWD_DEP_OBJS)
ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
	@echo  LD $@
else
	@echo " LD $@"
endif
	$(Q) $(LD) $(LFLAGS) $(LIBDIRS) $(LDOUT)$@ $^ $(UT_UTE_OBJS) $(ALTIPASSWD_DEP_LIBS)

install: $(BINS)
	$(COPY) $(ISQL_BAN) $(ALTI_HOME)/msg
	$(COPY) $(ISQLARC) $(ALTI_HOME)/lib

alint: $(SRCS:.cpp=.lint)

clean:
	$(RM) core $(TARGET_DIR)/ut/isql/dbc $(TARGET_DIR)/ut/isql/src *.d SunWS_cache iSQLPreLexer.cpp iSQLScanLexer.cpp iSQLParamLexer.cpp iSQLParamParser.cpp iSQLParamParser.cpp.h iSQLParamParser.hpp iSQLParamParser.output iSQLSriptParser.cpp.output iSQLLexer.cpp iSQLParser.cpp iSQLParser.cpp.h iSQLParser.hpp iSQLParser.output iSQLParser.cpp.output iSQLConnectLexer.cpp iSQLConnectParser.cpp iSQLConnectParser.cpp.h iSQLConnectParser.hpp iSQLConnectParser.output iSQLConnectParser.cpp.output $(ISQLARC) $(BINS)

clean_cli:
	$(RM) $(CLI_OBJ_DIR) $(CLI_SHOBJ_DIR)

cliclean:
	$(RM) filein.$(OBJEXT) filein

sure:

-include $(OBJS:.$(OBJEXT)=.d)
