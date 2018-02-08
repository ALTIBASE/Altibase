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

#include <oaContext.h>
#include <oaLog.h>
#include <oaMsg.h>

#include <oaLogRecord.h>

static void dumpCharacterValue( oaLogRecordColumn * aLogRecordColumn,
                                acp_uint32_t        aArrayDMLIndex )
{
    acp_sint32_t i = 0;
    acp_sint32_t sPosition;

    ACP_STR_DECLARE_DYNAMIC(sString);

    ACP_STR_INIT_DYNAMIC(sString, 4096, 4096);

    oaLogMessage(OAM_MSG_DUMP_LOG, "\t\tColumn value");
    oaLogMessage(OAM_MSG_DUMP_LOG, "\t\t-------------------");

    acpStrCatCString(&sString, "\t\t'");
    for (i = 0; i < (acp_sint32_t)aLogRecordColumn->mLength[aArrayDMLIndex]; i++)
    {
        sPosition = ((acp_sint32_t)aArrayDMLIndex * aLogRecordColumn->mMaxLength) + i;
        acpStrCatFormat(&sString, "%c", aLogRecordColumn->mValue[sPosition]);
    }
    acpStrCatCString(&sString, "'");
    oaLogMessage(OAM_MSG_DUMP_LOG, acpStrGetBuffer(&sString));

    oaLogMessage(OAM_MSG_DUMP_LOG, "\t\t-------------------");
    acpStrCpyFormat(&sString,
                    "\t\tColumn value length %hu", aLogRecordColumn->mLength[aArrayDMLIndex]);
    oaLogMessage(OAM_MSG_DUMP_LOG, acpStrGetBuffer(&sString));

    ACP_STR_FINAL(sString);
}

static void dumpNCharacterValue( oaLogRecordColumn * aLogRecordColumn,
                                 acp_uint32_t        aArrayDMLIndex )
{
    acp_sint32_t i = 0;
    acp_sint32_t sPosition;

    ACP_STR_DECLARE_DYNAMIC( sString );

    ACP_STR_INIT_DYNAMIC( sString, 4096, 4096 );

    oaLogMessage( OAM_MSG_DUMP_LOG, "\t\tColumn value" );
    oaLogMessage( OAM_MSG_DUMP_LOG, "\t\t-------------------" );

    oaLogMessage( OAM_MSG_DUMP_LOG, "\t\tnational character(varying) decimal value per byte:" );
    acpStrCatCString( &sString, "\t\t'" );

    if ( aLogRecordColumn->mLength[aArrayDMLIndex] > 0 )
    {
        for ( i = 0; i < (acp_sint32_t)aLogRecordColumn->mLength[aArrayDMLIndex]-1; i++ )
        {
            sPosition = ( (acp_sint32_t)aArrayDMLIndex * aLogRecordColumn->mMaxLength ) + i;
            acpStrCatFormat( &sString, "%hhu,", (acp_uint8_t)aLogRecordColumn->mValue[sPosition] );
        }
        sPosition = ( (acp_sint32_t)aArrayDMLIndex * aLogRecordColumn->mMaxLength ) + i;
        acpStrCatFormat( &sString, "%hhu'", (acp_uint8_t)aLogRecordColumn->mValue[sPosition] );
    }
    else
    {
        acpStrCatFormat( &sString, "'");
    }

    oaLogMessage( OAM_MSG_DUMP_LOG, acpStrGetBuffer( &sString ) );

    oaLogMessage( OAM_MSG_DUMP_LOG, "\t\t-------------------" );
    acpStrCpyFormat( &sString,
                     "\t\tColumn value length %hu", aLogRecordColumn->mLength[aArrayDMLIndex] );
    oaLogMessage( OAM_MSG_DUMP_LOG, acpStrGetBuffer( &sString ) );

    ACP_STR_FINAL( sString );

    return;
}

