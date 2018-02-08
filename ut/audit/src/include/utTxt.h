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
 * $Id: utTxt.h 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#ifndef _UT_SQL_TXT_H_
#define _UT_SQL_TXT_H_ 1

#include <utdb.h>

class utTxtConnection;
class utTxtQuery;
class utTxtRow;
class utTxtField;


class utTxtConnection : public Connection
{

    SChar    mDSN[512];
    FILE   * fd;

    const SChar  *mExt;


public:
    utTxtConnection(dbDriver *aDbd);
    ~utTxtConnection(void);

    IDE_RC  connect( const SChar * ,...);
    IDE_RC  connect();

    IDE_RC disconnect(void);

    IDE_RC autocommit(bool = true); /* on by default in init() */

    IDE_RC commit  (void);
    IDE_RC rollback(void);

    Query *query(void);

    bool isConnected( );

    metaColumns * getMetaColumns(SChar*,SChar*);


    SInt    getErrNo(void) { return mErrNo; }
    SChar   *error(void * = NULL);
    dba_t   getDbType(void);

    IDE_RC     initialize(SChar* = NULL,UInt = 0);
    IDE_RC     finalize  ();

    SInt  checkState(  SInt, SQLHSTMT = NULL );

    inline FILE * file() { return fd; };

protected: friend class utTxtQuery;
    friend class utTxtRow  ;
    friend class utTxtField;

    SInt          serverStatus;
};


class utTxtQuery : Query
{
    IDE_RC  nativeSQL();

public:
    utTxtQuery( utTxtConnection *);
    IDE_RC initialize(UInt = 0);


    IDE_RC close   (void);
    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    IDE_RC utaCloseCur(void);
    IDE_RC clear   (void);
    IDE_RC reset   (void);
    IDE_RC prepare (void);

    Row   *fetch   (dba_t, bool );
    UInt   rows    (void);

    IDE_RC execute (bool=true         );
    IDE_RC execute (const SChar*, ... );    // direct execution

    IDE_RC bindColumn(UShort,SInt);
    /* not implemented
    IDE_RC bind(const UInt   , void *, UInt, SInt  // SQL Type aka SQL_VARCHAR etc.
                , bool isNull // for Indicator // for Indicator
                ,SQLSMALLINT  // SQL_PARAM_INPUT/OUTUT etc.
                ,SQLSMALLINT  // C Type parametr
                );
    */

    IDE_RC bind(const UInt i, void *buf, UInt bufLen, SInt type, bool isNull);

    /* BUG-32569 The string with null character should be processed in Audit */
    IDE_RC bind(const UInt i, void *buf, UInt bufLen, SInt type, bool isNull, UInt aValueLength);
    inline IDE_RC bind(const SChar *, void *, UInt, SInt, bool) { return IDE_FAILURE; }


    IDE_RC finalize  ();

protected:  friend class utTxtConnection;
    friend class utTxtRow       ;
    friend class utTxtField     ;


    //** from utTxtcleConnect **//
    utTxtConnection *   _conn;

    struct binds_t
    {
        SChar         *value; // IN values pointer
        UInt        v_length; // IN values length

        SInt         sqlType; // SQL ODBC

        UInt        b_length;
        SChar        buff[1];
    };

    binds_t    **_binds; // binds array
    UInt         _bsize; // binds size array


    SChar        *mBuff; // string buffer
    UInt       mBufSize; // length size

    IDE_RC alloc(binds_t *&sBind, UInt aLength, SInt sqlType);
    IDE_RC freeBinds();

};

class  utTxtRow : Row
{
public:
    utTxtRow( utTxtQuery *, SInt &); // import from Query

    IDE_RC initialize();

protected:  friend class utTxtQuery;
    friend class utTxtField;
    utTxtQuery   *mQuery;
};

class utTxtField : Field
{
public:

/// IDE_RC  getSChar(SChar *,UInt );
    IDE_RC  bindColumn( SInt);

    inline bool isNull() { return false; }
    inline void setIsNull(bool) { }

protected:  friend class utTxtRow;

    IDE_RC initialize( UShort aNo ,utTxtRow * aRow);
    IDE_RC finalize  (void);

    utTxtRow        *  mRow;

    inline SInt mapType (SInt aSqlType) { return aSqlType; }
};

#endif
