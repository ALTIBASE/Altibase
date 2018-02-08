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
 * $Id: utdb.h 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#ifndef _UT_DB_H_
#define _UT_DB_H_ 1

#include <idl.h>
#include <ide.h>
#include <idu.h>
#include <ulo.h>
#include <mtcdTypes.h>
#include <sqlcli.h>
#include <ute.h>

/*
BUG-24688
insert일때 최대길이와 update일때 최대길이를 대략 계산해 보면
INSERT INTO 유저.테이블명 (컬럼,컬럼,......) VALUES (? , ? , ? ..... )
컬럼길이 "40", * 1024개 = 44032byte  ?, * 1024개 = 3072
즉 46 * 1024 + 알파(그외 문장 1k미만)
update일때 최대길이
UPDATE 유저.테이블명 SET 컬럼이름 = ?,컬럼이름 = ?,..... WHERE PK컬럼이름 = ? AND PK컬럼이름 = ? AND ......
"40" = ?, 는 47 x 1024 = 48128
PK컬럼 최대개수는 32 이므로 WHERE절 뒤에 는 "40" = ? AND  51 x 32 = 1632
48128 + 1632 + 알파(그외 문장 1k미만)
delete 는 더 작다.
그러므로 QUERY_BUFSIZE의 최대치를 대략 구할 수 있다.
64 * 1024면 충분함 
*/
#define QUERY_BUFSIZE   1024*64
#define ERROR_BUFSIZE   1024*8
#define BUF_LEN         1024*4
#define COLUMN_BUF_LEN  1024

#define UTDB_MAX_COLUMN_NAME_LEN  128+30

#if defined (PDL_DLL_SUFFIX) && defined (AIX)
#undef  PDL_DLL_SUFFIX
#define PDL_DLL_SUFFIX  ".so"
#endif

#define AUDIT_DLL_PREFIX  "lib"

IDL_EXTERN_C uteErrorMgr gErrorMgr;

class dbDriver;
class Connection;
class Query;
class Row;
class Field;
class Object;

typedef enum
{
    DBA_ATB = 0,   // ALTIBASE TYPE OF DATABASE
    DBA_ORA    ,   // ORACLE   TYPE OF DATABASE
    DBA_ATB3   ,   // ALTIBASE-3
    DBA_TXT    ,   // Simple text files
    DBA_MAX = DBA_TXT
}dba_t;

/* Dynamic load & link and create conection */
//extern "C" static Connection *create_connection(dba_t type,const SChar * aHome);


typedef struct nameList_t  {
    SChar            *name;
    nameList_t       *next;
} nameList_t;

// ** Convert Row to NATIVE FORMAT (Altibase problems fix ) ** //
typedef IDE_RC (*rowToNative)( Row * );

class Object
{
public:
    Object();
    virtual ~Object() {};

    IDE_RC add   ( Object * );
    IDE_RC remove( Object * );
    IDE_RC find  ( Object *, Object ** ); // return previose in list

    inline Object * next() { return mNext; }

    virtual IDE_RC finalize  (void);    // finalize all in list
    virtual IDE_RC initialize(void);

protected:
    Object * mNext;
};

class dbDriver : public Object
{
    SChar *         mDSN ; // Master DataBase Service Name ( USE RFC URL naming )
    SChar *         mUser; // User Name
    SChar *       mPasswd; // User Pasword

    dba_t                 mType; // DataBase Type

    SChar       *          mURL; // own memory for the keep copy of URL and Lib Name

public:
    dbDriver();
    virtual ~dbDriver();

    IDE_RC initialize(const SChar * URL);
    IDE_RC finalize();

    Connection * connection();

    inline dba_t   getDbType  () { return mType  ;};
    inline SChar * getUserName() { return mUser  ;};
    inline SChar * getPasswd  () { return mPasswd;};
    inline SChar * getDSN     () { return mDSN   ;};
};

class metaColumns : public Object
{
public:
    metaColumns();
    virtual ~metaColumns();

    SChar * getPK(UShort i );   // index 1..N
    SChar * getCL(UShort i, bool);   // index 1..N

    IDE_RC  addPK(SChar*);
    IDE_RC  addCL(SChar*, bool);

    IDE_RC  delCL(SChar*, bool);

    IDE_RC  initialize(SChar *);
    IDE_RC finalize(void);

    inline SChar * getTableName() {return mTableName; }
    inline bool    asc()          {return mASC      ; }
    inline void    asc(bool ord ) { mASC = ord      ; }
    inline UShort  getPKSize()    {return    pkCount; }
    inline UShort  getCLSize(bool isLobType)
        { if(isLobType == false) return clCount; else return lobClCount; }

    /* BUG-43519 Checking if the table exists */
    inline SInt    getColumnCount()
        { return pkCount + clCount + lobClCount; }
    
    bool    isPrimaryKey(SChar*);

    void dump();
protected:
    nameList_t   * primaryKey; //   primary key columns
    nameList_t   * columnList; //no primary key columns
    nameList_t   * lobColumnList; //lob columns

