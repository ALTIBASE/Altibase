# $Id: Makefile 29457 2008-11-20 05:24:42Z ksjall $ #

# 수정해야 할  변수
#  DESTDIR  : 테스팅 섹션 디렉토리
#  BINS     : 실행화일명
#  BINSRCS  : 실행화일 소스
#  run.sh   : 테스트 수행할 소스 코드

include ../tsmEnv.mk

DESTDIR = $(TSM_DIR)/tsm_sync
BINS    = tsm_sync tsm_sync_select
BINSRCS = tsm_sync.cpp
BINOBJS = $(BINSRCS:.cpp=.$(OBJEXT))
OBJS=$(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))
cmd_ld = $(LD) $(LFLAGS) $(LDOUT)$@ $^ $(TSMLIB) $(SMLIB) $(ID_LIB) $(LIBS)

BINSRCS2 =  tsm_sync_select.cpp
BINOBJS2 = $(BINSRCS2:.cpp=.$(OBJEXT))

all: $(COMPILE) install

compile:  $(OBJS) archive $(BINS)

install: $(BINS)
	$(RM) $(DESTDIR)
	mkdir $(DESTDIR)
	cp $(BINS) $(DESTDIR)

tsm_sync: $(BINOBJS) $(TSMLIB) $(SMLIB) 
	$(cmd_ld)

tsm_sync_select: $(BINOBJS2) $(TSMLIB) $(SMLIB) 
	$(cmd_ld)

clean:
	$(RM) core .dependency $(BINS) *.$(OBJEXT) *.d SunWS_cache *~ ./*.*~

dep:
	-$(RM) ./.dependency
	$(CXX) $(CXXOPT_DEPENDANCY) $(INCLUDES) $(DEFINES) $(SRCS) > .dependency

ifeq (./.dependency,$(wildcard ./.dependency))
include ./.dependency
endif





