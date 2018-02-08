include ../tsmEnv.mk

PUROPT=-static-checking=yes -static-checking-default=aggressive -static-checking-guardzone=128 -free-queue-length=10000 -free-queue-threshold=100000

SRCS =  tsm.cpp tsmLobAPI.cpp

OBJS=$(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))

all: ofiles archive

ofiles:	$(OBJS)

header :
	@for i in $(SRCS); do ( cp $$i $$i.org ; cat $$i.org | gawk  -f $(TOOL_DIR)/add_header/header.awk > $$i ) done

check :
	@for i in $(SRCS); do (gawk -v API_INPUT=$(TOOL_DIR)/coding_chk/API.txt -v SRC_INPUT=$$i -f $(TOOL_DIR)/coding_chk/apiCheck.awk ) done

pure: ofiles archive

clean:
	$(RM) core *.$(OBJEXT) *.d SunWS_cache ./*~ ./*/*~

archive: $(TSMLIB)

$(TSMLIB): tsm.$(OBJEXT) tsmLobAPI.$(OBJEXT)

dep depend: 
	\$(RM) ./.dependency
	$(CXX) $(CXXOPT_DEPENDANCY) $(INCLUDES) $(SRCS) > .dependency

ifeq (./.dependency,$(wildcard ./.dependency))
include ./.dependency
endif
