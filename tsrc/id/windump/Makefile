# $Id: Makefile 68602 2015-01-23 00:13:11Z sbjang $

include ../../idEnv.mk

LIBS		:= $(LIBOPT)id$(LIBAFT) $(LIBOPT)pd$(LIBAFT)  $(LIBS)
LIBDIRS		+= $(LDROPT)$(ALTI_HOME)/lib
INCLUDES	:= $(IDROPT). $(IDROPT)$(ID_DIR)/src/include $(IDROPT)$(PD_DIR)/src/include $(IDROPT)$(PD_DIR)/src/makeinclude $(IDROPT)$(ID_DIR)/src/ $(INCLUDES)

TEST_TARGET	= windump$(BINEXT)

LIB_SRC = windump1.c windump2.c windump3.c windump4.c windump5.c windump6.c windump7.c windump8.c
LIB_OBJ = $(LIB_SRC:.c=.$(OBJEXT))
BIN = windump$(BINEXT) StackWalker$(BINEXT)
LIBNAME=$(LIBPRE)windump.$(LIBEXT)

#CFLAGS := /nologo /TP /O2 /FD /EHsc /MT /GS /W3 /Zd  /Oy- /DPDL_HAS_DLL=0 /DWIN32 /D_WINDOWS /D_MBCS /DNDEBUG /D_LIB /DUSE_NEW_IOSTREAM /c /DTEST
#CFLAGS :=  /O2 /FD /EHsc  /GS /W3 /Zd  /Oy- /DPDL_HAS_DLL=0 

#CFLAGS := /nologo /MT /TP /Oy- /Ox /Oy- /DPDL_HAS_DLL=0 /DWIN32 /D_WINDOWS /D_MBCS /DNDEBUG /D_LIB /DUSE_NEW_IOSTREAM /c /DTEST

# from IDE
#CFLAGS := /Oi /Os /Ot  /Ob /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS"  /MT   /W3 /nologo /c  /Zi /TP /Fd"Release/vc70.pdb"
#CFLAGS := /Od /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS"  /MT   /W3 /nologo /c /Zi  /TP /Fd"Release/vc70.pdb"
# CFLAGS += /Oy-

all: $(BIN) 
#/Oi /Og /Oy- 

windump$(BINEXT): archive  windump.$(OBJEXT) StackWalker.obj 
	$(LD) /DEBUG $(LFLAGS) $(LDOUT)$@ windump.$(OBJEXT) StackWalker.obj  $(LDROPT). $(LIBOPT)$(LIBNAME) $(LIBDIRS) $(LIBS) /MAP:dump.map

StackWalker$(BINEXT): StackWalker.obj MainStackWalker.obj windump1.obj windump2.obj windump3.obj windump4.obj windump5.obj windump6.obj windump7.obj 
	$(LD) /DEBUG $(LFLAGS) $(LDOUT)$@ StackWalker.obj MainStackWalker.obj   $(LDROPT). $(LIBOPT)$(LIBNAME) $(LIBDIRS) $(LIBS) /MAP:StackWalker.map

minidump$(BINEXT): minidump.obj
	$(LD) /DEBUG $(LFLAGS) $(LDOUT)$@ minidump.obj  $(LDROPT). $(LIBDIRS) $(LIBS) /MAP:StackWalker.map


mydump$(BINEXT):   mydump.$(OBJEXT) 
	$(LD) /DEBUG $(LFLAGS) $(LDOUT)$@ mydump.$(OBJEXT)  $(LIBDIRS) $(LIBS) /MAP:dump.map


thrLeak$(BINEXT):  thrLeak.$(OBJEXT) 
	$(LD) /DEBUG $(LFLAGS) $(LDOUT)$@ thrLeak.$(OBJEXT)  $(LDROPT). $(LIBDIRS) $(LIBS) 

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

