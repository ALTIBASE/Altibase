# $Id: Makefile 22126 2007-06-13 03:10:38Z free1002 $

include ../../idEnv.mk

LIBDIRS := $(LDROPT)$(PD_DIR)/src/lib $(LDROPT)$(ALTI_HOME)/lib $(LIBDIRS)
DEFINES_ERR  = $(DEFOPT)GEN_ERR_MSG
DEFINES += $(DEFOPT)GEN_ERR_MSG
INCLUDES := $(INCLUDES) $(IDROPT).

SRCS = ../idl/idl.cpp ../ide/ideErrorMgr.cpp ../idu/iduVersion.cpp
ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
SRCS += ../idlw/idlwgetopt.cpp ../idlw/idlwgetrlimit.cpp
endif # INTEL_WINDOWS
OBJS = $(foreach src, $(SRCS),$(notdir $(src:.cpp=.$(OBJEXT))))

BINS	= genErrMsg$(BINEXT)

# 소스코드에 include 될 에러코드 화일
E_HEADER = idErrorCode.ih

# 에러코드 메시지 화일

all : fstr

fstr: fstr.o
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) -L$(ID_DIR)/src/lib $(LIBDIRS) $(LIBOPT)id$(LIBAFT) $(LIBOPT)pd$(LIBAFT) $(LIBS)




check :
	@for i in $(SRCS); do (gawk -v API_INPUT=$(TOOL_DIR)/coding_chk/API.txt -v SRC_INPUT=$$i -f $(TOOL_DIR)/coding_chk/apiCheck.awk ) done

alint: $(SRCS:.cpp=.lint)

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

