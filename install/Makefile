# 작성 가이드
# 1. echo 이후에 "를 넣지 말것!! 윈도우에서 "를 출력함
#    (Unix가 명확할 경우 한정해서 사용!!!)
# 2. echo  를 하지 말것..윈도우에서 에러가 출력됨

include ../env.mk

ifeq ($(ALTIBASE_PRODUCT),hdb)
ALTIBASE_ENV := altibase_env.mk
else
ALTIBASE_ENV := xdbaltibase_env.mk
endif

# if using odbc driver manager, not odbccli.., use this macros
ifeq ($(OS_TARGET),INTEL_WINDOWS)
ODBC_INCLUDES = 
ODBC_LIBDIRS  = 
ODBC_LIBS += odbc32.lib
else
 ifeq ($(compile64),1)
ODBC_INCLUDES = $(IDROPT)/usr/local/odbcDriverManager64/include
ODBC_LIBDIRS  = $(LDROPT)/usr/local/odbcDriverManager64/lib
 else
ODBC_INCLUDES = $(IDROPT)/usr/local/odbcDriverManager32/include
ODBC_LIBDIRS  = $(LDROPT)/usr/local/odbcDriverManager32/lib
 endif
ODBC_LIBS     = $(LIBOPT)odbc$(LIBAFT)
endif
.PHONY:print build

all: build

print:
	@cat altibase_env_head.mk
	@echo Q=$(Q)
	@echo NM=$(NM)
	@echo NMFLAGS=$(NMFLAGS)
	@echo OS_TARGET=$(OS_TARGET)
	@echo OS_MAJORVER=$(OS_MAJORVER)
	@echo OS_MINORVER=$(OS_MINORVER)
	@echo OS_LINUX_PACKAGE=$(OS_LINUX_PACKAGE)
	@echo OS_LINUX_VERSION=$(OS_LINUX_VERSION)
	@echo OS_LINUX_KERNEL=$(OS_LINUX_KERNEL)
	@echo compile64=$(compile64)
ifeq "$(BUILD_MODE)" "debug"
	@echo BUILD_MODE=$(BUILD_MODE)
endif
	@echo compat5=$(compat5)
	@echo CXX=$(CXX)
	@echo CC=$(CC)
	@echo AR=$(AR)
	@echo COMPILE.cc=$(COMPILE.cc)
	@echo COMPILE.c=$(COMPILE.c)
	@echo SHCOMPILE.cc=$(SHCOMPILE.cc)
	@echo SHCOMPILE.c=$(SHCOMPILE.c)
	@echo DEFOPT=$(DEFOPT)
	@echo IDROPT=$(IDROPT)
	@echo LDROPT=$(LDROPT)
	@echo LIBOPT=$(LIBOPT)
	@echo LIBAFT=$(LIBAFT)
	@echo AROUT=$(AROUT)
	@echo LDOUT=$(LDOUT)
	@echo SOOUT=$(SOOUT)
	@echo CCOUT=$(CCOUT)
	@echo ARFLAGS=$(ARFLAGS)
	@echo DEFINES=$(DEFINES)
	@echo CCFLAGS=$(CCFLAGS)
	@echo CFLAGS=$(CFLAGS)
	@echo DCFLAGS=$(DCFLAGS)
	@echo DCCFLAGS=$(DCCFLAGS)
	@echo OCFLAGS=$(OCFLAGS)
	@echo OCCFLAGS=$(OCCFLAGS)
	@echo EFLAGS=$(EFLAGS)
	@echo SFLAGS=$(SFLAGS)
	@echo LD=$(LD)
	@echo LFLAGS=$(LFLAGS)
	@echo OBJEXT=$(OBJEXT)
	@echo SOEXT=$(SOEXT)
	@echo BINEXT=$(BINEXT)
	@echo LIBEXT=$(LIBEXT)
	@echo LIBPRE=$(LIBPRE)
	@echo COPY=$(COPY)
	@echo RM=$(RM)
	@echo ODBC_INCLUDES=$(ODBC_INCLUDES)
	@echo ODBC_LIBDIRS=$(ODBC_LIBDIRS)
	@echo ODBC_LIBS=$(ODBC_LIBS)
ifeq "$(LIBS_SHIP)" ""
	@echo LIBS=$(LIBS)
else
	@echo LIBS=$(LIBS_SHIP)
endif
ifeq ($(OS_TARGET),IBM_AIX)
	@echo SH_LIBS=
	@echo RTL_FLAG=-brtl
else
ifeq "$(LIBS_SHIP)" ""
	@echo SH_LIBS=$(LIBS)
else
	@echo SH_LIBS=$(LIBS_SHIP)
endif
	@echo RTL_FLAG=
endif
ifeq ($(OS_TARGET),INTEL_WINDOWS)
else
	@echo "LDOUT +=  # intentionally put this "
	@echo "SOOUT +=  # intentionally put this "
	@echo "CCOUT +=  # intentionally put this "
endif

	@echo SOLINK.cc=$(SOLINK.cc)
	@echo SOFLAGS=$(SOFLAGS)
	@echo PIC=$(PIC)
# proj-1538 ipv6: pd/include/w2tcpip.h needed for some defines when VC9
ifeq ($(OS_TARGET),INTEL_WINDOWS)
	@cat altibase_env_tail_win.mk
else
	@cat $(PRODUCT_PREFIX)altibase_env_tail.mk
endif

build:
	@$(MAKE) -s -f Makefile print | grep -v directory > $(ALTIBASE_ENV)
