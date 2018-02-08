include $(ALTIBASE_HOME)/install/altibase_env.mk

TARGETS = FailOverSample FailOverCallbackSample DistributedOLTPSample_ULFailOver
BINS = $(addsuffix $(BINEXT),$(TARGETS))

all : $(TARGETS)

DistributedOLTPSample_ULFailOver: DistributedOLTPSample_ULFailOver.$(OBJEXT) 
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

FailOverSample: FailOverSample.$(OBJEXT) 
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

FailOverCallbackSample: FailOverCallbackSample.$(OBJEXT) 
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

clean:
	rm -rf core* *.$(OBJEXT) *.res *.d SunWS_cache $(BINS)

