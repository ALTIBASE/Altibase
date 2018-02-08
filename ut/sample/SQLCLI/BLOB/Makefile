include $(ALTIBASE_HOME)/install/altibase_env.mk

TARGETS = blobSample
BINS = $(addsuffix $(BINEXT),$(TARGETS))
	 
all: $(TARGETS)

blobSample: blobSample.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

clean:
	-$(RM) $(BINS) *.$(OBJEXT) core *.exe *.d

