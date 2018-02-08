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
 * $Id: dmlQuery.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <uto.h>

const SChar * _INSERT_= "INSERT INTO ";
const SChar * _DELETE_= "DELETE FROM ";
const SChar * _SELECT_= "SELECT "     ;
const SChar * _UPDATE_= "UPDATE "     ;

dmlQuery::dmlQuery() : Object()
{
    mRow   = NULL;
    mMeta  = NULL;
    mQuery = NULL;
    mExec  = 0;
    mFail  = 0;
    mType[0] = mType[1] = mType[2] = 0;
};

dmlQuery::~dmlQuery()
{
    if(mQuery) mQuery->close();
    mQuery = NULL;
};

IDE_RC dmlQuery::initialize(
    Query        *aQuery,
    SChar       aSrvType,
    SChar       aDmlType,
    const SChar *aSchema,
    const SChar * aTable,
    metaColumns *aMeta)
{
    IDE_TEST(aQuery == NULL);
    IDE_TEST(aMeta  == NULL);
    IDE_TEST(aTable == NULL);

    mQuery = aQuery;
    mMeta = aMeta;
    mTable = aTable;
    mSchema = aSchema;

    switch(aDmlType)
    {
        case 'I':
            IDE_TEST(prepareInsert() != IDE_SUCCESS);
            break;

        case 'D':
            IDE_TEST(prepareDelete() != IDE_SUCCESS);
            break;

        case 'U':
            IDE_TEST(prepareUpdate() != IDE_SUCCESS);
            break;
        default:
            return IDE_FAILURE;
    }
    /* set type string for SI/MI/... */
    mType[0] = aSrvType;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC dmlQuery::execute()
{
    IDE_TEST(mRow == NULL);

    mExec++;

#ifdef DEBUG
    idlOS::fprintf(stderr, "[Thr=%"ID_UINT64_FMT"]EXECUTE Before\n", idlOS::getThreadID());
#endif

    if(mQuery->execute() != IDE_SUCCESS)
    {
        mFail++;
        IDE_TEST(mFail);
    }

#ifdef DEBUG
    idlOS::fprintf(stderr, "[Thr=%"ID_UINT64_FMT"]EXECUTE After\n", idlOS::getThreadID());
#endif

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC dmlQuery::lobAtToAt(Query * aGetLob, Query * aPutLob, SChar * tblName)
{
    //IDE_TEST(mRow == NULL);

    IDE_TEST(mQuery->lobAtToAt(aGetLob, aPutLob, tblName) != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC dmlQuery::prepareInsert()
{
    SChar * sql;

    IDE_TEST(mQuery->close() != IDE_SUCCESS);

    sql = mQuery->statement();

    /* insert format SQL */
    IDE_TEST(sqlInsert(sql) != IDE_SUCCESS);

#ifdef DEBUG
    idlOS::fprintf(stderr, "[Thr=%"ID_UINT64_FMT"]PREPARE SQL : %s\n",
                           idlOS::getThreadID(),
                           sql);
#endif

    /* prepare */
    IDE_TEST(mQuery->prepare() != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC dmlQuery::prepareUpdate()
{
    SChar * sql;

    IDE_TEST(mQuery->close() != IDE_SUCCESS);
    sql = mQuery->statement();

    /* make SQL query string */
    if(mMeta->getCLSize(false) > 0)  // BUG-18675
    {
        IDE_TEST(sqlUpdate(sql) != IDE_SUCCESS);

#ifdef DEBUG
        idlOS::fprintf(stderr, "[Thr=%"ID_UINT64_FMT"]PREPARE SQL : %s\n",
                               idlOS::getThreadID(),
                               sql);
#endif

        IDE_TEST(mQuery->prepare() != IDE_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC dmlQuery::prepareDelete()
{
    SChar *sql;

    IDE_TEST(mQuery->close() != IDE_SUCCESS);

    sql = mQuery->statement();

    IDE_TEST(sqlDelete(sql) != IDE_SUCCESS);

#ifdef DEBUG
    idlOS::fprintf(stderr, "[Thr=%"ID_UINT64_FMT"]PREPARE SQL : %s\n",
                           idlOS::getThreadID(),
                           sql);
#endif

    IDE_TEST(mQuery->prepare() != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

#ifdef DEBUG
#define ELLIPSIS     '.'
void dmlQuery::log4Bind(SChar *aQueryType, UShort aColNumber, Field *aField)
{
    SInt    sPostfixPos;
    SChar   sTemp[COLUMN_BUF_LEN];

    if (aField->getSChar(sTemp, COLUMN_BUF_LEN) == -1)
    {
        if ( (sTemp[0] == '\'') &&
             (sTemp[COLUMN_BUF_LEN-2] == '\'') )
        {
            sPostfixPos = COLUMN_BUF_LEN - 5;
        }
        else
        {
            sPostfixPos = COLUMN_BUF_LEN - 4;
        }
        sTemp[sPostfixPos++] = ELLIPSIS;
        sTemp[sPostfixPos++] = ELLIPSIS;
        sTemp[sPostfixPos]   = ELLIPSIS;
    }
    else
    {
        /* do nothing */
    }
    idlOS::fprintf(stderr,
                   "[Thr=%"ID_UINT64_FMT"]"
                   "BIND:%s (Col=%"ID_INT32_FMT") "
                   "Data[%s] "
                   "isNull[%s]\n",
                   idlOS::getThreadID(),
                   aQueryType, aColNumber,
                   sTemp,
                   aField->isNull() ? "True" : "False");
}
#endif

IDE_RC dmlQuery::bind(Row * aRow)
{
    Field          *f;
    UShort  i,cl,size;

    IDE_TEST(aRow == NULL);
    aRow->mIsLobCol = false;

    switch(mType[1])
    {
        case 'I':
            //If there's column that data type is Lob, it will execute updating after inserting.
            for(i = 1, f = aRow->getField(1); f; f = aRow->getField(++i))
            {
#ifdef DEBUG
                log4Bind((SChar *)"Insert", i, f);
#endif
                IDE_TEST(mQuery->bind(i, f) != IDE_SUCCESS);
            }

            break;
        case 'U':
            // ** Its uses backward order links ** //
            for(cl = 1,i = aRow->size(); i; i--,cl++)
            {
                f = aRow->getField(i);
#ifdef DEBUG
                log4Bind((SChar *)"Update", i, f);
#endif
                IDE_TEST(mQuery->bind(cl, f) != IDE_SUCCESS);
            }
            break;
        case 'D':
            size = mMeta->getPKSize();

            for(i = 1 ; i <= size ; i++)
            {
                f = aRow->getField(i);
#ifdef DEBUG
                log4Bind((SChar *)"Delete", i, f);
#endif
                IDE_TEST(mQuery->bind(i, f) != IDE_SUCCESS);
            }
            break;
        default:
            IDE_TEST(1);
            break;
    }

    mRow = aRow;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC dmlQuery::sqlInsert(SChar *sql, const SChar *dl)
{
    SChar *s;
    UInt   i;
    bool sIsLobType = false; // common column type:0, lob column type:1

    mType[1] = 'I';
    idlOS::strcpy(sql, _INSERT_);

    if(mSchema != NULL)
    {
        idlOS::strcat(sql, mSchema );
        idlOS::strcat(sql, ".");
    }
    idlOS::strcat(sql, mTable);
    idlOS::strcat(sql, "(");

    for(i = 1, s= mMeta->getPK(1); s; s = mMeta->getPK(++i))
    {
        idlOS::strcat(sql,  s);
        idlOS::strcat(sql,",");
    }

    for(i = 1, s = mMeta->getCL(1, sIsLobType); s ; s = mMeta->getCL(++i, sIsLobType))
    {
        idlOS::strcat(sql,  s);
        idlOS::strcat(sql,",");
    }

    i = idlOS::strlen(sql);
    IDE_DASSERT(i < QUERY_BUFSIZE);

    sql[--i] = '\0';
    idlOS::strcat ( sql,") VALUES(");

    for(i = mMeta->getPKSize() + mMeta->getCLSize(sIsLobType); i--;)
    {
        idlOS::strcat(sql, dl);
        idlOS::strcat(sql,",");
    }

    i = idlOS::strlen(sql);
    IDE_DASSERT(i < QUERY_BUFSIZE);
    sql[--i] = ')';

    return IDE_SUCCESS;
}

IDE_RC dmlQuery::sqlDelete(SChar *sql, const SChar *dl)
{
    SChar *s;
    UInt   i;

    mType[1] = 'D';

    idlOS::strcpy(sql,_DELETE_);
    if(mSchema != NULL)
    {
        idlOS::strcat(sql, mSchema);
        idlOS::strcat(sql, ".");
    }

    idlOS::strcat(sql, mTable );
    idlOS::strcat(sql," WHERE " );

    for(i = 1, s= mMeta->getPK(1); s; s= mMeta->getPK(++i))
    {
        if(i > 1)
        {
            idlOS::strcat(sql," AND ");
        }
        idlOS::strcat(sql,s);
        idlOS::strcat(sql," =");
        idlOS::strcat(sql,dl);
    }

    return IDE_SUCCESS;
}
IDE_RC dmlQuery::sqlUpdate(SChar *sql, const SChar *dl)
{
    SChar *s;
    UInt   i;
    bool   sIsLobType = false; // common column type:0, lob column type:1

    mType[1] = 'U';

    idlOS::strcpy(sql,_UPDATE_);

    if( mSchema != NULL )
    {
        idlOS::strcat(sql, mSchema );
        idlOS::strcat(sql, ".");
    }

    idlOS::strcat(sql, mTable );
    idlOS::strcat(sql," SET ");

    for(i = mMeta->getCLSize(sIsLobType), s = mMeta->getCL(i, sIsLobType); s ; s = mMeta->getCL(--i, sIsLobType))
    {
        idlOS::strcat(sql,s);
        idlOS::strcat(sql," =");
        idlOS::strcat(sql,dl);
        idlOS::strcat(sql,",");
    }

    i = idlOS::strlen(sql);
    IDE_DASSERT(i < QUERY_BUFSIZE);

    sql[--i] = '\0';
    idlOS::strcat(sql," WHERE " );

    for(i = mMeta->getPKSize(), s= mMeta->getPK(i); s; s= mMeta->getPK(--i))
    {
        idlOS::strcat(sql,s);
        idlOS::strcat(sql," =");
        idlOS::strcat(sql,dl);
        idlOS::strcat(sql," AND ");
    }

    i = idlOS::strlen(sql) - sizeof("AND ");
    IDE_DASSERT(i < QUERY_BUFSIZE);
    sql[i] = '\0';

    return IDE_SUCCESS;
}

