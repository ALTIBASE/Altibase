#TASK-6461
#

include ../utEnv.mk

DIST_DIR=$(ALTI_HOME)/packages

PACKAGE_FILE_LIST=dbms_application_info dbms_lock dbms_output dbms_random dbms_sql utl_file utl_raw utl_tcp utl_smtp dbms_stats dbms_alert dbms_concurrent_exec dbms_recyclebin dbms_utility dbms_shard standard utl_copyswap

ifeq "$(ALTIBASE_PRODUCT)" "hdb"
PACKAGE_FILE_LIST += sys_context 
endif

.PHONY: all compile clean

all: compile

compile:
	$(COPY) catproc.sql $(DIST_DIR)/catproc.sql
	for packagename in ${PACKAGE_FILE_LIST}; \
	do ( \
		if [ -f $$packagename"_body.sql" ]; \
		then $(ALTI_HOME)/bin/$(PRODUCT_PREFIX)altiwrap$(BINEXT) --iname $$packagename"_body" --oname $(DIST_DIR)/$$packagename || exit $? ; \
		else $(ALTI_HOME)/bin/$(PRODUCT_PREFIX)altiwrap$(BINEXT) --iname $$packagename --oname $(DIST_DIR)/$$packagename || exit $?; \
		fi ;\
		echo "$(COPY) $$packagename.sql $(DIST_DIR)/$$packagename.sql;"; $(COPY) $$packagename.sql $(DIST_DIR)/$$packagename.sql; \
	) done
clean: 
	$(RM) $(DIST_DIR)/*

