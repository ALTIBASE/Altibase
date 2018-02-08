include ../../utEnv.mk
include ../checkServer.mk
include $(PD_DIR)/lib/pd_objs.mk
include $(ID_DIR)/lib/id_objs.mk
include $(DEV_DIR)/alticore.mk

SRCS = $(CHKSERV_DIR)/lib/checkServerPid.$(CHKSERV_EXT) \
       $(CHKSERV_DIR)/lib/checkServerLib.$(CHKSERV_EXT) \
       $(CHKSERV_DIR)/lib/checkServerLibWrap.$(CHKSERV_EXT) \
       $(CHKSERV_DIR)/lib/checkServerStat.cpp
DEP_SRCS = $(MM_DIR)/mmu/mmuProperty.cpp $(MM_DIR)/mmu/mmuAccessList.cpp
OBJS = $(SRCS:$(DEV_DIR)/%.$(CHKSERV_EXT)=$(TARGET_DIR)/%.$(OBJEXT))
DEP_OBJS = $(DEP_SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT)) \
           $(PD_OBJS) $(ID_OBJS)
SHOBJS = $(SRCS:$(DEV_DIR)/%.$(CHKSERV_EXT)=$(TARGET_DIR)/%_shobj.$(OBJEXT))
DEP_SHOBJS = $(DEP_SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%_shobj.$(OBJEXT)) \
             $(PD_SHOBJS) $(ID_SHOBJS)
BINS = $(CHKSERV_LIB_PATH)
LIBDIRS  += $(LDROPT)$(ALTI_HOME)/lib

CHKSERV_DEP_SHLIBS = $(foreach i,alticore_pic,$(LIBOPT)$(i)$(LIBAFT))

all : build

build : compile

compile : chksvr_static chksvr_shared

dist : $(CHKSERV_INC_PATH)

clean :
	$(RM) core .dependency $(BINS) $(TARGET_DIR)/ut/checkServer/lib $(CHKSERV_INC_PATH) *.d SunWS_cache

#################################
# checkServer api static library build
##################################

chksvr_static : $(CHKSERV_LIB_PATH)
ifeq "$(OS_TARGET)" "IBM_AIX"
  # AIX needs X64 or X32 flag according to the compile bit
  ifeq ($(compile64),1)
	$(Q) ar -x -X64 $(ALTICORE_LIBDIR)/libalticore.a
  else
	$(Q) ar -x -X32 $(ALTICORE_LIBDIR)/libalticore.a
  endif
else # ifneq "$(OS_TARGET)" "IBM_AIX"
	$(Q) ar -x $(ALTICORE_LIBDIR)/libalticore.a
endif
	ar $(ARFLAGS) $(CHKSERV_LIB_PATH) *.$(OBJEXT) > /dev/null 2>&1
	$(Q) -$(RM) *.$(OBJEXT)

$(CHKSERV_LIB_PATH) : $(OBJS) $(DEP_OBJS)

$(CHKSERV_INC_PATH) :
	$(Q) cp ../include/checkServer.h $(CHKSERV_INC_PATH)

#################################
# BUG-20728
# checkServer api shared library build
##################################

chksvr_shared:
ifeq "$(NEED_SHARED_LIBRARY)" "no"
	echo "SKIP Shared Library Build";
else
	$(MAKE) chksvr_shared_build
endif

chksvr_shared_build: $(SHOBJS) $(DEP_SHOBJS)
	$(Q) echo " SO $(CHKSERV_SHLIB_PATH)"
ifeq "$(OS_TARGET)" "X86_64_DARWIN"
	$(Q) $(SOLINK.cc) $(SOOUT)$(CHKSERV_SHLIB_PATH) $^ $(LIBDIRS) $(CLFLAGS) $(CHKSERV_DEP_SHLIBS)
else
	$(Q) $(SOLINK.c) $(SOOUT)$(CHKSERV_SHLIB_PATH) $^ $(LIBDIRS) $(CLFLAGS) $(CHKSERV_DEP_SHLIBS)
endif

#################################
# BUG-31162
# build rule for id, pd, mm
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

-include $(OBJS:.$(OBJEXT)=.d)

-include $(SHOBJS:.$(OBJEXT)=.d)
