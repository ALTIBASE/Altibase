#####################################
#     Alticore Library Setting      #
#####################################

ALTICORE_HOME=$(ALTI_HOME)
ALTICORE_LIBDIR=$(ALTICORE_HOME)/lib

ifeq ($(OS_TARGET),INTEL_WINDOWS)

CFLAGS += /D_GNU_SOURCE

ifeq "$(BUILD_MODE)" "debug"
CCFLAGS += /DACP_CFG_DEBUG
CFLAGS += /DACP_CFG_DEBUG
endif

ifeq ($(compile64),1)
CCFLAGS += /DACP_CFG_COMPILE_64BIT /DACP_CFG_COMPILE_BIT=64 /DACP_CFG_COMPILE_BIT_STR="64"
CFLAGS += /DACP_CFG_COMPILE_64BIT /DACP_CFG_COMPILE_BIT=64 /DACP_CFG_COMPILE_BIT_STR="64"
else
CCFLAGS += /DACP_CFG_COMPILE_32BIT /DACP_CFG_COMPILE_BIT=32 /DACP_CFG_COMPILE_BIT_STR="32"
CFLAGS += /DACP_CFG_COMPILE_32BIT /DACP_CFG_COMPILE_BIT=32 /DACP_CFG_COMPILE_BIT_STR="32"
endif

else

CFLAGS += -D_GNU_SOURCE

ifeq "$(BUILD_MODE)" "debug"
CCFLAGS += -DACP_CFG_DEBUG
CFLAGS += -DACP_CFG_DEBUG
endif

ifeq ($(compile64),1)
CCFLAGS += -DACP_CFG_COMPILE_64BIT -DACP_CFG_COMPILE_BIT=64 -DACP_CFG_COMPILE_BIT_STR="64"
CFLAGS += -DACP_CFG_COMPILE_64BIT -DACP_CFG_COMPILE_BIT=64 -DACP_CFG_COMPILE_BIT_STR="64"
else
CCFLAGS += -DACP_CFG_COMPILE_32BIT -DACP_CFG_COMPILE_BIT=32 -DACP_CFG_COMPILE_BIT_STR="32"
CFLAGS += -DACP_CFG_COMPILE_32BIT -DACP_CFG_COMPILE_BIT=32 -DACP_CFG_COMPILE_BIT_STR="32"
endif
endif

ALTICORE_DEFINES  += -D_GNU_SOURCE
ALTICORE_INCLUDES += -I$(CORE_DIR)/include
ALTICORE_LDFLAGS  += -L$(ALTICORE_HOME)/lib \
                     -lalticore -laltictest \
                     -lm -lpthread -ldl -lncurses \
                     -rdynamic

#####################################
# alticore static library link      #
#####################################
ifeq ($(OS_TARGET),INTEL_WINDOWS)
ALTICORE_STATIC_LIB=$(ALTICORE_LIBDIR)/$(LIBPRE)alticore_static.$(LIBEXT)
ALTICTEST_STATIC_LIB=$(ALTICORE_LIBDIR)/$(LIBPRE)altictest_static.$(LIBEXT)
else
ALTICORE_STATIC_LIB=$(ALTICORE_LIBDIR)/$(LIBPRE)alticore.$(LIBEXT)
ALTICTEST_STATIC_LIB=$(ALTICORE_LIBDIR)/$(LIBPRE)altictest.$(LIBEXT)
endif

ifeq ($(UNITTEST_USE_STATIC_LIB), no)
#####################################
# 1-1. default alticore library     #
#####################################
ALTICORE_LIB=$(LIBOPT)alticore$(LIBAFT)
ALTICTEST_LIB=$(LIBOPT)altictest$(LIBAFT)
else
#####################################
# 1-2. alticore static library link #
#####################################
ALTICORE_LIB=$(ALTICORE_STATIC_LIB)
ALTICTEST_LIB=$(ALTICTEST_STATIC_LIB)
endif

#####################################
# 1-3. alticore shared library link #
#####################################
#ALTICORE_LIB=$(ALTICORE_LIBDIR)/$(LIBPRE)alticore.$(SOEXT)
#ALTICTEST_LIB=$(ALTICORE_LIBDIR)/$(LIBPRE)altictest.$(SOEXT)

LIBDIRS += $(LDROPT)$(ALTICORE_LIBDIR)
