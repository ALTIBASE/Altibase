#! /bin/sh

echo "=== begin of TSM_SYSTEM SECTION ===" | tee -a $RESULT_FILE
. ./tsmlib.sh


echo "  * begin of stress db creation & destroy [$1][$2] " | tee -a $RESULT_FILE

if [ $EXPORT_TSM_REMOVE_IPC ]; then
    NOTHING=
else
    createDB M 4
    removeIPC
    removeDB mydb

    createDB M 8
    removeIPC
    removeDB mydb
fi


# ==================== for using test db ====================
wipeDB
createDB M 10
checkDBFILE

$EXPORT_TSM_REMOVE_IPC;

echo "  * end of stress db creation & destroy" | tee -a $RESULT_FILE


tsm_system/tsm_create | tee -a $RESULT_FILE


echo "=== end of TSM_SYSTEM SECTION ==="  | tee -a $RESULT_FILE
