#!/bin/sh

cd $ALTIBASE_HOME/altiMon

#JAVA_HOME=/usr/java/jdk1.5.0_22

JAVA_OPTIONS="-Xss200k -Xms25m -Xmx25m -XX:NewSize=20m"

PID_FILE=altimon.pid

case "$1" in
  start)
    echo "Starting up...";
    if [ -f logs/altimon.log ]; then
        before_line=`wc -l logs/altimon.log | awk {'print $1'}`
    else
        before_line=0
    fi

    GET_PID=`ps -ef|grep altimon.jar|grep -v grep | grep $USER | awk '{print $2}'`

    if [ "$GET_PID" = "" ]; then
      nohup $JAVA_HOME/bin/java $JAVA_OPTIONS -jar altimon.jar 1> /dev/null 2>> altimon_stderr.log &    
      ALTIMON_PID=$!
      echo "$ALTIMON_PID" > "$PID_FILE"
      sleep 3

      if [ -f logs/altimon.log ]; then
        after_line=`wc -l logs/altimon.log | awk {'print $1'}`
      else
        after_line=0
      fi

      total_line=`expr $after_line - $before_line`

      if [ -f logs/altimon.log ]; then
        tail -n $total_line logs/altimon.log
      fi

    else
      echo "Failed to start up. altimon is already up and running."
    fi

    ;;

  stop)
    GET_PID=`ps -ef|grep altimon.jar|grep -v grep | grep $USER | awk '{print $2}'`

    if [ "$GET_PID" = "" ]; then
      echo "altimon is not running."
    else
      ALTIMON_PID=`cat $PID_FILE`
#    if [ "$ALTIMON_PID" == "$GET_PID" ]; then

        echo "Shutting down...";
        if [ -f logs/altimon.log ]; then
            before_line=`wc -l logs/altimon.log | awk {'print $1'}`
        else
            before_line=0
        fi
        kill -s TERM $ALTIMON_PID;
        wait
        rm -f $PID_FILE
        sleep 1

        if [ -f logs/altimon.log ]; then
          after_line=`wc -l logs/altimon.log | awk {'print $1'}`
        else
          after_line=0
        fi

        total_line=`expr $after_line - $before_line`

        if [ -f logs/altimon.log ]; then
          tail -n $total_line logs/altimon.log
        fi

#    else
#      echo "altimon is not running";
#    fi
    fi

    ;;

  *)
    echo "Usage: altimon.sh { start | stop }"
    exit 1;;
esac

