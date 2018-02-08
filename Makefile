# $id: makefile 20363 2007-02-07 08:32:54Z shawn $
 #

include ./env.mk

###################################################################
#
#  Definition & Base clean section
#
###################################################################

DIRS=$(CORE_DIR) $(PD_DIR) $(ID_DIR) $(CM_DIR) $(SM_DIR) $(MT_DIR) $(RP_DIR) $(QP_DIR) $(SD_DIR) $(DK_DIR) $(MM_DIR) $(UL_DIR) $(UT_DIR) $(TOOL_DIR) $(ST_DIR) $(ID_DIR)/util $(QP_DIR)/util

SODIRS=$(PD_DIR) $(ID_DIR) $(CM_DIR) $(SM_DIR) $(MT_DIR) $(RP_DIR) $(QP_DIR) $(SD_DIR) $(ST_DIR) $(DK_DIR) $(MM_DIR)

CLIDIRS=$(CORE_DIR) $(PD_DIR) $(ID_DIR) $(CM_DIR) $(MT_DIR) $(UL_DIR) $(TOOL_DIR) $(ST_DIR)

# -------------------------------------------------------------------
#  TASK-3873
#  CodeSonar Build
# -------------------------------------------------------------------
DIRS_CODESONAR_SERVER=$(CORE_DIR) $(PD_DIR) $(ID_DIR) $(CM_DIR) $(SM_DIR) $(MT_DIR) $(RP_DIR) $(QP_DIR) $(SD_DIR) $(ST_DIR) $(DK_DIR) $(MM_DIR)

DIRS_CODESONAR_SM=$(CORE_DIR) $(PD_DIR) $(ID_DIR) $(SM_DIR)

DIRS_CODESONAR_CM=$(CORE_DIR) $(PD_DIR) $(ID_DIR) $(CM_DIR)

all: help

include ./moddep.mk

.PHONY: $(DIRS)

$(DIRS):
	$(MAKE) -C $@ $(SUBDIR_TARGET)


###################################################################
#
#  CLIENT (sure/release) section
#
###################################################################

# -------------------------------------------------------------------
#   CLIENT BUILD LOGIC
# -------------------------------------------------------------------

clean_cli: makemsg
	$(MAKE) $(CLIDIRS) SUBDIR_TARGET=clean

build_cli: makemsg alemon
	$(MAKE) $(CLIDIRS)
	$(MAKE) -C $(UT_DIR)  build_cli;

# bash, or cygwin env so we can use shell script grammer.
test_cli:
	cd $(ATC_HOME)/src && $(MAKE) clean;
	cd $(ATC_HOME)/src && $(MAKE) install;
	cd $(ATC_HOME)/; sh run.sh "TC/A4-Client.ts"

build_all_cli:
	$(MAKE) clean_cli
	$(MAKE) build_cli
	$(MAKE) $(S) distA3Developer # Need this for altibase_env.mk
	$(MAKE) test_cli

# -------------------------------------------------------------------
#   release mode
# -------------------------------------------------------------------
release_cli:
	cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=release
	$(MAKE) build_all_cli

release32_cli:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-bit32 --with-build_mode=release
	$(MAKE) build_all_cli

releasegcc_cli:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-gcc --with-build_mode=release
	$(MAKE) build_all_cli

releasegcc32_cli:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-bit32 --enable-gcc --with-build_mode=release
	$(MAKE) build_all_cli

release32compat4_cli:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-compat4 --enable-bit32 --with-build_mode=release
	$(MAKE) build_all_cli


# -------------------------------------------------------------------
#   debug mode
# -------------------------------------------------------------------
sure_cli:
	cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=debug
	$(MAKE) build_all_cli

sure32_cli:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-bit32 --with-build_mode=debug
	$(MAKE) build_all_cli

suregcc_cli:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-gcc --with-build_mode=debug
	$(MAKE) build_all_cli

suregcc32_cli:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-bit32 --enable-gcc --with-build_mode=debug
	$(MAKE) build_all_cli


# -------------------------------------------------------------------
#   release for gcc
# -------------------------------------------------------------------
releasegcc_cli_test:
	$(MAKE) releasegcc_cli
	$(MAKE) dist_client
	gzip -cd $(RELEASE_CLIENT_DIST_NAME) | tar xvf - -C $(ALTI_HOME)
	cd $(ATC_HOME)/; $(MAKE) all

releasegcc32_cli_test:
	$(MAKE) releasegcc32_cli
	$(MAKE) dist_client
	gzip -cd $(RELEASE_CLIENT_DIST_NAME) | tar xvf - -C $(ALTI_HOME)
	cd $(ATC_HOME)/; $(MAKE) all

# -------------------------------------------------------------------
#   debug for gcc
# -------------------------------------------------------------------
suregcc_cli_test:
	$(MAKE) suregcc_cli
	$(MAKE) dist_client
	gzip -d $(RELEASE_CLIENT_DIST_NAME) | tar xvf - -C $(ALTI_HOME)
	cd $(ATC_HOME)/; $(MAKE) all

suregcc32_cli_test:
	$(MAKE) suregcc32_cli
	$(MAKE) dist_client
	gzip -d $(RELEASE_CLIENT_DIST_NAME) | tar xvf - -C $(ALTI_HOME)
	cd $(ATC_HOME)/; $(MAKE) all

###################################################################
#
#  SERVER (sure/release) section
#
###################################################################

# -------------------------------------------------------------------
#  server relase mode
# -------------------------------------------------------------------
release:
	$(Q) cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=release
	$(Q) $(MAKE) $(S) build_all
	$(Q) $(MAKE) $(S) a3_test

release32:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-bit32 --with-build_mode=release
	$(MAKE) build_all
	$(MAKE) a3_test

release32_lf:
	$(Q) cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=release --enable-largefile
	$(Q) $(MAKE) $(S) build_all
	$(Q) $(MAKE) $(S) a3_test

releasepowerpclinux32:
	cd $(DEV_DIR); $(CONFIGURE) --target=powerpc-linux  --enable-bit32 --with-build_mode=release
	$(MAKE) build_all

releasearmlinux32:
	cd $(DEV_DIR); $(CONFIGURE) --enable-nosmp --enable-mobile --host=i686-redhat-linux-gnu --target=arm-linux  --enable-bit32 --with-build_mode=release
	$(MAKE) build_all

release32_eldk:
	cd $(DEV_DIR); $(CONFIGURE) --enable-nosmp --enable-mobile --host=i686-redhat-linux-gnu --target=eldk-linux  --enable-bit32 --with-build_mode=release
	$(MAKE) build_all

release32_mips64:
	cd $(DEV_DIR); $(CONFIGURE) --enable-nosmp --enable-mobile --host=i686-redhat-linux-gnu --target=mips64-linux  --enable-bit32 --with-build_mode=release
	$(MAKE) build_all

release32compat4:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-compat4 --enable-bit32 --with-build_mode=release
	$(MAKE) build_all
	$(MAKE) a3_test

releasegcc:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-gcc --with-build_mode=release
	$(MAKE) build_all
	$(MAKE) a3_test

releasegcc_blocksigterm:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-gcc --with-build_mode=release --enable-blocksigterm
	$(MAKE) build_all
	$(MAKE) a3_test

