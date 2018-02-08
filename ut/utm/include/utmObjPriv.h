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
 * $Id: utmObjPriv.h $
 **********************************************************************/

#ifndef _O_UTM_OBJ_PRIV_H_
#define _O_UTM_OBJ_PRIV_H_ 1

SQLRETURN getObjPrivQuery( FILE  *aFp,
                           SInt   aObjType, 
                           SChar *aUserName,
                           SChar *aObjName );


SQLRETURN searchObjPrivQuery( FILE  *aFp,
                              SInt   aObjType, 
                              SInt   aUserId, 
                              SInt   aObjId );

SQLRETURN checkObjPrivQuery( FILE  *aFp,
                              SInt   aObjType, 
                              SInt   aUserId, 
                              SInt   aObjId,
                             SInt   aPrivId) ;

SQLRETURN relateObjPrivQuery( FILE  *aFp,
                              SInt   aObjType, 
                              SInt   aUserId, 
                              SInt   aObjId,
                              SInt   aPrivId );

SQLRETURN recCycleObjPrivQuery( FILE *aFp,
                           SInt   aObjType, 
                           SInt   aUserId, 
                           SInt   aObjId, 
                                SInt   aPrivId);

SQLRETURN resultObjPrivQuery( FILE *aFp,
                              SInt  aGrantorId,
                              SInt  aGranteeId, 
                              SInt  aUserId,
                              SInt  aPrivId,
                              SInt  aObjId, 
                              SInt  aObjType );

#endif /* _O_UTM_OBJ_PRIV_H_ */
