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
 * $Id: utmDef.h 80542 2017-07-19 08:01:20Z daramix $
 **********************************************************************/

#ifndef _O_UTM_DEF_H_
#define _O_UTM_DEF_H_ 1

#define ENV_PRODUCT_PREFIX      ""
#define EXPORT_PRODUCT_LNAME    "aexport"
#define EXPORT_PRODUCT_UNAME    "AEXPORT"
#define ENV_ALTIBASE_PORT_NO    "ALTIBASE_PORT_NO"
#define PROPERTY_PORT_NO        "PORT_NO"
#define DEFAULT_PORT_NO         20300

#define A3EXPORT_CONFFILE       (SChar *)"conf"IDL_FILE_SEPARATORS EXPORT_PRODUCT_LNAME".properties"
#define ISQL_PRODUCT_NAME       PRODUCT_PREFIX"isql"
#define ILO_PRODUCT_NAME        PRODUCT_PREFIX"iloader"

/* BUG-40407 SSL */
#define ENV_ISQL_CONNECTION      ENV_PRODUCT_PREFIX"ISQL_CONNECTION"
#define ENV_ALTIBASE_SSL_PORT_NO ALTIBASE_ENV_PREFIX"SSL_PORT_NO"

#define CLI_CONNTYPE_TCP         1
#define CLI_CONNTYPE_SSL         6

#define CONN_STR_LEN  (2048)
#define MSG_LEN       (1024)
#define META_CNT      (18)
#define QUERY_LEN     (1024)
#define STR_LEN       (50)
//fix BUG-17481 aexport가 partion disk table을 지원해야 한다.
#define UTM_QUERY_LEN (2048)
#define UTM_NAME_LEN  (128+20) /* BUG-39622: object max length(128) + alpha */
/* PROJ-1107 Check Constraint 지원 */
#define UTM_CHECK_CONDITION_LEN (4000)
/* PROJ-1090 Function-based Index */
#define UTM_EXPRESSION_LEN      (4000)

/* PROJ-2211 Materialized View */
#define UTM_MVIEW_VIEW_SUFFIX       "$VIEW"
#define UTM_MVIEW_VIEW_SUFFIX_SIZE  (5)

#define UTM_NONE               (-1)
#define UTM_PROCEDURE          (0)
#define UTM_FUNCTION_PROCEDURE (1)
#define UTM_VIEW               (2)
#define UTM_SEQUENCE           (3)
#define UTM_TABLE              (4)
#define UTM_DIRECTORY          (5)
/* PROJ-2211 Materialized View */
#define UTM_MVIEW              (6)
/* BUG-36367 aexport must consider new objects, 'package' and 'library' */
#define UTM_PACKAGE_SPEC       (7)
#define UTM_PACKAGE_BODY       (8)
#define UTM_LIBRARY            (9)

//BUG-22769
#define UTM_PRIV_COUNT (10)   //사용자 계정 생성시 기본적으로 생성되는 권한의 수

/* BUG-44595 */
#define UTM_CREATE_PROCEDURE           (0)
#define UTM_CREATE_SEQUENCE            (1)
#define UTM_CREATE_SESSION             (2)
#define UTM_CREATE_SYNONYM             (3)
#define UTM_CREATE_TABLE               (4)
#define UTM_CREATE_TRIGGER             (5)
#define UTM_CREATE_VIEW                (6)
#define UTM_CREATE_MATERIALIZED_VIEW   (7)
#define UTM_CREATE_DATABASE_LINK       (8)
#define UTM_CREATE_LIBRARY             (9)

typedef struct FileStream
{
    FILE  *mObjFp;  
    FILE  *mObjConFp;  
    FILE  *mInvalidFp;  
    FILE  *mTbsFp;   
    FILE  *mUserFp;  
    FILE  *mSynFp;   
    FILE  *mDirFp;   
    FILE  *mTblFp;   
    FILE  *mSeqFp; 
    FILE  *mViewProcFp;  
    FILE  *mTrigFp;  
    FILE  *mIndexFp; 
    FILE  *mFkFp;    
    FILE  *mLinkFp;  
    FILE  *mReplFp;
    FILE  *mLibFp;              /* BUG-36367 aexport must consider new objects, 'package' and 'library' */
    FILE  *mRefreshMViewFp;     /* PROJ-2211 Materialized View */
    FILE  *mJobFp;              /* PROJ-1438 Job Scheduler */
    FILE  *mAltTblFp;           /* PROJ-2359 Table/Partition Access Option */
    FILE  *mDbStatsFp;          /* BUG-40174 Support export and import DBMS Stats */
    FILE  *mIsFp;
    FILE  *mIsConFp;
    FILE  *mIlInFp;
    FILE  *mIlOutFp;
    FILE  *mIsReplFp;
    FILE  *mIsIndexFp; 
    FILE  *mIsFkFp;
    FILE  *mIsRefreshMViewFp;   /* PROJ-2211 Materialized View */
    FILE  *mIsJobFp;            /* PROJ-1438 Job Scheduler */
    FILE  *mIsAltTblFp;         /* PROJ-2359 Table/Partition Access Option */
} FileStream;

typedef struct ObjectModeInfo
{
    SChar mObjUserName[UTM_NAME_LEN+1];
    SChar mObjObjectName[UTM_NAME_LEN+1];
    SInt  mObjUserId;
    SInt  mObjObjectId;
    SInt  mObjObjectType;
} ObjectModeInfo;

typedef struct Parsing 
{
    SChar mOption[50];
} Parsing;

typedef struct UserInfo
{
    SChar m_user[UTM_NAME_LEN+1];
    SChar m_passwd[50];
} UserInfo;

/* BUG-17491 */
/* 
 * server side 에서의 partition method 에 대한 정보 : 
 * qp/src/include/qcmTableInfo.h 의 qcmPartitionMethod 에 따라
 * UTM_PARTITION_METHOD 들을 정의함.
 */
typedef enum
{
    UTM_PARTITION_METHOD_RANGE  = 0,
    UTM_PARTITION_METHOD_HASH   = 1,
    UTM_PARTITION_METHOD_LIST   = 2,
    UTM_PARTITION_METHOD_NONE   = 100
} utmPartitionMethod;

/* BUG-17491 */
// system_.sys_part_tables_
typedef struct
{
    utmPartitionMethod m_partitionMethod;
    idBool m_isEnabledRowMovement;
} utmPartTables;

/* BUG-36367 aexport must consider new objects, 'package' and 'library' */
#define UTM_STR_ALL "ALL"
#define UTM_STR_TBL "TBL"
#define UTM_STR_INDEX "INDEX"
#define UTM_STR_FK   "FK"
#define UTM_STR_SEQ  "SEQ"
#define UTM_STR_DBLINK "LINK"
#define UTM_STR_PROC "PROC"
#define UTM_STR_VIEW "VIEW"
#define UTM_STR_TBS  "TBS"
#define UTM_STR_TRIG "TRIG"
#define UTM_STR_LIB  "LIB"
#define UTM_STR_JOB  "JOB"
#define UTM_STR_SYS  "SYS"
#define UTM_STR_MANAGER "MANAGER"
#define UTM_STR_IN   "in"
#define UTM_STR_OUT  "out"

/* BUG-44831 Exporting INDEX Stats for PK, Unique constraint
 * with system-given name */
typedef enum
{
    UTM_STAT_NONE   = 0,
    UTM_STAT_PK     = 1,
    UTM_STAT_UNIQUE = 2,
    UTM_STAT_INDEX  = 3
} utmIndexStatProcType;

#endif /* _O_UTM_DEF_H_ */
