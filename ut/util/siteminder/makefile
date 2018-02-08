###########################################################################
# Copyright (C) 1997-2000, Netegrity, Inc. All rights reserved.
#
# Netegrity, Inc. makes no representations concerning either
# the merchantability of this software or the suitability of this
# software for any particular purpose. It is provided "as is"
# without express or implied warranty of any kind.
#
# SiteMinder Directory API sample.
#
###########################################################################

#include $(ALTIBASE_HOME)/install/altibase_env.mk

DIRAPI = smdirapi
LIBNAME  = lib$(DIRAPI).so
SOURCES  = $(DIRAPI).cpp
OBJS     = $(DIRAPI).o

INCLUDE_DIR   = ./include 
INCLUDE_FLAGS = -I$(ALTIBASE_HOME)/include -I$(INCLUDE_DIR)

CC_CMD = g++
LD_CMD = g++ -G -lpthread  -O3
#LD_CMD = g++ -G -L. -O3 -funroll-loops 
#LIBS = /usr/lib/libC.so.5 /usr/lib/libCrun.so.1 
#LIBDIRS = -L$(ALTIBASE_HOME)/lib -lodbccli 
#LD_FLAGS += $(LIBDIRS)

all:
	@echo "Usage"
	@echo "GCC    : make gcc"

gcc: $(LIBNAME)

$(LIBNAME):$(OBJS)
	$(LD_CMD) $(OBJS) $(LD_FLAGS) -o $@ $(LIBS)

$(OBJS): $(SOURCES)
	$(CC_CMD) $(CCFLAGS) -c $(INCLUDE_FLAGS) -o $@ $(SOURCES)

clean:
	/usr/bin/rm -rf $(OBJS) $(LIBNAME)

depend:
	/usr/openwin/bin/makedepend *.cpp

# DO NOT DELETE THIS LINE -- make depend depends on it.
