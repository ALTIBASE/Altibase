#!/bin/sh

##############################
# Macro Define
##############################

TODAY=`date "+%y%m%d_%H%M%S"`

SYSTEM=`uname`
WHOAMI=`whoami`
PWD=`pwd`
INIT_DIR=`echo $PWD`
HOME=`echo $HOME`
SH=`echo $SHELL | awk '{print "basename ", $1}' | sh`
KMTUNE=/usr/sbin/kmtune

if [ $SYSTEM = "HP-UX" ]
then
	type=`uname -m`
	if [ $type = "ia64" ]
	then
		SYSTEM="HP-RX"
	fi

    GZIP=/usr/contrib/bin/gzip
else
    GZIP=gzip
fi

get_string ()
{
	if [ $SH = "ksh" ]
	then
		echo "$STRING"\\c
	else
		echo -n "$STRING"
	fi
}

echo
echo "Altibase Patch (1.0) Start!!"

if [ ! -d ./APatch ]
then
	echo
    echo "  ==> No Patch directory : $ALTIBASE_HOME/APatch"
	echo
else
	echo
    echo "  ==> Found $ALTIBASE_HOME/APatch !!"
	ls -l ./APatch/
	echo
fi

STRING="Please specify Altibase Patch File.( q for quit) "
get_string
read patch_file

if [ x$patch_file = "x" ] || [ $patch_file = "q" ]
then
	echo
	echo "  ==> Patch Altibase Canceled. "
	echo
    exit
fi

if [ -f $patch_file ]
then
	echo "  ==> $patch file found!"
else
	echo "  ==> No $patch_file file found!! Please check file."
    exit;
fi

STRING="Ready to patch? (yes) "
get_string 
read yesorno

if [ x$yesorno = "x" ] || [ ! $yesorno = "yes" ]
then
	echo "  ==> Patch Altibase Canceled. "
    exit
fi

$GZIP -cd $patch_file | \tar xvf - 

