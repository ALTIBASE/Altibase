# $Id: Makefile 68602 2015-01-23 00:13:11Z sbjang $

include ../../idEnv.mk

LIBS		:= -lid -lpd $(LIBS)
LIBDIRS		+= -L$(ID_DIR)/src/lib
INCLUDES	:= -I. -I$(ID_DIR)/src/include $(INCLUDES)

TEST_TARGET	= loadTest 

BIN = $(TEST_TARGET)

all:	$(BIN) 

aioTest: aioTest.cpp $(ID_DIR)/src/lib/libid.a
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

loadTest: loadTest.$(OBJEXT) loadTest.cpp $(ID_DIR)/src/lib/libid.a
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

clean:
	$(RM) *.$(OBJEXT) core.* t $(BIN) LogFile* *~

rebuild:
	$(MAKE) clean
	$(MAKE) all

tag ctags:
	ctags $(SM_DIR)/src/*/*.h $(SM_DIR)/src/*/*.cpp *.h *.cpp

