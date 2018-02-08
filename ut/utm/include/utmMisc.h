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
 * $Id: utmMisc.h $
 **********************************************************************/

#ifndef _O_UTM_MISC_H_
#define _O_UTM_MISC_H_ 1

IDE_RC utmSetErrorMsgAfterAllocEnv();
IDE_RC utmSetErrorMsgWithHandle(SQLSMALLINT aHandleType,
                                       SQLHANDLE   aHandle);
void printError();
void eraseWhiteSpace(SChar * buf);
void replace( SChar *aSrc );

SQLRETURN getPasswd(SChar *a_user, SChar *a_passwd);

SQLRETURN open_file(SChar *a_file_name,
                    FILE **a_fp);

SQLRETURN close_file(FILE *a_fp);

SQLRETURN init_handle();

SQLRETURN fini_handle();

void freeStmt(SQLHSTMT* aStmt);

SQLRETURN setQueryTimeout( SInt aTime );

void addSingleQuote( SChar * aSrc, SInt aSize, SChar * aDest, SInt aDestSize );

#endif /* _O_UTM_MISC_H_ */
