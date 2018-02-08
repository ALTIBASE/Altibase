#! /bin/sh

echo "=== MIXED test begin ==="  | tee -a $RESULT_FILE


# Normal Test
tsm_mixed/tsm_mixed -s tsm_cursor  | tee -a $RESULT_FILE

# refineDB test1
#$EXPORT_TSM_REMOVE_IPC;
tsm_mixed/tsm_mixed -s tsm_insert_commit2 | tee -a $RESULT_FILE

#refineDB test2
#$EXPORT_TSM_REMOVE_IPC;
tsm_mixed/tsm_mixed -s tsm_insert_rollback2 | tee -a $RESULT_FILE

# refineDB test3
#$EXPORT_TSM_REMOVE_IPC;
tsm_mixed/tsm_mixed -s tsm_update_commit2 | tee -a $RESULT_FILE

# refineDB test4
#$EXPORT_TSM_REMOVE_IPC;
tsm_mixed/tsm_mixed -s tsm_update_rollback2 | tee -a $RESULT_FILE

# refineDB test5
#$EXPORT_TSM_REMOVE_IPC;
tsm_mixed/tsm_mixed -s tsm_delete_commit_2 | tee -a $RESULT_FILE

# refineDB test6
#$EXPORT_TSM_REMOVE_IPC;
tsm_mixed/tsm_mixed -s tsm_delete_rollback_2 | tee -a $RESULT_FILE

# Index Rebuild Test
#$EXPORT_TSM_REMOVE_IPC;
tsm_mixed/tsm_mixed -s tsm_rebuild_index_stage2 | tee -a $RESULT_FILE

# Sequence Test
#$EXPORT_TSM_REMOVE_IPC;
tsm_mixed/tsm_mixed -s tsm_sequence2 | tee -a $RESULT_FILE

# Global Transaction Test
#$EXPORT_TSM_REMOVE_IPC;
tsm_mixed/tsm_mixed -s tsm_xa | tee -a $RESULT_FILE

# Global Transaction Test
#$EXPORT_TSM_REMOVE_IPC;
tsm_mixed/tsm_mixed -s tsm_xa2 | tee -a $RESULT_FILE

# Global Transaction Test
#$EXPORT_TSM_REMOVE_IPC;
#tsm_mixed/tsm_mixed -s tsm_xa_multi2 | tee -a $RESULT_FILE

# Sequence Test
#$EXPORT_TSM_REMOVE_IPC;
#tsm_mixed/tsm_mixed -s tsm_sequence1 | tee -a $RESULT_FILE

#Recovery Test : don't have to erase. because of strict recovery testing.
#wipeDB
#createDB s 100

#Recovery Test
$EXPORT_TSM_REMOVE_IPC;
tsm_mixed/tsm_mixed -s tsm_recovery_normal   | tee -a $RESULT_FILE
$EXPORT_TSM_REMOVE_IPC;
tsm_mixed/tsm_mixed -s tsm_recovery_makeState   | tee -a $RESULT_FILE
$EXPORT_TSM_REMOVE_IPC;
tsm_mixed/tsm_mixed -s tsm_recovery_makeSure   | tee -a $RESULT_FILE

echo "=== MIXED test end ==="  | tee -a $RESULT_FILE