releasegcc32:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-bit32 --enable-gcc --with-build_mode=release
	$(MAKE) build_all
	$(MAKE) a3_test

releaseicc:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-icc --with-build_mode=release
	$(MAKE) build_all
	$(MAKE) a3_test




release_mobile:
	$(Q) cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=release --enable-mobile --enable-bit32 --enable-nosmp
	$(Q) $(MAKE) $(S) build_all
	$(Q) $(MAKE) $(S) a3_test

release_shared:
	$(Q) cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=release --enable-shared=y
	$(Q) $(MAKE) $(S) build_all
	#$(Q) make $(S) a3_test

# -------------------------------------------------------------------
#   prerelease mode
# -------------------------------------------------------------------
prerelease:
	cd $(DEV_DIR); $(CONFIGURE) --enable-compat4 --with-build_mode=prerelease
	$(Q) $(MAKE) $(S) build_all

prerelease64:
	$(MAKE) -f Makefile prerelease

prerelease32:
	cd $(DEV_DIR); $(CONFIGURE) --enable-compat4 --enable-bit32 --with-build_mode=prerelease
	$(Q) $(MAKE) $(S) build_all



# -------------------------------------------------------------------
#   server debug
# -------------------------------------------------------------------
sure:
	$(Q) cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=debug
	$(Q) $(MAKE) $(S) build_all
	$(Q) $(MAKE) $(S) a3_test

sureicc:
	$(Q) cd $(DEV_DIR); $(CONFIGURE) --enable-icc  --with-build_mode=debug
	$(Q) $(MAKE) $(S) build_all
	$(Q) $(MAKE) $(S) a3_test

sure32:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-bit32 --with-build_mode=debug
	$(MAKE) build_all
	$(MAKE) a3_test

sure32_lf:
	cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=debug --enable-largefile
	$(MAKE) build_all
	$(MAKE) a3_test

surepowerpclinux32:
	cd $(DEV_DIR); $(CONFIGURE) --target=powerpc-linux  --enable-bit32 --with-build_mode=debug
	$(MAKE) build_all

surearmlinux32:
	cd $(DEV_DIR); $(CONFIGURE) --enable-nosmp --enable-mobile --host=i686-redhat-linux-gnu --target=arm-linux  --enable-bit32 --with-build_mode=debug
	$(MAKE) build_all

sure32_eldk:
	cd $(DEV_DIR); $(CONFIGURE) --enable-nosmp --enable-mobile --host=i686-redhat-linux-gnu --target=eldk-linux  --enable-bit32 --with-build_mode=debug
	$(MAKE) build_all

sure32_mips64:
	cd $(DEV_DIR); $(CONFIGURE) --enable-nosmp --enable-mobile --host=i686-redhat-linux-gnu --target=mips64-linux  --enable-bit32 --with-build_mode=debug
	$(MAKE) build_all


sure32compat4:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-compat4 --enable-bit32 --with-build_mode=debug
	$(MAKE) build_all
	$(MAKE) a3_test

suregcc:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-gcc --with-build_mode=debug
	$(MAKE) build_all
	$(MAKE) a3_test

suregcc32:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-gcc --enable-bit32 --with-build_mode=debug
	$(MAKE) build_all
	$(MAKE) a3_test

sure_mobile:
	$(Q) cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=debug --enable-mobile --enable-bit32 --enable-nosmp
	$(Q) $(MAKE) $(S) build_all
	$(Q) $(MAKE) $(S) a3_test

sure_shared:
	$(Q) cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=debug --enable-shared=y
	$(Q) $(MAKE) $(S) build_all
	#$(Q) make $(S) a3_test

# Task-1994 초고속 Coverage측정방법 고안
suregcov:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-gcc --enable-gcov --with-build_mode=debug
	$(MAKE) build_all

# -------------------------------------------------------------------
#   server (release/debug) memcheck
# -------------------------------------------------------------------
release_memcheck_valgrind:
	cd $(DEV_DIR); $(CONFIGURE) --enable-memcheck --enable-usevalgrind --with-build_mode=release
	$(MAKE) build_all

release_memcheck:
	cd $(DEV_DIR); $(CONFIGURE) --enable-memcheck  --with-build_mode=release
	$(MAKE) build_all
	$(MAKE) a3_test

release32_memcheck:
	cd $(DEV_DIR); $(CONFIGURE) --enable-memcheck  --enable-bit32 --with-build_mode=release
	$(MAKE) build_all
	$(MAKE) a3_test

release32compat4_memcheck:
	cd $(DEV_DIR); $(CONFIGURE) --enable-memcheck  --enable-compat4 --enable-bit32 --with-build_mode=release
	$(MAKE) build_all
	$(MAKE) a3_test

releasegcc_memcheck:
	cd $(DEV_DIR); $(CONFIGURE) --enable-memcheck  --enable-gcc --with-build_mode=release
	$(MAKE) build_all
	$(MAKE) a3_test

releasegcc32_memcheck:
	cd $(DEV_DIR); $(CONFIGURE) --enable-memcheck  --enable-bit32 --enable-gcc --with-build_mode=release
	$(MAKE) build_all
	$(MAKE) a3_test

sure_memcheck_valgrind:
	cd $(DEV_DIR); $(CONFIGURE) --enable-memcheck --enable-usevalgrind --with-build_mode=debug
	$(MAKE) build_all

sure_memcheck:
	cd $(DEV_DIR); $(CONFIGURE) --enable-memcheck  --with-build_mode=debug
	$(MAKE) build_all
	$(MAKE) a3_test

sure32_memcheck:
	cd $(DEV_DIR); $(CONFIGURE) --enable-memcheck  --enable-bit32 --with-build_mode=debug
	$(MAKE) build_all
	$(MAKE) a3_test

sure32compat4_memcheck:
	cd $(DEV_DIR); $(CONFIGURE) --enable-memcheck  --enable-compat4 --enable-bit32 --with-build_mode=debug
	$(MAKE) build_all
	$(MAKE) a3_test

suregcc_memcheck:
	cd $(DEV_DIR); $(CONFIGURE) --enable-memcheck  --enable-gcc --with-build_mode=debug
	$(MAKE) build_all
	$(MAKE) a3_test

suregcc32_memcheck:
	cd $(DEV_DIR); $(CONFIGURE) --enable-memcheck  --enable-gcc --enable-bit32 --with-build_mode=debug
	$(MAKE) build_all
	$(MAKE) a3_test



###################################################################
#
#  RELEASE FOR SM TEST
#
###################################################################

# -------------------------------------------------------------------
#  release sm-test
# -------------------------------------------------------------------
release_smtest:
	cd $(DEV_DIR); $(CONFIGURE) --enable-sm_module_test  --with-build_mode=release
	$(MAKE) build_all
	$(MAKE) a3_test

release32_smtest:
	cd $(DEV_DIR); $(CONFIGURE) --enable-sm_module_test --enable-bit32 --with-build_mode=release
	$(MAKE) build_all
	$(MAKE) a3_test

release32compat4_smtest:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-sm_module_test --enable-compat4 --enable-bit32 --with-build_mode=release
	$(MAKE) build_all
	$(MAKE) a3_test

releasegcc_smtest:
	cd $(DEV_DIR); $(CONFIGURE) --enable-sm_module_test  --enable-gcc --with-build_mode=release
	$(MAKE) build_all
	$(MAKE) a3_test

