include $(ALTIBASE_HOME)/install/altibase_env.mk

TARGETS = sslSample
BINS = $(addsuffix $(BINEXT),$(TARGETS))
	 
all: $(TARGETS)

sslSample: sslSample.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

clean:
	-$(RM) $(BINS) *.$(OBJEXT) core *.exe *.d

