# Library for TSM (Test of SM)

. ${ALTIBASE_HOME}/conf/tsm.env

setupTsmEnvironment()
{
    echo "=== setupTsmEnvironment() ==="
    export ORIG_FILE
    export RESULT_FILE
    export DIFF_FILE
    export SDIFF_FILE

    rm -f $RESULT_FILE
    rm -f $DIFF_FILE
    rm -f $SDIFF_FILE
    touch $RESULT_FILE

    # result file exist? # 존재하지 않으면 에러!
    if [ ! -f $ORIG_FILE ] ;  then
	echo ""
	echo "result file=>[$ORIG_FILE] is Not Exist!! check your environment." ;
	exit;
    fi
}

removeIPC()
{
    HOST=`uname`
    echo "=== removeIPC() ==="
    if [ $# -eq 0 ]; then
      userid=`whoami`
    else
      userid=$1
    fi
    if [ ${HOST} = "Linux" ];
    then
        ipcs | grep $userid | awk '{print "ipcrm shm " $2}' | while read line
        do
        eval $line
        done
    elif [ ${HOST} = "QNX" ];
    then
        rm -rf /dev/shmem/RtS*
    elif [ ${HOST} = "CYGWIN_NT-5.0" ];
    then
        eval "killwinproc ipc-daemon.exe"
        eval "ipc-daemon &"
    elif [ ${HOST} = "WindowsNT" ];
    then
        eval "tskill ipc-daemon"
		del %altibase_home%\trc\winipc*
        del %altibase_home%\trc\MultiFile*
        eval "ipc-daemon &"

    else
        ipcs | grep $userid | awk '{print "ipcrm -" $1 " " $2}' | while read line
        do
        eval $line
        done
        #eval "ipcrm -M ${TSM_DB_KEY}"
    fi
}

checkDBFileOk()
{
    FILE_PATH=$1
    if [ ! -f $FILE_PATH ] ;  then
	echo ""
	echo "$FILE_PATH does Not Exist. Check createdb utility." | tee -a $RESULT_FILE
	exit;
    fi

    if [ ! -s $FILE_PATH ] ;  then
	echo ""
	echo "The size of $FILE_PATH is Zero. Check createdb utility." | tee -a $RESULT_FILE
	exit;
    fi
}


checkDBFILE()
{
    echo "=== checkDBFILE() ==="
    checkDBFileOk $ALTIBASE_HOME/dbs/SYS_TBS_MEM_DATA-0-0

    checkDBFileOk $ALTIBASE_HOME/dbs/SYS_TBS_MEM_DATA-1-0

    checkDBFileOk $ALTIBASE_HOME/dbs/SYS_TBS_MEM_DIC-0-0

    checkDBFileOk $ALTIBASE_HOME/dbs/SYS_TBS_MEM_DIC-1-0

    checkDBFileOk $ALTIBASE_HOME/dbs/system001.dbf

    checkDBFileOk $ALTIBASE_HOME/dbs/undo001.dbf

    checkDBFileOk $ALTIBASE_HOME/dbs/temp001.dbf
}

createDB() # error logging
{
    echo "=== createDB() ==="
    echo "y" | $ALTIBASE_HOME/bin/sm_createdb -$1 $2 || echo "[ERROR] createdb -$1 $2 " | tee -a $RESULT_FILE
}

removeDB() # error logging
{
    echo "=== removeDB() ==="
    echo "y" | $ALTIBASE_HOME/bin/sm_destroydb -n $1 || echo "[ERROR] destroydb -n $1" | tee -a $RESULT_FILE
    rm -f $ALTIBASE_HOME/dbs/*-*
    rm -f $ALTIBASE_HOME/dbs/*.dbf*
    rm -f $ALTIBASE_HOME/logs/log*
}

destroyDB() # just only destroy for initialize
{
    echo "=== destroyDB() ==="
    echo "y" | $ALTIBASE_HOME/bin/sm_destroydb -n $1 
    rm -f $ALTIBASE_HOME/dbs/*-*
    rm -f $ALTIBASE_HOME/dbs/*.dbf*
    rm -f $ALTIBASE_HOME/logs/log*
}

wipeDB() # just only destroy for initialize
{
    echo "=== wipeDB() ==="
    removeIPC
    rm -f $ALTIBASE_HOME/dbs/*-*
    rm -f $ALTIBASE_HOME/dbs/*.dbf*
    rm -f $ALTIBASE_HOME/logs/log*
}

setDBKey()
{
    echo "=== setDBKey() ==="
    ALTIBASE_SHM_DB_KEY=$1
    export ALTIBASE_SHM_DB_KEY
}
