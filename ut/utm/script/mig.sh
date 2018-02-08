#!/bin/sh

#-----
# User Define
#-----
SERVER_IP='localhost'
FTERM='!*@&$^%'
RTERM='%^$&@*!'

# For DEBUG
echo ${SERVER_IP}
echo ${FTERM}
echo ${RTERM}

#-----
# Env check
#-----

if [ "x$ALTIBASE_HOME"  = "x" ]; then
        echo "ALTIBASE_HOME is not defined. please Check your environments"
        exit
elif [ ! -d $ALTIBASE_HOME ]; then
        echo "ALTIBASE_HOME is not a directory"
        exit
else
        echo "ALTIBASE_HOME = $ALTIBASE_HOME"
fi

#-----
# Argument Check
#-----

usage() {
        echo "mig.sh [username/passwd] {out|in}"
        exit 0
}

if [ $# -eq 1 ]; then
        ARGV1=
        ARGV2=$1
elif [ $# -eq 2 ]; then
        ARGV1=$1
        ARGV2=$2
else
        usage
fi

if [ "x${ARGV1}" = "x" ]; then
        USER=SYS
        PASSWD=MANAGER
else
        if [ `expr ${ARGV1} : '.*/'` -eq 0 ]; then
                usage
        fi

        USER=`echo ${ARGV1} | cut -d'/' -f1`
        PASSWD=`echo ${ARGV1} | cut -d'/' -f2`
fi

ILOADER="${ALTIBASE_HOME}/bin/iloader -S ${SERVER_IP} -U ${USER} -P ${PASSWD}"
ISQL="${ALTIBASE_HOME}/bin/isql -S ${SERVER_IP} -U SYS -P MANAGER -silent"
ISQLU="${ALTIBASE_HOME}/bin/isql -S ${SERVER_IP} -U ${USER} -P ${PASSWD} -silent"

# For DEBUG
#echo ${ISQL}
#echo ${ILOADER}

#-----
# Functions
#-----

getUserID() {

        TMPBUF=`${ISQL} << EOF
set heading off;
select userid from qpm_users_ where username = upper('${USER}');
exit
EOF
`

if [ `expr "${TMPBUF}" : '.*selected row count \[0\]'` -gt 0 ]; then
        echo "Invalid Username : ${USER}"
        exit
fi

USERID=`echo ${TMPBUF} | sed 's/iSQL>//g' | sed 's/selected.*$//' | sed 's/  *//g'`

# For DEBUG
#echo ${USERID}

}

getTableName() {

        TMPBUF=`${ISQL} << EOF
set heading off;
spool mig_list.tmp
select tablename from qpm_tables_ where userid = ${USERID};
spool off
exit
EOF
`

        if [ -f ${USER}_mig_list.txt ]; then
                rm ${USER}_mig_list.txt
        fi

        while
                read LINEBUF
        do
                if [ `expr "${LINEBUF}" : '.*iSQL'` -gt 0 ]; then
                        continue
                elif [ `expr "${LINEBUF}" : '.*selected'` -gt 0 ]; then
                        continue
                elif [ `expr "${LINEBUF}" : '.*QPM_'` -gt 0 ]; then
                        continue
                elif [ "x${LINEBUF}" = "x" ]; then
                        continue
                else
                        echo ${LINEBUF}
                        echo ${LINEBUF} >> ${USER}_mig_list.txt
                fi
        done < mig_list.tmp

        rm mig_list.tmp

        if [ ! -s ${USER}_mig_list.txt ]; then
                echo "There is no Table to Migration"
                exit
        fi

}

makeTableDesc() {
        while
                read TNAME
        do
                TMPBUF=`$ISQLU << EOF
spool ${USER}_${TNAME}.desc
desc ${TNAME}
spool off
quit
EOF
`
        done < ${USER}_mig_list.txt
}

makeTableScript() {
        echo ""
        echo ">>>> Make Table Create Script ..."
        echo ""

        while
                read TNAME
        do
                POS=NONE

                echo "connect ${USER}/${PASSWD};" > ${USER}_${TNAME}.sql
                while
                        read LINEBUF
                do
                        if [ `expr "${LINEBUF}" : '.*ATTRIBUTE'` -gt 0 ]; then
                                POS=ATTR
                                echo "CREATE TABLE ${TNAME} (" >> ${USER}_${TNAME}.sql
                        elif [ `expr "${LINEBUF}" : '.*PRIMARY'` -gt 0 ]; then
                                POS=PRI
                                echo "PRIMARY KEY (" >> ${USER}_${TNAME}.sql
                        elif [ `expr "${LINEBUF}" : '.*INDEX'` -gt 0 ]; then
                                POS=IDX
                                echo ")" >> ${USER}_${TNAME}.sql
                                echo ");" >> ${USER}_${TNAME}.sql
                        elif [ `expr "${LINEBUF}" : '.*not have index'` -gt 0 ]; then
                                POS=END
                                TMPBUF=`ed ${USER}_${TNAME}.sql << EOF
$
s/,$//g
w
q
EOF
`
                                echo ");" >> ${USER}_${TNAME}.sql
                        else
                                if [ `expr "${LINEBUF}" : '.*iSQL'` -gt 0 ]; then
                                        continue
                                elif [ `expr "${LINEBUF}" : '-----'` -gt 0 ]; then
                                        continue
                                elif [ `expr "${LINEBUF}" : '.*NAME.*TYPE.*IS NULL'` -gt 0 ]; then
                                        continue
                                elif [ `expr "${LINEBUF}" : '.*NAME.*IS UNIQUE.*COLUMN'` -gt 0 ]; then
                                        continue
                                elif [ "x${LINEBUF}" = "x" ]; then
                                        continue
                                else
                                        if [ "x${POS}" = "xATTR" ]; then
                                                echo ${LINEBUF}, >> ${USER}_${TNAME}.sql
                                        elif [ "x${POS}" = "xPRI" ]; then
                                                echo ${LINEBUF} >> ${USER}_${TNAME}.sql
                                        elif [ "x${POS}" = "xIDX" ]; then
                                                NAME=`echo ${LINEBUF} | cut -f1 -d' '`
                                                if [ `expr ${NAME} : '.*SYS'` -gt 0 ]; then
                                                        NAME=
                                                fi
                                                TYPE=`echo ${LINEBUF} | cut -f2 -d' '` >> ${USER}_${TNAME}.sql
                                                COLS=`echo ${LINEBUF} | cut -f3- -d' '` >> ${USER}_${TNAME}.sql
                                                if [ "x${NAME}" = "x" ]; then
                                                        COLS=`echo ${COLS} | sed 's/ASC//g' | sed 's/DESC//g'`
                                                        echo "ALTER TABLE ${TNAME} ADD ${TYPE}(${COLS});" >> ${USER}_${TNAME}.sql
                                                else
                                                        echo "CREATE ${TYPE} INDEX ${NAME} ON ${TNAME}(${COLS});" >> ${USER}_${TNAME}.sql
                                                fi
                                        elif [ "x${POS}" = "xEND" ]; then
                                                                                            echo ");" >> ${USER}_${TNAME}.sql
                                        fi
                                fi
                        fi
                done < ${USER}_${TNAME}.desc
        done < ${USER}_mig_list.txt

        echo "!!! Done !!!"

}

makeFormFile() {
        echo ""
        echo ">>>> Make formfile ..."
        echo ""

        if [ ! -r ${USER}_mig_list.txt ]; then
                echo "${USER}_mig_list.txt file does not exist or not readable"
                exit
        fi

        while
                read TNAME
        do
                TMPBUF=`${ILOADER} << EOF
formout -f ${USER}_${TNAME}.fmt -T ${TNAME}
quit
EOF
`

        echo "dateform YYYY/MM/DD HH:MI:SS:SSSSSS" >> ${USER}_${TNAME}.fmt

        done < ${USER}_mig_list.txt

        echo "!!! Done !!!"

}

dataImport () {
        echo ""
        echo ">>>> Start Import ..."
        echo ""

        if [ ! -r ${USER}_mig_list.txt ]; then
                echo "${USER}_mig_list.txt file does not exist or not readable"
                exit
        fi

        while
                read TNAME
        do
                ${ILOADER} << EOF
in -f ${USER}_${TNAME}.fmt -d ${USER}_${TNAME}.dat -t ${FTERM} -r ${RTERM} -log ${USER}_${TNAME}.ilog -bad ${USER}_${TNAME}.ibad
quit
EOF
        done < ${USER}_mig_list.txt

        echo "!!! Done !!!"

}

dataExport () {
        echo ""
        echo ">>>> Start Export ..."
        echo ""

        if [ ! -r ${USER}_mig_list.txt ]; then
                echo "${USER}_mig_list.txt file does not exist or not readable"
                exit
        fi

        while
                read TNAME
        do
                ${ILOADER} << EOF
out -f ${USER}_${TNAME}.fmt -d ${USER}_${TNAME}.dat -t ${FTERM} -r ${RTERM} -log ${USER}_${TNAME}.olog -bad ${USER}_${TNAME}.obad
quit
EOF
        done < ${USER}_mig_list.txt

        echo "!!! Done !!!"

}

#-----
# Main
#-----

case ${ARGV2} in 
        in)
                #makeTables
                dataImport
        ;;
        out)
                getUserID
                getTableName
                makeTableDesc
                makeTableScript
                makeFormFile
                dataExport
        ;;
        *)
                usage
        ;;
esac

