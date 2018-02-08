#
# Copyright 2011, Altibase Corporation or its subsidiaries.
# All rights reserved.
#

include ../../../utEnv.mk

ifeq "$(ALTI_CFG_OS)" "WINDOWS"
SHOBJTAILNAME = _shobj
else
SHOBJTAILNAME =
endif
HBP_MSG_INCLUDES += $(ALTICORE_INCLUDES)
CFLAGS += $(PIC) 
CPPFLAGS += $(HBP_MSG_INCLUDES)

build: msgEN100.mdl hbpMsg$(SHOBJTAILNAME).$(OBJEXT)

clean:
	rm -f hbpMsg.c hbpMsg.h hbpMsg.$(OBJEXT)
	rm -f msgEN100.c msgEN100_shobj.$(OBJEXT) msgEN100.mdl
	rm -f hbpMsg.mk

msgEN100.c hbpMsg.c hbpMsg.h: generate_msg

.PHONY: generate_msg
generate_msg: hbpMsg.msg
	$(ALTICORE_HOME)/bin/genmsg hbpMsg.msg

msgEN100.mdl: msgEN100$(SHOBJTAILNAME).$(OBJEXT)
	$(LD) $(CFLAGS) $(SOFLAGS) $(LDOUT)$@ $^

ifeq "$(ALTI_CFG_OS)" "WINDOWS"
msgEN100$(SHOBJTAILNAME).$(OBJEXT): msgEN100.c
	$(SHCOMPILE.c) $(CFLAGS)  $(HBP_MSG_INCLUDES) -c  $(CCOUT)$@ $<
hbpMsg$(SHOBJTAILNAME).$(OBJEXT): hbpMsg.c hbpMsg.h
	$(SHCOMPILE.c) $(CFLAGS)  $(HBP_MSG_INCLUDES) -c  $(CCOUT)$@ $<
else
msgEN100$(SHOBJTAILNAME).$(OBJEXT): msgEN100.c
	$(CC) $(CFLAGS)  $(CPPFLAGS) -c  $(CCOUT) $@ $<
hbpMsg$(SHOBJTAILNAME).$(OBJEXT): hbpMsg.c hbpMsg.h
	$(CC) $(CFLAGS)  $(CPPFLAGS) -c  $(CCOUT) $@ $<
endif
