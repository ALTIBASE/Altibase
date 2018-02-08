# PICL source for IBM aix
# $Id
#

PICL_SRCS = $(UT_DIR)/altiMon/com.altibase.picl/src/os/linux/picl_linux.c \
            $(UT_DIR)/altiMon/com.altibase.picl/src/os/linux/util_picl.c

PICL_SHOBJS  = $(patsubst $(DEV_DIR)/%,$(TARGET_DIR)/%_soc.$(OBJEXT)     ,$(basename $(PICL_SRCS)))

#PICL_SHLIB_PATH = $(UT_DIR)/altiMon/com.altibase.altimon/lib/$(LIBPRE)picl_sl.$(SOEXT)

ifeq "$(OS_TARGET)" "POWERPC64_LINUX"

PICL_SHLIB_PATH = $(UT_DIR)/altiMon/com.altibase.altimon/lib/linux-ppc64.so

else

ifeq "$(OS_TARGET)" "POWERPC64LE_LINUX"

PICL_SHLIB_PATH = $(UT_DIR)/altiMon/com.altibase.altimon/lib/linux-ppc64.so

else

ifeq ($(compile64),1)
    PICL_SHLIB_PATH = $(UT_DIR)/altiMon/com.altibase.altimon/lib/linux-x64.so
else
    PICL_SHLIB_PATH = $(UT_DIR)/altiMon/com.altibase.altimon/lib/linux-x32.so
endif # compile64=1

endif

endif
