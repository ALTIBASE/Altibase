# $Id: Makefile 70397 2015-04-20 01:43:07Z heesuk.shin $
#

include ../utEnv.mk




UT_E_HEADER = utErrorCode.ih

all: ofiles

ofiles: ErrorFiles

compile: ErrorFiles

install:

ErrorFiles: E_UT_US7ASCII.c

E_UT_US7ASCII.c: E_UT_US7ASCII.msg
	$(GENERRMSG) -c -i E_UT_US7ASCII.msg -o $(UT_E_HEADER)
	$(COPY) -f $(UT_E_HEADER) ../util/include
	$(COPY) -f $@ ../util/ute

manual:
	$(GENERRMSGMANUAL) -c -i E_UT_US7ASCII.msg > $(ALTI_HOME)/msg/E_UT_US7ASCII.txt

header:
	@echo "no header"

check:
	@echo "no check"

clean:
	$(RM) core *.$(OBJEXT) *.d genErrMsg SunWS_cache *.msb *.ih *.ic E_UT_US7ASCII.c ../util/include/$(UT_E_HEADER) ../util/ute/E_UT_US7ASCII.c

clean_bin:
