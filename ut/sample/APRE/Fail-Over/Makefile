include $(ALTIBASE_HOME)/install/altibase_env.mk

%.cpp : %.sc
	apre -t cpp $<

TARGETS = FailOverSample
BINS = $(addsuffix $(BINEXT),$(TARGETS))

all: $(TARGETS)

FailOverSample: FailOverSample.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $< $(LIBOPT)sesc$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

clean:
	$(Q) $(RM) $(BINS) *.$(OBJEXT) core *.class *.exe *.res *.d

