include $(ALTIBASE_HOME)/install/altibase_env.mk

CHKSVR_LIBS = $(foreach i, chksvr, $(LIBOPT)$(i)$(LIBAFT)) $(LIBS)

SAMPLE_NAMES = checkServer checkServerCancel
BINS = $(foreach i, $(SAMPLE_NAMES), $(i)$(BINEXT))

cmd_ld = $(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CHKSVR_LIBS)

all: $(SAMPLE_NAMES)

checkServer: checkServer.$(OBJEXT)
	$(cmd_ld)

checkServerCancel: checkServerCancel.$(OBJEXT)
	$(cmd_ld)

clean:
	-$(RM) $(BINS) *.$(OBJEXT) core
