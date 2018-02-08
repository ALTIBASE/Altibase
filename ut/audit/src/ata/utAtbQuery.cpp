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
 * $Id: utAtbQuery.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <uto.h>
#include <utAtb.h>

IDE_RC  utAtbQuery::prepare()
{
    //IDE_TEST(mSQL[0] == '\0');
    if(mIsPrepared == ID_TRUE)
    {
        return IDE_SUCCESS;
    }

    IDE_TEST(reset() != IDE_SUCCESS);
    IDE_TEST_RAISE(SQL_ERROR == SQLPrepare(_stmt, (SQLCHAR *)mSQL, SQL_NTS),
                   SQLPREPARE_ERR);

    mIsPrepared = ID_TRUE;

    return IDE_SUCCESS;

    IDE_EXCEPTION(SQLPREPARE_ERR)
    {
        mErrNo            = SQL_ERROR;
        _conn->error_stmt = _stmt;
        _conn->error(_stmt);
    }
    IDE_EXCEPTION_END;

    mIsPrepared = ID_FALSE;

    return IDE_FAILURE;
}


Row * utAtbQuery::fetch(dba_t, bool aFileMode)
{
    SInt  ret = SQL_INVALID_HANDLE;

    /* BUG-32569 The string with null character should be processed in Audit */
    SInt  sI;
    UInt  sValueLength;
    Field *sField;

    IDE_TEST(mRow == NULL);
    IDE_TEST(mRow->reset() != IDE_SUCCESS);
    ret = SQLFetch(_stmt);

    /* BUG-32569 The string with null character should be processed in Audit */
    for( sI = 1, sField = mRow->getField(1); sField != NULL; sField = mRow->getField(++sI) )
    {
        sValueLength = (UInt)(*((utAtbField *)sField)->getValueInd());
        sField->setValueLength(sValueLength);
    }

    IDE_TEST(!SQL_SUCCEEDED(ret));

    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    if ( aFileMode == true )
    {
        // do nothing
    }
    else
    {
        ++_rows;
    }

    return mRow;

    IDE_EXCEPTION_END;

    /* BUG-39193 'Unable to retrieve error information from CLI driver' is written
     * as a error message instead of exact one in log file. */
    if (ret != SQL_NO_DATA_FOUND)
    {
        mErrNo            = SQL_ERROR;
        _conn->error_stmt = _stmt;
        _conn->error(_stmt);
    }

    return NULL;
}

