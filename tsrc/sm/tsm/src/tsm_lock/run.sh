#! /bin/sh

echo "=== LOCK test begin ==="  | tee -a $RESULT_FILE

tsm_lock/tsm_lock | tee -a $RESULT_FILE


echo "=== LOCK test end ==="  | tee -a $RESULT_FILE
