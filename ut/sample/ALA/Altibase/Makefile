include $(ALTIBASE_HOME)/install/altibase_env.mk

INCLUDES += $(IDROPT)$(ALTIBASE_HOME)/include  

LIBDIRS += $(LDROPT)$(ALTIBASE_HOME)/lib $(LDROPT).

LIBS += $(LIBOPT)ala$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT)

SRCS=ReplToAltiSample.c
OBJS=$(SRCS:.c=.$(OBJEXT))
	OUT=ReplToAltiSample$(BINEXT)

all: build

build: $(OBJS)
	    $(LD) $(OBJS) $(LFLAGS) $(LDOUT)$(OUT) $(LIBS)

clean:
	rm -f $(OUT)
	rm -f $(OBJS)
	rm -f *.res
				
