include $(ALTIBASE_HOME)/install/altibase_env.mk

INCLUDES += $(IDROPT)$(ATC_HOME)/include
# for static library
CDBC_LIBS = $(foreach i, alticapi odbccli, $(LIBOPT)$(i)$(LIBAFT)) $(LIBS)
# for shared library
CDBC_SHLIBS = $(foreach i, alticapi_sl, $(LIBOPT)$(i)$(LIBAFT)) $(LIBS)



DEMONAMES = sslSample 
OBJS = $(foreach i, $(DEMONAMES), $(i).$(OBJEXT))
BINS = $(foreach i, $(DEMONAMES), $(i)$(BINEXT))


all : build

build : compile

compile : $(DEMONAMES)

clean :
	$(RM) core SunWS_cache $(OBJS) $(BINS) *.ilk *.pdb *.manifest *.d


sslSample : sslSample.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

