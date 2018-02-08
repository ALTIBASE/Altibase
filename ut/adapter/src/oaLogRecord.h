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
 
/*
 * Copyright 2011, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 */

#ifndef __OA_LOG_RECORD_H__
#define __OA_LOG_RECORD_H__

#define OA_SN_NULL (ACP_UINT64_MAX)

typedef acp_uint64_t oaLogSN;
/*
 *
 */
typedef enum oaLogRecordType {

    OA_LOG_RECORD_TYPE_UNKNOWN = 0,

    OA_LOG_RECORD_TYPE_COMMIT,
    OA_LOG_RECORD_TYPE_INSERT,
    OA_LOG_RECORD_TYPE_UPDATE,
    OA_LOG_RECORD_TYPE_DELETE,

    OA_LOG_RECORD_TYPE_KEEP_ALIVE,
    OA_LOG_RECORD_TYPE_STOP_REPLICATION,
    OA_LOG_RECORD_TYPE_CHANGE_META

} oaLogRecordType;

/*
 *
 */
typedef enum oaLogRecordValueType {

    OA_LOG_RECORD_VALUE_TYPE_UNKNOWN = 0,

    OA_LOG_RECORD_VALUE_TYPE_NUMERIC,
    OA_LOG_RECORD_VALUE_TYPE_FLOAT,
    OA_LOG_RECORD_VALUE_TYPE_DOUBLE,
    OA_LOG_RECORD_VALUE_TYPE_REAL,
    OA_LOG_RECORD_VALUE_TYPE_BIGINT,
    OA_LOG_RECORD_VALUE_TYPE_INTEGER,
    OA_LOG_RECORD_VALUE_TYPE_SMALLINT,

    OA_LOG_RECORD_VALUE_TYPE_CHAR,
    OA_LOG_RECORD_VALUE_TYPE_VARCHAR,
    OA_LOG_RECORD_VALUE_TYPE_NCHAR,
    OA_LOG_RECORD_VALUE_TYPE_NVARCHAR,

    OA_LOG_RECORD_VALUE_TYPE_DATE

} oaLogRecordValueType;

/*
 *
 */
typedef struct oaLogRecordColumn {
    
    acp_char_t *mName;

    oaLogRecordValueType mType;

    acp_sint32_t  mMaxLength;   /* Match for sb4 */
    acp_uint16_t *mLength;      /* Match for ub2 array */
    acp_char_t   *mValue;
    acp_bool_t    mIsHidden;    /* Is Hidden Column */
} oaLogRecordColumn;

/*
 *
 */
typedef struct oaLogRecordInsert 
{
    oaLogRecordType mType;

    oaLogSN mSN;

    acp_char_t *mTableName;

    acp_sint32_t mTableId;

    acp_char_t  *mToUser;

    acp_sint32_t mColumnCount;

    oaLogRecordColumn *mColumn;

    acp_uint64_t mArrayDMLCount;

} oaLogRecordInsert;

/*
 *
 */
typedef struct oaLogRecordUpdate 
{
    oaLogRecordType mType;

    oaLogSN mSN;

    acp_char_t *mTableName;

    acp_sint32_t mTableId;

    acp_char_t  *mToUser;

    acp_sint32_t mPrimaryKeyCount;

    oaLogRecordColumn *mPrimaryKey;

    acp_sint32_t mColumnCount;

    acp_sint32_t mInitializedColumnCount;

    oaLogRecordColumn *mColumn;
    acp_uint32_t      *mColumnIDMap;

    acp_uint64_t mArrayDMLCount;
} oaLogRecordUpdate;

/*
 *
 */
typedef struct oaLogRecordDelete 
{
    oaLogRecordType mType;

    oaLogSN mSN;

    acp_char_t *mTableName;

    acp_sint32_t mTableId;

    acp_char_t  *mToUser;

    acp_sint32_t mPrimaryKeyCount;

    oaLogRecordColumn *mPrimaryKey;

    acp_uint64_t mArrayDMLCount;

} oaLogRecordDelete;

/*
 *
 */
typedef struct oaLogRecordCommon
{
    oaLogRecordType mType;

    oaLogSN mSN;
} oaLogRecordCommon;
/*
 *
 */
/*
    Common Log : COMMIT, KEEP_ALIVE, REPL_STOP, CHANGE_META
    공통인 멤버  mType 과 mSN 을 검사할때도 mCommon 을 이용하여 접근한다.
*/
typedef union oaLogRecord 
{
    oaLogRecordCommon mCommon;
    oaLogRecordInsert mInsert;
    oaLogRecordUpdate mUpdate;
    oaLogRecordDelete mDelete;
} oaLogRecord;

extern void oaLogRecordDumpType(oaLogRecord *aLogRecord);
extern void oaLogRecordDumpDML( oaLogRecord  * aLogRecord,
                                acp_uint32_t   aArrayDMLIndex );

extern acp_bool_t oaLogRecordColumnIsHiddenColumn( oaLogRecordColumn * aColumn );

#endif /* __OA_LOG_RECORD_H__ */
