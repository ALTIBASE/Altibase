#!/bin/sh
LC_ALL=C svn info $ALTIBASE_DEV/src/ | grep URL | gawk '{print "#define ALTIBASE_SVN_URL \"" $2 "\""}'
LC_ALL=C svn info $ALTIBASE_DEV/src/ | grep Revision | gawk '{print "#define ALTIBASE_SVN_REVISION " $2}'
