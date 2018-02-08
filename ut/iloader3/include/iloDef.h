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
 * $Id: iloDef.h 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

#ifndef _O_ILO_DEF_H
#define _O_ILO_DEF_H

#define ENVIRON_FILE       "iloader.ini"

#define ENV_ISQL_PREFIX                        "ISQL_"
#define ENV_ILO_PREFIX                         "ILO_"
#define ENV_ALTIBASE_PORT_NO                   "ALTIBASE_PORT_NO"
#define ENV_ALTIBASE_IPCDA_PORT_NO             "ALTIBASE_IPCDA_PORT_NO"
#define PROPERTY_PORT_NO                       "PORT_NO"
#define PROPERTY_IPCDA_PORT_NO                 "IPCDA_PORT_NO"
#define DEFAULT_PORT_NO                        20300

#ifdef COMPILE_SHARDCLI
#define BAN_FILENAME                           "SHARDLOADER.ban"
#define ILO_PRODUCT_NAME                       "SHARDLOADER"
#else /* COMPILE_SHARDCLI */
#define BAN_FILENAME                           "ILOADER.ban"
#define ILO_PRODUCT_NAME                       "ILOADER"
#endif /* COMPILE_SHARDCLI */

/* BUG-41406 */
#define ENV_ALTIBASE_SSL_PORT_NO               ALTIBASE_ENV_PREFIX"SSL_PORT_NO"

#define ENV_ALTIBASE_DATE_FORMAT               ALTIBASE_ENV_PREFIX"DATE_FORMAT"
#define ENV_ALTIBASE_EDITOR                    ALTIBASE_ENV_PREFIX"EDITOR"
#define ENV_ISQL_CONNECTION                    ENV_ISQL_PREFIX"CONNECTION"
#define ENV_ILO_DATEFORM                       ENV_ILO_PREFIX"DATEFORM"

/* BUG-31404: 항상 STD_GEOHEAD_SIZE 보다 커야 함 */
#define ILO_GEOHEAD_SIZE   56

/* BUG-31387: define Connection Type */
#define ILO_CONNTYPE_NONE  0
#define ILO_CONNTYPE_TCP   1
#define ILO_CONNTYPE_UNIX  2
#define ILO_CONNTYPE_IPC   3
#define ILO_CONNTYPE_SSL   6
#define ILO_CONNTYPE_IPCDA 7

#define MAX_OBJNAME_LEN    128+30    //BUG-39621 "MaxLengthName"\0 + alpha
/* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
#define MAX_FILEPATH_LEN   1024     //BUG-24583
#define MAX_WORD_LEN       128
#define MAX_ATTR_COUNT     1024
#define MAX_VALUE_LEN      10000

#define INT_NULL           -2147483648
#define MSG_LEN            1024
#define STR_LEN            128+1
#define REM_LEN            256+1
#define COMMAND_LEN        2048

#define ILOADER_SUCCESS     0
#define ILOADER_ERROR       -1
#define ILOADER_WARNING     -2

#define MAX_VARCHAR_SIZE   32*1024 // BUGBUG : TODO ..(SMC_PERS_PAGE_BODY_SIZE - sizeof(smcVarPageHeader) - sizeof(smVarColumn))
/* 숫자 타입들에대한 길이제한 값 :
 * double을 기준으로 했을때, 입력될수 있는 숫자의 가장긴 string 길이는 312byte. 
 * 예외상황을 고려하여 넉넉히 511byte로 할당함.
 */
/* BUG - 18804 */
#define MAX_NUMBER_SIZE    511

#define MAX_INDEX_COUNT    100

#define UT_MAX_SEQ_ARRAY_CNT (8)
#define UT_MAX_SEQ_VAL_LEN   (8)

#define MAX_PASS_LEN       40

#define SYS_ERROR          -1   // BUG-28208: malloc 등에 실패했을 때 iloader 바로 종료 
#define READ_ERROR         0
#define READ_SUCCESS       1
#define END_OF_FILE        2

#define COMMAND_INVALID   -1
#define COMMAND_PROMPT     0
#define COMMAND_VALID      1
#define COMMAND_HELP       2

#define ILOADER_STATE_UNDEFINED     9999

// bug-21332
#define ILOADER_ERROR_FLOCK -3

//PROJ-1714
#define MAX_PARALLEL_COUNT      32
#define MAX_CIRCULAR_BUF        1024 * 1024 * 10    //10M
// BUG-18803 readsize 옵션 추가
// readsize 옵션의 기본값이다.
#define FILE_READ_SIZE_DEFAULT  1024 * 1024         //1M

/* BUG-21064 : CLOB type CSV up/download error */
// CSV 포맷 형태의 CLOB data일경우.
#define CLOB4CSV ( sHandle->mProgOption->mRule == csv ) && ( mLOBLocCType == SQL_C_CLOB_LOCATOR )

// PROJ-2075

/*iLoader build 시 현재 버전을 명시 한다. 
 * ALTIBASE_ILOADER_MAX_VER 은 ALTIBASE_ILOADER_V(x)의 
 * 최대 버전 값과 동일 해야 한다. 
 */

#define ALTIBASE_ILOADER_MAX_VER 1

#define CHECK_MAX_VER (sOptions->version == 0) || \
                       (ALTIBASE_ILOADER_MAX_VER < sOptions->version)


