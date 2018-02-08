include $(ALTIBASE_HOME)/install/altibase_env.mk

%.cpp : %.sc
	apre -t cpp  $<

TARGETS = blobSample
BINS = $(addsuffix $(BINEXT),$(TARGETS))
	 
all: $(TARGETS)

blobSample: blobSample.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $< $(LIBOPT)apre$(LIBAFT)  $(LIBOPT)odbccli$(LIBAFT)  $(LIBS)

clean:
	-$(RM) $(BINS) *.$(OBJEXT) core *.exe *.d

