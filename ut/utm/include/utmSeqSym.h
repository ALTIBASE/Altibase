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
 * $Id: utmSeqSym.h $
 **********************************************************************/

#ifndef _O_UTM_SYNONYM_H_
#define _O_UTM_SYNONYM_H_ 1

SQLRETURN getSeqQuery( SChar *a_user,
                       SChar *aObjName,
                       FILE  *aSeqFp );

SQLRETURN getSynonymUser( SChar *aUserName,
                          FILE  *aSynFp);

SQLRETURN getSynonymAll( FILE *aSynFp );

SQLRETURN getDirectoryAll( FILE *aDirFp );

/* BUG-36367 aexport must consider new objects, 'package' and 'library' */
SQLRETURN getLibQuery( FILE  *aLibFp,
                       SChar *aUserName,
                       SChar *aObjName);

/* PROJ-1438 Job Scheduler */
SQLRETURN getJobQuery( FILE * aJobFp );

#endif /* _O_UTM_SYNONYM_H_ */
