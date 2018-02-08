#!/bin/bash
export VERSION_FILE=./src/id/include/iduVersionDef.h
rm $VERSION_FILE
export LANG=C; svn up $VERSION_FILE
export COM_REVISION=`svn info  | grep "Last Changed Rev" | awk '{print $4}'`
echo $COM_REVISION
find $VERSION_FILE | xargs perl -pi -e "s/(STRING.*)\"/\$1-r${COM_REVISION}\"/g"
