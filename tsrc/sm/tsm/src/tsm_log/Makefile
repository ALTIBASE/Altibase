# $Id: Makefile 29457 2008-11-20 05:24:42Z ksjall $ #

# 수정해야 할  변수
#  DESTDIR  : 테스팅 섹션 디렉토리
#  BINS     : 실행화일명
#  BINSRCS  : 실행화일 소스
#  run.sh   : 테스트 수행할 소스 코드

include ../tsmEnv.mk

DESTDIR = $(TSM_DIR)/tsm_log
BINS    = tsm_log tsm_perf
BINSRCS = tsm_log.cpp tsm_perf.cpp
RUN_SHELL= run.sh
BINOBJS = $(BINSRCS:.cpp=.$(OBJEXT))
OBJS=$(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))
cmd_ld = $(LD) $(LFLAGS) $(LDOUT)$@ $^ $(TSMLIB) $(SMLIB) $(ID_LIB) $(LIBS)

all: $(COMPILE) install

compile:  $(OBJS) archive $(BINS)

install: $(BINS)
#	rm -rf $(DESTDIR)
#	mkdir $(DESTDIR)
#	cp $(BINS) $(DESTDIR)

tsm_log: tsm_log.$(OBJEXT) $(TSMLIB) $(SMLIB) 
	$(cmd_ld)

tsm_perf: tsm_perf.$(OBJEXT) $(TSMLIB) $(SMLIB) 
	$(cmd_ld)

clean:
	rm -rf core .dependency $(BINS) *.o *.d SunWS_cache *~ ./*.*~

dep:
	-rm -rf ./.dependency
	$(CXX) $(CXXOPT_DEPENDANCY) $(INCLUDES) $(DEFINES) $(SRCS) > .dependency

ifeq (./.dependency,$(wildcard ./.dependency))
include ./.dependency
endif





