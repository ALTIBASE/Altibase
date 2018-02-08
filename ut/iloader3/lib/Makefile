# $Id: Makefile 71172 2015-06-07 23:49:00Z bethy $
#

include ../../utEnv.mk
include ../iloApi_objs.mk
include ../iloader.mk
include $(PD_DIR)/lib/pd_objs.mk
include $(ID_DIR)/lib/id_objs.mk

ifeq ($(compile64),1)
COMPILEBIT=64
else
COMPILEBIT=32
endif

WIN32_ILOADER_DEF = $(PRODUCT_PREFIX)iloader$(COMPILEBIT).def

ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
    ILOADER_DEP_SHLIBS += $(foreach i, $(UTIL_LIB) id pd $(ODBCCLI_LIB), $(LIBOPT)$(i)$(LIBAFT))
    ILOADER_DEP_SHLIBS += netapi32.lib advapi32.lib iphlpapi.lib dbghelp.lib 
else
    ILOADER_DEP_SHLIBS += $(foreach i, $(ODBCCLI_LIB)_sl, $(LIBOPT)$(i)$(LIBAFT))
endif

# LIBRARY NAME
ILOADERAPI_LIB_PATH   = $(ALTI_HOME)/lib/$(LIBPRE)$(PRODUCT_PREFIX)iloader.$(LIBEXT)
ILOADERAPI_SHLIB_PATH = $(ALTI_HOME)/lib/$(LIBPRE)$(PRODUCT_PREFIX)iloader_sl.$(SOEXT)

LIB_AR_OBJS = $(UT_ILOAPI_OBJS) \
              $(UT_UTIL_OBJS)

LIB_SHOBJS = $(LIB_AR_OBJS:$(TARGET_DIR)/%.$(OBJEXT)=$(TARGET_DIR)/%_shobj.$(OBJEXT))

#################################
# BUG-31147
# build rule for id, pd
#################################
LIB_IDPD_OBJS =  $(PD_OBJS) $(ID_OBJS)
LIB_IDPD_SHOBJS =  $(PD_SHOBJS) $(ID_SHOBJS)

.PHONY: iloaderapi_static iloaderapi_shared iloaderapi_shared_build clean

compile: cmd iloaderapi_static iloaderapi_shared

iloaderapi_static: $(LIB_IDPD_OBJS) $(LIB_AR_OBJS)
	 $(Q) $(AR) $(ARFLAGS) $(AROUT)$(ILOADERAPI_LIB_PATH) $(LIB_IDPD_OBJS) $(LIB_AR_OBJS)

#################################
# BUG-30728
# iloader api shared library build
##################################

iloaderapi_shared:
ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
  ifeq "$(OS_TARGET2)" "WINCE"
	echo "SKIP Shared Library Build";
  else
	$(MAKE) iloaderapi_shared_build
  endif
else
  ifeq "$(NEED_SHARED_LIBRARY)" "no" # defined in pd/makefiles2/*.mk : only aix 4.x no
	echo "SKIP Shared Library Build";
  else
	$(MAKE) iloaderapi_shared_build
  endif
endif

iloaderapi_shared_build: $(LIB_IDPD_SHOBJS) $(LIB_SHOBJS)
	$(Q) echo " SO $(ILOADERAPI_SHLIB_PATH)"
ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
	$(Q) $(SOLINK.cc) $(SOOUT)$(ILOADERAPI_SHLIB_PATH) $(LIBDIRS) $(LIB_IDPD_SHOBJS) $(LIB_SHOBJS) $(ILOADER_DEP_SHLIBS) /DEF:$(WIN32_ILOADER_DEF)
else # ! INTEL_WINDOWS
ifeq "$(OS_TARGET)" "X86_64_DARWIN"
	$(Q) $(SOLINK.cc)  $(SOOUT)$(ILOADERAPI_SHLIB_PATH) $(LIBDIRS)  $(LIB_IDPD_SHOBJS) $(LIB_SHOBJS) $(CLFLAGS) $(ILOADER_DEP_SHLIBS) -lalticore
