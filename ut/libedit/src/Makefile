# $Id: Makefile 70973 2015-05-26 07:30:32Z bethy $
#

include ../../utEnv.mk
include ./libedit_objs.mk


INCLUDES += $(IDROPT)../include

SRCS = $(LIBEDIT_SRCS)

OBJS = $(LIBEDIT_OBJS)


.PHONY: all compile header check alint clean dep

all: compile

compile: $(OBJS)

header:
	@for i in $(SRCS); do ( $(COPY) $$i $$i.org ; cat $$i.org | gawk  -f $(TOOL_DIR)/add_header/header.awk > $$i ) done

check:
	@for i in $(SRCS); do (gawk -v API_INPUT=$(TOOL_DIR)/coding_chk/API.txt -v SRC_INPUT=$$i -f $(TOOL_DIR)/coding_chk/apiCheck.awk ) done

alint: $(SRCS:.c=.lint)

clean:
	$(RM) core .dependency $(TARGET_DIR)/ut/libedit/src SunWS_cache

-include $(OBJS:.$(OBJEXT)=.d)
