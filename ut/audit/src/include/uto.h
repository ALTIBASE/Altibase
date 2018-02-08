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
 
/*******************************************************************************
 * $Id: uto.h 81253 2017-10-10 06:12:54Z bethy $
 ******************************************************************************/
#ifndef _UTO_H_
#define _UTO_H_ 1

#include <idtBaseThread.h>
#include <idtContainer.h>
#include <iduMemMgr.h>
#include <utProperties.h>

//TASK-4212     audit툴의 대용량 처리시 개선
#define MAX_FILE_NAME    512
#define NUM_FILE_THR     2
#define MAX_COLUMN_NAME  128+30
#define MAX_DIFF_STR     512
#define MAX_COL_CNT      1024
// 1 Mb for file read buffer
#define MAX_FILE_BUF     (10240)
#define FILE_BUF_LIMIT   (10240*0.95)
/* 토큰의 최대 길이 = VARBIT 문자열 표현의 최대 길이 */
#define MAX_TOKEN_VALUE_LEN 131070

typedef enum
{
    CMP_OK     = 0, // Compare Ok
    CMP_PKA    = 1, // Compare PK diff detected A Bigger : MXSO
    CMP_PKB    =-1, // Compare PK diff detected B Bigger : MOSX
    CMP_CL     = 2  // Compare CL diff detected
} compare_t;

typedef enum
{
    MOSX = 0,  // Master exist Slave none
    MXSO    ,  // Master none  Slave exist
    MOSO       // Master exist Slave exist
} ps_t;

/* TASK-4212: audit툴의 대용량 처리시 개선 */
typedef enum
{
    T_INIT = 0,
    T_EOF,
    T_VALUE,
    T_NULL_VALUE,
    T_FIELD_TERM,
    T_ROW_TERM,
    T_ERR
} utaCSVTOKENTYPE;


typedef enum
{
    /*******************************************************
    * stStart   : state of starting up reading a field
    * stCollect : state of in the middle of reading a field
    * stTailSpace : at tailing spaces out of quotes
    * stEndQuote  : at tailing quote 
    * stError     : state of a wrong csv format is read
    *******************************************************/
    stStart= 0,
    stCollect,
    stTailSpace,
    stEndQuote,
    stError
} utaCSVSTATE;

typedef struct
diff_t
{
    SChar  *  name;  // Column Name
    UShort    indx;  // Column index

    SShort sqlType;  // SQL Type of columns
} diff_t;

/* TASK-4212: audit툴의 대용량 처리시 개선 */
typedef struct utaFileMArg
{
    SChar *mFilename;
    FILE  *mFile;
    Query *mQuery;
    Row   *mRow;
} utaFileMArg;

typedef struct utaFileInfo
{
    FILE     *mFile;
    SChar     mName[MAX_FILE_NAME];
    SChar     mBuffer[MAX_FILE_BUF];
    SInt      mOffset;
    // file로 부터 읽은 size. (<=MAX_FILE_BUF)
    SInt      mFence;
} utaFileInfo;

class utScanner;
class utTaskThread;
class utTaskList;

class utTaskList
{
    utProperties  &prop;
    UInt           size;
    utTaskThread **task;

public:
    utTaskList(utProperties &);
    ~utTaskList();

    IDE_RC initialize();
    IDE_RC      start();
    IDE_RC       join();
    IDE_RC   finalize();
};

class utTaskThread : public idtBaseThread
{
    utScanner           *mScanner; // current scanner
    Connection   *mConnA, *mConnB; // Master (A), Slave(B) connection
    utProperties           *mProp; // Properties link pointer

    SChar   _error[ERROR_BUFSIZE]; // Private error buffer is shared

    PDL_Time_Value     mCheckTime; // Timer check interval

public:
    IDE_RC initialize(utProperties *);
    IDE_RC finalize();
    void   run();

    SChar* error();

};

/* type of function for process SU,SI,MI,SD */
typedef IDE_RC (*process)(utScanner*);
IDE_RC processDUMMY(utScanner*);

