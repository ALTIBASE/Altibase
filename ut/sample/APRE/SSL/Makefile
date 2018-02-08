include $(ALTIBASE_HOME)/install/altibase_env.mk

%.cpp : %.sc
	apre -t cpp $<

TARGETS = sslSample
BINS = $(addsuffix $(BINEXT),$(TARGETS))

all: $(BINS) 

sslSample: sslSample.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $< $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

clean:
	$(Q) $(RM) $(BINS) *.$(OBJEXT) core *.class *.exe *.d

