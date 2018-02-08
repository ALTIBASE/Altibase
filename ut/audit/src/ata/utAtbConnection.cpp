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
 * $Id: utAtbConnection.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <uto.h>
#include <utAtb.h>

// BUG-40205 insure++ warning
// SQLHENV utAtbConnection::envhp = NULL;

IDE_RC utAtbConnection::connect(const SChar * userName, ...)
{
    SChar *   sDSN;
    SChar *   passwd;
    va_list   args;

    IDE_TEST(userName == NULL);
    va_start(args, userName);
    passwd   = va_arg(args,SChar*);
    sDSN     = va_arg(args,SChar*);
    va_end(args);

    mSchema = (SChar*)userName;

    /* BUG-39893 Validate connection string according to the new logic, BUG-39767 */
    // sDSN variable means IP + PORT + NLS
    IDE_TEST( sprintf(mDSN, "%s;", sDSN) < 0 );
    IDE_TEST( AppendConnStrAttr(mDSN, ID_SIZEOF(mDSN), (SChar *)"UID", mSchema) );
    IDE_TEST( AppendConnStrAttr(mDSN, ID_SIZEOF(mDSN), (SChar *)"PWD", passwd) );

    Connection::mDSN = mDSN;

    IDE_TEST( connect() != IDE_SUCCESS);

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC utAtbConnection::connect()
{
    SChar sDateFormatString[512];

    IDE_TEST(disconnect() != IDE_SUCCESS);

    if(mSchema == NULL)
    {
        mSchema = dbd->getUserName();
    }

    if(Connection::mDSN == NULL)
    {
        /* BUG-39893 Validate connection string according to the new logic, BUG-39767 */
        // Return value of getDSN means IP + PORT + NLS
        IDE_TEST( sprintf(mDSN, "%s;", dbd->getDSN()) < 0 );
        IDE_TEST( AppendConnStrAttr(mDSN, ID_SIZEOF(mDSN), (SChar *)"UID", dbd->getUserName()) );
        IDE_TEST( AppendConnStrAttr(mDSN, ID_SIZEOF(mDSN), (SChar *)"PWD", dbd->getPasswd()) );

        Connection::mDSN = mDSN;
    }

    IDE_TEST(SQLDriverConnect(dbchp, NULL, (SQLCHAR *)mDSN, SQL_NTS,
                              NULL, 0, NULL, SQL_DRIVER_NOPROMPT)
             == SQL_ERROR);

    IDE_TEST(SQLSetConnectAttr(dbchp, SQL_ATTR_AUTOCOMMIT,
                               (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0)
             == SQL_ERROR);

    serverStatus = SERVER_NORMAL;

    if(mQuery == NULL)
    {
        mQuery =  new utAtbQuery(this);
        IDE_TEST(mQuery == NULL);
        IDE_TEST(mQuery->initialize() != IDE_SUCCESS);
    }

    idlOS::snprintf(sDateFormatString, ID_SIZEOF(sDateFormatString), "\'%s\'", DATE_FMT);
     IDE_TEST(SQLSetConnectAttr(dbchp, ALTIBASE_DATE_FORMAT, sDateFormatString,
                 idlOS::strlen(sDateFormatString))
              != SQL_SUCCESS);

    IDE_TEST(mQuery->execute("alter session set FETCH_TIMEOUT=%"ID_UINT32_FMT,
                             TIME_OUT)
             != IDE_SUCCESS);
    IDE_TEST(mQuery->execute("alter session set QUERY_TIMEOUT=%"ID_UINT32_FMT,
                             TIME_OUT)
             != IDE_SUCCESS);
    IDE_TEST(mQuery->execute("alter session set IDLE_TIMEOUT =%"ID_UINT32_FMT,
                             TIME_OUT)
             != IDE_SUCCESS);

    IDE_TEST(mQuery->execute("alter session set replication=false") != IDE_SUCCESS);

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    mErrNo = SQL_ERROR;
    return IDE_FAILURE;
}

IDE_RC utAtbConnection::autocommit(bool ac)
{
    if(ac)
    {
        IDE_TEST(SQLSetConnectAttr(dbchp,SQL_ATTR_AUTOCOMMIT,
                                   (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0)
                 != SQL_SUCCESS);
    }else{
        IDE_TEST(SQLSetConnectAttr(dbchp,SQL_ATTR_AUTOCOMMIT,
                                   (SQLPOINTER)SQL_AUTOCOMMIT_OFF,0)
                 != SQL_SUCCESS);
    }
    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}


IDE_RC utAtbConnection::commit()
{
  IDE_TEST(SQLEndTran(SQL_HANDLE_DBC,(SQLHANDLE)dbchp,SQL_COMMIT)
           != SQL_SUCCESS);

  return IDE_SUCCESS;
  IDE_EXCEPTION_END;
  return IDE_FAILURE;
}

IDE_RC utAtbConnection::rollback()
{
  IDE_TEST(SQLEndTran(SQL_HANDLE_DBC,(SQLHANDLE)dbchp,SQL_ROLLBACK)
           != SQL_SUCCESS);

  return IDE_SUCCESS;
  IDE_EXCEPTION_END;
  return IDE_FAILURE;
}


Query * utAtbConnection::query(void)
{
    utAtbQuery * sQuery  = new utAtbQuery(this);
    if(sQuery != NULL)
    {
        IDE_TEST(sQuery->initialize() != IDE_SUCCESS);
        IDE_TEST(mQuery->add( sQuery )!= IDE_SUCCESS);
    }

    return sQuery;

    IDE_EXCEPTION_END;

    sQuery->finalize();
    delete sQuery;

    return NULL;
}

IDE_RC utAtbConnection::disconnect()
{
    if(serverStatus == SERVER_NORMAL)
    {
        SQLDisconnect(dbchp);
    }
    serverStatus = SERVER_NOT_CONNECTED;

    return IDE_SUCCESS;
}

bool utAtbConnection::isConnected( )
{
    UInt res;

    if(serverStatus ==  SERVER_NOT_CONNECTED)
    {
        return false;
    }
    if(SQLGetConnectAttr(dbchp, SQL_ATTR_CONNECTION_DEAD, (SQLPOINTER)&res,
                         (SQLINTEGER)ID_SIZEOF(res), NULL) != SQL_SUCCESS)
    {
        serverStatus = SERVER_NOT_CONNECTED;

        return false;
    }
    serverStatus = (res == SQL_CD_FALSE) ? SERVER_NORMAL : SERVER_NOT_CONNECTED;

    return (serverStatus == SERVER_NORMAL);
}

utAtbConnection::utAtbConnection(dbDriver *aDbd):Connection(aDbd)
{
    mDSN[0]      ='\0';
    serverStatus =  SERVER_NOT_CONNECTED;
    envhp     = NULL;
    dbchp     = NULL;
    error_stmt= NULL;
}

IDE_RC utAtbConnection::initialize(SChar * buffErr, UInt buffSize)
{
    IDE_TEST(Connection::initialize(buffErr, buffSize) != IDE_SUCCESS);
    mErrNo = SQL_SUCCESS;

    if(envhp == NULL)
    { /* Allocate Envirounment */
        IDE_TEST(SQLAllocEnv(&envhp) != SQL_SUCCESS);
    }

    IDE_TEST_RAISE(envhp == NULL, err_init1);
    IDE_TEST_RAISE(dbchp != NULL, err_init2);

    IDE_TEST(SQLAllocConnect(envhp, &dbchp) == SQL_ERROR);

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_init1)
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_AUDIT_Alloc_Handle_Error,
                "SQLAllocEnv");
        uteSprintfErrorCode(_error, ERROR_BUFSIZE, &gErrorMgr);
    }
    IDE_EXCEPTION(err_init2)
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_AUDIT_Already_Conn_Error);
        uteSprintfErrorCode(_error, ERROR_BUFSIZE, &gErrorMgr);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

