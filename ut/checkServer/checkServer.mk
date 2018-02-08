CHKSERV_DIR = $(UT_DIR)/checkServer

CHKSERV_LIB_NAME = chksvr
CHKSERV_LIB_PATH = $(ALTI_HOME)/lib/$(LIBPRE)$(CHKSERV_LIB_NAME).$(LIBEXT)
CHKSERV_SHLIB_PATH = $(ALTI_HOME)/lib/$(LIBPRE)$(CHKSERV_LIB_NAME)_sl.$(SOEXT)
CHKSERV_INC_PATH = $(ALTI_HOME)/include/$(CHKSERV_LIB_NAME).h

INCLUDES += $(foreach i,../include $(MM_DIR)/include $(QP_DIR)/include $(RP_DIR)/include, $(IDROPT)$(i))

CHKSERV_EXT = cpp