releasegcc32_smtest:
	cd $(DEV_DIR); $(CONFIGURE) --enable-sm_module_test  --enable-bit32 --enable-gcc --with-build_mode=release
	$(MAKE) build_all
	$(MAKE) a3_test


# -------------------------------------------------------------------
#  debug sm-test
# -------------------------------------------------------------------
sure_smtest:
	cd $(DEV_DIR); $(CONFIGURE) --enable-sm_module_test  --with-build_mode=debug
	$(MAKE) build_all
	$(MAKE) a3_test

sure32_smtest:
	cd $(DEV_DIR); $(CONFIGURE) --enable-sm_module_test  --enable-bit32 --with-build_mode=debug
	$(MAKE) build_all
	$(MAKE) a3_test

sure32compat4_smtest:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-sm_module_test --enable-compat4 --enable-bit32 --with-build_mode=debug
	$(MAKE) build_all
	$(MAKE) a3_test

suregcc_smtest:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-sm_module_test --enable-gcc --with-build_mode=debug
	$(MAKE) build_all
	$(MAKE) a3_test

suregcc32_smtest:
	cd $(DEV_DIR); $(CONFIGURE)  --enable-sm_module_test --enable-gcc --enable-bit32 --with-build_mode=debug
	$(MAKE) build_all
	$(MAKE) a3_test


###################################################################
#  BUILD COMMON
###################################################################

.PHONY: alemon
alemon:
	$(MAKE) $(S) -C tool/alemon

build_eclipse:
build: alemon
	$(MAKE) $(S) makemsg
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=$(S)
	$(MAKE) -C $(SM_DIR)/util all_after
# Link shared libs after all other modules bulid
# Build shared libs only if unittesting enabled.
ifeq ($(DO_UNITTEST), yes)
	$(MAKE) $(S) $(SODIRS) SUBDIR_TARGET="-s link_solib"
endif
	$(MAKE) $(S) distA3Developer
ifeq ($(DO_UNITTEST), yes)
	$(MAKE) unittest
endif

build_all:
	$(MAKE) clean
	$(MAKE) build

a3_test:
	$(MAKE) -C $(ATC_HOME)/src  clean;
	$(MAKE) -C $(ATC_HOME)/src install;
	$(MAKE) -C $(ATC_HOME) all;

tsm_test:
	$(Q) $(MAKE) -C $(SM_DIR) all
	$(Q) $(MAKE) -C $(DEV_DIR)/tsrc/sm/tsm/src all
	$(Q) $(MAKE) -C $(DEV_DIR)/tsrc/sm/tsm all

