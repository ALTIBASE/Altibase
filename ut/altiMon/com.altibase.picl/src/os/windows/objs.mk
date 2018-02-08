# PICL source for IBM aix
# $Id
#

PICL_SRCS = $(UT_DIR)/altiMon/com.altibase.picl/src/os/windows/picl_windows.c \
            $(UT_DIR)/altiMon/com.altibase.picl/src/os/windows/util_picl.c

PICL_SHOBJS  = $(patsubst $(DEV_DIR)/%,$(TARGET_DIR)/%_soc.$(OBJEXT)     ,$(basename $(PICL_SRCS)))

#PICL_SHLIB_PATH = $(UT_DIR)/altiMon/com.altibase.altimon/lib/$(LIBPRE)picl_sl.$(SOEXT)
ifeq ($(compile64),1)
    PICL_SHLIB_PATH = $(UT_DIR)/altiMon/com.altibase.altimon/lib/win-x64-nt.dll
else
    PICL_SHLIB_PATH = $(UT_DIR)/altiMon/com.altibase.altimon/lib/win-x32-nt.dll
endif # compile64=1
