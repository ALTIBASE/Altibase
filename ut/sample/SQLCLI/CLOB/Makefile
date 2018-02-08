include $(ALTIBASE_HOME)/install/altibase_env.mk

TARGETS = clobSample
BINS = $(addsuffix $(BINEXT),$(TARGETS))
	 
all: $(TARGETS)

clobSample: clobSample.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

clean:
	-$(RM) $(BINS) *.$(OBJEXT) core *.exe *.d

