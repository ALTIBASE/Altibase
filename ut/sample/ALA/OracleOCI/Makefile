#
# Copyright 2011, Altibase Corporation or its subsidiaries.
# All rights reserved.
#

include $(ALTIBASE_HOME)/install/altibase_env.mk

# for ALA
INCLUDES += $(IDROPT)$(ALTIBASE_HOME)/include $(IDROPT).
LIBDIRS += $(LDROPT)$(ALTIBASE_HOME)/lib $(LDROPT).
LIBS += $(LIBOPT)ala$(LIBAFT) $(LIBOPT)alticore$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT)

# for OCI
INCLUDES += $(IDROPT)$(ORACLE_HOME)/rdbms/public 
LIBDIRS += $(LDROPT)$(ORACLE_HOME)/lib
LIBS += $(LIBOPT)clntsh

SRCS=Main.c OciApplier.c AlaReceiver.c
OBJS=$(SRCS:.c=.$(OBJEXT))
OUT=AlaToOci$(BINEXT)

all: build

build: $(OBJS)
	    $(LD) $(OBJS) $(LFLAGS) $(LDOUT)$(OUT) $(LIBS)

clean:
	rm -f $(OUT)
	rm -f $(OBJS)
