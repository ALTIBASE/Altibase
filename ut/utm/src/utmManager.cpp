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
 * $Id: utmManager.cpp 80542 2017-07-19 08:01:20Z daramix $
 **********************************************************************/
#include <idl.h>
#include <sqlcli.h>
#include <utmDef.h>

SQLHENV m_henv = SQL_NULL_HENV;
SQLHDBC m_hdbc = SQL_NULL_HDBC;
SChar   m_revokeStr[1024*1024];         //BUG-22769, BUG-36721
SChar   m_alterStr[1024*1024];          /* PROJ-2359 Table/Partition Access Option */
SChar   m_collectDbStatsStr[1024*1024];   // BUG-40174
