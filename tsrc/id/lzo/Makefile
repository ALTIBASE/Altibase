# $Id: Makefile 25182 2008-02-21 12:05:25Z jdlee $

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

ofiles: $(BINS) ErrorFiles
shobjs: 

install: $(BINS)
	@cp $(BINS) ../bin

genErrMsg$(BINEXT): genErrMsg.cpp
ifneq "$(OS_LINUX_PACKAGE)" "ARM_WINCE"
	$(COMPILE.cc) $(DEFINES_ERR) $(INCLUDES) $^
	$(COMPILE.cc) $(DEFINES_ERR) $(INCLUDES) $(SRCS)
	$(LD) $(LFLAGS) $(LDOUT)$@ $(subst .cpp,.$(OBJEXT),$^) $(OBJS) $(LIBOPT)pd$(LIBAFT) $(LIBS)
else # ARM_WINCE
	$(COPY) $(DEV_DIR)/win32-build/bin/genErrMsg$(BINEXT) .
endif # ARM_WINCE

genErrMsgManual$(BINEXT): genErrMsgManual.$(OBJEXT)
	$(CXX) $(CCFLAGS) $(DEFINES_ERR) $(CPPFLAGS) $(PTDIRS) $(INCLUDES) $(LDFLAGS) $(LIBDIRS) -o $@ $< ../idl/idl.cpp ../ide/ideErrorMgr.cpp ../idu/iduVersion.cpp $(LIBOPT)pd$(LIBAFT) $(LIBS)
#	@cp $(BINS) ../bin


checkProp: checkProp.o
	$(CXX) $(CCFLAGS) $(DEFINES_ERR) $(CPPFLAGS) $(PTDIRS) $(INCLUDES) $(LIBDIRS) -L$(ID_DIR)/src/lib -o $@ $<  $(LIBOPT)pd$(LIBAFT) $(LIBOPT)id$(LIBAFT) $(LIBS)

testmini: testmini.o
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) -L$(ID_DIR)/src/lib $(LIBDIRS) $(LIBOPT)id$(LIBAFT) $(LIBOPT)pd$(LIBAFT) $(LIBS)


benchmark: benchmark.$(OBJEXT) 
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) -L$(ID_DIR)/src/lib $(LIBDIRS) $(LIBOPT)id$(LIBAFT) $(LIBOPT)pd$(LIBAFT) $(LIBS)



ErrorFiles : E_ID_US7ASCII.msb

E_ID_US7ASCII.msb : E_ID_US7ASCII.msg
	./genErrMsg$(BINEXT) -i E_ID_US7ASCII.msg -o $(E_HEADER)
	cp -f $(E_HEADER) ../include	
	cp -f $@ $(ALTI_HOME)/msg
	cp -f $@ $(ALTI_HOME)/msg/E_ID_KO16KSC5601.msb

manual: genErrMsgManual
	./genErrMsgManual -i E_ID_US7ASCII.msg  > E_ID_US7ASCII.txt


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

