# $Id: Makefile 13578 2005-09-10 11:34:32Z jdlee $

include ../../idEnv.mk

LIBS		:= -lid -lpd $(LIBS)
LIBDIRS		+= -L$(ID_DIR)/src/lib
INCLUDES	:= -I. -I$(ID_DIR)/src/include $(INCLUDES)

BIN = selectHang iduObject

all:	$(BIN) 

iduObject: iduObject.$(OBJEXT) $(ID_DIR)/src/lib/libid.a
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

selectHang: selectHang.$(OBJEXT) $(ID_DIR)/src/lib/libid.a
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

clean:
	$(RM) *.$(OBJEXT) core t $(BIN)

tag ctags:
	ctags $(SM_DIR)/src/*/*.h $(SM_DIR)/src/*/*.cpp *.h *.cpp