static void dumpLogRecordColumn( oaLogRecordColumn * aLogRecordColumn,
                                 acp_uint32_t        aArrayDMLIndex )
{
    acp_sint32_t sPosition;
    acp_bool_t   sIsHiddenColumn = ACP_FALSE;
    ACP_STR_DECLARE_DYNAMIC(sString);

    ACP_STR_INIT_DYNAMIC(sString, 128, 128);

    acpStrCpyFormat(&sString, "\t\tColumn name %s", aLogRecordColumn->mName);
    oaLogMessage(OAM_MSG_DUMP_LOG, acpStrGetBuffer(&sString));

    sIsHiddenColumn = oaLogRecordColumnIsHiddenColumn( aLogRecordColumn );

    acpStrCpyFormat( &sString,
                     "\t\tHidden Column %s",
                     ( sIsHiddenColumn == ACP_TRUE )? "YES" : "NO" );
    oaLogMessage(OAM_MSG_DUMP_LOG, acpStrGetBuffer(&sString));

    if ( aLogRecordColumn->mLength[aArrayDMLIndex] != 0 )
    {
        sPosition = aLogRecordColumn->mMaxLength * aArrayDMLIndex;

        switch (aLogRecordColumn->mType)
        {
            case OA_LOG_RECORD_VALUE_TYPE_UNKNOWN:
                break;

            case OA_LOG_RECORD_VALUE_TYPE_NUMERIC:
            case OA_LOG_RECORD_VALUE_TYPE_FLOAT:
            case OA_LOG_RECORD_VALUE_TYPE_DOUBLE:
            case OA_LOG_RECORD_VALUE_TYPE_REAL:
            case OA_LOG_RECORD_VALUE_TYPE_BIGINT:
            case OA_LOG_RECORD_VALUE_TYPE_INTEGER:
            case OA_LOG_RECORD_VALUE_TYPE_SMALLINT:
                acpStrCpyFormat(&sString,
                                "\t\tColumn value '%s'", &(aLogRecordColumn->mValue[sPosition]));
                oaLogMessage(OAM_MSG_DUMP_LOG, acpStrGetBuffer(&sString));
                break;

            case OA_LOG_RECORD_VALUE_TYPE_CHAR:
            case OA_LOG_RECORD_VALUE_TYPE_VARCHAR:
                dumpCharacterValue(aLogRecordColumn, aArrayDMLIndex);
                break;
            case OA_LOG_RECORD_VALUE_TYPE_NCHAR:
            case OA_LOG_RECORD_VALUE_TYPE_NVARCHAR:
                dumpNCharacterValue( aLogRecordColumn, aArrayDMLIndex );
                break;
            case OA_LOG_RECORD_VALUE_TYPE_DATE:
                acpStrCpyFormat(&sString,
                                "\t\tColumn value '%s'", &(aLogRecordColumn->mValue[sPosition]));
                oaLogMessage(OAM_MSG_DUMP_LOG, acpStrGetBuffer(&sString));
                break;
        }
    }
    else
    {
        acpStrCpyFormat( &sString,
                         "\t\tColumn value is null" );
        oaLogMessage( OAM_MSG_DUMP_LOG, acpStrGetBuffer(&sString) );
    }

    ACP_STR_FINAL(sString);
}

static void dumpInsertLogRecord( oaLogRecordInsert * aLogRecord,
                                 acp_uint32_t        aArrayDMLIndex )
{
    acp_sint32_t i;
    ACP_STR_DECLARE_DYNAMIC(sString);

    ACP_STR_INIT_DYNAMIC(sString, 128, 128);

    oaLogMessage( OAM_MSG_DUMP_LOG, "INSERT : " );

    acpStrCpyFormat(&sString, "\tTable name %s", aLogRecord->mTableName);
    oaLogMessage(OAM_MSG_DUMP_LOG, acpStrGetBuffer(&sString));

    acpStrCpyFormat(&sString, "\tTable Id %d", aLogRecord->mTableId);
    oaLogMessage(OAM_MSG_DUMP_LOG, acpStrGetBuffer(&sString));

    acpStrCpyFormat(&sString, "\tColumn count %d", aLogRecord->mColumnCount);
    oaLogMessage(OAM_MSG_DUMP_LOG, acpStrGetBuffer(&sString));

    for (i = 0; i < aLogRecord->mColumnCount; i++)
    {
        dumpLogRecordColumn(&(aLogRecord->mColumn[i]), aArrayDMLIndex);
    }

    ACP_STR_FINAL(sString);
}

