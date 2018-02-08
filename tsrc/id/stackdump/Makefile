# $Id: Makefile 13578 2005-09-10 11:34:32Z jdlee $

include ../../idEnv.mk

SRCS    = Latch.cpp
OBJS    = $(OBJS:.cpp = .o)
BINS	= lockptest
RESULT_FILE = result.txt
GREPED_FILE = grep.txt

LIBS    := -lid -lpd $(LIBS)
LIBDIRS := -L$(ID_DIR)/src/lib $(LIBDIRS)
all: mytest mytest_o3# stackdump



stackdump: stackdump.o
	 $(LD)  $(ACE_FLAG) $(LFLAGS) $(GOPT) $(INCLUDES) $(LIBDIRS) -o  $@ stackdump.$(OBJEXT) $(LIBS)

mytest: mytest.o
	g++  -g -D__ACE_INLINE__ -DACE_LACKS_ACE_TOKEN -DACE_LACKS_ACE_OTHER -L.   -funroll-loops -L/home/sjkim/Work/altibasedev/id/src/lib -L. -L/home/sjkim/Work/altibasedev/id/src/ACE/lib  -I/home/sjkim/Work/altibasedev/id/src/include/ -I/home/sjkim/Work/altibasedev/id/src/ACE -I/usr/local/include  -L/home/sjkim/Work/altibasedev/id/src/lib -L. -L/home/sjkim/Work/altibasedev/id/src/ACE/lib -o  mytest mytest.$(OBJEXT) -lid  -ldl -lpthread -lcrypt 

mytest_o3: mytest_o3.o
	g++  -g -D__ACE_INLINE__ -DACE_LACKS_ACE_TOKEN -DACE_LACKS_ACE_OTHER -L.   -O3 -funroll-loops -L/home/sjkim/Work/altibasedev/id/src/lib -L. -L/home/sjkim/Work/altibasedev/id/src/ACE/lib  -I/home/sjkim/Work/altibasedev/id/src/include/ -I/home/sjkim/Work/altibasedev/id/src/ACE -I/usr/local/include  -L/home/sjkim/Work/altibasedev/id/src/lib -L. -L/home/sjkim/Work/altibasedev/id/src/ACE/lib -o  mytest_o3 mytest_o3.$(OBJEXT) -lid  -ldl -lpthread -lcrypt 

mytest.$(OBJEXT) : mytest.cpp
	g++ -g -W -Wall -pipe -D_POSIX_PTHREAD_SEMANTICS -D_POSIX_THREADS -D_POSIX_THREAD_SAFE_FUNCTIONS -D_REENTRANT -DACE_HAS_AIO_CALLS -DACE_NDEBUG -fno-implicit-templates  -fno-exceptions -fcheck-new -D__ACE_INLINE__ -DACE_LACKS_ACE_TOKEN -DACE_LACKS_ACE_OTHER  -I/home/sjkim/Work/altibasedev/id/src/ACE  -c -I/home/sjkim/Work/altibasedev/id/src/include/ -I/home/sjkim/Work/altibasedev/id/src/ACE -I/usr/local/include  -o mytest.$(OBJEXT) mytest.cpp

mytest_o3.o: mytest.cpp
	g++ -g -W -Wall -pipe -D_POSIX_PTHREAD_SEMANTICS -D_POSIX_THREADS -D_POSIX_THREAD_SAFE_FUNCTIONS -D_REENTRANT -DACE_HAS_AIO_CALLS -O3 -DACE_NDEBUG -fno-implicit-templates  -fno-exceptions -fcheck-new -D__ACE_INLINE__ -DACE_LACKS_ACE_TOKEN -DACE_LACKS_ACE_OTHER  -I/home/sjkim/Work/altibasedev/id/src/ACE  -c -I/home/sjkim/Work/altibasedev/id/src/include/ -I/home/sjkim/Work/altibasedev/id/src/ACE -I/usr/local/include  -o mytest_o3.$(OBJEXT) mytest.cpp


check :
	@for i in $(SRCS); do (gawk -v API_INPUT=$(TOOL_DIR)/coding_chk/API.txt -v SRC_INPUT=$$i -f $(TOOL_DIR)/coding_chk/apiCheck.awk ) done

clean:
	$(RM) core *.$(OBJEXT) *.d stackdump SunWS_cache  *.msb *.ih *~ $(all)

clean_bin:
	$(RM) $(BINS)

dep depend:
	-$(RM) ./.dependency
	$(CXX) $(CXXOPT_DEPENDANCY) $(INCLUDES) $(SRCS) > .dependency

ifeq (./.dependency,$(wildcard ./.dependency))
include ./.dependency
endif 
