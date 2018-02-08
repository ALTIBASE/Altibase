# $Id: Makefile 70973 2015-05-26 07:30:32Z bethy $
#

include ../../utEnv.mk
include ../ut_util_objs.mk


SRCS = $(UT_UTE_SRCS)
OBJS = $(UT_UTE_OBJS)


.PHONY: all compile shobjs archive alint clean dep

all: compile shobjs

compile: $(OBJS)

shobjs: $(VSHOBJS)

archive:

alint: $(SRCS:.cpp=.lint)

clean:
	$(RM) core $(TARGET_DIR)/ut/util/ute *.d SunWS_cache

-include $(OBJS:.$(OBJEXT)=.d)
