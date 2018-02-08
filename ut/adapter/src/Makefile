# $Id$
include adapter_objs.mk

all: pkg_jdbcAdapter

oraAdapter: clean_oraAdpater $(ORA_ADAPTER_SRCS)
	@echo "make oraAdapter"
	$(MAKE) -f Makefile.ora build

altiAdapter: clean_altiAdapter $(ALTIBASE_ADAPTER_SRCS)
	@echo "make altiAdapter"
	$(MAKE) -f Makefile.alti build

jdbcAdapter: clean_jdbcAdapter $(JDBC_ADAPTER_SRCS)
	@echo "make jdbcAdapter"
	$(MAKE) -f Makefile.jdbc build

dist_oraAdapter:
	$(MAKE) -f Makefile.ora dist

dist_altiAdapter:
	$(MAKE) -f Makefile.alti pkg

dist_jdbcAdapter:
	$(MAKE) -f Makefile.jdbc pkg

pkg_jdbcAdapter: clean_jdbcAdapter $(JDBC_ADAPTER_SRCS)
	@echo "make jdbcAdapter"
	$(MAKE) -f Makefile.jdbc build
	$(MAKE) -f Makefile.jdbc dist

clean: clean_oraAdpater clean_altiAdapter clean_jdbcAdapter

clean_oraAdpater:
	$(MAKE) -f Makefile.ora clean
	
clean_altiAdapter:
	$(MAKE) -f Makefile.alti clean

clean_jdbcAdapter:
	$(MAKE) -f Makefile.jdbc clean