    UShort            pkCount;
    UShort            clCount;
    UShort            lobClCount;

private:
    SChar *        mTableName;
    bool              mASC;
};



class Connection : public  Object
{
    PDL_mutex_t mLock;

public:
    Connection(dbDriver *);

    virtual ~Connection(void);

    virtual IDE_RC initialize(SChar * = NULL, UInt = 0);
    // parametr is pointer to Buffer

    virtual IDE_RC autocommit (bool = true)=0; /* on by default in init() */
    virtual IDE_RC commit     (void       )=0;
    virtual IDE_RC rollback   (void       )=0;

    virtual IDE_RC connect( const SChar *, ... ) =0;
    virtual IDE_RC connect(                    ) =0;
    virtual IDE_RC disconnect(void)       =0;

    virtual SQLHDBC getDbchp();
    
    virtual IDE_RC        delMetaColumns(metaColumns *);

    virtual Query *query(void)            =0;
    virtual Query *query  (const char *,...);
    virtual IDE_RC execute(const char *,...);

    virtual bool isConnected( )           =0;

    virtual SChar * error   (void * =NULL)=0;
    virtual SInt    getErrNo(void)=0;
    inline  dba_t   getDbType(void) { return dbd->getDbType(); }

    virtual metaColumns * getMetaColumns(
        SChar *,       // Table Name
        SChar * = NULL // Schema Name default is UserName
        ) = 0;

    virtual IDE_RC     finalize  ();


    inline void   lock() { IDE_ASSERT(idlOS::mutex_lock  (&mLock) == 0 ); }
    inline void unlock() { IDE_ASSERT(idlOS::mutex_unlock(&mLock) == 0 ); }

   inline SChar* getSchema() { return mSchema; }
    inline metaColumns * getTCM() { return mTCM; }

protected: friend class Query;

    dbDriver *         dbd;  // Database Connection
    
    SChar *         mSchema; // UserName/Schema
    SChar *            mDSN; // Oracle server ID or DSN

    SInt            mErrNo;

    bool      malloc_error;
    SChar *         _error; // Pointer to Buffer
    UInt        _errorSize; // Buffer Size set by Self

    Query * mQuery; // Based Query - use for Direct Execute, For prepare add inside to List

    bool  mIsConnected;

    metaColumns * mTCM;
};


class Query : public Object
{

public:
    virtual ~Query();
    virtual IDE_RC clear   (void) =0;
    virtual IDE_RC close   (void) =0;
    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    // 단순히 커서를 닫는다.
    virtual IDE_RC utaCloseCur(void) =0;
    virtual IDE_RC reset   (void) =0;
    virtual IDE_RC prepare (void) =0;
    virtual IDE_RC prepare (const SChar*, ...);

    virtual IDE_RC execute (bool  = true)     =0;// exec and (convertToNative)
    virtual IDE_RC execute (const SChar*, ...)=0;// direct execution
    virtual IDE_RC lobAtToAt (Query *, Query *, SChar *);

    virtual IDE_RC assign  (const SChar*, ...)  ;// Format assign SQL

    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    virtual Row   *fetch   ( dba_t = DBA_ATB, bool = false ) =0;

    virtual IDE_RC bindColumn(UShort,SInt );

    /* BUG-32569 The string with null character should be processed in Audit */
    virtual IDE_RC bind(const UInt    // Position of Binding
                        , void * // Address  of Binding buffer
                        , UInt   // Size of buffer for binding
                        , SInt   // SQL Type of Date
                        , bool   // Indicator
                        , UInt   // Length of the binded data
                        )=0;

    virtual IDE_RC bind(const UInt    // Position of Binding
                        , Field * // Field Bind
                        );

    virtual IDE_RC finalize  ();
    virtual IDE_RC initialize(UInt = 0);

    virtual SInt    getSQLType(UInt);

    inline UInt   rows     (void) { return _rows; }
    inline UInt   rrows    (void) { UInt r=_rows;_rows=0;return r; }
    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    // csv file로부터 한row를 읽어온후 _rows를 증가시킴.
    inline void   utaIncRows(void) { _rows++; }

    inline UInt   columns  (void) { return _cols; }
    
    inline SChar *statement(void) { return  mSQL; }
    inline Row * getRow(void) { return mRow; }

    inline bool    isException() { return (mErrNo != 0); }
    inline SChar *error   (void) { return _conn->error()   ; }
    inline SInt   getErrNo(void) { return _conn->getErrNo(); }
    inline dba_t  getDbType(void) { return _conn->getDbType(); }
    inline Connection * getConn() { return _conn; }
    inline SChar * getLobDiffCol() { return lobDiffCol; }
    inline void    setLobDiffCol(SChar * colName) { lobDiffCol = colName; }
    bool     lobCompareMode;

    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    void   setArrayCount( SInt aArrayCount );
    IDE_RC setStmtAttr4Array( void );

protected: friend class Connection;

    Query( Connection * );

    Connection * _conn;
    SChar         mSQL  [QUERY_BUFSIZE];

