# 다음 파일을 먼저 include해야 함:
#     include ../utEnv.mk
#     include iloApi_objs.mk

ILO_SRC_DIR = $(DEV_DIR)/ut/iloader3/src

LIBDIRS += $(LDROPT)$(ALTI_HOME)/lib

INCLUDES += $(foreach i, $(UT_DIR)/lib ../include ., $(IDROPT)$(i))

LEX = $(FLEX)

LEXFLAGS = -Cfar
FORMLEXFLAGS = -R -Cfar

YACC = $(BISON)

YACCFLAGS = -d -t -v



.PHONY: all compile cmd cmdclean form formclean

all: compile



cmd: $(ILO_SRC_DIR)/iloCommandParser.cpp $(ILO_SRC_DIR)/iloCommandLexer.cpp

cmdclean:
	$(Q) $(RM) $(ILO_SRC_DIR)/iloCommandParser.cpp $(ILO_SRC_DIR)/iloCommandParser.cpp.h $(ILO_SRC_DIR)/iloCommandParser.hpp $(ILO_SRC_DIR)/iloCommandParser.output $(ILO_SRC_DIR)/iloCommandParser.cpp.output $(ILO_SRC_DIR)/iloCommandLexer.cpp $(ILO_SRC_DIR)/iloCommandLexer.h

$(ILO_SRC_DIR)/iloCommandParser.cpp: $(ILO_SRC_DIR)/iloCommandParser.y
ifeq "$(BISON_ENV_NEEDED)" "yes"
	$(MAKE) $@ BISON_SIMPLE=$(BISON_SIMPLE_PATH) BISON_HAIRY=$(BISON_HAIRY_PATH) BISON_ENV_NEEDED=no
else
	$(YACC) $(YACCFLAGS) -p iloCommandParser -o $(ILO_SRC_DIR)/iloCommandParser.cpp $(ILO_SRC_DIR)/iloCommandParser.y
endif

$(ILO_SRC_DIR)/iloCommandLexer.cpp:
	$(LEX) $(LEXFLAGS) -o$(ILO_SRC_DIR)/iloCommandLexer.cpp $(ILO_SRC_DIR)/iloCommandLexer.l
	$(SED) s,$(FLEX_REPLACE_BEFORE),$(FLEX_REPLACE_AFTER), < $(ILO_SRC_DIR)/iloCommandLexer.cpp > $(ILO_SRC_DIR)/iloCommandLexer.cpp.old
	$(COPY) $(ILO_SRC_DIR)/iloCommandLexer.cpp.old $(ILO_SRC_DIR)/iloCommandLexer.cpp
	$(RM) $(ILO_SRC_DIR)/iloCommandLexer.cpp.old
ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
	$(COPY) $(ILO_SRC_DIR)/iloFormParser.hpp $(ILO_SRC_DIR)/iloFormParser.cpp.h
endif



cmdform: $(ILO_SRC_DIR)/iloFormParser.cpp $(ILO_SRC_DIR)/iloFormLexer.cpp

cmdformclean:
	$(Q) $(RM) $(ILO_SRC_DIR)/iloFormParser.cpp $(ILO_SRC_DIR)/iloFormParser.hpp $(ILO_SRC_DIR)/iloFormParser.cpp.h $(ILO_SRC_DIR)/iloFormParser.output $(ILO_SRC_DIR)/iloFormParser.cpp.output $(ILO_SRC_DIR)/iloFormLexer.cpp

$(ILO_SRC_DIR)/iloFormParser.cpp: $(ILO_SRC_DIR)/iloFormParser.y
ifeq "$(BISON_ENV_NEEDED)" "yes"
	$(MAKE) $@ BISON_SIMPLE=$(BISON_SIMPLE_PATH) BISON_HAIRY=$(BISON_HAIRY_PATH) BISON_ENV_NEEDED=no
else
	$(YACC) $(YACCFLAGS) -p Form -o $(ILO_SRC_DIR)/iloFormParser.cpp $(ILO_SRC_DIR)/iloFormParser.y
endif

$(ILO_SRC_DIR)/iloFormLexer.cpp:
	$(LEX) $(FORMLEXFLAGS) -o$(ILO_SRC_DIR)/iloFormLexer.cpp $(ILO_SRC_DIR)/iloFormLexer.l
	$(SED) s,$(FLEX_REPLACE_BEFORE),$(FLEX_REPLACE_AFTER), < $(ILO_SRC_DIR)/iloFormLexer.cpp > $(ILO_SRC_DIR)/iloFormLexer.cpp.old
	$(COPY) $(ILO_SRC_DIR)/iloFormLexer.cpp.old $(ILO_SRC_DIR)/iloFormLexer.cpp
	$(RM) $(ILO_SRC_DIR)/iloFormLexer.cpp.old

