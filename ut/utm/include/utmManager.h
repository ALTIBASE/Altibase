/** 
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
/***********************************************************************
 * $Id: utmManager.h 80542 2017-07-19 08:01:20Z daramix $
 **********************************************************************/

#ifndef _O_UTM_MANAGER_H_
#define _O_UTM_MANAGER_H_ 1

#include <utmDef.h>
#include <utmConstr.h>
#include <utmObjPriv.h>
#include <utmUser.h>
#include <utmTbs.h>
#include <utmViewProc.h>
#include <utmPartition.h>
#include <utmObjMode.h>
#include <utmSeqSym.h>
#include <utmQueue.h>
#include <utmReplDbLink.h>
#include <utmTable.h>
#include <utmMisc.h>
#include <utmSQLApi.h>

#if (SIZEOF_LONG == 8) || defined(HAVE_LONG_LONG) || defined(VC_WIN32)
# define SQLBIGINT_EQ_SQLBIGINT(aSQLBIGINT1, aSQLBIGINT2) \
    ((aSQLBIGINT1) == (aSQLBIGINT2))
# define SQLBIGINT_TO_SLONG(aSQLBIGINT) ((SLong)(aSQLBIGINT))
#else
# define SQLBIGINT_EQ_SQLBIGINT(aSQLBIGINT1, aSQLBIGINT2) \
    ((aSQLBIGINT1).hiword == (aSQLBIGINT2).hiword && \
    (aSQLBIGINT1).loword == (aSQLBIGINT2).loword)
# define SQLBIGINT_TO_SLONG(aSQLBIGINT) \
    ((SLong)(aSQLBIGINT).hiword * ID_LONG(0x100000000)\
    + (SLong)(ULong)(aSQLBIGINT).loword)
#endif

#define FORMOUT_SCRIPT ILO_PRODUCT_NAME" %s -u %s -p %s formout -f %s_%s.fmt -T %s\n"
#define INOUT_SCRIPT   ILO_PRODUCT_NAME" %s -u %s -p %s %s -f %s_%s.fmt -d %s_%s.dat"

/* BUG-32114 aexport must support the import/export of partitioned tables. */
#define FORMOUT_PARTITION_SCRIPT \
        ILO_PRODUCT_NAME" %s -u %s -p %s formout -f %s_%s.fmt -T %s -PARTITION\n"
#define INOUT_PARTITION_SCRIPT   \
        ILO_PRODUCT_NAME" %s -u %s -p %s %s -f %s_%s.fmt.%s -d %s_%s.dat.%s"

#if defined( ALTI_CFG_OS_WINDOWS )
#define UTM_FILE_QUOT  "\""
#define UTM_BEGIN_QUOT "\"\\\""
#define UTM_END_QUOT   "\\\"\""
#else
#define UTM_FILE_QUOT  "'"
#define UTM_BEGIN_QUOT "'\""
#define UTM_END_QUOT   "\"'"
#endif

#endif /* _O_UTM_MANAGER_H_ */
