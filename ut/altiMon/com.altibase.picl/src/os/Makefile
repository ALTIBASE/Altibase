
include ../../../../../utEnv.mk

include objs.mk

#.SUFFIXES : .c .o
quiet_cmd_cc_o_c_soc = CC $@
      cmd_cc_o_c_soc = mkdir -p $(dir $@); $(COMPILE.cc) $(DEFOPT)ACP_CFG_DL_STATIC $(DEFOPT)LIB_BUILD $(INCLUDES)  $(PIC) $(CCOUT)$@ $<

quiet_cmd_c_o_c_soc = CC $@
      cmd_c_o_c_soc = mkdir -p $(dir $@); $(COMPILE.c) $(DEFOPT)ACP_CFG_DL_STATIC $(DEFOPT)LIB_BUILD $(INCLUDES) $(PIC) $(CCOUT)$@ $<

define COMPILE_IT_CC_SOC
  $(Q) $(if $(quiet),echo ' $($(quiet)cmd_cc_o_c_soc)')
  $(Q) $(cmd_cc_o_c_soc)
endef

define COMPILE_IT_C_SOC
  $(Q) $(if $(quiet),echo ' $($(quiet)cmd_c_o_c_soc)')
  $(Q) $(cmd_c_o_c_soc)
endef

$(TARGET_DIR)/%_soc.$(OBJEXT): $(DEV_DIR)/%.c
	$(COMPILE_IT_C_SOC)

$(TARGET_DIR)/%_soc.$(OBJEXT): $(DEV_DIR)/%.cpp
	$(COMPILE_IT_CC_SOC)

INCLUDES += -I$(JAVA_HOME)/include

ifeq "$(OS_TARGET)" "POWERPC64_LINUX"
    INCLUDES += -I$(JAVA_HOME)/include/linux
endif

ifeq "$(OS_TARGET)" "POWERPC64LE_LINUX"
    INCLUDES += -I$(JAVA_HOME)/include/linux
endif

ifeq "$(OS_TARGET)" "X86_64_LINUX"
    INCLUDES += -I$(JAVA_HOME)/include/linux
endif

ifeq "$(OS_TARGET)" "INTEL_LINUX"
    INCLUDES += -I$(JAVA_HOME)/include/linux
endif

ifeq "$(OS_TARGET)" "IBM_AIX"
    INCLUDES += -I$(JAVA_HOME)/include/aix
endif

ifeq "$(OS_TARGET)" "X86_SOLARIS"
	CFLAGS+=-erroff=E_STATEMENT_NOT_REACHED
    INCLUDES += -I$(JAVA_HOME)/include/solaris
endif

ifeq "$(OS_TARGET)" "IA64_HP_HPUX"
	CFLAGS+=+W2111
    INCLUDES += -I$(JAVA_HOME)/include/hp-ux
endif

ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
    INCLUDES += -I$(JAVA_HOME)/include/win32
endif

all: picl_lib

$(foreach i,$(sort $(PICL_SHOBJS)),$(eval $(call ListRuleFunc,$i)))

picl_lib: $(PICL_SHOBJS_TARGET) $(PICL_SHOBJS)
	$(Q) -$(RM) $(PICL_SHLIB_PATH)
ifeq "$(NEED_INDIRECTION_BUILD)" "yes" # now only windows use indirection build
ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
ifeq "$(OS_TARGET2)" "WINCE"
	$(SOLINK.cc) $(LIBDIRS) $(PICL_SHOBJS) $(SOOUT)$(PICL_SHLIB_PATH)
else
	$(SOLINK.cc) $(LIBDIRS) $(PICL_SHOBJS) $(SOOUT)$(PICL_SHLIB_PATH)
endif # WIN_CE
endif # INTEL_WINDOWS
else
	$(Q) echo " SO $(PICL_SHLIB_PATH)"
ifeq "$(OS_TARGET)" "IBM_AIX"
    ifeq ($(compile64),1)
	$(Q) $(SOLINK.c) $(SOOUT)$(PICL_SHLIB_PATH) $(PICL_SHOBJS) -lperfstat -lodm -lcfg
    else
	$(Q) $(SOLINK.c) $(LIBDIRS) $(SOOUT)$(PICL_SHLIB_PATH) $(PICL_SHOBJS) $(LIBOPT)gcc_s$(LIBAFT) -lperfstat -lodm -lcfg
    endif # compile64=1
else
	$(Q) $(SOLINK.c) $(SOOUT)$(PICL_SHLIB_PATH) $(PICL_SHOBJS) $(CLFLAGS)
endif # IBM_AIX
endif # NEED_INDIRECTION_BUILD

clean: 
	$(Q) -$(RM) $(PICL_SHOBJS) $(PICL_SHLIB_PATH)

