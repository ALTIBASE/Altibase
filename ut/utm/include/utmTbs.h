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
 * $Id: utmTbs.h $
 **********************************************************************/

#ifndef _O_UTM_TBS_H_
#define _O_UTM_TBS_H_ 1

SQLRETURN getTBSQuery( FILE *aTbsFp );

SQLRETURN getMemTBSFileQuery( SChar *a_file_query,
                              SInt   a_tbs_id);

SQLRETURN getTBSFileQuery( SChar *a_file_query,
                           SInt   a_tbs_id);

SQLRETURN getTBSQueryVolatile( SChar *a_file_query,
                               SInt   a_tbs_id);

SQLRETURN getTBSFileQuery2( SChar *a_file_query,
                            SInt   a_tbs_id);

/* BUG-40469 user mode에서 사용자를 위한 tablespace 생성문 필요 */
SQLRETURN getTBSInfo4UserMode( FILE *aTbsFp,
                               SInt  aTbsId,
                               SInt  aTbsType );

#endif /* _O_UTM_TBS_H_ */
