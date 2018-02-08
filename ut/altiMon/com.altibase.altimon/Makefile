# Makefile for altiMon
# $Id: Makefile 75190 2016-04-21 07:44:19Z bethy $
#

include ../../utEnv.mk
include objs.mk

java_version = 1.5
JAVAC = $(JAVA_HOME)/bin/javac
JAR = $(JAVA_HOME)/bin/jar

ALTIMON_JAR = altimon.jar
ALTIMON_TARBALL = altimon.tar.gz
DIST_DIR = altiMon

ifeq "$(BUILD_MODE)" "debug"
	JFLAGS = -g -nowarn
else
	JFLAGS = -g:none
endif
JFLAGS += -source $(java_version) -target $(java_version)
DEST_DIR = classes

ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
	CLASSPATH = -cp "$(DEST_DIR);lib/com.altibase.picl.jar;lib/commons-collections-3.2.jar;lib/log4j.jar;lib/stax-1.2.0.jar;lib/commons-logging-1.1.jar;lib/quartz-all-1.6.6.jar;lib/stax-api-1.0.1.jar;lib/commons-codec-1.6.jar"
else
	CLASSPATH = -cp $(DEST_DIR):lib/com.altibase.picl.jar:lib/commons-collections-3.2.jar:lib/log4j.jar:lib/stax-1.2.0.jar:lib/commons-logging-1.1.jar:lib/quartz-all-1.6.6.jar:lib/stax-api-1.0.1.jar:lib/commons-codec-1.6.jar
endif

MANIFEST = src/MANIFEST.MF

ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
	ACTION_SCRIPTS=action_scripts/*.bat
	ifeq ($(compile64),1)
    	SRC_BIN=altimon_win64.bat
	else
    	SRC_BIN=altimon_win32.bat
    endif
    DEST_BIN=$(PRODUCT_PREFIX)altimon.bat
else
	ACTION_SCRIPTS=action_scripts/*.sh
    SRC_BIN=altimon.sh
    DEST_BIN=$(PRODUCT_PREFIX)altimon.sh
endif

BINS=altimon.sh altimon_win64.bat altimon_win32.bat

ACTION_SCRIPTS=action_scripts/*.sh action_scripts/*.bat

all: compile

compile: altimon_jar

altimon_jar: classes
	$(Q) echo "make $(ALTIMON_JAR)"
	$(Q) $(JAR) cfm $(ALTIMON_JAR) $(MANIFEST) -C $(DEST_DIR) .

classes: $(ALTIMON_SRCS)
	$(Q) echo "compile ..."
	$(Q) $(RM) $(DEST_DIR)
	$(Q) mkdir $(DEST_DIR)
	$(Q) $(JAVAC) $(JFLAGS) $(CLASSPATH) $(ALTIMON_SRCS) -d $(DEST_DIR)

package: compile
	$(Q) echo "package $(ALTIMON_TARBALL)"
	$(Q) $(RM) $(DIST_DIR) bin
	$(Q) mkdir $(DIST_DIR)
	$(Q) mkdir $(DIST_DIR)/conf
	$(Q) mkdir $(DIST_DIR)/lib
	$(Q) mkdir $(DIST_DIR)/action_scripts
	$(Q) mkdir $(DIST_DIR)/action_logs
	$(Q) mkdir bin
	$(Q) $(COPY) *.exe *.txt $(ALTIMON_JAR) $(DIST_DIR)/
	$(Q) $(COPY) conf/log4j.* $(DIST_DIR)/conf
	$(Q) $(COPY) conf/config.xml $(DIST_DIR)/conf/config.xml
	$(Q) $(COPY) conf/Metrics.xml $(DIST_DIR)/conf/Metrics.xml
	$(Q) $(COPY) conf/GroupMetrics.xml $(DIST_DIR)/conf/GroupMetrics.xml
	$(Q) $(COPY) lib/* $(DIST_DIR)/lib/
	$(Q) $(COPY) action_scripts/*.sql $(DIST_DIR)/action_scripts
	$(Q) $(COPY) $(ACTION_SCRIPTS) $(DIST_DIR)/action_scripts
#	$(Q) $(COPY) $(SRC_BIN) bin/$(DEST_BIN)
	$(Q) $(COPY) $(BINS) bin/
	$(Q) tar czf $(ALTIMON_TARBALL) $(DIST_DIR) bin
#	$(Q) $(RM) $(DIST_DIR) bin
	
clean:
	$(RM) $(DEST_DIR) $(ALTIMON_JAR) $(ALTIMON_TARBALL) $(DIST_DIR) bin

