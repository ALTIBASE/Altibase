
SRCS = $(DEV_DIR)/ut/iloader3/src/iloCommandCompiler.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloCommandLexer.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloCommandParser.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloDataFile.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloDownLoad.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloFormCompiler.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloFormDown.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloFormLexer.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloFormParser.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloSQLApi.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloLoadPrepare.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloLoad.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloProgOption.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloTableInfo.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloMain.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloLogFile.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloBadFile.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloPassword.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloMisc.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloCircularBuff.cpp \
       $(DEV_DIR)/ut/iloader3/lib/iloApi.cpp \
       $(DEV_DIR)/ut/iloader3/lib/iloProgOptApi.cpp \
       $(DEV_DIR)/ut/iloader3/src/iloConnect.cpp

UT_ILOAPI_OBJS = $(TARGET_DIR)/ut/iloader3/src/iloBadFile.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/src/iloCircularBuff.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/src/iloCommandCompiler.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/src/iloCommandLexer.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/src/iloCommandParser.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/src/iloDataFile.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/src/iloDownLoad.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/src/iloFormCompiler.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/src/iloFormDown.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/src/iloFormLexer.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/src/iloFormParser.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/src/iloLoadPrepare.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/src/iloLoad.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/src/iloLogFile.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/src/iloMisc.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/src/iloPassword.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/src/iloProgOption.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/src/iloSQLApi.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/src/iloTableInfo.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/src/iloConnect.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/lib/iloProgOptApi.$(OBJEXT) \
                 $(TARGET_DIR)/ut/iloader3/lib/iloApi.$(OBJEXT) 

UT_UTIL_OBJS =  $(TARGET_DIR)/ut/util/ute/uteErrorMgr.$(OBJEXT) \
                $(TARGET_DIR)/ut/util/utt/uttMemory.$(OBJEXT) \
                $(TARGET_DIR)/ut/util/utt/utString.$(OBJEXT) \
                $(TARGET_DIR)/ut/util/utt/uttTime.$(OBJEXT)

UT_LIBEDIT_OBJS = $(TARGET_DIR)/ut/libedit/src/history.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/chared.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/common.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/el.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/emacs.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/fcns.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/fgetln.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/help.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/hist.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/key.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/local_ctype.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/map.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/parse.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/prompt.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/read.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/refresh.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/search.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/sig.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/strlcat.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/strlcpy.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/term.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/tokenizer.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/tty.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/unvis.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/vi.$(OBJEXT) \
                  $(TARGET_DIR)/ut/libedit/src/vis.$(OBJEXT) 
