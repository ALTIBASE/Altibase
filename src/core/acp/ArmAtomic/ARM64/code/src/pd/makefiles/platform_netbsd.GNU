# platform_netbsd.GNU,v 4.6 1998/02/08 08:07:07 nanbor Exp

# platform_netbsd.GNU

# ***** Notice: This file is still under testing. *****
# To be used on netbsd platform without pthread support.

debug = 1
optimize = 1
versioned_so = 1

CC              = gcc
CXX		= g++

CXX_VERSION := $(shell $(CXX) -dumpversion)

ifeq (3.,$(findstring 3.,$(CXX_VERSION)))
 use_strict_prototype=no
else
 use_strict_prototype=yes
endif

CFLAGS          += -w

ifeq ($(use_strict_prototype),yes)
  CFLAGS  +=  -fno-strict-prototypes
endif

CCFLAGS         += $(CFLAGS) -fno-implicit-templates
DCFLAGS         += -g
DLD             = ld
LD		= $(CXX)
LIBS            += -lstdc++  #-lcompat -lresolv
##OCFLAGS         += -O2
PIC             = -fpic -DPIC
AR              = ar
ARFLAGS         = ruv
RANLIB          = ranlib

SOFLAGS = -Bshareable -x /usr/lib/c++rt0.o
SOBUILD = $(COMPILE.cc) $(PIC) -o $(VSHDIR)$*.so  $<
PRELIB = @echo
