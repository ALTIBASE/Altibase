# $Id: Makefile 19507 2006-12-18 07:08:58Z sjkim $

include ../../idEnv.mk

LIBDIRS := $(LDROPT)$(PD_DIR)/src/lib $(LDROPT)$(ALTI_HOME)/lib $(LIBDIRS)
INCLUDES := $(INCLUDES) $(IDROPT).

# 에러코드 메시지 화일
all: testSystem$(BINEXT)

testSystem$(BINEXT): testSystem.$(OBJEXT)
	$(LD)  $(LFLAGS)  $(LIBDIRS)  $(LDOUT)testSystem$(BINEXT) testSystem.$(OBJEXT)  $(LIBOPT)pd$(LIBAFT) $(LIBOPT)id$(LIBAFT) $(LIBS)


ErrorFiles : E_ID_US7ASCII.msb


clean:
	$(RM) core *.$(OBJEXT) *.d genErrMsg  SunWS_cache  *.msb *.ih *~ .shobj

clean_bin:
	$(RM) $(BINS)

dep depend:
	-$(RM) ./.dependency
	$(CXX) $(CXXOPT_DEPENDANCY) $(INCLUDES) $(SRCS) > .dependency

ifeq (./.dependency,$(wildcard ./.dependency))
include ./.dependency
endif

