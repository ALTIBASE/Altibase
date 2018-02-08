include $(ALTIBASE_HOME)/install/altibase_env.mk

# for static library
CDBC_LIBS = $(LIBS) $(foreach i, alticapi odbccli, $(LIBOPT)$(i)$(LIBAFT))
# for shared library
CDBC_SHLIBS = $(LIBS) $(foreach i, alticapi_sl, $(LIBOPT)$(i)$(LIBAFT))

ifeq "$(ALTICORE_HOME)" ""
    ALTICORE_HOME=/home/ataf/alticore_home
endif
ifeq "$(compile64)" "1"
    ALTI_CFG_BITTYPE=64
else
    ALTI_CFG_BITTYPE=32
endif



TESTNAMES = FailOverSample FailOverCallbackSample
OBJS = $(foreach i, $(TESTNAMES), $(i).$(OBJEXT))
BINS = $(foreach i, $(TESTNAMES), $(i)$(BINEXT))



all : build

build : compile

compile : $(TESTNAMES)

clean:
	$(RM) core SunWS_cache $(OBJS) $(BINS) *.ilk *.pdb *.manifest *.d



FailOverSample: FailOverSample.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

FailOverCallbackSample : FailOverCallbackSample.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)
