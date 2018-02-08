# 수정해야 할  변수
#  DESTDIR  : 테스팅 섹션 디렉토리
#  BINS     : 실행화일명
#  BINSRCS  : 실행화일 소스
#  run.sh   : 테스트 수행할 소스 코드


include ../tsmEnv.mk

PUROPT=-static-checking=yes -static-checking-default=aggressive -static-checking-guardzone=128 -free-queue-length=10000 -free-queue-threshold=100000

DESTDIR = $(TSM_DIR)/tsm_compression
BINS =  tsm_iduCompression
BINSRCS = tsm_iduCompression.cpp

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

tsm_iduCompression: tsm_iduCompression.$(OBJEXT) $(TSMLIB)  $(SMLIB) 
	$(cmd_ld)

tsm_iduCompression.purify: tsm_iduCompression.$(OBJEXT) $(TSMLIB)  $(SMLIB) 
	purify $(PUROPT) $(TSM_MODE) $(cmd_ld) 

pure: tsm_iduCompression.purify install
	cp tsm_iduCompression.purify $(DESTDIR)/tsm_iduCompression


clean:
	$(RM) core .dependency $(BINS) *.$(OBJEXT) *.d SunWS_cache *~ ./*.*~

dep:
	-$(RM) ./.dependency
	$(CXX) $(CXXOPT_DEPENDANCY) $(INCLUDES) $(DEFINES) $(SRCS) > .dependency

ifeq (./.dependency,$(wildcard ./.dependency))
include ./.dependency
endif