clean:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=clean
	$(RM) $(ALTI_HOME)/lib/*.$(LIBEXT)
	$(RM) $(ALTI_HOME)/lib/*.$(SOEXT)
	$(RM) $(ALTI_HOME)/lib/*.jar
	$(RM) $(ALTI_HOME)/msg/*.msb
	$(RM) $(ALTI_HOME)/msg/*.ban
	$(RM) $(TARGET_DIR)

clean_all: clean

# -------------------------------------------------------------------
#  TASK-3873
#  CodeSonar Build
# -------------------------------------------------------------------

build_codesonar_tsm:
	cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=release
	$(MAKE) $(S) makemsg
	$(MAKE) $(S) $(DIRS_CODESONAR_SM) SUBDIR_TARGET=$(S)
	$(Q) $(MAKE) -C $(DEV_DIR)/tsrc/sm/tsm/src all

build_codesonar_cm:
	cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=release
	$(MAKE) $(S) makemsg
	$(MAKE) $(S) $(DIRS_CODESONAR_CM) SUBDIR_TARGET=$(S)

build_codesonar_sm:
	cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=release
	$(MAKE) $(S) makemsg
	$(MAKE) $(S) $(DIRS_CODESONAR_SM) SUBDIR_TARGET=$(S)

build_codesonar_server:
	cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=release
	$(MAKE) $(S) makemsg
	$(MAKE) $(S) $(DIRS_CODESONAR_SERVER) SUBDIR_TARGET=$(S)

###################################################################
#
# BUILD FOR ATAF
#
# ATAF build를 위해서 altibase를 최소한으로 빌드한다.
###################################################################

build_ataf:
ifeq "$(OS_TARGET)" "HP_HPUX"
	cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=release --enable-bit32
else
ifeq "$(OS_TARGET)" "IA64_HP_HPUX"
	cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=release --enable-bit32
else
ifeq "$(OS_TARGET)" "SPARC_SOLARIS"
	cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=release --enable-bit32 --enable-gcc
else
ifeq "$(OS_TARGET)" "X86_SOLARIS"
	cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=release --enable-bit32 --enable-gcc
else
ifeq "$(OS_TARGET)" "IBM_AIX"
	cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=release --enable-bit32
else
ifeq "$(OS_TARGET)" "DEC_TRU64"
	cp $(DEV_DIR)/../ataf/port/dec/platform_tru64_g++.GNU $(DEV_DIR)/pd/makefiles/platform_tru64_cxx.GNU
	cp $(DEV_DIR)/../ataf/port/dec/dec_tru64_g++.mk $(DEV_DIR)/pd/makefiles2/dec_tru64_cxx.mk
	cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=release --enable-bit32 --enable-gcc
else
ifeq "$(OS_TARGET)" "INTEL_LINUX"
	cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=release --enable-bit32
else
	cd $(DEV_DIR); $(CONFIGURE) --with-build_mode=release
endif
endif
endif
endif
endif
endif
endif

#    make clean_cli;
	-cd $(CORE_DIR); $(MAKE)
	-cd $(PD_DIR); $(MAKE)
	-cd $(ID_DIR); $(MAKE)
	-cd $(CM_DIR); $(MAKE)
	-cd $(SM_DIR); $(MAKE)
	-cd $(MT_DIR)/msg; $(MAKE)
	-cd $(MT_DIR)/mtd; $(MAKE) mtddl.o
	-cd $(QP_DIR)/msg; $(MAKE)
	-cd $(SD_DIR)/msg; $(MAKE)
	-cd $(RP_DIR)/msg; $(MAKE)
	-cd $(DK_DIR)/msg; $(MAKE)
	-cd $(MM_DIR)/msg; $(MAKE)
	-cd $(UL_DIR)/msg; $(MAKE)
	-cd $(UL_DIR)/lib; $(MAKE) odbccli_shared
	-cd $(UL_DIR)/ulp; $(MAKE)
	-cd $(UT_DIR)/msg; $(MAKE)
	-cd $(UT_DIR)/util; $(MAKE)
	-cd $(UT_DIR)/lib; $(MAKE)
	-cd $(UT_DIR)/ses3/src; $(MAKE)
	-cd $(TOOL_DIR)/altipkg; $(MAKE)
	-$(MAKE) distA3Developer
	-cd $(ATC_HOME)/src; $(MAKE)

###################################################################
#
#  Message File & Manual Section
#
###################################################################

MSG_TARGET_DIR =    \
    $(ID_DIR)/msg  \
	$(CM_DIR)/msg  \
	$(SM_DIR)/msg  \
	$(MT_DIR)/msg  \
	$(RP_DIR)/msg  \
	$(QP_DIR)/msg  \
	$(SD_DIR)/msg  \
	$(ST_DIR)/msg  \
	$(MM_DIR)/msg  \
	$(UL_DIR)/msg  \
	$(UT_DIR)/msg  \
	$(DK_DIR)/msg  \

.PHONY: $(MSG_TARGET_DIR)

$(MSG_TARGET_DIR):
	$(MAKE) -C $@ $(SUBDIR_TARGET)

.PHONY: cleanmsg
cleanmsg:
	$(MAKE) $(MSG_TARGET_DIR) SUBDIR_TARGET=clean

.PHONY: makemsg
makemsg:
	$(MAKE) $(MSG_TARGET_DIR) SUBDIR_TARGET=all

manual:
	$(MAKE) $(MSG_TARGET_DIR) SUBDIR_TARGET=manual
	cat $(ALTI_HOME)/msg/E_ID_US7ASCII.txt $(ALTI_HOME)/msg/E_SM_US7ASCII.txt $(ALTI_HOME)/msg/E_MT_US7ASCII.txt $(ALTI_HOME)/msg/E_RP_US7ASCII.txt $(ALTI_HOME)/msg/E_QP_US7ASCII.txt $(ALTI_HOME)/msg/E_SD_US7ASCII.txt $(ALTI_HOME)/msg/E_ST_US7ASCII.txt $(ALTI_HOME)/msg/E_MM_US7ASCII.txt $(ALTI_HOME)/msg/E_UL_US7ASCII.txt $(ALTI_HOME)/msg/E_LP_US7ASCII.txt $(ALTI_HOME)/msg/E_UT_US7ASCII.txt $(ALTI_HOME)/msg/E_CM_US7ASCII.txt $(ALTI_HOME)/msg/E_DK_US7ASCII.txt $(ALTI_HOME)/msg/E_LA_US7ASCII.txt > install/manual.txt

manual_clean:
	$(MAKE) -C $(MSG_TARGET_DIR) clean


###################################################################
#
#  OLD DIST Section
#
###################################################################

# back compatibility
distcopy:
	$(Q) $(MAKE) pkgdistForDev

distA3Developer: altibase_env.mk
	$(Q) $(MAKE) pkgdistForDev
	$(Q) $(MAKE) pkgdistForNATC

dist: altibase_env.mk manual
	$(Q) $(MAKE) pkg_server

dist_global: altibase_env.mk manual
	$(Q) $(MAKE) pkg_server

#===============================================================================
#  Package Section
#===============================================================================

PKG_MAP=pkg.map
PKG_TEMP_MAP=pkg_temp.map
PKG_DOTNET_PROVIDER_MAP=pkg_dotnet_provider.map
PKG_NATC_MAP=pkg_natc.map
PKG_EXE="$(TOOL_DIR)/altipkg/altipkg$(BINEXT)"
VALUEPACK_NAME=Altibase_Client_$(PKG_VERSION_INFO)_Value_Package
INSTALLBUILDER_DIR=$(DEV_DIR)/ut/Installer
ALTI_PKG_IB_XML_NAME_VP=clientValuepack.xml

# ifeq ($(CROSS_COMPILE),)
# PKG_VERSION_INFO=$(shell $(GENERRMSG) -k)
# else
# PKG_VERSION_INFO=$(OS_TARGET)-$(BUILD_MODE)-GCC
# endif

# ex) 5,5,1  /  5_5_1

ifeq "$(OS_TARGET)" "SPARC_SOLARIS"
PGM_MAJOR_VER=$(shell $(GENERRMSG) -k|awk -F'-' '{print $$4}'|awk -F'.' '{print $$1}')
PGM_MINOR_VER=$(shell $(GENERRMSG) -k|awk -F'-' '{print $$4}'|awk -F'.' '{print $$2}')
PGM_TERM_VER=$(shell $(GENERRMSG) -k|awk -F'-' '{print $$4}'|awk -F'.' '{print $$3}')
PGM_TAGSET_VER=$(shell $(GENERRMSG) -k|awk -F'-' '{print $$4}'|awk -F'.' '{print $$4}')
PGM_TAG_VER=$(shell $(GENERRMSG) -k|awk -F'-' '{print $$4}'|awk -F'.' '{print $$5}')
else
PGM_MAJOR_VER=$(shell $(GENERRMSG) -k|awk -F'-' '{print $$3}'|awk -F'.' '{print $$1}')
PGM_MINOR_VER=$(shell $(GENERRMSG) -k|awk -F'-' '{print $$3}'|awk -F'.' '{print $$2}')
PGM_TERM_VER=$(shell $(GENERRMSG) -k|awk -F'-' '{print $$3}'|awk -F'.' '{print $$3}')
PGM_TAGSET_VER=$(shell $(GENERRMSG) -k|awk -F'-' '{print $$3}'|awk -F'.' '{print $$4}')
PGM_TAG_VER=$(shell $(GENERRMSG) -k|awk -F'-' '{print $$3}'|awk -F'.' '{print $$5}')
endif

PKG_VERSION_INFO="$(PGM_MAJOR_VER).$(PGM_MINOR_VER).$(PGM_TERM_VER)"
PKG_VERSION_INFO_PKGNAME="$(PGM_MAJOR_VER)_$(PGM_MINOR_VER)_$(PGM_TERM_VER)"
PKG_FULL_VERSION_INFO="$(PKG_VERSION_INFO).$(PGM_TAGSET_VER).$(PGM_TAG_VER)"

SVN_REPOSITORY=$(shell svn info | grep URL |awk -F'/' '{print "/"$$4"/"$$5"/"$$6}')

ifeq "$(OS_TARGET)" "POWERPC64LE_LINUX"
    ALTI_CFG_CPU:="$(ALTI_CFG_CPU)LE"
endif

PKG_PLATFORM_INFO=$(ALTI_CFG_OS)-$(ALTI_CFG_CPU)-$(ALTI_CFG_BITTYPE)bit-$(BUILD_MODE)
PKG_REVISION_INFO=$(shell LC_ALL=C svn info | grep Revision)
PKG_REVISION_NUM=$(shell echo $(PKG_REVISION_INFO) | awk -F' ' '{print $$2}')

UNOFFICIAL_SERVER_DIST_NAME="altibase-server-$(PKG_FULL_VERSION_INFO)-$(ALTI_CFG_OS)-$(ALTI_CFG_CPU)-$(ALTI_CFG_BITTYPE)bit-UNOFFICIAL-R$(PKG_REVISION_NUM)"
UNOFFICIAL_CLIENT_DIST_NAME="altibase-client-$(PKG_FULL_VERSION_INFO)-$(ALTI_CFG_OS)-$(ALTI_CFG_CPU)-$(ALTI_CFG_BITTYPE)bit-UNOFFICIAL-R$(PKG_REVISION_NUM)"
RELEASE_SERVER_DIST_NAME="altibase-server-$(PKG_VERSION_INFO).$(PKG_TAGSET_NAME)-$(PKG_PLATFORM_INFO)"
RELEASE_CLIENT_DIST_NAME="altibase-client-$(PKG_VERSION_INFO).$(PKG_TAGSET_NAME)-$(PKG_PLATFORM_INFO)"
DIAGNOSTIC_SERVER_DIST_NAME="altibase-server-$(PKG_FULL_VERSION_INFO)-$(ALTI_CFG_OS)-$(ALTI_CFG_CPU)-$(ALTI_CFG_BITTYPE)bit-DIAGNOSTIC-R$(PKG_REVISION_NUM)"
DIAGNOSTIC_CLIENT_DIST_NAME="altibase-client-$(PKG_FULL_VERSION_INFO)-$(ALTI_CFG_OS)-$(ALTI_CFG_CPU)-$(ALTI_CFG_BITTYPE)bit-DIAGNOSTIC-R$(PKG_REVISION_NUM)"
OFFICIAL_SERVER_DIST_NAME="altibase-server-$(PKG_FULL_VERSION_INFO)-$(ALTI_CFG_OS)-$(ALTI_CFG_CPU)-$(ALTI_CFG_BITTYPE)bit-$(BUILD_MODE)"
OFFICIAL_CLIENT_DIST_NAME="altibase-client-$(PKG_FULL_VERSION_INFO)-$(ALTI_CFG_OS)-$(ALTI_CFG_CPU)-$(ALTI_CFG_BITTYPE)bit-$(BUILD_MODE)"
TAR_SERVER_DIST_NAME="altibase-server-$(PKG_VERSION_INFO).$(PKG_TAGSET_NAME).$(PKG_TAG_NAME)-$(PKG_PLATFORM_INFO)"
COMMUNITY_SERVER_DIST_NAME="altibase-CE-server-$(PKG_FULL_VERSION_INFO)-$(ALTI_CFG_OS)-$(ALTI_CFG_CPU)-$(ALTI_CFG_BITTYPE)bit-$(BUILD_MODE)"
OPEN_SERVER_DIST_NAME="altibase-OE-server-$(PKG_FULL_VERSION_INFO)-$(ALTI_CFG_OS)-$(ALTI_CFG_CPU)-$(ALTI_CFG_BITTYPE)bit-$(BUILD_MODE)"
ODBC_DIST_NAME="altibase-ODBC-$(PKG_FULL_VERSION_INFO)-$(ALTI_CFG_OS)-$(ALTI_CFG_CPU)-$(ALTI_CFG_BITTYPE)bit-$(BUILD_MODE)"

RELEASE_SERVER_DIST_NAME_PATCH=$(RELEASE_SERVER_DIST_NAME)-patch_$(FROM_PKG_TAGSET_NAME)_$(FROM_PKG_TAG_NAME)_$(PKG_TAGSET_NAME)_$(PKG_TAG_NAME)
RELEASE_CLIENT_DIST_NAME_PATCH=$(RELEASE_CLIENT_DIST_NAME)-patch_$(FROM_PKG_TAGSET_NAME)_$(FROM_PKG_TAG_NAME)_$(PKG_TAGSET_NAME)_$(PKG_TAG_NAME)

SERVER_DIST_DIR=$(DEV_DIR)/altipkgdir_server
CLIENT_DIST_DIR=$(DEV_DIR)/altipkgdir_client

SERVER_DIST_DIR_PATCH=$(SERVER_DIST_DIR)_patch
CLIENT_DIST_DIR_PATCH=$(CLIENT_DIST_DIR)_patch

PATCH_DIR=$(WORK_DIR)/APatch
PATCH_SVN_BUGINFO_FILE=$(PATCH_DIR)/pkg_patch_$(FROM_PKG_TAGSET_NAME)_$(FROM_PKG_TAG_NAME)_$(PKG_TAGSET_NAME)_$(PKG_TAG_NAME).txt

INSTALL_BUILDER=builder

DIST_ALL_ARG    = -a "$(ALTI_CFG_CPU)" -a "$(ALTI_CFG_OS)" -a "$(ALTI_CFG_OS_MAJOR)" -a "$(ALTI_CFG_OS_MINOR)" -a "$(ALTI_CFG_EDITION)" -a "$(ALTI_PKG_TYPE)" -a "$(ALTI_CFG_BITTYPE)" -a "$(COMPILER_NAME)" -a "$(BINEXT)" -a ".$(OBJEXT)" -a "$(LIBPRE)" -a ".$(LIBEXT)" -a ".$(SOEXT)"

#defualt Value : base pkg tagset= 0
FROM_PKG_TAGSET_NAME=0
#defualt Value : base pkg tag = 0
FROM_PKG_TAG_NAME=0

client_name:
	@echo $(RELEASE_CLIENT_DIST_NAME)

server_name:
	@echo $(RELEASE_CLIENT_DIST_NAME)

# copy files for users
pkgdistForDev: altibase_env.mk
	$(MAKE) -f Makefile pkgCopyforDev ALTI_PKG_TYPE="*"

# copy files for NATC
pkgdistForNATC: altibase_env.mk
	$(MAKE) -f Makefile pkgCopyforNATC ALTI_PKG_TYPE="*"

pkgCopyforNATC:
	-$(PKG_EXE) -m "$(PKG_NATC_MAP)" -q -e -r "$(ALTI_HOME)" -d $(DIST_ALL_ARG)

pkgCopyforDev:
	$(PKG_EXE) -m "$(PKG_MAP)" -q -e -r "$(ALTI_HOME)" -d $(DIST_ALL_ARG)

pkgdist:
	$(RM) $(WORK_DIR)

# bug-24436: add .net driver to package when vc >= 8(MSC_VER:14)
	"$(TOOL_DIR)/altipkg/altipkg$(BINEXT)" -m "$(PKG_MAP)" -r "$(WORK_DIR)" -d $(DIST_ALL_ARG)

	echo "Repository: $(SVN_REPOSITORY)" >$(PATCH_SVN_BUGINFO_FILE) 
	echo $(PKG_REVISION_INFO) >>$(PATCH_SVN_BUGINFO_FILE);
	LC_ALL=C svn info | grep "Last Changed Rev" >>$(PATCH_SVN_BUGINFO_FILE);

	$(MAKE) patch_Info;
	$(MAKE) platform_Info

pkgdist_patch:
	$(RM) $(WORK_DIR)

	"$(TOOL_DIR)/altipkg/altipkg$(BINEXT)" -m "$(PKG_MAP)" -r "$(WORK_DIR)" -d $(DIST_ALL_ARG)

	echo "Repository: $(SVN_REPOSITORY)" >$(PATCH_SVN_BUGINFO_FILE)
	echo $(PKG_REVISION_INFO) >>$(PATCH_SVN_BUGINFO_FILE);
	LC_ALL=C svn info | grep "Last Changed Rev" >>$(PATCH_SVN_BUGINFO_FILE);

	$(MAKE) patch_Info
	$(MAKE) platform_Info

#MAKE PATCH INFO
patch_Info:

ifeq "$(ALTI_PKG_TYPE)" "*" # SERVER
	echo "PRODUCT_SIGNATURE=server-$(PKG_VERSION_INFO)-"$(ALTI_CFG_BITTYPE)"-"$(BUILD_MODE) >>$(PATCH_DIR)/patchinfo
else # CLIENT 
	echo "PRODUCT_SIGNATURE=client-$(PKG_VERSION_INFO)-"$(ALTI_CFG_BITTYPE)"-"$(BUILD_MODE) >>$(PATCH_DIR)/patchinfo
endif    
	echo "PATCH_VERSION="$(FROM_PKG_TAGSET_NAME)_$(FROM_PKG_TAG_NAME)_$(PKG_TAGSET_NAME)_$(PKG_TAG_NAME) >>$(PATCH_DIR)/patchinfo

platform_Info :
	echo >>$(PATCH_DIR)/patchinfo
	echo "=======  OS INFO  =======" >>$(PATCH_DIR)/patchinfo
	uname -a >>$(PATCH_DIR)/patchinfo

ifeq "$(ALTI_CFG_OS)" "HPUX"
	echo "=======  PATCH INFO  =======" >>$(PATCH_DIR)/patchinfo
	LC_ALL=C swlist |grep -E -i "gold|qpk" >>$(PATCH_DIR)/patchinfo
	echo "=======  COMPILER INFO  =======" >>$(PATCH_DIR)/patchinfo
	/opt/aCC/bin/aCC -Version >>$(PATCH_DIR)/patchinfo 2>&1
endif

ifeq "$(ALTI_CFG_OS)" "LINUX"
	echo "=======  PATCH INFO  =======" >>$(PATCH_DIR)/patchinfo
	uname -r >>$(PATCH_DIR)/patchinfo
	echo "=======  COMPILER INFO  =======" >>$(PATCH_DIR)/patchinfo
	LC_ALL=C gcc -v 2>&1 |grep -i "gcc version" >>$(PATCH_DIR)/patchinfo
	getconf GNU_LIBC_VERSION >>$(PATCH_DIR)/patchinfo
endif

ifeq "$(ALTI_CFG_OS)" "AIX"
	echo "=======  PATCH INFO  =======" >>$(PATCH_DIR)/patchinfo
	oslevel -r >>$(PATCH_DIR)/patchinfo
	echo "=======  COMPILER INFO  =======" >>$(PATCH_DIR)/patchinfo
	echo $(shell /usr/vac/bin/xlc -qversion) >>$(PATCH_DIR)/patchinfo
endif

ifeq "$(ALTI_CFG_OS)" "SOLARIS"
	echo "=======  PATCH INFO  =======" >>$(PATCH_DIR)/patchinfo
	ls /var/sadm/patch | tail -1 >>$(PATCH_DIR)/patchinfo
	echo "=======  COMPILER INFO  =======" >>$(PATCH_DIR)/patchinfo
	/opt/SUNWspro/bin/CC -V >>$(PATCH_DIR)/patchinfo  2>&1
endif

	echo "=======  JAVA INFO  =======" >>$(PATCH_DIR)/patchinfo
	java -version >>$(PATCH_DIR)/patchinfo 2>&1

additional_files_tar : 
	cp ${HOME}/work/pgm_pkg/altibase_java/Altibase$(PGM_MAJOR_VER)_$(PGM_MINOR_VER).jar $(WORK_DIR)/lib/Altibase$(PGM_MAJOR_VER)_$(PGM_MINOR_VER).jar
	cp ${HOME}/work/pgm_pkg/altibase_java/Altibase.jar $(WORK_DIR)/lib/Altibase.jar
	cp ${HOME}/work/pgm_pkg/altibase_java/Altibase_t.jar $(WORK_DIR)/lib/Altibase_t.jar
	cp ${HOME}/work/pgm_pkg/altibase_java/altilinker.jar $(WORK_DIR)/bin/altilinker.jar

additional_files_jdbc :
	cp $(ALTI_HOME)/lib/Altibase$(PGM_MAJOR_VER)_$(PGM_MINOR_VER).jar $(WORK_DIR)/lib/Altibase$(PGM_MAJOR_VER)_$(PGM_MINOR_VER).jar
	cp $(ALTI_HOME)/lib/Altibase.jar $(WORK_DIR)/lib/Altibase.jar
	cp $(ALTI_HOME)/lib/Altibase_t.jar $(WORK_DIR)/lib/Altibase_t.jar

additional_files_altilinker :
# Altibase altilinker : altilinker.jar
	cp $(ALTI_HOME)/bin/altilinker.jar $(WORK_DIR)/bin/altilinker.jar

pkgcreat: altibase_env.mk
ifeq "$(ALTI_PKG_IB)" "yes"
ifeq "${BUILD_MODE_DIAG}" "DIAGNOSTIC"
	$(INSTALL_BUILDER) build $(ALTI_PKG_IB_XML_NAME) --setvars altibase_target_os=${ALTI_CFG_OS} altibase_cfg_cpu_value=${ALTI_CFG_CPU} altibase_compile_bit=$(ALTI_CFG_BITTYPE) altibase_build_mode=$(BUILD_MODE) altibase_base_version=${PKG_VERSION_INFO} altibase_from_tagset_version=$(FROM_PKG_TAGSET_NAME) altibase_from_tag_version=$(FROM_PKG_TAG_NAME) altibase_tagset_version=$(PKG_TAGSET_NAME) altibase_tag_version=$(PKG_TAG_NAME) altibase_build_dir=$(WORK_DIR) altibase_revision_num=${PKG_REVISION_NUM} altibase_dist_type=$(BUILD_MODE_DIAG) altibase_major_version=$(PGM_MAJOR_VER) altibase_minor_version=$(PGM_MINOR_VER)
else
	$(INSTALL_BUILDER) build $(ALTI_PKG_IB_XML_NAME) --setvars altibase_target_os=${ALTI_CFG_OS} altibase_cfg_cpu_value=${ALTI_CFG_CPU} altibase_compile_bit=$(ALTI_CFG_BITTYPE) altibase_build_mode=$(BUILD_MODE) altibase_base_version=${PKG_VERSION_INFO} altibase_from_tagset_version=$(FROM_PKG_TAGSET_NAME) altibase_from_tag_version=$(FROM_PKG_TAG_NAME) altibase_tagset_version=$(PKG_TAGSET_NAME) altibase_tag_version=$(PKG_TAG_NAME) altibase_build_dir=$(WORK_DIR) altibase_dist_type= altibase_major_version=$(PGM_MAJOR_VER) altibase_minor_version=$(PGM_MINOR_VER)
endif

	$(RM) $(WORK_DIR);
else 
ifeq "$(ALTI_PKG_IB)" "VALUE_PACK"
	$(INSTALL_BUILDER) build $(ALTI_PKG_IB_XML_NAME_VP) --setvars valuepack_name=${VALUEPACK_NAME} valuepack_version=$(valuePack_VERSION)
else
	-cd $(WORK_DIR) && tar cvf - * | gzip -c > ../$(ALTI_PKG_NAME).tgz

	$(RM) $(WORK_DIR);
endif
endif


#TASK-6469
additional_files_checksum :
	echo "=======  CREATE CHECKSUM FILE  =======" > /dev/null
	$(CHECKSUM_MD5) $(CHECKSUM_FILE_NAME).run > $(CHECKSUM_FILE_NAME).run.md5


##############################
#  User Command for Package
##############################

TAG_NAME_MSG="Please specify PKG_TAG_NAME (Alpha, Beta1, Beta2, Beta3, RC1, RC2, 0, 1, 2, ... )"
TAGSET_NAME_MSG="Please specify PKG_TAGSET_NAME"

pkg_server: altibase_env.mk manual
ifeq ($(PKG_TAGSET_NAME),)
	@echo $(TAGSET_NAME_MSG)
else
ifeq ($(PKG_TAG_NAME),)  
	@echo $(TAG_NAME_MSG)
else
	$(MAKE) -f Makefile pkgdist ALTI_PKG_TYPE="*" WORK_DIR=$(SERVER_DIST_DIR)
	$(MAKE) -f Makefile pkgcreat ALTI_PKG_TYPE="*" ALTI_PKG_NAME="$(RELEASE_SERVER_DIST_NAME)" WORK_DIR=$(SERVER_DIST_DIR)
endif
endif

pkg_client: altibase_env.mk manual
ifeq ($(PKG_TAGSET_NAME),)
	@echo $(TAGSET_NAME_MSG)
else
ifeq ($(PKG_TAG_NAME),)  
	@echo $(TAG_NAME_MSG)
else
	$(MAKE) -f Makefile pkgdist ALTI_PKG_TYPE="CLIENT" WORK_DIR=$(CLIENT_DIST_DIR)
	$(MAKE) -f Makefile pkgcreat ALTI_PKG_TYPE="CLIENT" ALTI_PKG_NAME="$(RELEASE_CLIENT_DIST_NAME)" WORK_DIR=$(CLIENT_DIST_DIR)
endif
endif

pkg_server_patch: altibase_env.mk manual
ifeq ($(PKG_TAGSET_NAME),)
	@echo $(TAGSET_NAME_MSG)
else
ifeq ($(PKG_TAG_NAME),)  
	@echo $(TAG_NAME_MSG)
else
	$(MAKE) -f Makefile pkgdist_patch ALTI_PKG_TYPE="*" WORK_DIR=$(SERVER_DIST_DIR_PATCH) ALTI_CFG_EDITION=$(ALTI_CFG_EDITION)
	$(MAKE) -f Makefile pkgcreat ALTI_PKG_TYPE="*" ALTI_PKG_NAME="$(RELEASE_SERVER_DIST_NAME_PATCH)" WORK_DIR=$(SERVER_DIST_DIR_PATCH)
endif
endif

pkg_client_patch: altibase_env.mk manual
ifeq ($(PKG_TAGSET_NAME),)
	    @echo $(TAGSET_NAME_MSG)
else
ifeq ($(PKG_TAG_NAME),)  
	@echo $(TAG_NAME_MSG)
else
	$(MAKE) -f Makefile pkgdist_patch ALTI_PKG_TYPE="CLIENT" WORK_DIR=$(CLIENT_DIST_DIR_PATCH)
	$(MAKE) -f Makefile pkgcreat ALTI_PKG_TYPE="CLIENT" ALTI_PKG_NAME="$(RELEASE_CLIENT_DIST_NAME_PATCH)" WORK_DIR=$(CLIENT_DIST_DIR_PATCH)
endif
endif

pkg_server_tar: altibase_env.mk manual
ifeq ($(PKG_TAGSET_NAME),)
	@echo $(TAGSET_NAME_MSG)
else
ifeq ($(PKG_TAG_NAME),)
	@echo $(TAG_NAME_MSG)
else
	$(MAKE) -f Makefile pkgdist_patch ALTI_PKG_TYPE="*" WORK_DIR=$(SERVER_DIST_DIR_PATCH) ALTI_CFG_EDITION=$(ALTI_CFG_EDITION)
	$(MAKE) additional_files_tar WORK_DIR=$(SERVER_DIST_DIR_PATCH)
	$(MAKE) -f Makefile pkgcreat ALTI_PKG_TYPE="*" ALTI_PKG_NAME="$(TAR_SERVER_DIST_NAME)" WORK_DIR=$(SERVER_DIST_DIR_PATCH)
	echo "=======  CREATE CHECKSUM FILE  =======" > /dev/null
	$(CHECKSUM_MD5) $(TAR_SERVER_DIST_NAME)* > $(TAR_SERVER_DIST_NAME).tgz.md5
endif
endif

unoff_pkg_server: altibase_env.mk manual
	$(MAKE) -f Makefile pkgdist ALTI_PKG_TYPE="*" WORK_DIR=$(SERVER_DIST_DIR) PKG_TAGSET_NAME=test PKG_TAG_NAME=test
	$(MAKE) additional_files_jdbc WORK_DIR=$(SERVER_DIST_DIR)
	$(MAKE) additional_files_altilinker WORK_DIR=$(SERVER_DIST_DIR)
	$(MAKE) -f Makefile pkgcreat ALTI_PKG_TYPE="*" ALTI_PKG_NAME="$(UNOFFICIAL_SERVER_DIST_NAME)" WORK_DIR=$(SERVER_DIST_DIR)

unoff_pkg_client: altibase_env.mk manual
	$(MAKE) -f Makefile pkgdist ALTI_PKG_TYPE="CLIENT" WORK_DIR=$(CLIENT_DIST_DIR)
	$(MAKE) additional_files_jdbc WORK_DIR=$(CLIENT_DIST_DIR)
	$(MAKE) -f Makefile pkgcreat ALTI_PKG_TYPE="CLIENT" ALTI_PKG_NAME="$(UNOFFICIAL_CLIENT_DIST_NAME)" WORK_DIR=$(CLIENT_DIST_DIR)

diag_pkg_server_ib:
	$(MAKE) pkg_server_patch ALTI_PKG_IB=yes ALTI_PKG_IB_XML_NAME=serverPatchInstall.xml BUILD_MODE_DIAG=DIAGNOSTIC PKG_TAGSET_NAME=$(PGM_TAGSET_VER) PKG_TAG_NAME=$(PGM_TAG_VER)

diag_pkg_client_ib:
	$(MAKE) pkg_client_patch ALTI_PKG_IB=yes ALTI_PKG_IB_XML_NAME=clientPatchInstall.xml BUILD_MODE_DIAG=DIAGNOSTIC PKG_TAGSET_NAME=$(PGM_TAGSET_VER) PKG_TAG_NAME=$(PGM_TAG_VER)

pkg_server_ib:
	$(MAKE) pkg_server ALTI_PKG_IB=yes  ALTI_PKG_IB_XML_NAME=serverInstall.xml


pkg_client_ib: 
	$(MAKE) pkg_client ALTI_PKG_IB=yes ALTI_PKG_IB_XML_NAME=clientInstall.xml

pkg_server_patch_ib:  
	$(MAKE) pkg_server_patch ALTI_PKG_IB=yes ALTI_PKG_IB_XML_NAME=serverPatchInstall.xml
	$(MAKE) -f Makefile additional_files_checksum CHECKSUM_FILE_NAME="$(OFFICIAL_SERVER_DIST_NAME)"

pkg_client_patch_ib: 
	$(MAKE) pkg_client_patch ALTI_PKG_IB=yes ALTI_PKG_IB_XML_NAME=clientPatchInstall.xml
	$(MAKE) -f Makefile additional_files_checksum CHECKSUM_FILE_NAME="$(OFFICIAL_CLIENT_DIST_NAME)"

pkg_server_community_edition_ib:
	$(MAKE) pkg_server_patch ALTI_PKG_IB=yes ALTI_CFG_EDITION=COMMUNITY ALTI_PKG_IB_XML_NAME=serverCommunityEditionInstall.xml
	$(MAKE) -f Makefile additional_files_checksum CHECKSUM_FILE_NAME="$(COMMUNITY_SERVER_DIST_NAME)"

pkg_server_open_edition_ib:
	$(MAKE) pkg_server_patch ALTI_PKG_IB=yes ALTI_CFG_EDITION=OPEN ALTI_PKG_IB_XML_NAME=serverOpenEditionInstall.xml
	$(MAKE) -f Makefile additional_files_checksum CHECKSUM_FILE_NAME="$(OPEN_SERVER_DIST_NAME)"

pkg_valuepack: altibase_env.mk
ifeq ($(valuePack_VERSION),)
	@echo "Please specify valuePack_VERSION"
else
	$(MAKE) -f Makefile pkgcreat ALTI_PKG_IB="VALUE_PACK"
endif

dist_server: altibase_env.mk manual
	$(MAKE) -f Makefile pkgdist ALTI_PKG_TYPE="*" WORK_DIR=$(SERVER_DIST_DIR) 

dist_client: altibase_env.mk manual
	$(MAKE) -f Makefile pkgdist ALTI_PKG_TYPE="CLIENT" WORK_DIR=$(CLIENT_DIST_DIR)

dist_server_patch: altibase_env.mk manual
	$(MAKE) -f Makefile pkgdist_patch ALTI_PKG_TYPE="*" WORK_DIR=$(SERVER_DIST_DIR_PATCH) 

dist_client_patch: altibase_env.mk  manual
	$(MAKE) -f Makefile pkgdist_patch ALTI_PKG_TYPE="CLIENT" WORK_DIR=$(CLIENT_DIST_DIR_PATCH)


altibase_env.mk:
	$(MAKE) -s -C install all


###################################################################
#
#  TAGS section
#
###################################################################

SRC_DIRS=src/core src/pd src/id src/mt src/qp src/sd src/st src/dk src/cm src/mm src/rp src/sm src/ul ut

tags:
	-$(RM) TAGS
	for i in '$(SRC_DIRS)'; do find $$i -name "*.c" -o -name "*.cpp" -o -name "*.java" -o -name "*.h" -o -name "*.i" | grep -v win32 | xargs etags --append; done

ctags:
	ctags -R --language-force=C++ --exclude='\.svn' $(DIRS)
	for i in $(DIRS); do (rm -f $$i/tags; ln -s `pwd`/tags $$i/tags; ) done

###################################################################
#
#  ETC / DEBUG section
#
###################################################################

productTime =$(shell $(GENERRMSG) -k)

pktest:
	@echo $(productTime);
	@echo $(DIST_NAME);
	@echo $(RELEASE_SERVER_DIST_NAME);
	@echo $(RELEASE_CLIENT_DIST_NAME);

betarelease:
	sh add_revisioninfo.sh
	$(Q) cd $(DEV_DIR); $(CONFIGURE)  --with-build_mode=release
	$(MAKE) build_all


###################################################################
#
#  HELP
#
###################################################################
help:
	@echo "to see more help.."
	@echo "make build_help"
	@echo "make pkg_help"

pkg_help:
	@echo "Unofficial package - Zip"
	@echo "make unoff_pkg_server --> to make server package file"
	@echo "make unoff_pkg_client --> to make client package file"
	@echo ""
	@echo "Diagnostic package - InstallBuilder"
	@echo "make diag_pkg_server_ib --> to make server package file"
	@echo "make diag_pkg_client_ib --> to make client package file"
	@echo ""
	@echo "Zip (packaging of package file)"
	@echo "make pkg_server PKG_TAGSET_NAME=tagset PKG_TAG_NAME=tag --> to make server package file"
	@echo "make pkg_client PKG_TAGSET_NAME=tagset PKG_TAG_NAME=tag --> to make client package file"
	@echo "make pkg_server_patch PKG_TAGSET_NAME=tagset PKG_TAG_NAME=tag --> to make server package file"
	@echo "make pkg_client_patch PKG_TAGSET_NAME=tagset PKG_TAG_NAME=tag --> to make server package file"
	@echo ""
	@echo "InstallBuilder Package"
	@echo "make pkg_server_ib PKG_TAGSET_NAME=tagset PKG_TAG_NAME=tag --> to make server package file"
	@echo "make pkg_client_ib PKG_TAGSET_NAME=tagset PKG_TAG_NAME=tag --> to make client package file"
	@echo "make pkg_server_patch_ib PKG_TAGSET_NAME=tagset PKG_TAG_NAME=tag --> to make server package file"
	@echo "make pkg_client_patch_ib PKG_TAGSET_NAME=tagset PKG_TAG_NAME=tag --> to make server package file"
	@echo "make pkg_server_community_edition_ib PKG_TAGSET_NAME=tagset PKG_TAG_NAME=tag --> to make community edition server package file"	
	@echo "make pkg_server_open_edition_ib PKG_TAGSET_NAME=tagset PKG_TAG_NAME=tag --> to make open edition server package file"	
	@echo ""
	@echo "Client ValuePack"
	@echo "make pkg_valuepack valuePack_VERSION=V1.0"
	@echo ""
	@echo "Copy (distribution of package file)"
	@echo "make dist_server PKG_TAGSET_NAME=tagset PKG_TAG_NAME=tag --> To copy server package files to SERVER_PKG_DIRECTORY"
	@echo "make dist_client PKG_TAGSET_NAME=tagset PKG_TAG_NAME=tag --> To copy server package files to CLIENT_PKG_DIRECTORY"
	@echo "make dist_server_patch PKG_TAGSET_NAME=tagset PKG_TAG_NAME=tag"
	@echo "make dist_client_patch PKG_TAGSET_NAME=tagset PKG_TAG_NAME=tag"

build_help:
	@echo "make build_all : clean all && build all source code"
	@echo "make build : just build all source code"

###################################################################
#
#  UnitTest section
#
###################################################################

unittest:
	$(MAKE) -C $(ID_DIR) build_ut
	$(MAKE) -C $(CM_DIR) build_ut
	$(MAKE) -C $(DK_DIR) build_ut
	$(MAKE) -C $(SM_DIR) build_ut
	$(MAKE) -C $(ST_DIR) build_ut
	$(MAKE) -C $(QP_DIR) build_ut
	$(MAKE) -C $(SD_DIR) build_ut
	$(MAKE) -C $(RP_DIR) build_ut
	$(MAKE) -C $(MM_DIR) build_ut
	$(MAKE) -C $(MT_DIR) build_ut
	$(MAKE) -C $(UL_DIR) build_ut
	$(MAKE) -C $(UT_DIR) build_ut

clean_ut:
	$(MAKE) -C $(ID_DIR) clean_ut
	$(MAKE) -C $(CM_DIR) clean_ut
	$(MAKE) -C $(DK_DIR) clean_ut
	$(MAKE) -C $(SM_DIR) clean_ut
	$(MAKE) -C $(ST_DIR) clean_ut
	$(MAKE) -C $(QP_DIR) clean_ut
	$(MAKE) -C $(SD_DIR) clean_ut
	$(MAKE) -C $(RP_DIR) clean_ut
	$(MAKE) -C $(MM_DIR) clean_ut
	$(MAKE) -C $(MT_DIR) clean_ut
	$(MAKE) -C $(UL_DIR) clean_ut
	$(MAKE) -C $(UT_DIR) clean_ut
