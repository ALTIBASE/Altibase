# $Id: Makefile 13578 2005-09-10 11:34:32Z jdlee $

include ../../idEnv.mk

SRCS    = main.cpp Latch.cpp
OBJS    = $(OBJS:.cpp = .o)
BINS	= lockptest
RESULT_FILE = result.txt
GREPED_FILE = grep.txt

LIBS    := -lACE $(LIBS)
LIBDIRS := -L$(ID_DIR)/src/ACE/lib $(LIBDIRS)
all: checkPlatform 

checkPlatform : lockptest

lockptest: main.$(OBJEXT) Latch.$(OBJEXT) swap.o
	 $(CXX)  $(ACE_FLAG) $(LFLAGS) $(GOPT) $(INCLUDES) $(LIBDIRS) -o  $@ main.$(OBJEXT) Latch.$(OBJEXT) swap.$(OBJEXT) $(LIBS)

check :
	@for i in $(SRCS); do (gawk -v API_INPUT=$(TOOL_DIR)/coding_chk/API.txt -v SRC_INPUT=$$i -f $(TOOL_DIR)/coding_chk/apiCheck.awk ) done

clean:
	$(RM) core *.$(OBJEXT) *.d lockptest SunWS_cache  *.msb *.ih *~

clean_bin:
	$(RM) $(BINS)

dep depend:
	-$(RM) ./.dependency
	$(CXX) $(CXXOPT_DEPENDANCY) $(INCLUDES) $(SRCS) > .dependency

swap.$(OBJEXT) : swap.c
	gcc -c -g swap.c

ifeq (./.dependency,$(wildcard ./.dependency))
include ./.dependency
endif 
