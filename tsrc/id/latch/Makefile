# $Id: Makefile 19507 2006-12-18 07:08:58Z sjkim $

include ../../idEnv.mk

LIBDIRS := $(LDROPT)$(PD_DIR)/src/lib $(LDROPT)$(ALTI_HOME)/lib $(LIBDIRS)
DEFINES_ERR  = $(DEFOPT)GEN_ERR_MSG
DEFINES += $(DEFOPT)GEN_ERR_MSG
INCLUDES := $(INCLUDES) $(IDROPT).

SRCS = ../idl/idl.cpp ../ide/ideErrorMgr.cpp ../ide/ideErrorMgr_server.cpp ../idu/iduVersion.cpp
OBJS = $(foreach src, $(SRCS),$(notdir $(src:.cpp=.$(OBJEXT))))

BINS	= genErrMsg$(BINEXT)

# 소스코드에 include 될 에러코드 화일
E_HEADER = idErrorCode.ih

# 에러코드 메시지 화일

ofiles: $(BINS) ErrorFiles
shobjs: 

install: $(BINS)
	@cp $(BINS) ../bin


checkProp: checkProp.o
	$(CXX) $(CCFLAGS) $(DEFINES_ERR) $(CPPFLAGS) $(PTDIRS) $(INCLUDES) $(LIBDIRS) -L$(ID_DIR)/src/lib -o $@ $<  $(LIBOPT)pd$(LIBAFT) $(LIBOPT)id$(LIBAFT) $(LIBS)

#testMutex.$(OBJEXT):testMutex.cpp
#	$(LD) $(LDOUT)testMutex $(LIBDIRS)

testMutex$(BINEXT): testMutex.$(OBJEXT)
	$(LD)  $(CCFLAGS) $(PTDIRS)  $(LIBDIRS)  $(LDOUT)testMutex$(BINEXT) testMutex.$(OBJEXT)  $(LIBOPT)pd$(LIBAFT) $(LIBOPT)id$(LIBAFT) $(LIBS)

testLatch.$(OBJEXT):testLatch.cpp
	$(CXX) $(CCFLAGS) $(DEFINES_ERR) $(CPPFLAGS) $(PTDIRS) $(INCLUDES) $(LIBDIRS) -L$(ID_DIR)/src/lib -c testLatch.cpp 

testLatch$(BINEXT): testLatch.$(OBJEXT)
	$(LD)  $(CCFLAGS)  $(PTDIRS)  $(LIBDIRS)  $(LDOUT)testLatch$(BINEXT) testLatch.$(OBJEXT)  $(LIBOPT)pd$(LIBAFT) $(LIBOPT)id$(LIBAFT) $(LIBS)

testLatchFinal$(BINEXT): testLatchFinal.$(OBJEXT)
	$(LD)  $(CCFLAGS)  $(PTDIRS)  $(LIBDIRS)  $(LDOUT)testLatchFinal$(BINEXT) testLatchFinal.$(OBJEXT)  $(LIBOPT)pd$(LIBAFT) $(LIBOPT)id$(LIBAFT) $(LIBS)

testmini: testmini.o
	$(LD) $(LFLAGS) -g -o $@ $< $(INCLUDES) -L$(ID_DIR)/src/lib $(LIBDIRS) $(LIBOPT)id$(LIBAFT) $(LIBOPT)pd$(LIBAFT) $(LIBS)


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

