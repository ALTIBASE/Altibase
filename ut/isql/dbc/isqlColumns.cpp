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
 * $Id$
 **********************************************************************/

#include <utString.h>
#include <utISPApi.h>
#if !defined(PDL_HAS_WINCE)
#include <errno.h>
#endif

utColumns::utColumns()
{
    mColumns     = NULL;
    /* BUG-45586 Invalid column count exception occurs 
                 when executing a function */
    m_Col = 0;
}

utColumns::~utColumns()
{
    freeMem();
}

void utColumns::freeMem()
{
    int i;

    if (mColumns != NULL)
    {
        for (i=0; i<m_Col; i++)
        {
            if (mColumns[i] != NULL)
            {
                delete mColumns[i];
                mColumns[i] = NULL;
            }
        }

        idlOS::free(mColumns);
        mColumns = NULL;
    }

    m_Col = 0;
}

IDE_RC utColumns::SetSize(SInt a_ColCount)
{
    IDE_TEST(a_ColCount < 0);

    freeMem();

    mColumns = (isqlType **)idlOS::calloc( a_ColCount, ID_SIZEOF(isqlType *) );
    IDE_TEST(mColumns == NULL);

    m_Col = a_ColCount;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    freeMem();

    return IDE_FAILURE;
}

/* BUG-43911 Refactoring of printing fetch result
 * 
 * Description: Column Factory 
 */
IDE_RC utColumns::AddColumn(SInt         aIndex,
                            SChar       *aName,
                            SInt         aSqlType,
                            SInt         aPrecision,
                            uteErrorMgr *aErrorMgr,
                            idBool       aExecute)
{
    IDE_TEST(aIndex >= m_Col);

    switch (aSqlType)
    {
    case SQL_CHAR :
        mColumns[aIndex] = (isqlType *) new isqlChar();
        break;
    case SQL_VARCHAR :
    case SQL_WCHAR :
    case SQL_WVARCHAR :
        mColumns[aIndex] = (isqlType *) new isqlVarchar();
        break;
    case NULL :
        mColumns[aIndex] = (isqlType *) new isqlNull();
        break;
    case SQL_SMALLINT :
    case SQL_INTEGER :
        mColumns[aIndex] = (isqlType *) new isqlInteger();
        break;
    case SQL_NUMERIC :
    case SQL_DECIMAL :
    case SQL_FLOAT :
        mColumns[aIndex] = (isqlType *) new isqlNumeric();
        break;
    case SQL_REAL :
        mColumns[aIndex] = (isqlType *) new isqlReal();
        break;
    case SQL_DOUBLE :
        mColumns[aIndex] = (isqlType *) new isqlDouble();
        break;
    case SQL_BIGINT :
    case SQL_INTERVAL :
    case SQL_INTERVAL_YEAR:
    case SQL_INTERVAL_MONTH:
    case SQL_INTERVAL_DAY:
    case SQL_INTERVAL_HOUR:
    case SQL_INTERVAL_MINUTE:
    case SQL_INTERVAL_SECOND:
    case SQL_INTERVAL_YEAR_TO_MONTH:
    case SQL_INTERVAL_DAY_TO_HOUR:
    case SQL_INTERVAL_DAY_TO_MINUTE:
    case SQL_INTERVAL_DAY_TO_SECOND:
    case SQL_INTERVAL_HOUR_TO_MINUTE:
    case SQL_INTERVAL_HOUR_TO_SECOND:
    case SQL_INTERVAL_MINUTE_TO_SECOND:
        mColumns[aIndex] = (isqlType *) new isqlLong();
        break;
    case SQL_TYPE_DATE :
    case SQL_TIMESTAMP :
    case SQL_DATE :
    case SQL_TYPE_TIMESTAMP:
        mColumns[aIndex] = (isqlType *) new isqlDate();
        break;
    case SQL_BYTES :
    case SQL_VARBYTE :
    case SQL_NIBBLE :
        mColumns[aIndex] = (isqlType *) new isqlBytes();
        break;
    case SQL_BIT:
    case SQL_VARBIT:
        mColumns[aIndex] = (isqlType *) new isqlBit();
        break;
    case SQL_CLOB:
        mColumns[aIndex] = (isqlType *) new isqlClob();
        break;
    case SQL_BINARY :
    case SQL_BLOB :
    case SQL_GEOMETRY :
        // BUG-24273 plan only 일때는 target절 blob이 포함되어도 조회 가능하여야 합니다.
        if (aExecute == ID_TRUE)
        {
            IDE_RAISE(undisplayable_datatype);
        }
        else
        {
            mColumns[aIndex] = (isqlType *) new isqlBlob();
        }
        break;
    default :
        IDE_RAISE(wrong_datatype);
    }

    IDE_TEST_RAISE(mColumns[aIndex] == NULL, MAllocError);

    mColumns[aIndex]->SetName(aName);
    mColumns[aIndex]->SetSqlType(aSqlType);
    mColumns[aIndex]->SetPrecision(aPrecision);

    IDE_TEST_RAISE(mColumns[aIndex]->Init() != IDE_SUCCESS,
                   MAllocError);

    return IDE_SUCCESS;

    IDE_EXCEPTION(MAllocError);
    {
        uteSetErrorCode(aErrorMgr, utERR_ABORT_memory_error,
                        __FILE__, __LINE__);
    }
    IDE_EXCEPTION(undisplayable_datatype);
    {
        uteSetErrorCode(aErrorMgr, utERR_ABORT_UNDISPLAYABLE_DATATYPE_Error);
    }
    IDE_EXCEPTION(wrong_datatype);
    {
        uteSetErrorCode(aErrorMgr, utERR_ABORT_WRONG_DATATYPE_Error);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void utColumns::Reformat()
{
    SInt  sI;

    for (sI = 0; sI < m_Col; sI++)
    {
        mColumns[sI]->Reformat();
    }
}

/* BUG-43911 Refactoring of printing fetch result
 * 
 * Description: 컬럼 데이터를 spool buffer에 옮겨줌
 *              set vertical off 일 때 호출됨.
 */
SInt utColumns::AppendToBuffer(SInt aColNum, SChar *aBuf, SInt *aBufLen)
{
    return mColumns[aColNum]->AppendToBuffer(aBuf, aBufLen);
}

/* BUG-43911 Refactoring of printing fetch result
 * 
 * Description: 컬럼 전체 데이터를 spool buffer에 옮겨줌.
 *              set vertical on 일 때 호출됨.
 */
void utColumns::AppendAllToBuffer(SInt aColNum, SChar *aBuf)
{
    (void) mColumns[aColNum]->AppendAllToBuffer(aBuf);
}

