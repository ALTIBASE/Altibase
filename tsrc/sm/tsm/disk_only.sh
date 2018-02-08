#!/bin/sh
# $Id: disk_only.sh 48196 2011-07-25 05:41:12Z a464 $

. ${ALTIBASE_HOME}/conf/tsm.env
. ./tsmlib.sh

# -----------------------------------------------------------
#  0. Testing Scenario Group
#  시나리오 추가시 EXPORT_TSM_REMOVE_IPC 를 앞에 추가할 것!
#                  ==> shared create의 경우 shm 삭제하는 것임
# -----------------------------------------------------------
doIt()
{
    $EXPORT_TSM_REMOVE_IPC; sh ./tsm_system/run.sh

	$EXPORT_TSM_REMOVE_IPC; sh ./tsm_trans/run.sh

	$EXPORT_TSM_REMOVE_IPC; sh ./tsm_lock/run.sh

	$EXPORT_TSM_REMOVE_IPC; sh ./tsm_mixed/run.sh

    $EXPORT_TSM_REMOVE_IPC; sh ./tsm_compression/run.sh
}

# -----------------
# For Durability-1
# -----------------
doIt1()
{
    $EXPORT_TSM_REMOVE_IPC; sh ./tsm_system/run1.sh

    $EXPORT_TSM_REMOVE_IPC; sh ./tsm_trans/run.sh

    $EXPORT_TSM_REMOVE_IPC; sh ./tsm_lock/run.sh

    $EXPORT_TSM_REMOVE_IPC; sh ./tsm_mixed/run1.sh

    $EXPORT_TSM_REMOVE_IPC; sh ./tsm_compression/run.sh
}

HOST=`uname`
PWD=`pwd`
if [ ${HOST} = "CYGWIN_NT-5.0" ];
then
    eval "make killwinproc"
    eval "export PATH=$PATH;$PWD"
fi



# -------------------------------------
#  3-4. Run Testing (DISK DB)
# -------------------------------------
ORIG_FILE=disk_result.tsm
TSM_INDEX_TYPE="BTREE"; export TSM_INDEX_TYPE
RESULT_FILE=disk_btree_result.now
DIFF_FILE=disk_btree_result.diff
SDIFF_FILE=disk_btree_result.sdiff
setupTsmEnvironment
TSM_TABLE_TYPE="DISK"; export TSM_TABLE_TYPE
wipeDB
setDBKey 0
echo "[Start TSM Section : DISK DB]" | tee -a $RESULT_FILE
doIt
echo "[END TSM Section]" | tee -a $RESULT_FILE


# =====================
# [4-2] Check Result- disk.
# =====================
ORIG_FILE=disk_result.tsm
RESULT_FILE=disk_btree_result.now
DIFF_FILE=disk_btree_result.diff
SDIFF_FILE=disk_btree_result.sdiff

rm   -f $DIFF_FILE $SDIFF_FILE 
diff $ORIG_FILE $RESULT_FILE >$DIFF_FILE 

if [ -s $DIFF_FILE ] ; then
    sdiff $ORIG_FILE $RESULT_FILE >$SDIFF_FILE 
    echo "TSM DISK RESULT IS FAILED!!! examine [$DIFF_FILE and $SDIFF_FILE] file";
else
    echo "TSM DISK RESULT IS ** SUCCESS **"
fi
