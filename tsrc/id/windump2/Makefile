# $Id: Makefile 17053 2006-07-12 01:30:55Z sjkim $

include ../../idEnv.mk

LIBS		:= $(LIBOPT)id$(LIBAFT) $(LIBOPT)pd$(LIBAFT)  $(LIBS)
LIBDIRS		+= $(LDROPT)$(ALTI_HOME)/lib
INCLUDES	:= $(IDROPT). $(IDROPT)$(ID_DIR)/src/include $(IDROPT)$(PD_DIR)/src/include $(IDROPT)$(PD_DIR)/src/makeinclude $(IDROPT)$(ID_DIR)/src/ $(INCLUDES)

TEST_TARGET	= windump$(BINEXT)

LIB_SRC = windump1.c windump2.c windump3.c windump4.c windump5.c windump6.c windump7.c windump8.c
LIB_OBJ = $(LIB_SRC:.c=.$(OBJEXT))
BIN =  sw$(BINEXT)  winsw$(BINEXT)
LIBNAME=$(LIBPRE)windump.$(LIBEXT)


all: $(BIN) 

sw$(BINEXT): StackWalker.obj main.obj windump1.obj windump2.obj windump3.obj windump4.obj windump5.obj windump6.obj windump7.obj windump8.obj 
	$(LD) /DEBUG $(LFLAGS) $(LDOUT)$@  StackWalker.obj main.obj windump1.obj windump2.obj windump3.obj windump4.obj windump5.obj windump6.obj windump7.obj windump8.obj   $(LDROPT).  $(LIBDIRS) $(LIBS) /MAP:StackWalker.map

winsw$(BINEXT): iduWinCallstack.obj main.obj windump1.obj windump2.obj windump3.obj windump4.obj windump5.obj windump6.obj windump7.obj windump8.obj 
	$(LD) /DEBUG $(LFLAGS) $(LDOUT)$@  iduWinCallstack.obj main.obj windump1.obj windump2.obj windump3.obj windump4.obj windump5.obj windump6.obj windump7.obj windump8.obj   $(LDROPT).  $(LIBDIRS) $(LIBS) /MAP:StackWalker.map


archive: $(LIBNAME)

$(LIBNAME): $(LIB_OBJ)

clean:
	$(RM) *.$(OBJEXT) core.* t $(BIN) LogFile* *~

rebuild:
	$(MAKE) clean
	$(MAKE) all

asm:
	PEDasm windump.exe
tag ctags:
	ctags $(SM_DIR)/src/*/*.h $(SM_DIR)/src/*/*.cpp *.h *.cpp

