
include ../../../utEnv.mk

AUDIT_DIR=$(UT_DIR)/audit 

INCLUDES += $(foreach i,$(UT_DIR)/audit/src/include , $(IDROPT)$(i))
LIBDIRS  += $(foreach i,$(ALTI_HOME)/lib, $(LDROPT)$(i))

# for OCI
ifeq "$(ALTI_CFG_OS)" "WINDOWS"
	ifeq "$(ALTI_CFG_BITTYPE)" "64"
		INCLUDES += $(IDROPT)./include/window64_include
		LIBDIRS += $(LDROPT)./lib/window64_lib $(LDROPT).
		#export LD_LIBRARY_PATH=./lib/window64_lib:$LD_LIBRARY_PATH
		LIBS_DYNAMIC += $(LIBOPT)oci.lib
		LIBS_STATIC += $(LIBOPT)oci.lib
	else # WINDOWS 32 bit
		INCLUDES += $(IDROPT)./include/window32_include
		LIBDIRS += $(LDROPT)./lib/window32_lib $(LDROPT).
		#export LD_LIBRARY_PATH=./lib/window32_lib:$LD_LIBRARY_PATH
		LIBS_DYNAMIC += $(LIBOPT)oci.dll
		LIBS_STATIC += $(LIBOPT)oci.dll
	endif
else
	ifeq "$(ALTI_CFG_OS)" "AIX"
		INCLUDES += $(IDROPT)./include/aix_include
		LIBDIRS += $(LDROPT)./lib/aix_lib $(LDROPT).
		export LD_LIBRARY_PATH=./lib/aix_lib:$LD_LIBRARY_PATH
		LIBS_DYNAMIC += $(LIBOPT)clntsh
		LIBS_STATIC += $(LIBOPT)clntst10 `cat ./lib/aix_lib/ldflags` `cat ./lib/aix_lib/sysliblist`
	else # LINUX
		ifeq "$(ALTI_CFG_BITTYPE)" "64"
			INCLUDES += $(IDROPT)./include/linux64_include
			LIBDIRS += $(LDROPT)./lib/linux64_lib $(LDROPT).
			export LD_LIBRARY_PATH=./lib/linux64_lib:$LD_LIBRARY_PATH
			LIBS_DYNAMIC += $(LIBOPT)clntsh
			LIBS_STATIC += $(LIBOPT)clntst10 `cat ./lib/linux64_lib/ldflags` `cat ./lib/linux64_lib/sysliblist`
		else # LINUX 32 bit
			INCLUDES += $(IDROPT)./include/linux32_include
			LIBDIRS += $(LDROPT)./lib/linux32_lib $(LDROPT).
			export LD_LIBRARY_PATH=./lib/linux32_lib:$LD_LIBRARY_PATH
			LIBS_DYNAMIC += $(LIBOPT)clntsh
			LIBS_STATIC += $(LIBOPT)clntst10 `cat ./lib/linux32_lib/ldflags` `cat ./lib/linux32_lib/sysliblist`
		endif
	endif
endif

ifeq "$(ALTI_CFG_OS)" "WINDOWS"
INCLUDES += $(IDROPT)$(ORACLE_HOME)/oci/include 
LIBDIRS += $(LDROPT)$(ORACLE_HOME)/oci/lib/msvc
LIBS_DYNAMIC += $(LIBOPT)oci.lib
LIBS_STATIC += $(LIBOPT)oci.lib
else
INCLUDES += $(IDROPT)$(ORACLE_HOME)/rdbms/public 
# For linking dynamic library
LIBDIRS += $(LDROPT)$(ORACLE_HOME)/lib
LIBS_DYNAMIC += $(LIBOPT)clntsh
# For linking OCI static library
LIBS_STATIC += $(LIBOPT)clntst10 `cat $(ORACLE_HOME)/lib/ldflags` `cat $(ORACLE_HOME)/lib/sysliblist`
endif

LIB_OPT += $(foreach i, $(PRODUCT_PREFIX)atDb $(UTIL_LIB)_sl $(ODBCCLI_LIB)_sl, $(LIBOPT)$(i)$(LIBAFT))

LIB   = $(LIBPRE)atOra.$(LIBEXT)
SHLIB = $(LIBPRE)atOra.$(SOEXT)

SRCS  = $(DEV_DIR)/ut/audit/src/ato/utOraField.cpp \
	$(DEV_DIR)/ut/audit/src/ato/utOraRow.cpp \
	$(DEV_DIR)/ut/audit/src/ato/utOraQuery.cpp \
	$(DEV_DIR)/ut/audit/src/ato/utOraConnection.cpp \
	$(DEV_DIR)/ut/audit/src/ato/utOra.cpp

OBJS   = $(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))
SHOBJS = $(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%_shobj.$(OBJEXT))
all: shlib lib
compile: shlib

lib: 	$(LIB)
shlib:	$(SHLIB)

$(LIB): $(OBJS)	

$(SHLIB): $(SHOBJS)
	$(SOLINK.cc)  $(LFLAGS) $(LIBDIRS) $(SHOBJS) $(LIB_OPT) $(LIBS_DYNAMIC) $(SO_OUTPUT_FLAG) $(SHLIB)
	@cp $(SHLIB) $(ALTI_HOME)/lib	

clean:
	@$(RM) -f  .shobj SunWS_cache core* $(TARGET_DIR)/ut/audit/src/ato $(LIB) $(SHLIB)

cleanall:clean

check:
	-echo $(EXTRA_CXXOPT)

-include $(OBJS:.$(OBJEXT)=.d)
