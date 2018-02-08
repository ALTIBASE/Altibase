#! /bin/sh

echo "=== compression test begin ==="  | tee -a $RESULT_FILE

tsm_compression/tsm_iduCompression | tee -a $RESULT_FILE


echo "=== compression test end ==="  | tee -a $RESULT_FILE