metaColumns * utAtbConnection::getMetaColumns(SChar * aTable, SChar * aSchema)
{
    metaColumns * tmp     = NULL;
    Row         * i;
    SChar         sTable[UTDB_MAX_COLUMN_NAME_LEN];
    SChar         sSchema[UTDB_MAX_COLUMN_NAME_LEN];
    SChar         sColumn[UTDB_MAX_COLUMN_NAME_LEN];
    SChar       * sKey;

    IDE_TEST(aTable == NULL);

    if(*aTable == '\"')
    {
        idlOS::memcpy(sTable, aTable + 1, strlen(aTable) - 2);
        sTable[strlen(aTable)-2] = '\0';
    }
    else
    {
        idlOS::strcpy(sTable, aTable);
        idlOS::strUpper(sTable, idlOS::strlen(sTable));
    }

    if(aSchema)
    {
        mSchema = aSchema;
    }

    if(*mSchema == '\"')
    {
        idlOS::memcpy(sSchema, mSchema + 1, strlen(mSchema) - 2);
        sSchema[strlen(mSchema)-2] = '\0';
    }
    else
    {
        idlOS::strcpy(sSchema, mSchema);
        idlOS::strUpper(sSchema, idlOS::strlen(mSchema));
    }


    tmp = new metaColumns();
    IDE_TEST_RAISE(tmp == NULL, err_mem);
    IDE_TEST(tmp->initialize(aTable) != IDE_SUCCESS);

    // * 2. Primary key in Key Order * //
    IDE_TEST(mQuery->execute(
                 "SELECT C.sort_order, D.column_name"
                 " FROM system_.sys_tables_ A LEFT OUTER JOIN system_.sys_constraints_   B"
                 " ON A.table_id = B.table_id LEFT OUTER JOIN system_.sys_index_columns_ C"
                 " ON A.table_id = C.table_id and B.index_id  = C.index_id"
                 "  LEFT OUTER JOIN system_.sys_columns_ D"
                 " ON A.table_id = D.table_id and C.column_id = D.column_id"
                 " WHERE  B.CONSTRAINT_TYPE = 3"
                 "  AND A.user_id = ( SELECT user_id FROM system_.sys_users_ WHERE user_name = '%s' )"
                 "  AND A.table_name='%s'"
                 " ORDER BY C.index_col_order"
                 , sSchema, sTable )
             != SQL_SUCCESS );

    i = mQuery->fetch();
    if(i)
    {
        tmp->asc((*(i->getField(1)->getValue() ) == 'A'));
    }
    for(;i; i = mQuery->fetch())
    {
        sKey = i->getField(2)->getValue();
        if((isSpecialCharacter((SChar *) sKey) == ID_TRUE) ||
            (idlOS::strcasecmp(sKey, "COLUMN") == 0))
        {
            idlOS::strcpy(sColumn, "\"");
            idlOS::strcat(sColumn, sKey);
            idlOS::strcat(sColumn, "\"");
        }
        else
        {
            idlOS::strcpy(sColumn, sKey);
        }

        IDE_TEST(tmp->addPK(sColumn) != IDE_SUCCESS);
    }

    /* 1. Column List in column Order */
    IDE_TEST(mQuery->execute(
                 "SELECT B.column_name "
                 " FROM system_.sys_tables_ A  LEFT OUTER JOIN system_.sys_columns_ B"
                 "  ON A.table_id = B.table_id "
                 " WHERE A.user_id    = ( SELECT user_id FROM system_.sys_users_ WHERE user_name = '%s' ) "
                 "  AND  A.table_name ='%s' "
                 " ORDER BY B.column_order "
                 , sSchema, sTable)
             != SQL_SUCCESS);

    for(i = mQuery->fetch();i;i = mQuery->fetch())
    {
        sKey = i->getField(1)->getValue();

        if((isSpecialCharacter(sKey) == ID_TRUE) ||
           (idlOS::strcasecmp(sKey, "COLUMN") == 0))
        {
            idlOS::strcpy(sColumn, "\"");
            idlOS::strcat(sColumn, sKey);
            idlOS::strcat(sColumn, "\"");
        }
        else
        {
            idlOS::strcpy(sColumn, sKey);
        }

        if(!tmp->isPrimaryKey(sColumn))
        {
            IDE_TEST(tmp->addCL(sColumn, false) != IDE_SUCCESS);
        }
    }

    /* Lob Column List */
    IDE_TEST(mQuery->execute(
                 "SELECT B.column_name"
                 " FROM system_.sys_tables_ A  LEFT OUTER JOIN system_.sys_columns_ B"
                 " ON A.table_id = B.table_id"
                 " WHERE A.user_id    = ( SELECT user_id FROM system_.sys_users_ WHERE user_name = '%s' )"
                 " AND  A.table_name ='%s' AND (B.data_type=%d OR B.data_type=%d)"
                 " ORDER BY B.column_order "
                 , sSchema, sTable, SQL_CLOB, SQL_BLOB)  //SQL_BLOB=40, SQL_CLOB=30
             != SQL_SUCCESS);


    
    for(i = mQuery->fetch();i;i = mQuery->fetch())
    {
        sKey = i->getField(1)->getValue();
        if((isSpecialCharacter((SChar *) sKey) == ID_TRUE) ||
           (idlOS::strcasecmp(sKey, "COLUMN") == 0))
        {
            idlOS::strcpy(sColumn, "\"");
            idlOS::strcat(sColumn, sKey);
            idlOS::strcat(sColumn, "\"");
        }
        else
        {
            idlOS::strcpy(sColumn, sKey);
        }
        
        if(!tmp->isPrimaryKey(sColumn))
        {
            IDE_TEST(tmp->addCL(sColumn, true) != IDE_SUCCESS);
        }
    }

    if(mTCM)
    {
        mTCM->add(tmp);
    }
    else
    {
        mTCM = tmp;
    }

    return  tmp;

    IDE_EXCEPTION(err_mem)
    {
        uteSetErrorCode(&gErrorMgr,
                utERR_ABORT_AUDIT_Create_Instance_Error, "metaColumns");
        uteSprintfErrorCode(_error, ERROR_BUFSIZE, &gErrorMgr);
    }
    IDE_EXCEPTION_END;

    if(tmp)
    {
        delete tmp;
    }
    if(mErrNo == 0)
    {
        mErrNo  = ATB_ERROR;
    }
    mQuery->clear();
    return  NULL;
}