#define DEFAULT_ERROR_COUNT     (50)
#define DEFAULT_COMMIT_UNIT     (1000)
#define DEFAULT_PARALLEL_COUNT  (1)
#define DEFAULT_SPLIT_ROW_COUNT (0)
#define DEFAULT_ARRAY_COUNT     (0)
#define DEFAULT_FIRST_ROW       (0)
#define DEFAULT_LAST_ROW        (0)
#define UNKNOWN_PORT_NUM        (-1)

#define SAFE_FREE(aPtr) do\
{\
    if ((aPtr) != NULL)\
    {\
        idlOS::free(aPtr);\
    }\
} while (ID_FALSE)

#define SAFE_DELETE(aPtr) do\
{\
    if ((aPtr) != NULL)\
    {\
        delete aPtr;\
    }\
} while (ID_FALSE)

#define SAFE_COPY_ERRORMGR(aDes, aSrc) do\
{\
    if (aDes != NULL)\
    {\
        (aDes)->errorCode    = E_ERROR_CODE((aSrc)->mErrorCode);\
        (aDes)->errorState   = (aSrc)->mErrorState;\
        (aDes)->errorMessage = (aSrc)->mErrorMessage;\
    }\
} while (ID_FALSE)

#define ILO_CALLBACK \
    do { \
        if (( sHandle->mUseApi == SQL_TRUE ) && \
                ( sHandle->mLogCallback != NULL )) \
        { \
            sHandle->mLog.errorMgr.errorCode = E_ERROR_CODE(sErrorMgr->mErrorCode); \
            sHandle->mLog.errorMgr.errorState = sErrorMgr->mErrorState; \
            sHandle->mLog.errorMgr.errorMessage = sErrorMgr->mErrorMessage; \
            sHandle->mLogCallback( ILO_LOG, &sHandle->mLog ); \
            sHandle->mLog.record      = 0; \
            sHandle->mLog.recordData  = NULL; \
            sHandle->mLog.errorMgr.errorCode = 0; \
            sHandle->mLog.errorMgr.errorState = NULL; \
            sHandle->mLog.errorMgr.errorMessage = NULL; \
        } \
    } while (0)

enum ELobFileSizeType
{
    SIZE_NUMBER,
    SIZE_UNIT_G,
    SIZE_UNIT_T
};

enum ETableNodeType
{
    TABLE_NODE,
    DOWN_NODE,
    SEQ_NODE,
    TABLENAME_NODE,
    ATTR_NODE,
    ATTRNAME_NODE,
    ATTRTYPE_NODE
};

enum EispAttrType
{
    ISP_ATTR_NONE,
    ISP_ATTR_INTEGER,

    ISP_ATTR_DOUBLE,
    ISP_ATTR_SMALLINT,
    ISP_ATTR_BIGINT,
    ISP_ATTR_DECIMAL,
    ISP_ATTR_FLOAT,
    ISP_ATTR_REAL,
    ISP_ATTR_INTEVAL,
    ISP_ATTR_BOOLEAN,
    ISP_ATTR_BLOB,

    ISP_ATTR_NIBBLE,
    ISP_ATTR_BYTES,
    ISP_ATTR_VARBYTE, /* BUG-40973 */
    ISP_ATTR_CHAR,
    ISP_ATTR_VARCHAR,
    ISP_ATTR_BIT,
    ISP_ATTR_VARBIT,
    ISP_ATTR_NUMERIC_LONG,
    ISP_ATTR_NUMERIC_DOUBLE,
    ISP_ATTR_DATE,
    ISP_ATTR_TIMESTAMP,
    ISP_ATTR_CLOB,
    ISP_ATTR_NCHAR,
    ISP_ATTR_NVARCHAR,
    /* BUG-24359  Geometry FormFile*/
    ISP_ATTR_GEOMETRY
};

enum ECommandType
{
    NON_COM,
    DATA_IN,
    DATA_OUT,
    STRUCT_OUT,
    FORM_OUT,
    EXIT_COM,
    HELP_COM,
    HELP_HELP
};

enum EDataToken
{
    TEOF,
    TFIELD_TERM,
    TROW_TERM,
    TVALUE,
/* TASK-2657 */
    TERROR
};

enum TimestampType
{
    ILO_TIMESTAMP_DAT,
    ILO_TIMESTAMP_DEFAULT,
    ILO_TIMESTAMP_NULL,
    ILO_TIMESTAMP_VALUE
};

typedef struct seqInfo
{
    char seqName[MAX_OBJNAME_LEN];
    char seqCol[MAX_OBJNAME_LEN];
    char seqVal[UT_MAX_SEQ_VAL_LEN];
} seqInfo;

/* TASK-2657 */
enum eRule
{
    csv = 0,
    old
};

/* BUG-21332 */
enum eLockType
{
    LOCK_RD = 0,
    LOCK_WR
};

/* BUG-44187 Support Asynchronous Prefetch */
typedef enum asyncPrefetchType
{
    ASYNC_PREFETCH_OFF = 0,
    ASYNC_PREFETCH_ON,
    ASYNC_PREFETCH_AUTO_TUNING
} asyncPrefetchType;

void gSetInputStr( SChar *s );

SInt yyparse(void*);


SInt iloCommandParserparse(void*);

#endif /* _O_ILO_DEF_H */

