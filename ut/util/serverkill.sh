#!/bin/sh    
# Linux server thread kill all
    HOST=`uname`
    echo "=== remove server thread linux ==="
    if [ $# -eq 0 ]; then
      userid=`whoami`
    else
      userid=$1
    fi
    if [ ${HOST} = "Linux" ];
    then
        ps -ef | grep $userid | grep altibase | awk '{print "kill -9 " $2}' | while read line
        do
        eval $line
        done
    fi
