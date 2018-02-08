# Makefile for SM volatile modules
#

include ../../../../smEnv.mk

SRCS = svrLogMgrTest.cpp


OBJS		= $(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))
all: ofiles bin

bin: svrLogMgrTest$(BINEXT)

ofiles:	$(OBJS) $(OBJS_CRDB)

clean:
	$(RM) core *.$(OBJEXT) *.d SunWS_cache svrLogMgrTest$(BINEXT) *.txt

svrLogMgrTest$(BINEXT): svrLogMgrTest.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@ $^ $(SMLIB) $(ID_LIB) $(LIBS)
	./svrLogMgrTest > result.txt

