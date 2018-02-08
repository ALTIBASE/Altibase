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
 * $Id: utmUser.h$
 **********************************************************************/

#ifndef _O_UTM_USER_H_
#define _O_UTM_USER_H_ 1

void setErrNoExistTBSUser();

SQLRETURN getUserPrivileges( FILE  *aFp,
                             SChar *aUserName,
                             idBool aIsRole );

SQLRETURN getUserQuery_user( FILE  *aUserFp,
                             SChar *a_user_name,
                             SChar *a_passwd );

SQLRETURN getUserQuery( FILE  *aUserFp,
                        SInt  *a_user_cnt,
                        SChar *aUserName,
                        SChar *aPasswd );

SQLRETURN getRoleToUser( FILE  *aFp,
                         SChar *aRoleName );

#endif /* _O_UTM_USER_H_ */
