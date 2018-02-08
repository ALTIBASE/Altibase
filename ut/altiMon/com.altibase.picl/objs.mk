# PICL (Platform Information Collection Library) Source Files
# $Id$
#

SRC_DIR := src/com/altibase/picl

PICL_SRCS = \
	$(SRC_DIR)/Cpu.java \
	$(SRC_DIR)/Device.java \
	$(SRC_DIR)/Disk.java \
	$(SRC_DIR)/DiskLoad.java \
	$(SRC_DIR)/Iops.java \
	$(SRC_DIR)/Memory.java \
	$(SRC_DIR)/Picl.java \
	$(SRC_DIR)/PiclLogger.java \
	$(SRC_DIR)/ProcCpu.java \
	$(SRC_DIR)/ProcessFinder.java \
	$(SRC_DIR)/ProcMemory.java \
	$(SRC_DIR)/Swap.java \
	$(SRC_DIR)/LibLoader/LibLoader.java \
	$(SRC_DIR)/LibLoader/OsNotSupportedException.java \
	$(SRC_DIR)/Test.java \

PICL_TEST_SRCS = \
	$(SRC_DIR)/TestAIX53TL9.java \
	$(SRC_DIR)/TestLinux.java \
	$(SRC_DIR)/TestRX6600.java \
	$(SRC_DIR)/TestT5140.java \
	$(SRC_DIR)/TestWindows.java
