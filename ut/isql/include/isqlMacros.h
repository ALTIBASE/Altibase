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
 * $Id$
 **********************************************************************/

#ifndef _O_ISQLMACROS_H_
#define _O_ISQLMACROS_H_ 1

#define ENV_ALTIBASE_DATE_FORMAT ALTIBASE_ENV_PREFIX"DATE_FORMAT"

#define SERVER_BINARY_NAME PRODUCT_PREFIX"altibase"

#define COMMAND_LEN             1024*64
#define MSG_LEN                 2048
#define WORD_LEN                256
#define QP_MAX_NAME_LEN         128
#define TIMEZONE_STRING_LEN     40
// To Fix BUG-17430
#define UT_MAX_NAME_BUFFER_SIZE (QP_MAX_NAME_LEN + 3) // "MaxName"\0
#define QC_MAX_KEY_COLUMN_COUNT 32
// BUG-22162
#define ISQL_INDEX_MAX_COUNT    (64)

/* PROJ-1107 Check Constraint 지원 */
#define UT_MAX_CHECK_CONDITION_LEN  (4000)

/* FLOAT_SIZE는 175바이트면 되나 넉넉하게 잡은 것임. */
#define FLOAT_SIZE              191
#define SMALLINT_SIZE           6
#define INTEGER_SIZE            11
#define BIGINT_SIZE             20
#define DATE_SIZE               64*3
#define FETCH_CNT               10

#define TYPE_UNKNOWN              0
#define TYPE_SYSTEM_TABLE         1
#define TYPE_SYSTEM_VIEW          2  // BUG-33915 system view     
#define TYPE_TABLE                3
#define TYPE_VIEW                 4
#define TYPE_QUEUE                5
#define TYPE_PSM                  6
#define TYPE_SYNONYM              7
#define TYPE_MVIEW                8
#define TYPE_PKG                  9  // BUG-37002
#define TYPE_TEMP_TABLE           10 // BUG-40103

/* BUG-43911 Refactoring */
#define REAL_DISPLAY_SIZE    13
#define DOUBLE_DISPLAY_SIZE  22
#define INT_DISPLAY_SIZE     11
#define BIGINT_DISPLAY_SIZE  20
#define DATE_DISPLAY_SIZE    20
#define CLOB_DISPLAY_SIZE    gProperty.GetLobSize()

#endif /* _O_ISQLMACROS_H_ */
