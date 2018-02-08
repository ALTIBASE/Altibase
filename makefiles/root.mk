# Copyright 1999-2007, ALTIBASE Corporation or its subsidiaries.
# All rights reserved.

# $Id: root.mk 11332 2010-06-24 02:17:05Z djin $
#

#
# [0] GNU make
#

#
# [0.1] Check GNU make features
#
ifeq ($(filter else-if,$(.FEATURES)),)
$(error Unsupported make: You need GNU make version 3.81 or higher)
else ifeq ($(filter second-expansion,$(.FEATURES)),)
$(error Unsupported make: You need GNU make version 3.81 or higher)
else ifeq ($(filter target-specific,$(.FEATURES)),)
$(error Unsupported make: You need GNU make version 3.81 or higher)
endif

#
# [0.3] Silent mode
#
ifneq ($(findstring s,$(filter-out -%,$(MAKEFLAGS))),)
SILENT_MODE = 1
Q = @
ifneq ($(MAKELEVEL),0)
ifeq ($(findstring w,$(filter-out -%,$(MAKEFLAGS))),)
MAKEFLAGS := w$(MAKEFLAGS)
endif
endif
else
Q =
endif

#
# [1] Project directory hierarchy
#
# Directory structure initialization moved to dirs.mk file
# in same directory where root.mk placed.
# The reason that the directory structure of the source code
# CORE repository and binary distribution package differs.
# Configuration of the binary distribution package directory
# hierarchy moved to dirs_deploy.mk which replace dirs.mk
# on binary package creation.
# BUG30088 - AIX works strange with realpath.
# On Windows realpath corrupt path.
ifeq ($(realpath $(realpath ./)/../), $(realpath ./../))
include $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/dirs.mk)
else
include $(dir $(lastword $(MAKEFILE_LIST)))/dirs.mk
endif
#
# [2] Variant
#

#
# [2.1] Override variant
#
ifneq ($(origin variant), undefined)
override variant_parts := $(subst -, , $(variant))
override compiler      := $(word 1, $(variant_parts))
override tool          := $(word 2, $(variant_parts))
override build_mode    := $(word 3, $(variant_parts))
override compile_bit   := $(word 4, $(variant_parts))
endif

#
# [2.2] VARIANT setup
#
ifeq ($(static_anlz), yes)
VARIANT = $(COMPILER)-$(TOOL)-$(BUILD_MODE)-$(COMPILE_BIT)-static
else
VARIANT = $(COMPILER)-$(TOOL)-$(BUILD_MODE)-$(COMPILE_BIT)
endif

#
# [3] Load platform-dependent makefiles
#

#
# [3.1] Load core platform-dependent makefiles
#
-include $(MAKEFILES_DIR)/config.mk

# Need this for OCFLAGS
-include $(ALTIDEV_HOME)/src/pd/makefiles/platform_macros.GNU

ifneq ($(CONFIG_MK_INCLUDED),)
include $(MAKEFILES_DIR)/platform.mk
endif

#
# [3.2] COMPILER setup
#
ifneq ($(DEFAULT_COMPILER),)
ifeq ($(origin compiler), undefined)
COMPILER = $(DEFAULT_COMPILER)
else
COMPILER = $(filter $(compiler), $(AVAILABLE_COMPILER))
ifneq ($(COMPILER), $(compiler))
$(error Unknown compiler: $(compiler))
endif
endif
endif

#
# [3.3] TOOL setup
ifneq ($(DEFAULT_TOOL),)
ifeq ($(origin tool), undefined)
TOOL = $(DEFAULT_TOOL)
else
TOOL = $(filter $(tool), $(AVAILABLE_TOOL))
ifneq ($(TOOL), $(tool))
$(error Unknown tool: $(tool))
endif
endif
endif

#
# [3.4] BUILD_MODE setup
#
# BUILD_MODE is set up in upstream configuration i.e. vars.mk[.in]
# Consequently, build_mode parameter is now unused.  Use BUILD_MODE
# instead.

#
# [3.5] COMPILE_BIT setup
#
ifneq ($(DEFAULT_COMPILE_BIT),)
ifeq ($(origin compile_bit), undefined)
COMPILE_BIT = $(DEFAULT_COMPILE_BIT)
else
COMPILE_BIT = $(filter $(compile_bit), $(AVAILABLE_COMPILE_BIT))
ifneq ($(COMPILE_BIT), $(compile_bit))
$(error Unknown compile bit: $(compile_bit))
endif
endif
endif

#
# [3.6] Additional predefined macros
#
DEFINES += $(DEFINES_M$(COMPILE_BIT))
DEFINES += ACP_CFG_COMPILER=$(COMPILER)
DEFINES += ACP_CFG_COMPILER_STR=\"$(COMPILER)\"
DEFINES += ACP_CFG_COMPILE_BIT=$(COMPILE_BIT)
DEFINES += ACP_CFG_COMPILE_BIT_STR=\"$(COMPILE_BIT)\"
DEFINES += ACP_CFG_COMPILE_$(COMPILE_BIT)BIT

OLDDEFINES += $(DEFINES_M$(COMPILE_BIT))
OLDDEFINES += ACP_CFG_COMPILER=$(COMPILER)
OLDDEFINES += ACP_CFG_COMPILER_STR=\"$(COMPILER)\"
OLDDEFINES += ACP_CFG_COMPILE_BIT=$(COMPILE_BIT)
OLDDEFINES += ACP_CFG_COMPILE_BIT_STR=\"$(COMPILE_BIT)\"
OLDDEFINES += ACP_CFG_COMPILE_$(COMPILE_BIT)BIT

ifeq ($(BUILD_MODE),debug)
DEFINES += ACP_CFG_DEBUG
DEFINES += ACP_CFG_BUILDMODE=\"d\"
DEFINES += ACP_CFG_BUILDMODEFULL=\"debug\"
else
ifeq ($(BUILD_MODE),prerelease)
DEFINES += ACP_CFG_BUILDMODE=\"p\"
DEFINES += ACP_CFG_BUILDMODEFULL=\"prerelease\"
else
DEFINES += ACP_CFG_BUILDMODE=\"r\"
DEFINES += ACP_CFG_BUILDMODEFULL=\"release\"
endif
endif

DEFINES += $(EXT_DEFINES)

#
# [3.7] Version includes
#

include $(MAKEFILES_DIR)/version.mk
DEFINES += ALTICORE_MAJOR_VERSION=$(ALTICORE_MAJOR_VERSION)
DEFINES += ALTICORE_MINOR_VERSION=$(ALTICORE_MINOR_VERSION)
DEFINES += ALTICORE_TERM_VERSION=$(ALTICORE_TERM_VERSION)
DEFINES += ALTICORE_PATCH_VERSION=$(ALTICORE_PATCH_VERSION)

ifeq ($(NOINLINE),yes)
DEFINES += ACP_CFG_NOINLINE
endif

#
# [4] Make build directory
#
ifneq ($(CONFIG_MK_INCLUDED),)
make_build_dir := $(shell $(MKDIR) $(BUILD_DIR))
endif



#
# [4-1] Unit Guard Executor Regist 
#

UNIT_GUARDER  := $(BINS_DIR)/utilUGuard$(EXEC_SUF)


#
# [5] Make default goal
#
.DEFAULT_GOAL  = build
