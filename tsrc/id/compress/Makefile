# $Id: Makefile 68602 2015-01-23 00:13:11Z sbjang $

include ../../idEnv.mk

LIBS		:= -lid -lpd $(LIBS)
LIBDIRS		+= -L$(ALTI_HOME)/lib
INCLUDES	:= -I. -I$(ID_DIR)/src/include $(INCLUDES)

TEST_TARGET	= compTest loadTest uncompTest cacheTest

BIN = $(TEST_TARGET)

all:	$(BIN) 

compTest: compTest.$(OBJEXT) $(ALTI_HOME)/lib
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

randTest: randTest.$(OBJEXT) $(ALTI_HOME)/lib
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

uncompTest: uncompTest.$(OBJEXT) $(ALTI_HOME)/lib
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

loadTest: loadTest.$(OBJEXT) $(ALTI_HOME)/lib
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

storeTest: storeTest.$(OBJEXT) $(ALTI_HOME)/lib
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

cacheTest: cacheTest.$(OBJEXT) $(ALTI_HOME)/lib
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

clean:
	$(RM) *.$(OBJEXT) core.* t $(BIN) LogFile* *~

rebuild:
	$(MAKE) clean
	$(MAKE) all

tag ctags:
	ctags $(SM_DIR)/src/*/*.h $(SM_DIR)/src/*/*.cpp *.h *.cpp

