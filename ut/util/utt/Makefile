# $Id: Makefile 79235 2017-03-06 05:21:11Z bethy $
#

include ../../utEnv.mk
include ../ut_util_objs.mk


SRCS = $(UT_UTT_SRCS)
OBJS = $(UT_UTT_OBJS)


.PHONY: all compile compile_pp compile_pg compile_pt shobjs archive alint clean dep

all: qpKeywords.c compile shobjs

qpKeywords.c : $(QP_DIR)/qcp/qcpll.l
	$(GREP) TR_ $< | $(GAWK) '{print $$1}' > a
	$(GREP) TK_ $< | $(GAWK) '{print $$1}' >> a
	$(GREP) TO_ $< | $(GAWK) '{print $$1}' >> a
	$(GREP) TA_ $< | $(GAWK) '{print $$1}' >> a
	$(GREP) TM_ $< | $(GAWK) '{print $$1}' >> a
	cat a | $(GREP) -v "\[" | $(GREP) -v "\/\*" | $(GREP) -v "\*\/" | $(GREP) -v return | sort > b
	$(SED) "s/^/(SChar *)\"/g" b > c
	$(SED) "s/$$/\",/g" c > $@
	$(RM) a b c

compile: $(OBJS)

compile_pp: $(PPOBJS)

compile_pg: $(PGOBJS)

compile_pt: $(PTOBJS)

shobjs: $(VSHOBJS)

archive:

alint: $(SRCS:.cpp=.lint)

clean:
	-$(RM) -f core .dependency .pure $(TARGET_DIR)/ut/util/utt *.d SunWS_cache

-include $(OBJS:.$(OBJEXT)=.d)
