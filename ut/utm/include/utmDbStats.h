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
 * $Id: utmDbStats.h $
 **********************************************************************/

#ifndef _O_UTM_STATS_H_
#define _O_UTM_STATS_H_ 1

#define META_COLUMN_SIZE4STATS  48

/* BUG-40174 Support export and import DBMS Stats */
SQLRETURN getSystemStats( FILE  *a_DbStatsFp );

/* BUG-40174 Support export and import DBMS Stats */
SQLRETURN getTableStats( SChar *a_user,
                         SChar *a_table,
                         SChar *a_partition,
                         FILE  *a_DbStatsfp );

/* BUG-40174 Support export and import DBMS Stats */
SQLRETURN getColumnStats( SChar *a_user,
                          SChar *a_table,
                          SChar *a_column,
                          SChar *a_partition,
                          FILE  *a_DbStatsFp );

/* BUG-44831 Exporting INDEX Stats for PK, Unique constraint
 * with system-given name */
/* BUG-40174 Support export and import DBMS Stats */
SQLRETURN getIndexStats( SChar *a_user,
                         SChar *a_table,
                         SChar *a_index,
                         FILE  *a_DbStatsfp,
                         utmIndexStatProcType aIdxStatProcType );

#endif /* _O_UTM_STATS_H_ */