utAtbConnection::~utAtbConnection(void)
{
    // finalize();
}

IDE_RC utAtbConnection::finalize()
{
    serverStatus = SERVER_NOT_CONNECTED;

    IDE_TEST(Connection::finalize());

    // * free handles * //
    if(dbchp)
    {
        (void)SQLDisconnect(dbchp);
        (void)SQLFreeConnect(dbchp);
        dbchp  = NULL;
    }
    if(envhp)
    {
        (void)SQLFreeEnv(envhp);
        envhp  = NULL;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


SChar * utAtbConnection::error(void * errhp)
{
    error_stmt = (SQLHSTMT)errhp;

    if(error_stmt)
    {
        mErrNo = checkState(mErrNo, error_stmt);
        error_stmt = NULL;
    }

    return (mErrNo)?_error: NULL;
}

SInt utAtbConnection::getErrNo(void)
{
    if(error_stmt)
    {
        mErrNo = checkState(mErrNo, error_stmt);
        error_stmt = NULL;
    }

    return mErrNo;
}

dba_t utAtbConnection::getDbType(void)
{
    return DBA_ATB;
}

SInt utAtbConnection::checkState(SInt status, SQLHSTMT aStmt)
{
    switch(status)
    {
        case SQL_SUCCESS:
            break;
        case SQL_SUCCESS_WITH_INFO:
            {
                sprintf(_error," SQL_SUCCESS_WITH_INFO ");
                status = 0;
            }
            break;
        case SQL_NO_DATA:
            sprintf(_error," SQL_NO_DATA ");
            break;
        case SQL_ERROR:
            {
                SQLINTEGER errNo;
                SQLSMALLINT msgLength;
                if(SQLError(envhp, dbchp, aStmt, NULL, (SQLINTEGER *)&errNo,
                            (SQLCHAR *)_error, (SQLSMALLINT)_errorSize,
                            (SQLSMALLINT *)&msgLength ) == SQL_SUCCESS)
                {
#ifdef DEBUG
                    idlOS::fprintf(stderr,"[ERROR:%"ID_INT32_FMT"] %s\n", errNo, _error);
#endif
                    status = errNo;
                }
                else
                {
                    idlOS::sprintf(_error,"SQLCLI  SQLError FAILURE!\n");

                    status = 1; // 에러가 발생 했다는 것만을 의미
                    idlOS::snprintf(_error, _errorSize,
                                    "Unable to retrieve error information from CLI driver");
                }
            }
            break;
        case SQL_INVALID_HANDLE:
            sprintf(_error," SQL_INVALID_HANDLE ");
            break;
        case SQL_STILL_EXECUTING:
            sprintf(_error," SQL_STILL_EXECUTE " );
            break;
        case SQL_NEED_DATA:
            sprintf(_error,"ERROR: SQL_NEED_DATA");
            break;
        default:
            break;
    }

    return status;
}

/* BUG-39893 Validate connection string according to the new logic, BUG-39767 */
IDE_RC utAtbConnection::AppendConnStrAttr( SChar *aConnStr, UInt aConnStrSize, SChar *aAttrKey, SChar *aAttrVal )
{
    UInt sAttrValLen;

    IDE_TEST_RAISE( aAttrVal == NULL || aAttrVal[0] == '\0', NO_NEED_WORK );

    idlVA::appendString( aConnStr, aConnStrSize, aAttrKey, idlOS::strlen(aAttrKey) );
    idlVA::appendString( aConnStr, aConnStrSize, (SChar *)"=", 1 );
    sAttrValLen = idlOS::strlen(aAttrVal);
    if ( (aAttrVal[0] == '"' && aAttrVal[sAttrValLen - 1] == '"') ||
         (idlOS::strchr(aAttrVal, ';') == NULL &&
          acpCharIsSpace(aAttrVal[0]) == ACP_FALSE &&
          acpCharIsSpace(aAttrVal[sAttrValLen - 1]) == ACP_FALSE) )
    {
        idlVA::appendString( aConnStr, aConnStrSize, aAttrVal, sAttrValLen );
    }
    else if ( idlOS::strchr(aAttrVal, '"') == NULL )
    {
        idlVA::appendFormat( aConnStr, aConnStrSize, "\"%s\"", aAttrVal );
    }
    else
    {
        IDE_RAISE( InvalidConnAttr );
    }
    idlVA::appendString( aConnStr, aConnStrSize, (SChar *)";", 1 );

    IDE_EXCEPTION_CONT( NO_NEED_WORK );

    return IDE_SUCCESS;

    IDE_EXCEPTION( InvalidConnAttr )
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_INVALID_CONN_ATTR);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