else
	$(Q) $(SOLINK.c)  $(SOOUT)$(ILOADERAPI_SHLIB_PATH) $(LIBDIRS)  $(LIB_IDPD_SHOBJS) $(LIB_SHOBJS) $(CLFLAGS) $(ILOADER_DEP_SHLIBS)
endif
endif

clean: cmdclean
	$(Q) $(RM) $(TARGET_DIR)/ut/iloader3/lib $(TARGET_DIR)/ut/iloader3/src $(TARGET_DIR)/ut/util/ute $(TARGET_DIR)/ut/util/utt $(TARGET_DIR)/ut/lib $(ILOADERAPI_LIB_PATH) $(ILOADERAPI_SHLIB_PATH)

#################################
# BUG-31147
# build rule for id, pd
#################################

quiet_cmd_cc_o_c_aoc = CC $@
      cmd_cc_o_c_aoc = mkdir -p $(dir $@); $(COMPILE.cc) $(INCLUDES) $(CC_OUTPUT_FLAG)$@ $<

quiet_cmd_c_o_c_aoc = CC $@
      cmd_c_o_c_aoc = mkdir -p $(dir $@); $(COMPILE.c) $(INCLUDES) $(CC_OUTPUT_FLAG)$@ $<

define COMPILE_IT_CC_AOC
  $(Q) $(if $(quiet),echo ' $($(quiet)cmd_cc_o_c_aoc)')
  $(Q) $(cmd_cc_o_c_aoc)
endef

define COMPILE_IT_C_AOC
  $(Q) $(if $(quiet),echo ' $($(quiet)cmd_c_o_c_aoc)')
  $(Q) $(cmd_c_o_c_aoc)
endef

$(TARGET_DIR)/%.$(OBJEXT): $(DEV_DIR)/src/%.cpp
	$(COMPILE_IT_CC_AOC)

$(TARGET_DIR)/%.$(OBJEXT): $(DEV_DIR)/src/%.c
	$(COMPILE_IT_C_AOC)

$(TARGET_DIR)/%.$(OBJEXT): $(DEV_DIR)/src/%.s
	$(COMPILE_IT_C_AOC)

$(TARGET_DIR)/%.$(OBJEXT): $(DEV_DIR)/src/%.S
	$(COMPILE_IT_C_AOC)


quiet_cmd_cc_o_c_soc = CC $@
      cmd_cc_o_c_soc = mkdir -p $(dir $@); $(COMPILE.cc) $(INCLUDES) $(PIC) $(CC_OUTPUT_FLAG)$@ $<

quiet_cmd_c_o_c_soc = CC $@
      cmd_c_o_c_soc = mkdir -p $(dir $@); $(COMPILE.c) $(INCLUDES) $(PIC) $(CC_OUTPUT_FLAG)$@ $<

define COMPILE_IT_CC_SOC
  $(Q) $(if $(quiet),echo ' $($(quiet)cmd_cc_o_c_soc)')
  $(Q) $(cmd_cc_o_c_soc)
endef

define COMPILE_IT_C_SOC
  $(Q) $(if $(quiet),echo ' $($(quiet)cmd_c_o_c_soc)')
  $(Q) $(cmd_c_o_c_soc)
endef

$(TARGET_DIR)/%_shobj.$(OBJEXT): $(DEV_DIR)/src/%.cpp
	$(COMPILE_IT_CC_SOC)

$(TARGET_DIR)/%_shobj.$(OBJEXT): $(DEV_DIR)/src/%.c
	$(COMPILE_IT_C_SOC)

$(TARGET_DIR)/%_shobj.$(OBJEXT): $(DEV_DIR)/src/%.s
	$(COMPILE_IT_C_SOC)

$(TARGET_DIR)/%_shobj.$(OBJEXT): $(DEV_DIR)/src/%.S
	$(COMPILE_IT_C_SOC)

-include $(LIB_IDPD_OBJS:.$(OBJEXT)=.d)

-include $(LIB_AR_OBJS:.$(OBJEXT)=.d)

-include $(LIB_IDPD_SHOBJS:.$(OBJEXT)=.d)

-include $(LIB_SHOBJS:.$(OBJEXT)=.d)
