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
 * $Id: utmConstr.h $
 **********************************************************************/

#ifndef _O_UTM_CONSTR_H_
#define _O_UTM_CONSTR_H_ 1

SQLRETURN getIndexQuery( SChar *a_user,
                         SChar *a_puser,
                         SChar *a_table,
                         FILE  *a_IndexFp,
                         FILE  *a_DbStatsFp
                         );

/* PROJ-1107 Check Constraint Áö¿ø */
SQLRETURN getCheckConstraintQuery( SChar * aUserName,
                                   SChar * aPrevUserName,
                                   SChar * aTableName,
                                   FILE  * aFilePointer );

SQLRETURN getForeignKeys( SChar *a_user,
                          FILE  *aFkFp );

SQLRETURN resultForeignKeys( SChar *a_user,
                             SChar *a_puser,
                             SChar *a_table,
                             FILE  *a_fp );

SQLRETURN getTrigQuery( SChar *a_user,
                        FILE *aTrigFp);

#endif /* _O_UTM_CONSTR_H_ */
