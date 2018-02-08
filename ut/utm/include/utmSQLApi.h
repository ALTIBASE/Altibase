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
 * $Id: utmSQLApi.h $
 **********************************************************************/

#ifndef _O_UTM_SQLAPI_H_
#define _O_UTM_SQLAPI_H_ 1

SQLRETURN init_handle();

SQLRETURN fini_handle();

SQLRETURN Prepare(SChar *aQuery, SQLHSTMT aStmt);

SQLRETURN Execute(SQLHSTMT aStmt);

// BUG-33995 aexport have wrong free handle code
void FreeStmt( SQLHSTMT *aStmt );

SQLRETURN setQueryTimeout( SInt aTime );
IDE_RC AppendConnStrAttr( SChar *aConnStr, UInt aConnStrSize, SChar *aAttrKey, SChar *aAttrVal );

#endif /* _O_UTM_SQLAPI_H_ */
