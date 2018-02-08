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
 * $Id: utOra.h 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/
#ifndef _UT_ORACLE_H_
#define _UT_ORACLE_H_ 1

#include <utdb.h>
#include <oci.h>

#define NUM_FMT  "FM99999999999999999999999999999999999D999999999999999999999999999"
#define LANG     ""

#define TIMESTAMP_FMT    "YYYY-MM-DD HH24:MI:SS.FF6"
#define TIMESTAMP_TZ_FMT "YYYY-MM-DD HH24:MI:SS.FF6"
#define DATE_FMT "YYYY-MM-DD HH24:MI:SS"

#ifndef	SQL_NULL
#define	SQL_NULL	0
#endif

extern "C"
{
    ub2  sqlTypeToOracle(SInt );
    SInt oraTypeToSql   (ub2  );

    IDE_RC oraOCIDateToDATETIME(void *, void *, OCIError *, OCISession *);

    IDE_RC atbToOraOCIDate   (void* ,void* ,sb2 &,OCIError*,OCISession *);
    IDE_RC atbToOraOCIString (void* ,void* ,sb2 &);
};

/* copy of OCITime but with fraction of seconds */
struct ORATime
{
    ub1       hh; /* hours;   range is 0 <= hours   <= 23 */
    ub1       mi; /* minutes; range is 0 <= minutes <= 59 */
    ub1       ss; /* seconds; range is 0 <= seconds <= 59 */
    ub4       fs; /* fraction of second nanoseconds       */
    ub2       tz;
};

typedef struct ORATime ORATime;

struct ORADate
{
    sb2       yy; /* gregorian year; range is -4712 <= year <= 9999 */
    ub1       mm; /* month; range is 1 <= month < 12 */
    ub1       dd; /* day; range is 1 <= day <= 31 */
    ORATime   ts; /* time stamp */
};


inline IDE_RC oci_check( SInt aErrNo)
{
    switch( aErrNo )
    {
        case OCI_SUCCESS:
        case OCI_SUCCESS_WITH_INFO:
        case OCI_NO_DATA:
            return IDE_SUCCESS;
        default:
            return IDE_FAILURE;
    }
}

class utOraConnection;
class utOraQuery;
class utOraRow;
class utOraField;



class utOraConnection : public Connection
{
public:
    static OCIEnv *envhp;         /* Environment handle     */

    utOraConnection(dbDriver *dbd = NULL);
    ~utOraConnection(void);



    IDE_RC    connect( const SChar *, ... );
    IDE_RC    connect(                    );
    IDE_RC disconnect(void                );

    IDE_RC autocommit (bool);
    IDE_RC commit     (void);
    IDE_RC rollback   (void);


    Query *query(void);

    bool isConnected( );


    metaColumns * getMetaColumns(SChar*,SChar*);

    SChar   *  error(void * = NULL);
    SInt    getErrNo(void);
    dba_t   getDbType(void);

    IDE_RC     initialize(SChar* = NULL,UInt = 0);
    IDE_RC     finalize  ();

    SInt checkState(  SInt );

    OCIEnv    * getEnvhp(){ return envhp; }
    OCISvcCtx * getSvchp(){ return svchp; }
    OCIServer * getSrvhp(){ return srvhp; }
    OCIError  * getErrhp(){ return errhp; }
    OCISession* getUsrhp(){ return usrhp; }

protected: friend class utOraQuery;
    friend class utOraRow  ;
    friend class utOraField;

    ub4      serverStatus; // Oracle server status
    ub4         _execmode;

    OCISvcCtx      *svchp;         /* Service context handle */
    OCIServer      *srvhp;         /* Server handles         */
    OCIError       *errhp;         /* Error handle           */
    OCISession     *usrhp;         /* User handle            */

};




class utOraQuery : Query
{
public:
    utOraQuery( utOraConnection *);

    IDE_RC close   (void);
    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    IDE_RC utaCloseCur(void);
    IDE_RC clear   (void);
    IDE_RC reset   (void);
    IDE_RC prepare (void);

    IDE_RC execute (bool = true      ); // exec prepared with convert to native binds
    IDE_RC execute (const SChar*, ...); // direct execution

    Row   *fetch   ( dba_t, bool);
    UInt   rows    (void);

    IDE_RC bind(const SChar *, void *, UInt, SInt, bool);
    IDE_RC bind(const UInt   , void *, UInt, SInt, bool);

    // BUG-32569: added aValueLength argument
    IDE_RC bind(const UInt i, void *buf, UInt bufLen, SInt type, bool isNull, UInt aValueLength);

    IDE_RC initialize(UInt = 0);
    IDE_RC finalize  ();

    OCIStmt * getStmt() { return _stmt; }

protected:  friend class utOraConnection;
    friend class utOraRow       ;
    friend class utOraField     ;

    //** from utOracleConnect **//
    utOraConnection * _conn;

    IDE_RC nativeSQL();

    OCIStmt   *_stmt;
    ub2        _type;
    ub4     _typelen;

// structure of binds //
    struct binds_t
    {
        binds_t *  next;

        void    * links; // Descriptor/property
        UInt      linksLength;

        void    * value; // Buffer of data
        UInt      valueLength;

        SInt     sqlType;
        OCIBind    *bind;
        sb2          ind;   // Indicator

    };

    IDE_RC alloc(binds_t **,SInt sqlType);
    void  free(binds_t *);

    IDE_RC callbackToNative( binds_t * ); // convert to Native format
    binds_t    * _binds;
    utOraQuery *  mNext;
};

class  utOraRow : Row
{
public:
    utOraRow( utOraQuery *, OCIError *&, SInt &); // import from Query

    IDE_RC initialize();

//Field * getField(UInt   );
    IDE_RC setStmtAttr4Array(void);

protected:  friend class utOraQuery;
    friend class utOraField;
    IDE_RC                   Ora2Atb();

    utOraQuery   *mQuery;
    OCIStmt      *&_stmt;
    OCIError     *&_error;

};

class utOraField : Field
{
public:

    IDE_RC  bindColumn( SInt,void* = NULL);
    //  SInt    compare(Field * );

    // BUG-32569
    inline ub2  getValueLen() { return (mValueLen); }

    inline bool isNull() { return (isnull == -1); }
    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    inline void setIsNull(bool aIsNull) { isnull = (aIsNull)?-1:0; }

protected:  friend class utOraRow;

    utOraRow  *mRow      ; // Row Owner

    OCIDefine *mOCIDefine; // defenition pointer
    OCIParam      *mParam; // define parameter
    sb2             isnull; // Oracle indicator
    ub2             mValueLen; // BUG-32569

    IDE_RC finalize  (void);
    IDE_RC initialize(UShort      // Column Number (ID 1..N )
                      ,utOraRow*); // Link to Owned Row

    inline
    SInt mapType(SInt sqlType) { return (SInt)sqlTypeToOracle( sqlType);  };
};
#endif