class dmlQuery : public Object
{
    Query * mQuery;
    
    UInt     mExec;
    UInt     mFail;

    SChar mType[3];
    
    metaColumns *mMeta;
    const SChar * mSchema;
    const SChar *  mTable;

    Row         * mRow;
    
public:
    dmlQuery();
    ~dmlQuery();

    IDE_RC initialize(Query       *, // Query for execute
                      SChar        , // Server  Type 'M' or 'S' ..
                      SChar        , // SQL DML Type 'I','U','D'
                      const SChar *, // schema
                      const SChar *, // Table
                      metaColumns *);// Metadata

    IDE_RC bind(Row*);               // bind Row to DML
    IDE_RC execute ();               // execute sequence of Operators
    IDE_RC lobAtToAt (Query *, Query *, SChar *);
    
    inline const SChar * getType() { return mType; };
    inline const SChar * error () { return (mQuery)?mQuery->error():""; }

    inline UInt did  (){ return  mExec; }
    inline UInt fail (){ return  mFail; }
    inline void reset(){ mExec=mFail=0; }

protected:
    IDE_RC prepareInsert();
    IDE_RC prepareUpdate();
    IDE_RC prepareDelete();

    IDE_RC sqlInsert(SChar *sql, const SChar *dl = " ?");
    IDE_RC sqlDelete(SChar *sql, const SChar *dl = " ?");
    IDE_RC sqlUpdate(SChar *sql, const SChar *dl = " ?");

private:
#ifdef DEBUG
    void log4Bind(SChar *aQueryType, UShort aColumNumber, Field *aField);
#endif
};

class utScanner: public Object
{
    pmode_t       mMode;  // Execution mode SYNC/DIFF/MOVE
    //TASK-4212     audit툴의 대용량 처리시 개선
    // utaFileModeWrite() 에서 사용하기위해 static으로 변경.
    SChar *_error; // error pointer // BUG-43607 remove static

    dmlQuery  *  mSD;
    dmlQuery  *  mSI;
    dmlQuery  *  mSU;

    dmlQuery  *  mMI;
    bool      * mDML;

    SInt mCountToCommit;


    inline UInt  did(dmlQuery *v) { return (v) ?  v->did() :0; }
    inline UInt fail(dmlQuery *v) { return (v) ? v->fail() :0; }

    UInt  didSl(){ return  did(mSI)  + did(mSD) + did(mSU); };
    UInt failSl(){ return fail(mSI) + fail(mSD) +fail(mSU); };

    UInt  didMa(){ return  did(mMI); };
    UInt failMa(){ return fail(mMI); };

    void     reset();
    IDE_RC exec(dmlQuery*);

    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    // property 정보 pointer.
    static    utProperties *mProp;
    bool      mIsFileMode;

    // master node file 관련변수들.
    utaFileInfo mMasterFile;
    // slave node file 관련변수들.
    utaFileInfo mSlaveFile;

    // 다음에 올 csv token을 저장해두는 변수.
    utaCSVTOKENTYPE      mCSVNextToken;

    // fetch된 data를 CSV formatting 한뒤 파일에 써준다.
    static void *utaFileModeWrite( void *aFileName );

    // CSV format의 구분자 변수들.
    static SChar mFieldTerm;
    static SChar mRowTerm;
    static SChar mEnclosing;

    // 일반data를 CSV formatting 해주는 함수.
    static IDE_RC utaCSVWrite ( SChar *aValue, UInt aValueLen, FILE *aWriteFp );
    // 파일버퍼로 부터 CSV 관련 token을 하나씩 얻어오는 함수.

