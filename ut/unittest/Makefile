####################################################
#   Unit Test Makefile
#
#   Usage :
#       unittestUtUtString.cpp
#       unittestUtIloCommandLexer.cpp
#       unittestUtIloCommandCompiler.cpp
####################################################

BLD_DIR = ut/unittest

include $(ALTIDEV_HOME)/unittest.mk

INCLUDES += $(foreach i, isql iloader3 utm audit util, $(ALTIDEV_HOME)/ut/$(i)/include)
INCLUDES += $(ALTIDEV_HOME)/ut/lib

ISQL_UNITTEST_TARGETS = 

ILOADER_UNITTEST_TARGETS =  utIloCommandLexer \
							utIloCommandCompiler

AEXPORT_UNITTEST_TARGETS =

AUDIT_UNITTEST_TARGETS =

ISPAPI_UNITTEST_TARGETS = utUtString 

UNITTEST_TARGETS = utSample
UNITTEST_TARGETS += $(ISQL_UNITTEST_TARGETS) $(ILOADER_UNITTEST_TARGETS) $(AEXPORT_UNITTEST_TARGETS) $(AUDIT_UNITTEST_TARGETS) $(ISPAPI_UNITTEST_TARGETS)

utSample_NAME = unittestUtSample
utSample_SRCS = unittestUtSample.cpp
utSample_SHLIBS = alticore altictest id_so pd_so

utUtString_NAME = unittestUtUtString
utUtString_SRCS = unittestUtUtString.cpp
utUtString_SHLIBS = alticore altictest ispapi_sl odbccli_sl altiutil_sl id_so pd_so

utIloCommandLexer_NAME = unittestUtIloCommandLexer
utIloCommandLexer_SRCS = unittestUtIloCommandLexer.cpp
utIloCommandLexer_SHLIBS = alticore altictest iloader_sl id_so pd_so

utIloCommandCompiler_NAME = unittestUtIloCommandCompiler
utIloCommandCompiler_SRCS = unittestUtIloCommandCompiler.cpp
utIloCommandCompiler_SHLIBS = alticore altictest iloader_sl id_so pd_so

# Run unittests one by one.
.NOTPARALLEL: unittest_content

include $(MAKEFILES_DIR)/rule.mk
