# $Id: Makefile 28459 2008-10-09 02:45:45Z newdaily $ #

# 수정해야 할  변수
#  DESTDIR  : 테스팅 섹션 디렉토리
#  BINS     : 실행화일명
#  BINSRCS  : 실행화일 소스
#  run.sh   : 테스트 수행할 소스 코드

include ../tsmEnv.mk

DESTDIR = $(TSM_DIR)/tsm_mempool2
BINS =  tsm_mempool2
BINSRCS = tsm_mempool2.cpp

RUN_SHELL= run.sh
BINOBJS = $(BINSRCS:.cpp=.$(OBJEXT))

OBJS=$(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))
cmd_ld = $(LD) $(LFLAGS) $(LDOUT)$@ $^ $(TSMLIB) $(SMLIB) $(ID_LIB) $(LIBS)

all: $(COMPILE) install

compile:  $(OBJS) archive $(BINS)

install: $(BINS)
	$(RM) $(DESTDIR)
	mkdir $(DESTDIR)
	cp $(BINS) $(RUN_SHELL) $(DESTDIR)

tsm_mempool2: tsm_mempool2.$(OBJEXT) $(SMLIB) $(DEPLIBS)
	$(TSM_MODE) $(LD) $(LFLAGS) -o $@ $< -lsm $(ID_LIB) $(LIBS)

clean:
	$(RM) core .dependency $(BINS) *.$(OBJEXT) *.d SunWS_cache *~ ./*.*~

dep:
	-$(RM) ./.dependency
	$(CXX) $(CXXOPT_DEPENDANCY) $(INCLUDES) $(DEFINES) $(SRCS) > .dependency

ifeq (./.dependency,$(wildcard ./.dependency))
include ./.dependency
endif
