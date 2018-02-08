# $Id: Makefile 16842 2006-06-26 10:10:13Z shsuh $

include ../../idEnv.mk

LIBDIRS := $(LDROPT)$(ALTI_HOME)/lib
INCLUDES := $(INCLUDES) $(IDROPT).

BINS = iduPriorityQueueTest$(BINEXT) iduHeapSortTest$(BINEXT)


all: $(BINS)

iduPriorityQueueTest$(BINEXT): iduPriorityQueueTest.$(OBJEXT)
	$(LD)  $(CCFLAGS)  $(PTDIRS)  $(LDOUT)$@ $^  $(LIBDIRS) $(LIBOPT)id$(LIBAFT) $(LIBOPT)pd$(LIBAFT) $(LIBS)

iduHeapSortTest$(BINEXT): iduHeapSortTest.$(OBJEXT)
	$(LD)  $(CCFLAGS)  $(PTDIRS)  $(LDOUT)$@ $^  $(LIBDIRS) $(LIBOPT)id$(LIBAFT) $(LIBOPT)pd$(LIBAFT) $(LIBS)



alint: $(SRCS:.cpp=.lint)

clean:
	$(RM) core *.$(OBJEXT) *.d genErrMsg  SunWS_cache  *.msb *.ih *~ .shobj $(BINS)
