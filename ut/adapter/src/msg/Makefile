#
# Copyright 2011, Altibase Corporation or its subsidiaries.
# All rights reserved.
#

include $(ALTIDEV_HOME)/env.mk

ifeq "$(ALTI_CFG_OS)" "WINDOWS"
SHOBJTAILNAME = _shobj
else
SHOBJTAILNAME =
endif

INCLUDES += $(ALTICORE_INCLUDES)
CFLAGS += $(PIC) 

build: msgEN82.mdl oaMsg.$(OBJEXT)

clean:
	rm -f oaMsg.c oaMsg.h oaMsg.$(OBJEXT)
	rm -f msgEN82.c msgEN82_shobj.$(OBJEXT) msgEN82.mdl
	rm -f oaMsg.mk

oaMsg.c: oaMsg.msg
	genmsg oaMsg.msg

oaMsg$(SHOBJTAILNAME).$(OBJEXT): oaMsg.c
	$(COMPILE.c) $(LDOUT)$@ $^ $(INCLUDES) 

msgEN82.c: oaMsg.msg
	genmsg oaMsg.msg

msgEN82$(SHOBJTAILNAME).$(OBJEXT): msgEN82.c
	$(COMPILE.c) $(INCLUDES) $(LDOUT)$@ $^

msgEN82.mdl: msgEN82$(SHOBJTAILNAME).$(OBJEXT)
	$(LINK.c) $(SOFLAGS) $(LDOUT)$@ $^
