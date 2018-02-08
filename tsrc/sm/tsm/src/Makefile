# Generated automatically from Makefile.in by configure.

# $Id: Makefile 68602 2015-01-23 00:13:11Z sbjang $

SMSRC	= $(SM_DIR)/src

DIRS	= lib tsm_system tsm_trans tsm_lock tsm_cursor tsm_mixed tsm_test tsm_sync tsm_init tsm_multi tsm_update tsm_temp tsm_insert tsm_mempool2 tsm_compression #tsm_parallel_logging

PURE_DIRS	= lib tsm_system tsm_trans tsm_lock tsm_mixed tsm_temp 


all:	ofiles

ofiles:
	$(Q) for i in $(DIRS); do (cd $$i; $(MAKE) $(S) ) done

lib: ofiles
	cd lib; $(MAKE) lib

check :
	for i in $(DIRS); do (cd $$i; $(MAKE) $(S) check) done

alint:
	for i in $(DIRS); do (cd $$i; $(MAKE) $(S) alint) done

pure:
	for i in $(PURE_DIRS); do (cd $$i; $(MAKE) pure) done


clean:
	for i in $(DIRS); do (cd $$i; $(MAKE) $(S) clean) done
	cd lib; $(MAKE) clean
	$(RM) *~ *.*~ 

dep depend:
	for i in $(DIRS); do (cd $$i; $(MAKE) $(S) depend) done


tags tag:
	\$(RM) TAGS
	etags  */*.cpp  */*.h

