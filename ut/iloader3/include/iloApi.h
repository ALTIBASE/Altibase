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
 * $Id: ilo.h 35892 2009-09-29 02:28:49Z reznoa $
 **********************************************************************/

#ifndef _O_ILO_API_H_
#define _O_ILO_API_H_

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ALTIBASE_ILOADER_V1 1

#define ALTIBASE_ILO_SUCCESS     0
#define ALTIBASE_ILO_ERROR       -1
#define ALTIBASE_ILO_WARNING     -2

typedef enum
{
    ILO_APPEND,
    ILO_REPLACE,
    ILO_TRUNCATE
} iloLoadMode;

typedef enum
{
    ILO_FALSE = 0,
    ILO_TRUE  = 1
} iloBool;

typedef enum
{
    ILO_DIRECT_NONE,   /* default NO use direct path */
    ILO_DIRECT_LOG,    /* direct path log */
    ILO_DIRECT_NOLOG   /* direct path nolog */
}iloDirectMode;

typedef enum
{
    ILO_LOG,
    ILO_STATISTIC_LOG
} ALTIBASE_ILOADER_LOG_TYPE;
typedef void * ALTIBASE_ILOADER_HANDLE;

#define ALTIBASE_ILOADER_NULL_HANDLE    NULL

typedef struct ALTIBASE_ILOADER_OPTIONS_V1
{
    int            version;            /* option version */
    char           loginID[128 * 2];   /* 사용자 이름 */
    char           password[128];      /* 사용자 패스워드 */
    char           serverName[128];    /* 서버 이름 */
    int            portNum;            /* 포트 번호 */
    char           NLS[128];           /* 캐릭터 셋 */
    char           DBName[128];        /* 데이타 베이스 이름 */
    char           tableOwner[50];     /* 특정 사용자에게 존재 하는 테이블 이름 */
    char           tableName[50];      /* 테이블 이름 */
    char           formFile[1024];     /* 포맷 파일 이름  */
    char           dataFile[32][1024]; /* 데이타 파일 이름 */
    int            dataFileNum;        /* 데이타 파일 개수 */
    int            firstRow;           /* 첫번째 행의 번호 */
    int            lastRow;            /* 마지막 행의 번호 */
    char           fieldTerm[11];      /* 필드 사이의 구분자 */
    char           rowTerm[11];        /* 행 사이의 구분자 */
    char           enclosingChar[11];  /* 필드를 enclosing 시킬 값 */
    iloBool        useLobFile;         /* LOB 파일 사이즈 */
    iloBool        useSeparateFile;    /* LOB 파일을 사용할 때 하나의 LOB entry에 대해 하나씩 저장 */
    char           lobFileSize[11];    /* LOB 파일의 최대 사이즈 */
    char           lobIndicator[11];   /* LOB 파일의 indicator */
    iloBool        replication;        /* 이중화 off 데이타 로딩 */
    iloLoadMode    loadModeType;       /* 데이타 출력 형태 지정 */
    char           bad[1024];          /* bad 파일 이름 */ 
    char           log[1024];          /* log 파일 이름 */
    int            splitRowCount;      /* 파일 마다 복사할 레코드 개수 */
    int            errorCount;         /* 에러 개수 */
    int            arrayCount;         /* array 개수 */
    int            commitUnit;         /* commit 개수 지정 */
    iloBool        atomic;             /* atomic array insert 수행 지정 */
    iloDirectMode  directLog;          /* nolog, log 방식 지정 */
    int            parallelCount;      /* parallel 개수 지정 */
    int            readSize;           /* read size */
    iloBool        informix;           /* 마지막 컬럼 다음에도 컬럼 구분자 인식 */
    iloBool        flock;              /* 포맷 파일 lock */
    iloBool        mssql;              /* date 출력 형태를 mssql 형태로 출력 */
    iloBool        getTotalRowCount;   /* 데이타 파일의 총 개수를 구하여 셋팅 여부 결정 */
    int            setRowFrequency;    /* 콜백 함수를 호출 할 row 개수 지정 */ 
}ALTIBASE_ILOADER_OPTIONS_V1;

typedef struct ALTIBASE_ILOADER_ERROR
{
    int   errorCode;
    char *errorState;
    char *errorMessage;
}ALTIBASE_ILOADER_ERROR;

typedef struct ALTIBASE_ILOADER_LOG
{
    char                        tableName[50];
    int                         totalCount;
    int                         loadCount;
    int                         errorCount;
    int                         record;
    char                      **recordData;
    int                         recordColCount;
    ALTIBASE_ILOADER_ERROR      errorMgr;
}ALTIBASE_ILOADER_LOG;

typedef struct ALTIBASE_ILOADER_STATISTIC_LOG
{
    char                        tableName[50];
    time_t                      startTime;
    int                         totalCount;
    int                         loadCount;
    int                         errorCount;
}ALTIBASE_ILOADER_STATISTIC_LOG;

typedef int(*ALTIBASE_ILOADER_CALLBACK)(ALTIBASE_ILOADER_LOG_TYPE, void*);

int altibase_iloader_init( ALTIBASE_ILOADER_HANDLE *handle );

int altibase_iloader_final( ALTIBASE_ILOADER_HANDLE *handle );

int altibase_iloader_options_init( int version, void *options );

int altibase_iloader_formout( ALTIBASE_ILOADER_HANDLE *handle, 
                              int                      version,
                              void                    *options, 
                              ALTIBASE_ILOADER_ERROR  *error );

int altibase_iloader_dataout( ALTIBASE_ILOADER_HANDLE  *handle, 
                              int                       version,
                              void                     *options, 
                              ALTIBASE_ILOADER_CALLBACK logCallback,
                              ALTIBASE_ILOADER_ERROR   *error );

int altibase_iloader_datain( ALTIBASE_ILOADER_HANDLE   *handle, 
                             int                        version,
                             void                      *options, 
                             ALTIBASE_ILOADER_CALLBACK  logCallback,
                             ALTIBASE_ILOADER_ERROR     *error );


#ifdef __cplusplus
}
#endif
#endif /* _O_ILO_H_ */
