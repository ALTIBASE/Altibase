#include /home/sjkim/Work/sm/src/makefiles/sparc_solaris_cc.mk

all : compat4 compat5 compat64 solaris_gnu

solaris_gnu : solaris_gnu.o
	g++  -L../lib   -o  solaris_gnu  solaris_gnu.$(OBJEXT) -lACE_ssolaris27_gcc  -lsocket -ldl -lnsl -lgen -lposix4

solaris_gnu.$(OBJEXT) : compat.cpp
	g++  -g -DACE_LACKS_RTTI -instances=explicit   -DACE_NO_INLINE -DACE_LACKS_ACE_TOKEN -DACE_LACKS_ACE_OTHER -DWHAT=\"gnu\"  -I..  -c compat.cpp -o solaris_gnu.o


compat4 : compat4.o
	CC -compat=4 -mt -L../lib -library=iostream,no%Cstd   -o compat4  compat4.$(OBJEXT) -lACE_ssolaris27_compat4  -lsocket -ldl -lnsl -lgen -lposix4

compat4.$(OBJEXT) : compat.cpp
	CC -compat=4 -mt -g -DACE_LACKS_RTTI -instances=explicit -noex  -DACE_NO_INLINE -DACE_LACKS_ACE_TOKEN -DACE_LACKS_ACE_OTHER -DWHAT=\"compat4\" -mt -library=iostream,no%Cstd -I..  -c compat.cpp -o compat4.o

compat5 : compat5.o
	CC -mt  -library=iostream,no%Cstd -L../lib  -o compat5  compat5.$(OBJEXT) -lACE_ssolaris27_compat5 -lsocket -ldl -lnsl -lgen -lposix4

compat5.$(OBJEXT) : compat.cpp
	CC -mt -g -library=iostream,no%Cstd -instances=explicit -noex  -DACE_NO_INLINE -DACE_LACKS_ACE_TOKEN -DACE_LACKS_ACE_OTHER -DWHAT=\"compat5\" -mt -library=iostream,no%Cstd -I..  -c compat.cpp -o compat5.o

compat64 : compat64.o
	CC -xarch=v9 -mt -library=iostream,no%Cstd -L../lib -L/opt/SUNWspro/SC5.0/lib/v9 -L/usr/lib/sparcv9 -o compat64  compat64.$(OBJEXT) -lACE_ssolaris27_64    -lsocket -ldl -lnsl -lgen -lposix4

compat64.$(OBJEXT) : compat.cpp
	CC -xarch=v9 -mt -g -library=iostream,no%Cstd -instances=explicit -noex  -DACE_NO_INLINE -DACE_LACKS_ACE_TOKEN -DACE_LACKS_ACE_OTHER -DWHAT=\"compat64\" -mt -library=iostream,no%Cstd -I..  -c compat.cpp -o compat64.o


clean :
	\$(RM) compat4 compat5 compat64 *.$(OBJEXT) *~

