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
 * $Id: utmViewProc.h $
 **********************************************************************/

#ifndef _O_UTM_VIEW_PROC_H_
#define _O_UTM_VIEW_PROC_H_ 1

SQLRETURN getViewProcQuery( SChar * aUserName,
                            FILE  * aViewProcFp,
                            FILE  * aRefreshMViewFp );

SQLRETURN dropTempTblQuery();

SQLRETURN resultTopViewProcQuery( SChar * aUserName,
                                  FILE  * aViewProcFp,
                                  FILE  * aRefreshMViewFp );

SQLRETURN resultViewProcQuery( FILE * aViewProcFp,
                               FILE * aRefreshMViewFp,
                               SInt   aObjId );

#endif /* _O_UTM_VIEW_PROC_H_ */