    /* BUG-32569 The string with null character should be processed in Audit */
    utaCSVTOKENTYPE utaGetCSVTokenFromBuff( utaFileInfo *aFileInfo,
                                            SChar       *aTokenBuff,
                                            UInt         aTokenBuffLen,
                                            UInt        *aTokenValueLength );
    // utaGetCSVTokenFromBuff 함수를 반복 호출하여 한 Row의 Field들에 대한 data값을 쎄팅한다.
    Row *utaReadCSVRow( utaFileInfo *aFileInfo, Query *aQuery );

public:
    utScanner();
    IDE_RC initialize(Connection * aConnA, // Master connection
                      Connection * aConnB, // Slave  connection
                      SInt aCountToCommit, // Process commit count
                      SChar      * =NULL); // Error message buffer
    IDE_RC finalize();

    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    IDE_RC prepare(utProperties *aProp);

    IDE_RC execute(void);

    IDE_RC fetch(bool,bool);  // DIFF_NO is END of scanning
    IDE_RC setModeDIFF(bool = true);     // Set DIFF role


    IDE_RC setSI();
    IDE_RC setSU();
    IDE_RC setSD();

    IDE_RC setMI();

    IDE_RC setTable(utTableProp*);
    void   generateLogName(SChar              *aFullLogName,
                           const SChar        *aLogDir,
                           const SChar        *aTableNameA,
                           const SChar        *aSchemaA,
                           const SChar        *aTableNameB);

    IDE_RC exclude  (SChar *); // Exclude Field list separate by ","

    IDE_RC printResult(FILE* = stdout);

    SChar *error();

    inline Row * getMasterRow()   { return mRowA; }
    inline Row * getSlaveRow ()   { return mRowB; }
    inline Query * getSelectA ()   { return mSelectA; }
    inline Query * getSelectB ()   { return mSelectB; }

    inline void setReport(bool   v){ report  = v; }
    inline void setLogDir(SChar *s){ mLogDir = s; }

    static ULong getTimeStamp();

    SInt   selectCount();
    
protected:
    IDE_RC bindColumns();
    compare_t compare(void); // comparator of

    IDE_RC doMOSO();
    IDE_RC doMXSO();
    IDE_RC doMOSX();

    diff_t        diffColumn; // current diff descriptor
    IDE_RC  log_diff(ps_t);
    IDE_RC  log_record(ps_t); /* BUG-44461 */

    static IDE_RC  processDIFF_MI(utScanner* );
    static IDE_RC  processDIFF_SI(utScanner* );
    static IDE_RC  processDIFF_SU(utScanner* );

    IDE_RC  processUN_IDX (Query*,bool=false); //  error handle UNIQUE INDEX etc..., default no delete

    /* BUG-43455 Slave table not found error due to different user id from SCHEMA at altiComp.cfg */
    IDE_RC select_sql(SChar *, const SChar*, const SChar*); // create SQL for fetch
    
    /* write to log file primary key conteint */
    IDE_RC logWritePK(Row *);
    IDE_RC logWriteRow(Row *); /* BUG-44461 */

    Connection   *mConnA,   *mConnB;

    const SChar        *mTableNameA;
    const SChar        *mTableNameB;

    const SChar        *mSchemaA;
    const SChar        *mSchemaB;

    const SChar         *mQueryCond;

    metaColumns              *mMeta; // Common Meta Columns of Table

    SInt                mTableCount;


    Query      *mSelectA, *mSelectB;
    Row           *mRowA,    *mRowB;

    UInt                 mDiffCount;

    UShort             mFiledsCount;
    UShort                 mPKCount;
    bool                     ordASC; // order in primary key is ASC(true) or DSC(false)

    compare_t                   cmp;

    // BUG-17951
    UInt             mMOSODiffCount;
    UInt             mMXSODiffCount;
    UInt             mMOSXDiffCount;

private:
    bool fetchA, fetchB; // fetch ahead states
    bool         report; // print report on/off
    bool         un_idx; // delete UNIQUE INDEX Conflict only for mode (SI && SD)

    ULong    mTimeStamp; // use for calculate perfomance

    SChar     *   mLogDir; // Dir for Log File
    FILE      *      flog; // Log file descriptor

    UInt           _fetch; // for dislay progress
    UInt           _limit; // LIMIT of query

    UInt progress();
};
#endif
