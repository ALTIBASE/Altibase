# $Id: Makefile 68602 2015-01-23 00:13:11Z sbjang $

include ../../idEnv.mk

LIBS		:= -lid -lpd $(LIBS)
LIBDIRS		+= -L$(ID_DIR)/src/lib
INCLUDES	:= -I. -I$(ID_DIR)/src/include $(INCLUDES)
TEST_TARGET	= iduSyncTest

BIN = $(TEST_TARGET)

all:	$(BIN) 

$(TEST_TARGET): $(TEST_TARGET).cpp $(ID_DIR)/src/lib/libid.a
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

clean:
	$(RM) *.$(OBJEXT) core* LogFile* DataFile* Result* $(BIN) *~

rebuild:
	$(MAKE) clean
	$(MAKE) all

tag ctags:
	ctags $(ID_DIR)/src/*/*.h $(ID_DIR)/src/*/*.cpp *.h *.cpp

