#! /usr/local/bin/bash

export ALTIBASE_SYNC_INTERVAL_SEC=0
export ALTIBASE_SYNC_INTERVAL_MSEC=200
export ALTIBASE_SYNC_CREATE_=1

echo "Sync Test Begin###" > result.txt

while [ $ALTIBASE_SYNC_INTERVAL_USEC-lt1000 ]; do
    echo "Sync Interval is " $ALTIBASE_SYNC_INTERVAL_MSEC >> result.txt
    cleandb
    tsm_init/tsm_init
    sync
    sync
    tsm_sync/tsm_sync >> result.txt
    let "ALTIBASE_SYNC_INTERVAL_MSEC+=200"
done
