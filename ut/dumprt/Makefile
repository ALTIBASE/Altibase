include $(ALTI_HOME)/install/altibase_env.mk

INCLUDES +=$(IDROPT)$(ATC_HOME)/include $(IDROPT). $(IDROPT)$(ALTIBASE_DEV)/src/id/include $(IDROPT)$(ALTIBASE_DEV)/src/pd/include  $(IDROPT)$(ALTIBASE_DEV)/pd/src/makeinclude
LIBDIRS  +=$(LDROPT)$(ALTI_HOME)/lib $(LDROPT).

BINS=dumprt$(BINEXT)
SRCS=
OBJS=$(SRCS:.cpp=.$(OBJEXT) )

all : compile

compile: $(BINS)

dumprt$(BINEXT) : dumprt.$(OBJEXT) $(OBJS) $(QPLIB)
	$(LD) $(LFLAGS) $(LDOUT)$@ dumprt.$(OBJEXT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

clean:
	rm -rf core *.$(OBJEXT)  *.d SunWS_cache $(BINS)

-include $(OBJS:.$(OBJEXT)=.d)