IDE_RC utAtbQuery::initialize(UInt)
{
    IDE_TEST(SQLAllocStmt(_conn->dbchp, &_stmt) != SQL_SUCCESS);

    mIsCursorOpened = ID_FALSE;
    // BUG-40205 insure++ warning 어떤 값으로 초기값 설정???
    lobCompareMode  = ID_FALSE;

    return Query::initialize();

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utAtbQuery::close()
{
    //mSQL[0] = '\0';
    if(mIsCursorOpened == ID_TRUE || mIsPrepared == ID_TRUE)
    {
        IDE_TEST(SQLFreeStmt(_stmt, SQL_CLOSE) == SQL_ERROR);
        IDE_TEST(SQLFreeStmt(_stmt, SQL_UNBIND) == SQL_ERROR);
        IDE_TEST(SQLFreeStmt(_stmt, SQL_RESET_PARAMS) == SQL_ERROR);
        mIsPrepared = ID_FALSE;
        mIsCursorOpened = ID_FALSE;
    }

    return reset();

    IDE_EXCEPTION_END;

    reset();

    return IDE_FAILURE;
}


/* TASK-4212: audit툴의 대용량 처리시 개선 */
IDE_RC utAtbQuery::utaCloseCur(void)
{
/***********************************************************************
 *
 * Description :
 *    statement를 해제함. SQL_CLOSE, SQL_UNBIND, SQL_RESET_PARAMS 해준다.
 *
 ***********************************************************************/

    if(mIsCursorOpened == ID_TRUE || mIsPrepared == ID_TRUE)
    {
        IDE_TEST(SQLFreeStmt(_stmt, SQL_CLOSE)  == SQL_ERROR);
        IDE_TEST(SQLFreeStmt(_stmt, SQL_UNBIND) == SQL_ERROR);
        IDE_TEST(SQLFreeStmt(_stmt, SQL_RESET_PARAMS) == SQL_ERROR);
        mIsPrepared     = ID_FALSE;
        mIsCursorOpened = ID_FALSE;
    }

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}


IDE_RC utAtbQuery::clear()
{
    mSQL[0] = '\0';
    if(mIsPrepared == ID_TRUE)
    {
        IDE_TEST(SQLFreeStmt(_stmt, SQL_DROP) == SQL_ERROR);
        mIsPrepared = ID_FALSE;
    }

    return reset();

    IDE_EXCEPTION_END;

    reset();

    return IDE_FAILURE;
}


IDE_RC utAtbQuery::execute(bool)
{
    SQLLEN sRowCount = 0;

    if(mIsPrepared != ID_TRUE)
    {
        IDE_TEST(prepare() != IDE_SUCCESS);
    }

    /* Execute process */
    IDE_TEST(SQLExecute(_stmt) == SQL_ERROR);

    if(_rescols == 0)
    {
        IDE_TEST(SQL_ERROR == SQLNumResultCols(_stmt,(SQLSMALLINT *)&_rescols));

        if(_rescols == 0)
        {
            _rescols = -1; //set NO RES ROWS
            _cols    =  0;
        }
        else
        {
            _cols = _rescols;
            if(mRow == NULL)
            {
                mRow = new utAtbRow(this,mErrNo);
                IDE_TEST(mRow == NULL);
                IDE_TEST(mRow->initialize() != IDE_SUCCESS);
            }
        }
    }

    // BUG-18732 : close for DML
    IDE_TEST(SQLRowCount(_stmt, &sRowCount) == SQL_ERROR);
    if(sRowCount != -1) // PROJ-2396, 2370
    {
        IDE_TEST(close() != IDE_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if(mErrNo == SQL_SUCCESS)
    {
        mErrNo            = SQL_ERROR;
        _conn->error_stmt = _stmt;
        _conn->error(_stmt);
    }

    return IDE_FAILURE;
}


IDE_RC utAtbQuery::execute(const SChar * aSQL, ...)
{
    va_list args;

    IDE_TEST(aSQL == NULL);
    IDE_TEST(close() != IDE_SUCCESS);

    va_start(args, aSQL);
#if defined(DEC_TRU64) || defined(VC_WIN32)
    IDE_TEST(vsprintf(mSQL, aSQL, args) < 0);
#else
    IDE_TEST(vsnprintf(mSQL, sizeof(mSQL)-1, aSQL, args) < 0);
#endif
    va_end(args);

    IDE_TEST(SQLExecDirect(_stmt, (SQLCHAR *)mSQL, SQL_NTS) == SQL_ERROR);

    IDE_TEST(SQL_ERROR == SQLNumResultCols (_stmt,(SQLSMALLINT *)&_rescols));

    if(_rescols)
    {
        _cols = _rescols;
        if(mRow == NULL)
        {
            mRow = new utAtbRow(this,mErrNo);
            IDE_TEST(mRow == NULL);
            IDE_TEST(mRow->initialize() != IDE_SUCCESS);
        }
    }
    if(mIsCursorOpened == ID_FALSE)
    {
        mIsCursorOpened = ID_TRUE;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    mErrNo            = SQL_ERROR;
    _conn->error_stmt = _stmt;
    _conn->error(_stmt);

    return IDE_FAILURE;
}

IDE_RC utAtbQuery::lobAtToAt(Query* aGetLob, Query * aPutLob, SChar * tblName)
{
    UInt         i, lobColCount, fromPosition;
    SInt         tempCount, diff; 
    SChar        selectSql[QUERY_BUFSIZE], temp[BUF_LEN];
    SChar       *lobColName, *colName, *sTblName = NULL;
    metaColumns *sTCM;
    Field       *f;
    
    SQLHSTMT     getLobStmt;
    SQLHSTMT     putLobStmt;
    SQLHDBC      getConn;
    SQLHDBC      putConn;

    SQLPOINTER   buf[BUF_LEN], compareBuf[BUF_LEN];
    SQLUINTEGER   getForLength, putForLength;

    SInt         sSqlType[MAX_COL_CNT];
    SInt         locatorCType[MAX_COL_CNT];
    SQLSMALLINT  sourceCType[MAX_COL_CNT];
    SQLUBIGINT   getLobLoc[MAX_COL_CNT], putLobLoc[MAX_COL_CNT];
    SQLUINTEGER   getLobLength[MAX_COL_CNT], putLobLength[MAX_COL_CNT];
    SQLINTEGER   ALobLength[MAX_COL_CNT], BLobLength[MAX_COL_CNT];
    
    /* Get Meta Info */
    if(!(sTCM = aGetLob->getConn()->getTCM()))
    {
        sTblName = tblName;
        sTCM = aPutLob->getConn()->getTCM();
    }
    
    if(sTCM == NULL)
    {
        return IDE_FAILURE;
    }

    lobColCount = sTCM->getCLSize(true);

    /**********************************************/
    /* GETLOB PREPARE                             */
    /**********************************************/

    /* Select Query */
    idlOS::strcpy(selectSql, "SELECT ");

    for(i = 1; i <= lobColCount; i ++)
    {
        idlOS::strcat(selectSql, sTCM->getCL(i, true));
        if(i != lobColCount)
        {
            idlOS::strcat(selectSql, ", ");
        }
    }
    strcat(selectSql, " FROM ");
    strcat(selectSql, (sTblName == NULL) ? sTCM->getTableName() : sTblName);
    strcat(selectSql, " WHERE ");

    for(i = sTCM->getPKSize(); sTCM->getPK(i); --i)
    {
        idlOS::strcat(selectSql, sTCM->getPK(i));
        idlOS::strcat(selectSql," = ");
        
        IDE_ASSERT(aGetLob->getRow()->getField(i)->getSChar(temp, BUF_LEN) != -1);
        idlOS::strcat(selectSql, temp);
        if(i > 1)
        {
            idlOS::strcat(selectSql," AND ");
        }
    }
    
    strcat(selectSql, " FOR UPDATE");
    i = idlOS::strlen(selectSql);
    selectSql[i] = '\0';
    
    /* Fetch LOB */
    getConn = aGetLob->getConn()->getDbchp();

    IDE_TEST(SQLAllocStmt(getConn, &getLobStmt) != SQL_SUCCESS);

    IDE_TEST(SQLExecDirect(getLobStmt, (SQLCHAR*)selectSql, SQL_NTS)
             != SQL_SUCCESS);

    for(tempCount = 0; tempCount < (SInt)lobColCount; tempCount ++)
    {
        sSqlType[tempCount] = 0;
        for(i = 1, f = aGetLob->getRow()->getField(1);
            f; f = aGetLob->getRow()->getField(++i))
        {
            lobColName = sTCM->getCL(tempCount + 1, true);
            colName = f->getName();
            if(idlOS::strcmp(lobColName, colName) == 0)
            {
                sSqlType[tempCount] = f->getSQLType();
                break;
            }
        }
        if(sSqlType[tempCount] == SQL_BLOB)
        {
            locatorCType[tempCount] = SQL_C_BLOB_LOCATOR;
            sourceCType[tempCount] = SQL_C_BINARY;
        }
        else if(sSqlType[tempCount] == SQL_CLOB)
        {
            locatorCType[tempCount] = SQL_C_CLOB_LOCATOR;
            sourceCType[tempCount] = SQL_C_CHAR;
        }
        else
        {
            return IDE_FAILURE;
        }
        IDE_TEST(SQLBindCol(getLobStmt,
                            tempCount + 1,
                            locatorCType[tempCount],
                            &getLobLoc[tempCount],
                            0,
                            NULL)
                 != SQL_SUCCESS);
    }

    if (SQLFetch(getLobStmt) != SQL_SUCCESS)
    {
        return IDE_FAILURE;
    }

    
    /* LOBLength for GET */

    for(tempCount = 0; tempCount < (SInt)lobColCount; tempCount ++)
    {
        /* BUG-30301 */
        IDE_TEST(SQLGetLobLength(getLobStmt,
                                 getLobLoc[tempCount],
                                 locatorCType[tempCount],
                                 &getLobLength[tempCount])
                 != SQL_SUCCESS);
    }

    /**********************************************/
    /* PUTLOB PREPARE                             */
    /**********************************************/

    /* LobLocator */

    /* Select Query */
    idlOS::strcpy(selectSql, "SELECT ");

    for(i = 1; i <= lobColCount; i ++)
    {
        idlOS::strcat(selectSql, sTCM->getCL(i, true));
        if(i != lobColCount)
        {
            idlOS::strcat(selectSql, ", ");
        }
    }
    strcat(selectSql, " FROM ");
    strcat(selectSql, (sTblName == NULL) ? sTCM->getTableName() : sTblName);
    strcat(selectSql, " WHERE ");

    for(i = sTCM->getPKSize(); sTCM->getPK(i); --i)
    {
        idlOS::strcat(selectSql, sTCM->getPK(i));
        idlOS::strcat(selectSql," = ");
        
        /* 
         * BUG-32566
         *
         * Query가 잘못되어 LOB Column Select 안되는 문제 수정
         * 디버깅 중 실수로 주석처리 하지 않았을까????
         */
        IDE_ASSERT(aGetLob->getRow()->getField(i)->getSChar(temp, BUF_LEN) != -1);
        idlOS::strcat(selectSql, temp);
        if(i > 1)
        {
            idlOS::strcat(selectSql," AND ");
        }
    }
    
    strcat(selectSql, " FOR UPDATE");
    i = idlOS::strlen(selectSql);
    selectSql[i] = '\0';

    /* Fetch LOB */
    putConn = aPutLob->getConn()->getDbchp();

    IDE_TEST(SQLAllocStmt(putConn, &putLobStmt) != SQL_SUCCESS);

    if(SQLExecDirect(putLobStmt, (SQLCHAR*)selectSql, SQL_NTS) != SQL_SUCCESS)
    {
        return IDE_FAILURE;
    }

    for(tempCount = 0; tempCount < (SInt)lobColCount; tempCount ++)
    {
        lobColName = sTCM->getCL(tempCount + 1, true);
        for(i = 1, f = aPutLob->getRow()->getField(1); f;
            f = aPutLob->getRow()->getField(++i))
        {
            colName = f->getName();
            if(idlOS::strcmp(lobColName, colName) == 0)
            {
                sSqlType[tempCount] = f->getSQLType();
                break;
            }
        }
        if(sSqlType[tempCount] == SQL_BLOB)
        {
            locatorCType[tempCount] = SQL_C_BLOB_LOCATOR;
            sourceCType[tempCount] = SQL_C_BINARY;
        }
        else if(sSqlType[tempCount] == SQL_CLOB)
        {
            locatorCType[tempCount] = SQL_C_CLOB_LOCATOR;
            sourceCType[tempCount] = SQL_C_CHAR;
        }
        else
        {
            return IDE_FAILURE;
        }
        IDE_TEST(SQLBindCol(putLobStmt,
                            tempCount + 1,
                            locatorCType[tempCount],
                            &putLobLoc[tempCount],
                            0,
                            NULL)
                 != SQL_SUCCESS);
    }

    if (SQLFetch(putLobStmt) != SQL_SUCCESS)
    {
        return IDE_FAILURE;
    }

    /* LOBLength for PUT */
    for(tempCount = 0; tempCount < (SInt)lobColCount; tempCount ++)
    {
        if(SQLGetLobLength(putLobStmt,
                           putLobLoc[tempCount],
                           locatorCType[tempCount],
                           &putLobLength[tempCount])
           != SQL_SUCCESS)
        {
           return IDE_FAILURE;
        }
    }

    //Lob Compare Mode
    if(aGetLob->lobCompareMode || aPutLob->lobCompareMode) 
    {
        for(tempCount = 0; tempCount < (SInt)lobColCount; tempCount ++)
        {
            fromPosition = 0;
            getForLength = 0;
            if(getLobLength[tempCount] != putLobLength[tempCount])
            {
                aGetLob->setLobDiffCol(sTCM->getCL(tempCount + 1, true));
                aPutLob->setLobDiffCol(sTCM->getCL(tempCount + 1, true));
                break;
            }
            ALobLength[tempCount] = getLobLength[tempCount];
            BLobLength[tempCount] = putLobLength[tempCount];

            while((ALobLength[tempCount] > 0) && (BLobLength[tempCount] > 0))
            {
                idlOS::memset(buf, 0x00, sizeof(buf));
                idlOS::memset(compareBuf, 0x00, sizeof(compareBuf));

                getForLength = (ALobLength[tempCount] <= BUF_LEN)
                    ? ALobLength[tempCount] : BUF_LEN;

                putForLength = (ALobLength[tempCount] <= BUF_LEN)
                    ? putLobLength[tempCount] : BUF_LEN;

                /* GET LOB A */
                IDE_TEST(SQLGetLob(getLobStmt,
                                   locatorCType[tempCount],
                                   getLobLoc[tempCount],
                                   fromPosition,
                                   getForLength,
                                   sourceCType[tempCount],
                                   buf,
                                   BUF_LEN,
                                   &getForLength)
                         != SQL_SUCCESS);
                /* GET LOB B */
                IDE_TEST(SQLGetLob(putLobStmt,
                                   locatorCType[tempCount],
                                   putLobLoc[tempCount],
                                   fromPosition,
                                   putForLength,
                                   sourceCType[tempCount],
                                   compareBuf,
                                   BUF_LEN,
                                   &getForLength)
                         != SQL_SUCCESS);

                diff = idlOS::memcmp(buf,compareBuf, BUF_LEN);
                if(diff != 0)
                {
                    aGetLob->setLobDiffCol(sTCM->getCL(tempCount + 1, true));
                    aPutLob->setLobDiffCol(sTCM->getCL(tempCount + 1, true));
                    break;
                }
                fromPosition += getForLength;
                ALobLength[tempCount] -= getForLength;
                BLobLength[tempCount] -= putForLength;
            }
        }
    }
    //Lob Update Mode
    else
    {
        /* Initialize target */
        idlOS::memset(buf, 0x00, sizeof(buf));
        for(tempCount = 0; tempCount < (SInt)lobColCount; tempCount ++)
        {
            IDE_TEST(SQLPutLob(putLobStmt,
                               locatorCType[tempCount],
                               putLobLoc[tempCount],
                               0,
                               putLobLength[tempCount],
                               sourceCType[tempCount],
                               buf,
                               0)
                     != SQL_SUCCESS);
        }

        /**********************************************/
        /* GETLOB   &   PUTLOB                        */
        /**********************************************/

        for(tempCount = 0; tempCount < (SInt)lobColCount; tempCount ++)
        {
            fromPosition = 0;
            getForLength = 0;

            while(getLobLength[tempCount] > 0)
            {
                getForLength = (getLobLength[tempCount] <= BUF_LEN)
                    ? getLobLength[tempCount] : BUF_LEN;

                putForLength = (putLobLength[tempCount] <= BUF_LEN)
                    ? putLobLength[tempCount] : BUF_LEN;

                /* GET LOB */
                IDE_TEST(SQLGetLob(getLobStmt,
                                   locatorCType[tempCount],
                                   getLobLoc[tempCount],
                                   fromPosition,
                                   getForLength,
                                   sourceCType[tempCount],
                                   buf,
                                   BUF_LEN,
                                   &getForLength)
                         != SQL_SUCCESS);
                /* PUT LOB */
                IDE_TEST(SQLPutLob(putLobStmt,
                                   locatorCType[tempCount],
                                   putLobLoc[tempCount],
                                   fromPosition,
                                   0,
                                   sourceCType[tempCount],
                                   buf,
                                   getForLength)
                         != SQL_SUCCESS);

                fromPosition += getForLength;
                getLobLength[tempCount] -= getForLength;
            }
        }
    }
    
    IDE_TEST(SQLFreeStmt(getLobStmt, SQL_DROP) != SQL_SUCCESS);
    IDE_TEST(SQLFreeStmt(putLobStmt, SQL_DROP) != SQL_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


IDE_RC utAtbQuery::reset()
{
    binds_t * binds;
    _rescols = 0;

    IDE_TEST(_stmt == NULL);

    for(binds = _binds; binds; binds = _binds)
    {
        _binds = binds->next;
        idlOS::free(binds);
    }

    //*** Delete Row ***/
    if(mRow)
    {
        IDE_TEST(mRow->finalize() != IDE_SUCCESS);
        delete mRow;
        mRow = NULL;
    }
    _rows = 0;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* BUG-32569 The string with null character should be processed in Audit */
IDE_RC utAtbQuery::bind(const UInt i, void *buf, UInt bufLen, SInt sqlType, bool isNull, UInt aValueLength)
{
    SQLSMALLINT sCType = sqlTypeToAltibase(sqlType);

    if(sqlType == SQL_TYPE_TIMESTAMP)
    {
        bufLen = sizeof(SQL_TIMESTAMP_STRUCT);
    }

    /* BUG-32569 The string with null character should be processed in Audit */
    return bind(i, buf, bufLen, sqlType, isNull, SQL_PARAM_INPUT, sCType, aValueLength);
}

/* BUG-32569 The string with null character should be processed in Audit */
IDE_RC utAtbQuery::bind(const UInt aPosition, void *aBuff,UInt aWidth, SInt sqlType,
                        bool isNull ,SQLSMALLINT pType ,SQLSMALLINT sCType, UInt aValueLength)
{
    SInt locatorType = 0;
    SInt locatorCType = 0;
    binds_t * sBinds   = NULL;
    
    if(mIsPrepared != ID_TRUE)
    {
        IDE_TEST(prepare() != IDE_SUCCESS);
    }

    sBinds = (binds_t*)idlOS::calloc(1,sizeof(binds_t));

    IDE_TEST(sBinds == NULL);
    sBinds->value         = aBuff  ;
    sBinds->pType         = pType  ;
    sBinds->sqlType       = sqlType;
    sBinds->scale         =  0     ;
    sBinds->columnSize    =  aWidth; // disable aWidth

    switch(sCType)
    {
        /* BUG-32569 The string with null character should be processed in Audit */
        case SQL_C_CHAR: sBinds->valueLength = aValueLength; break;
        default: sBinds->valueLength =  (SQLLEN)aWidth; break;
    }
    if (isNull)
    {
        sBinds->valueLength = SQL_NULL_DATA;
    }

    if(sqlType == SQL_BLOB || sqlType == SQL_CLOB)
    {
        if(sqlType == SQL_BLOB)
        {
            locatorType = SQL_BLOB_LOCATOR;
            locatorCType = SQL_C_BLOB_LOCATOR;        
        }
        else
        {
            locatorType = SQL_CLOB_LOCATOR;
            locatorCType = SQL_C_CLOB_LOCATOR;        
        }
        /* BUG-40205 insure++ warning
         * LOB의 경우 SQL_PARAM_OUTPUT으로 설정하여 locator를 받아온 후,
         * SQLPubLob에서 데이터를 입력해야 함: manual 참조
         * 여기에서 SQLBindParameter에 사용한 locator를 실제로는 사용하지 않으며,
         * utAtbQuery::lobAtToAt 함수에서 lob 칼럼을 별도로 처리하고 있음.
         */
        pType = SQL_PARAM_OUTPUT;

        mErrNo = SQLBindParameter(_stmt,
                                  (SQLUSMALLINT)aPosition,
                                  (SQLSMALLINT) pType, //TODO get from statement ????
                                  (SQLSMALLINT) locatorCType, // C type parametr        ???
                                  (SQLSMALLINT) locatorType, // SQL TYPE
                                  0, // column size
                                  0, // Scale
                                  &mLobLoc,
                                  0,
                                  &mLobInd); 
    }
    else
    {
        mErrNo = SQLBindParameter(_stmt,
                                  (SQLUSMALLINT)aPosition,
                                  (SQLSMALLINT) pType, //TODO get from statement ????
                                  (SQLSMALLINT) sCType, // C type parametr        ???
                                  (SQLSMALLINT) sqlType, // SQL TYPE
                                  (SQLULEN) sBinds->columnSize, // column size
                                  (SQLSMALLINT) sBinds->scale, // Scale
                                  (SQLPOINTER)  aBuff,
                                  (SQLLEN)  aWidth,
                                  &sBinds->valueLength);
    }

    IDE_TEST(mErrNo == SQL_ERROR);

    sBinds->next  =   _binds;
    _binds        =   sBinds;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if(sBinds != NULL)
    {
        idlOS::free(sBinds);
    }

    return IDE_FAILURE;
}


IDE_RC utAtbQuery::finalize()
{
    if (_stmt)
    {
        IDE_TEST(reset() != IDE_SUCCESS);
        IDE_TEST(SQLFreeStmt(_stmt, SQL_DROP) == SQL_ERROR);
        _stmt = NULL;
    }

    return Query::finalize();

    IDE_EXCEPTION_END;

    _stmt = NULL;

    return IDE_FAILURE;
}


utAtbQuery::utAtbQuery(utAtbConnection * conn) : Query(conn)
{
    _conn     = conn;
    _stmt     = NULL;
    _binds    = NULL;
}