    SInt       &mErrNo;

    UInt         _rows;
    UInt         _cols;

    Row       *     mRow;
    idBool   mIsPrepared;
    idBool   mIsCursorOpened;
    SChar *  lobDiffCol;
    // ** CallBack for format Row ** //
    rowToNative formatRow;
};

class Row : public Object
{
public:
    Row( SInt& );
    virtual  Field * getField(SChar * );
    virtual  Field * getField(UInt = 1);

    SInt    getSQLType(UInt  );
    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    SInt    getRealSqlType(UInt  );
    bool mIsLobCol;

    virtual  IDE_RC  bindColumn(UShort,SInt );


    virtual IDE_RC reset     (void);
    virtual IDE_RC initialize(void);
    virtual IDE_RC finalize  (void);


    inline virtual UShort size() { return        mCount; }
    inline bool    isException() { return (mErrNo != 0); }

    //TASK-4212     audit툴의 대용량 처리시 개선
    virtual IDE_RC setStmtAttr4Array() = 0;
    inline void    setArrayCount( SInt aArrayCount ) { mArrayCount = aArrayCount; }
    // 각 필드들의 filemode flag를 설정해준다.
    void setFileMode4Fields( bool aVal );

    //TASK-4212     audit툴의 대용량 처리시 개선
    //Array Fetch
    SInt                  mArrayCount;
    SQLUINTEGER           mRowsFetched;        //Fetch된 Row Count
    SQLUSMALLINT         *mRowStatusArray;     //Fetch된 Row의 Status

protected: friend class Query;
    SInt  &mErrNo;

    UShort         mCount;  // Columns count  in query
    UShort  mColumnBinded;  // Columns count alrady binding

    Field ** mField;  // Columns zero enter poin
};


class  Field : public Object
{
public:
    bool isName(SChar *);

    virtual IDE_RC bindColumn(SInt,void* = NULL); // mType SQL type of field
    virtual bool   isNull() = 0;
    //TASK-4212     audit툴의 대용량 처리시 개선
    virtual void   setIsNull(bool) = 0;
    virtual SInt   compareLogical(Field *f, idBool aUseFraction);
    virtual bool   comparePhysical(Field *f, idBool aUseFraction);  // BUG-17167

    virtual SInt   mapType (SInt aType)     { return aType  ; }
    inline  virtual SChar * getValue     () { return mValue ; }
    /* BUG-32569 The string with null character should be processed in Audit */
    inline  virtual UInt    getValueLength   () { return mValueLength ; }
    inline  virtual UInt    getValueSize () { return mWidth ; }
    inline  virtual SInt    getNativeType() { return mType  ; }
    inline  virtual SInt    getSQLType   () { return sqlType; }
    inline  virtual SInt    getRealSqlType   () { return realSqlType; }
    inline          SChar * getName      () { return mName  ; }

    /* BUG-32569 The string with null character should be processed in Audit */
    inline  void setValueLength(UInt aValue) { mValueLength = aValue; }
    //TASK-4212     audit툴의 대용량 처리시 개선
    inline  void setFileMode( bool aVal)  { mIsFileMode = aVal; }

    IDE_RC initialize(UShort , Row* ); // Column number 1..N order

    virtual SInt   getSChar(       SChar *, UInt);
    static  SInt   getSChar(SInt , SChar *, UInt, SChar*, UInt);
    static IDE_RC  makeAtbDate(SQL_TIMESTAMP_STRUCT * aFrom, mtdDateType * aTo);

protected: friend class Row;
    /* Name description */
    SChar           mName[UTDB_MAX_COLUMN_NAME_LEN+1];     // Field Name
    UShort       mNameLen;       // Field Length
    UShort            mNo;       // Field No in order from 1..N
    SQLUBIGINT    mLobLoc;
    
    SChar     *    mLinks;       // Descriptor/Locator
    SChar     *    mValue;       // Field data buffer pointer

    /* BUG-32569 The string with null character should be processed in Audit */
    UInt           mValueLength;       // Field Length
    UInt           mWidth;       // Field Length

    SInt          sqlType;       // SQL DataType
    SInt      realSqlType;       // real SQL DataType
    SInt            mType;       // Native DataType Code

    Row       *      mRow;

    //TASK-4212     audit툴의 대용량 처리시 개선
    bool      mIsFileMode;
    virtual IDE_RC finalize  (void);
};

inline bool isSpecialCharacter(SChar * aStr)
{
    UInt    i;
    SChar * sStr;
    /* permitted special characters */
    SChar   sSpecialCharacter[] = 
    {' ', '~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '+', '|'};

    sStr = aStr;
    *(sStr + idlOS::strlen(sStr)) = '\0';
    for(; *sStr != '\0'; sStr++)
    {
        for(i = 0; i < sizeof(sSpecialCharacter); i++)
        {
            if(*sStr == sSpecialCharacter[i])
            {
                return ID_TRUE;
            }
        }
    }
    return ID_FALSE;
}

IDE_RC printSelect( Query *  );


#endif /*_UT_DB_H_*/
