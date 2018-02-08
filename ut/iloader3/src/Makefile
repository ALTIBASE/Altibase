# $Id: Makefile 79821 2017-04-25 01:50:09Z kit.lee $
#

include ../../utEnv.mk
include ../iloApi_objs.mk
include ../iloader.mk

ifeq "$(NEED_BUILD_LIBEDIT)" "yes"
include  ../../libedit/src/libedit_objs.mk
endif

include moddep.mk

ILOADER_BINS = $(ALTI_HOME)/bin/$(PRODUCT_PREFIX)iloader$(BINEXT)
SHARDLOADER_BINS = $(ALTI_HOME)/bin/shardLoader$(BINEXT)

ifeq "$(NEED_BUILD_LIBEDIT)" "yes"
	CCFLAGS += $(DEFOPT)USE_READLINE
endif

LIBDIRS  += $(LDROPT)$(ALTI_HOME)/lib

ILOADER_BAN=ILOADER.ban
SHARDLOADER_BAN=SHARDLOADER.ban

ifeq "$(NEED_BUILD_LIBEDIT)" "yes"
ifeq  "$(USE_NCURSES)" "1"
	DEP_LIBS += $(LIBOPT)ncurses$(LIBAFT)
else
	DEP_LIBS += $(LIBOPT)curses$(LIBAFT)
endif
endif
DEP_LIBS += $(LIBS)

ILOADER_DEP_LIBS := $(DEP_LIBS) $(foreach i, altiutil id pd odbccli, $(LIBOPT)$(i)$(LIBAFT))
SHARDLOADER_DEP_LIBS := $(DEP_LIBS) $(foreach i, altiutil id pd shardcli, $(LIBOPT)$(i)$(LIBAFT))

ILOADERARC = $(LIBPRE)$(PRODUCT_PREFIX)iloaderobj.$(LIBEXT)
SHARDLOADERARC = $(LIBPRE)$(PRODUCT_PREFIX)shardloaderobj.$(LIBEXT)

ifeq "$(NEED_BUILD_LIBEDIT)" "yes"
	INCLUDES += $(IDROPT)$(UT_DIR)/libedit/include
endif

ILOADER_OBJS=$(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))
SHARDLOADER_OBJS=$(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%_sd.$(OBJEXT))

quiet_cmd_c_o_cc_sd = CC $@
      cmd_c_o_cc_sd = mkdir -p $(dir $@); $(COMPILE.cc) -DCOMPILE_SHARDCLI $(INCLUDES) $(CCOUT)$@ $<

define COMPILE_IT_CC_SD
  $(Q) $(if $(quiet),echo ' $($(quiet)cmd_c_o_cc_sd)')
  $(Q) $(cmd_c_o_cc_sd)
endef

$(TARGET_DIR)/%_sd.$(OBJEXT): $(DEV_DIR)/%.cpp
	$(COMPILE_IT_CC_SD)

.PHONY: compile archive install clean 

shard:$(SHARDLOADER_BINS)
	echo "shard="$(SHARDLOADER_BINS)

cli:$(ILOADER_BINS)
	echo "bin="$(ILOADER_BINS)

compile: cmd archive install

archive:
	$(MAKE) archive_iloader
	$(MAKE) archive_shardloader

archive_iloader: $(ILOADERARC)

archive_shardloader: $(SHARDLOADERARC)

ifeq "$(NEED_BUILD_LIBEDIT)" "yes"
$(ILOADERARC): $(ILOADER_OBJS) $(LIBEDIT_OBJS)
else
$(ILOADERARC): $(ILOADER_OBJS)
endif

ifeq "$(NEED_BUILD_LIBEDIT)" "yes"
$(SHARDLOADERARC): $(SHARDLOADER_OBJS) $(LIBEDIT_OBJS)
else
$(SHARDLOADERARC): $(SHARDLOADER_OBJS)
endif

ifeq "$(NEED_BUILD_LIBEDIT)" "yes"
$(ILOADER_BINS): $(ILOADER_OBJS) $(LIBEDIT_OBJS)
else
$(ILOADER_BINS): $(ILOADER_OBJS)
endif
ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
	@echo  LD $@
else
	@echo " LD $@"
endif
	$(Q) $(LD) $(LFLAGS) $(LIBDIRS) $(LDOUT)$@ $^ $(ILOADER_DEP_LIBS)

ifeq "$(NEED_BUILD_LIBEDIT)" "yes"
$(SHARDLOADER_BINS): $(SHARDLOADER_OBJS) $(LIBEDIT_OBJS)
else
$(SHARDLOADER_BINS): $(SHARDLOADER_OBJS)
endif
ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
	@echo  LD $@
else
	@echo " LD $@"
endif
	$(Q) $(LD) $(LFLAGS) $(LIBDIRS) $(LDOUT)$@ $^ $(SHARDLOADER_DEP_LIBS)

install:
	$(MAKE) install_iloader
	$(MAKE) install_shardloader

install_iloader: $(ILOADER_BINS)
	$(COPY) $(ILOADER_BAN) $(ALTI_HOME)/msg
	$(COPY) $(ILOADERARC) $(ALTI_HOME)/lib

install_shardloader: $(SHARDLOADER_BINS)
	$(COPY) $(SHARDLOADER_BAN) $(ALTI_HOME)/msg
	$(COPY) $(SHARDLOADERARC) $(ALTI_HOME)/lib

form: cmdform

formclean: cmdformclean

clean: cmdclean
	$(Q) $(RM) core $(TARGET_DIR)/ut/iloader3/src $(ILOADER_BINS) $(SHARDLOADER_BINS) $(ILOADERARC) $(SHARDLOADERARC) *.d SunWS_cache

-include $(ILOADER_OBJS:.$(OBJEXT)=.d)
-include $(SHARDLOADER_OBJS:.$(OBJEXT)=.d)
