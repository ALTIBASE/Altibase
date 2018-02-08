# $Id: Makefile 68602 2015-01-23 00:13:11Z sbjang $

include ../../idEnv.mk

LIBS		:= $(LIBOPT)id $(LIBOPT)pd $(LIBS)
LIBDIRS		+= $(LDROPT)$(ALTI_HOME)/lib
INCLUDES	:= $(IDROPT). $(IDROPT)$(ID_DIR)/src/include $(INCLUDES)

TEST_TARGET	= logTest

BIN = $(TEST_TARGET)

all:	$(BIN) 

logTest: logTest.$(OBJEXT) $(ALTI_HOME)/lib
	$(LD) $(LFLAGS) -g $(LDOUT) $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)


clean:
	$(RM) *.$(OBJEXT) core.* t $(BIN) LogFile* *~

rebuild:
	$(MAKE) clean
	$(MAKE) all

tag ctags:
	ctags $(SM_DIR)/src/*/*.h $(SM_DIR)/src/*/*.cpp *.h *.cpp

