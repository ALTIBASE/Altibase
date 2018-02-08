# $Id: ut_util_objs.mk 71172 2015-06-07 23:49:00Z bethy $
#

UT_UTE_SRCS = $(UT_DIR)/util/ute/uteErrorMgr.cpp

UT_UTE_OBJS = $(UT_UTE_SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))

UT_UTE_SHOBJS = $(UT_UTE_SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%_shobj.$(OBJEXT))

UT_UTT_SRCS = $(UT_DIR)/util/utt/uttTime.cpp \
			  $(UT_DIR)/util/utt/uttMemory.cpp \
			  $(UT_DIR)/util/utt/utString.cpp

UT_UTT_OBJS = $(UT_UTT_SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))

UT_UTT_SHOBJS = $(UT_UTT_SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%_shobj.$(OBJEXT))

