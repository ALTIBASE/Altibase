# $Id: Makefile 68602 2015-01-23 00:13:11Z sbjang $

include ../../idEnv.mk

LIBS		:= -lid -lpd $(LIBS)
LIBDIRS		+= -L$(ALTI_HOME)/lib
INCLUDES	:= -I. -I$(ID_DIR)/src/include $(INCLUDES)

TEST_TARGET	= loadTest 

BIN = clock BUG-18636

all:	$(BIN) 

clock: clock.$(OBJEXT) $(ALTI_HOME)/lib/libid.a
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

BUG-18636: BUG-18636.$(OBJEXT) $(ALTI_HOME)/lib/libid.a
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

chk_speed: chk_speed.$(OBJEXT) $(ALTI_HOME)/lib/libid.a
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

aixclock: aixclock.$(OBJEXT) $(ALTI_HOME)/lib/libid.a
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

hpclock: hpclock.$(OBJEXT) $(ALTI_HOME)/lib/libid.a
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

hpclock2: hpclock2.$(OBJEXT) $(ALTI_HOME)/lib/libid.a
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

hpclock3: hpclock3.o $(ALTI_HOME)/lib/libid.a
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

x86_32_clock: x86_32_clock.o 
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

x86_64_clock: x86_64_clock.o
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

amd_clock: amd_clock.o
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

compare: compare.o
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) $(LIBDIRS) $(LIBS)

clean:
	$(RM) *.$(OBJEXT) core.* t $(BIN) LogFile* *~

rebuild:
	$(MAKE) clean
	$(MAKE) all

tag ctags:
	ctags $(SM_DIR)/src/*/*.h $(SM_DIR)/src/*/*.cpp *.h *.cpp

