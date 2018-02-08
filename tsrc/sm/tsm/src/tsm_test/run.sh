#! /bin/sh

echo "=== Just test begin ==="  | tee -a $RESULT_FILE

tsm_init/tsm_init
tsm_test/tsm_test >>$RESULT_FILE


echo "=== Just test end ==="  | tee -a $RESULT_FILE
