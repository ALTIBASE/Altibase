# $Id: Makefile 16842 2006-06-26 10:10:13Z shsuh $

include ../../idEnv.mk

LIBDIRS := $(LDROPT)$(ALTI_HOME)/lib
INCLUDES := $(INCLUDES) $(IDROPT).

BINS = varmemlist$(BINEXT) varmemlist2$(BINEXT)


all: $(BINS)


varmemlist$(BINEXT): varmemlist.$(OBJEXT)
	$(LD)  $(CCFLAGS)  $(PTDIRS)  $(LDOUT)$@ $^  $(LIBDIRS) $(LIBOPT)id$(LIBAFT) $(LIBOPT)pd$(LIBAFT) $(LIBS)

varmemlist2$(BINEXT): varmemlist2.$(OBJEXT)
	$(LD)  $(CCFLAGS)  $(PTDIRS)  $(LDOUT)$@ $^  $(LIBDIRS) $(LIBOPT)id$(LIBAFT) $(LIBOPT)pd$(LIBAFT) $(LIBS)

alint: $(SRCS:.cpp=.lint)

clean:
	$(RM) core *.$(OBJEXT) *.d genErrMsg  SunWS_cache  *.msb *.ih *~ .shobj $(BINS)
