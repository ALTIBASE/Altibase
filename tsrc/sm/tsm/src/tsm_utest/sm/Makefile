# Makefile for SM volatile modules
#

include ../../../../smEnv.mk

SRCS = bug17603Test.cpp


OBJS		= $(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))
all: ofiles bin

bin: bug17603Test$(BINEXT)

ofiles:	$(OBJS) $(OBJS_CRDB)

clean:
	$(RM) core *.$(OBJEXT) *.d SunWS_cache bug17603Test$(BINEXT) *.txt

bug17603Test$(BINEXT): bug17603Test.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@ $^ $(SMLIB) $(ID_LIB) $(LIBS)
	./bug17603Test > result.txt

