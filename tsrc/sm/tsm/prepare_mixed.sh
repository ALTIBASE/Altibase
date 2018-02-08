. ${ALTIBASE_HOME}/conf/tsm.env

. ./tsmlib.sh

export ORIG_FILE=disk_result.tsm
export TSM_INDEX_TYPE="BTREE"
export RESULT_FILE=disk_btree_result.now
export DIFF_FILE=disk_btree_result.diff
export SDIFF_FILE=disk_btree_result.sdiff
setupTsmEnvironment
export TSM_TABLE_TYPE="DISK"
wipeDB
setDBKey 0

echo "y" | sm_createdb -M 10

rm tsm_meta_info.txt

# init
tsm_init/tsm_init

# Now you can run.. with ddd
#tsm_mixed/tsm_mixed ..
