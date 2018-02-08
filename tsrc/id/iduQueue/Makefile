# $Id: Makefile 16842 2006-06-26 10:10:13Z shsuh $

include ../../idEnv.mk

LIBDIRS := $(LDROPT)$(ALTI_HOME)/lib
INCLUDES := $(INCLUDES) $(IDROPT).

BINS = iduQueueTest$(BINEXT) iduQueuePerf$(BINEXT)


all: $(BINS)


iduQueueTest$(BINEXT): iduQueueTest.$(OBJEXT)
	$(LD)  $(CCFLAGS)  $(PTDIRS)  $(LDOUT)$@ $^  $(LIBDIRS) $(LIBOPT)id$(LIBAFT) $(LIBOPT)pd$(LIBAFT) $(LIBS)

iduQueuePerf$(BINEXT): iduQueuePerf.$(OBJEXT)
	$(LD)  $(CCFLAGS)  $(PTDIRS)  $(LDOUT)$@ $^  $(LIBDIRS) $(LIBOPT)id$(LIBAFT) $(LIBOPT)pd$(LIBAFT) $(LIBS)

alint: $(SRCS:.cpp=.lint)

clean:
	$(RM) core *.$(OBJEXT) *.d genErrMsg  SunWS_cache  *.msb *.ih *~ .shobj $(BINS)
