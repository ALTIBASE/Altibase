# Copyright 1999-2010, ALTIBASE Corporation or its subsidiaries.
# All rights reserved.

# $Id: dirs.mk
#

include $(ALTIDEV_HOME)/vars.mk

#
# [1.0] ALTICORE_HOME setup
#
# BUG30088 - AIX works strange with realpath.
# On Windows realpath corrupt path.
ifeq ($(realpath $(realpath ./)/../), $(realpath ./../))
ALTICORE_HOME        := $(realpath $(realpath $(dir $(lastword $(MAKEFILE_LIST))))/../../)
else
ALTICORE_HOME        := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/../../)
endif
ALTICORE_HOME        := $(patsubst %/,%,$(ALTICORE_HOME))

#
# [1.1] WORK_DIR setup
#
WORK_DIR             := $(subst $(DEV_DIR)/, , $(CURDIR))
WORK_DIR             := $(patsubst %/,%,$(WORK_DIR))

#
# [1.2] project dirs setup
#
MAKEFILES_DIR         := $(DEV_DIR)/makefiles
UTILS_DIR             := $(CORE_DIR)/utils
BINS_DIR              := $(ALTI_HOME)/bin
LIBS_DIR              := $(ALTI_HOME)/lib

#
# [1.3] Product DIR setup
#
ATF_DIR              := $(ALTICORE_HOME)/tool/atf
CORE_DIR             := $(DEV_DIR)/src/core
ALEMON_DIR           := $(DEV_DIR)/tool/alemon
ALM_DIR              := $(ALTICORE_HOME)/alm

#
# [1.4] BUILD_ROOT setup
#
BUILD_ROOT            = $(TARGET_DIR)

#
# [1.5] BUILD_DIR setup
#
BUILD_DIR             = $(BUILD_ROOT)/$(WORK_DIR)

CORE_BUILD_DIR        = $(BUILD_ROOT)/src/core
ALM_BUILD_DIR         = $(BUILD_ROOT)/alm
