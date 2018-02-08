include $(ALTIDEV_HOME)/vars.mk
include $(ALTIDEV_HOME)/alticore.mk
include $(ALTIDEV_HOME)/makefiles/root.mk
BUILD_DIR = $(TARGET_DIR)/$(BLD_DIR)

make_build_dir := $(shell $(MKDIR) $(BUILD_DIR))

LIBS =
ifeq ($(ALTI_CFG_OS),LINUX)
SHLIBS = rt crypt
else ifeq ($(ALTI_CFG_OS),SOLARIS)
SHLIBS = kstat demangle iostream
else
SHLIBS =
endif
LIBOPT =

LD_USE_CPP = 1

INCLUDES  = $(INC_OPT)$(CORE_DIR)/include
INCLUDES += $(INC_OPT)$(ALTIDEV_HOME)/src/id/include
INCLUDES += $(INC_OPT)$(ALTIDEV_HOME)/src/mt/include
INCLUDES += $(INC_OPT)$(ALTIDEV_HOME)/src/sm/include
INCLUDES += $(INC_OPT)$(ALTIDEV_HOME)/src/pd/include
INCLUDES += $(INC_OPT)$(ALTIDEV_HOME)/src/qp/include
INCLUDES += $(INC_OPT)$(ALTIDEV_HOME)/src/dk/include
INCLUDES += $(INC_OPT)$(ALTIDEV_HOME)/src/cm/include
INCLUDES += $(INC_OPT)$(ALTIDEV_HOME)/src/mm/include
INCLUDES += $(INC_OPT)$(ALTIDEV_HOME)/src/rp/include
INCLUDES += $(INC_OPT)$(ALTIDEV_HOME)/src/ul/include
INCLUDES += $(INC_OPT)$(ALTIDEV_HOME)/src/ul/ulp/include
INCLUDES += $(INC_OPT)$(ALTIDEV_HOME)/src/pd/makeinclude
INCLUDES += $(INC_OPT)$(ALTI_HOME)/include

LIBDIRS  = $(ALTICORE_LIBDIR)
LIBDIRS += $(ALTI_HOME)/lib

ALINT_USE = 0
ALINT_OUTPUT_DIR =

ifeq ($(ALTI_CFG_OS),WINDOWS)
DEFINES += ACP_CFG_DL_STATIC
LD_FLAGS +=/NODEFAULTLIB:LIBCMT
LIB_SUF = .lib
endif

ifeq ($(ALTI_CFG_OS),DARWIN)
LD_FLAGS += -flat_namespace
endif

