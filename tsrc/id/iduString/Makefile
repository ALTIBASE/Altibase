# $Id: Makefile 15141 2006-02-16 01:46:49Z shsuh $

include ../../idEnv.mk

LIBDIRS := $(LDROPT)$(ALTI_HOME)/lib
INCLUDES := $(INCLUDES) $(IDROPT).

BINS = testAppendString$(BINEXT) testString$(BINEXT)


all: $(BINS)


testAppendString$(BINEXT): testAppendString.$(OBJEXT)
	$(LD)  $(CCFLAGS)  $(PTDIRS)  $(LDOUT)$@ $^  $(LIBDIRS) $(LIBOPT)id$(LIBAFT) $(LIBOPT)pd$(LIBAFT) $(LIBS)

testString$(BINEXT): testString.$(OBJEXT)
	$(LD)  $(CCFLAGS)  $(PTDIRS)  $(LDOUT)$@ $^  $(LIBDIRS) $(LIBOPT)id$(LIBAFT) $(LIBOPT)pd$(LIBAFT) $(LIBS)

alint: $(SRCS:.cpp=.lint)

clean:
	$(RM) core *.$(OBJEXT) *.d genErrMsg  SunWS_cache  *.msb *.ih *~ .shobj $(BINS)
