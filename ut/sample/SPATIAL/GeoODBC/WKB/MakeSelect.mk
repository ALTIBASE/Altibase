include $(ALTIBASE_HOME)/install/altibase_env.mk

CASENAME = selectWKB

BINS = $(CASENAME)$(BINEXT)

all : compile

compile: $(BINS)

$(BINS) : $(CASENAME).$(OBJEXT)  $(OBJS) $(QPLIB)
	$(LD) $(LFLAGS) $(LDOUT)$@ $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

clean:
	rm -rf core *.$(OBJEXT) *.res *.d SunWS_cache $(BINS)

