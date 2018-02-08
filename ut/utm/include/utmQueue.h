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
 * $Id: utmQueue.h $
 **********************************************************************/

#ifndef _O_UTM_QUEUE_H_
#define _O_UTM_QUEUE_H_ 1

SQLRETURN getQueueInfo( SChar *a_user,
                        FILE  *aTblFp,
                        FILE  *aIlOutFp,
                        FILE  *aIlInFp );

SQLRETURN getQueueQuery( SChar *a_user,
                         SChar *a_table,
                         FILE  *a_crt_fp);

#endif /* _O_UTM_QUEUE_H_ */
