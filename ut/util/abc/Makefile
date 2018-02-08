#
# $Id: Makefile 70397 2015-04-20 01:43:07Z heesuk.shin $
#

include ../../utEnv.mk
COMPILE = compile
UTUTILLIB = ../lib/altilibutil.a

INCLUDES += $(IDROPT)../include
LIBDIRS  += $(LDROPT)../lib

ifeq "$(OS_LINUX_KERNEL)" "1"
	CFLAGS+=-DLINUX_TIME
endif

SRCS=abc_analysis.cpp abc_time.cpp
OBJS=$(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))


all: $(COMPILE) archive

compile: $(OBJS)

compile_pp: $(PPOBJS)

compile_pg: $(PGOBJS)

compile_pt: $(PTOBJS)

alint: $(SRCS:.cpp=.lint)

clean:
	$(RM) core .dependency .pure *.$(OBJEXT) *.d SunWS_cache

archive: $(UTUTILLIB)

$(UTUTILLIB): $(OBJS) $(GENOBJS) $(ANTLROBJS)

-include $(OBJS:.$(OBJEXT)=.d)
