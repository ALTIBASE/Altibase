DIRS	= sm svr

all:	ofiles

ofiles:
	$(Q) for i in $(DIRS); do (cd $$i; $(MAKE) $(S) ) done

lib: ofiles
	cd lib; $(MAKE) lib

check :
	for i in $(DIRS); do (cd $$i; $(MAKE) $(S) check) done

alint:
	for i in $(DIRS); do (cd $$i; $(MAKE) $(S) alint) done

clean:
	for i in $(DIRS); do (cd $$i; $(MAKE) $(S) clean) done
	$(RM) *~ *.*~ 

dep depend:
	for i in $(DIRS); do (cd $$i; $(MAKE) $(S) depend) done


tags tag:
	\$(RM) TAGS
	etags  */*.cpp  */*.h

