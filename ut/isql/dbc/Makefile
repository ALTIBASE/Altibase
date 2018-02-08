# $Id$
#

include ../../utEnv.mk
include ../isql_objs.mk

INCLUDES += $(foreach i,$(ALTICORE_INCLUDES) $(ALTI_HOME)/lib $(UT_DIR)/lib ../include ., $(IDROPT)$(i))

SRCS = $(ISQL_DBC_SRCS)

OBJS=$(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))

all: $(OBJS)

clean:
	$(RM) core $(OBJS)

sure:

-include $(OBJS:.$(OBJEXT)=.d)