static void dumpUpdateLogRecord( oaLogRecordUpdate * aLogRecord,
                                 acp_uint32_t        aArrayDMLIndex )
{
    acp_sint32_t i;
    acp_uint32_t sColumnID;

    ACP_STR_DECLARE_DYNAMIC(sString);

    ACP_STR_INIT_DYNAMIC(sString, 128, 128);

    oaLogMessage( OAM_MSG_DUMP_LOG, "UPDATE : " );

    acpStrCpyFormat(&sString, "\tTable name %s", aLogRecord->mTableName);
    oaLogMessage(OAM_MSG_DUMP_LOG, acpStrGetBuffer(&sString));

    acpStrCpyFormat(&sString, "\tTable Id %d", aLogRecord->mTableId);
    oaLogMessage(OAM_MSG_DUMP_LOG, acpStrGetBuffer(&sString));

    acpStrCpyFormat(&sString,
                    "\tPrimary key count %d", aLogRecord->mPrimaryKeyCount);
    oaLogMessage(OAM_MSG_DUMP_LOG, acpStrGetBuffer(&sString));

    for (i = 0; i < aLogRecord->mPrimaryKeyCount; i++)
    {
        dumpLogRecordColumn(&(aLogRecord->mPrimaryKey[i]), aArrayDMLIndex);
    }

    acpStrCpyFormat(&sString, "\tColumn count %d", aLogRecord->mColumnCount);
    oaLogMessage(OAM_MSG_DUMP_LOG, acpStrGetBuffer(&sString));

    for (i = 0; i < aLogRecord->mColumnCount; i++)
    {
        sColumnID = aLogRecord->mColumnIDMap[i];
        dumpLogRecordColumn(&(aLogRecord->mColumn[sColumnID]), aArrayDMLIndex);
    }

    ACP_STR_FINAL(sString);
}

static void dumpDeleteLogRecord( oaLogRecordDelete * aLogRecord,
                                 acp_uint32_t        aArrayDMLIndex )
{
    acp_sint32_t i;
    ACP_STR_DECLARE_DYNAMIC(sString);

    ACP_STR_INIT_DYNAMIC(sString, 128, 128);

    oaLogMessage( OAM_MSG_DUMP_LOG, "DELETE : " );

    acpStrCpyFormat(&sString, "\tTable name %s", aLogRecord->mTableName);
    oaLogMessage(OAM_MSG_DUMP_LOG, acpStrGetBuffer(&sString));

    acpStrCpyFormat(&sString, "\tTable Id %d", aLogRecord->mTableId);
    oaLogMessage(OAM_MSG_DUMP_LOG, acpStrGetBuffer(&sString));

    acpStrCpyFormat(&sString,
                    "\tPrimary key count %d", aLogRecord->mPrimaryKeyCount);
    oaLogMessage(OAM_MSG_DUMP_LOG, acpStrGetBuffer(&sString));

    for (i = 0; i < aLogRecord->mPrimaryKeyCount; i++)
    {
        dumpLogRecordColumn(&(aLogRecord->mPrimaryKey[i]), aArrayDMLIndex);
    }

    ACP_STR_FINAL(sString);
}

void oaLogRecordDumpType(oaLogRecord *aLogRecord)
{
    switch (aLogRecord->mCommon.mType)
    {
        case OA_LOG_RECORD_TYPE_UNKNOWN:
            oaLogMessage( OAM_MSG_DUMP_LOG, "Log Record : UNKNOWN" );
            break;

        case OA_LOG_RECORD_TYPE_COMMIT:
            oaLogMessage( OAM_MSG_DUMP_LOG, "Log Record : COMMIT" );
            break;

        case OA_LOG_RECORD_TYPE_INSERT:
        case OA_LOG_RECORD_TYPE_UPDATE:
        case OA_LOG_RECORD_TYPE_DELETE:
            /* Array DML이 적용되어, 개별적으로 기록한다. */
            break;

        case OA_LOG_RECORD_TYPE_KEEP_ALIVE :
            oaLogMessage( OAM_MSG_DUMP_LOG, "Log Record : KEEP ALIVE" );
            break;

        case OA_LOG_RECORD_TYPE_STOP_REPLICATION :
            oaLogMessage( OAM_MSG_DUMP_LOG, "Log Record : STOP REPLICATION" );
            break;

        default:
            break;
    }
}

/**
 * @breif  Array DML 중 하나를 출력한다.
 *
 * @param  aLogRecord     Log Record Union
 * @param  aArrayDMLIndex Array DML Index
 */
void oaLogRecordDumpDML( oaLogRecord  * aLogRecord,
                         acp_uint32_t   aArrayDMLIndex )
{
    switch ( aLogRecord->mCommon.mType )
    {
        case OA_LOG_RECORD_TYPE_INSERT :
            dumpInsertLogRecord( &(aLogRecord->mInsert), aArrayDMLIndex );
            break;

        case OA_LOG_RECORD_TYPE_UPDATE :
            dumpUpdateLogRecord( &(aLogRecord->mUpdate), aArrayDMLIndex );
            break;

        case OA_LOG_RECORD_TYPE_DELETE :
            dumpDeleteLogRecord( &(aLogRecord->mDelete), aArrayDMLIndex );
            break;

        default :
            break;
    }
}

acp_bool_t oaLogRecordColumnIsHiddenColumn( oaLogRecordColumn * aColumn )
{
    return aColumn->mIsHidden;
}
