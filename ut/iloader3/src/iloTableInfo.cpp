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
 * $Id: iloTableInfo.cpp 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

#include <ilo.h>

void InsertSeq( ALTIBASE_ILOADER_HANDLE  aHandle,
                SChar                   *sName,
                SChar                   *cName, 
                SChar                   *val)
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    //구조체에 넣어준다.
    if ( sHandle->mTableInfomation.mSeqIndex > UT_MAX_SEQ_ARRAY_CNT)
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_too_many_Seq_Error, UT_MAX_SEQ_ARRAY_CNT);
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
        
        sHandle->mTableInfomation.mSeqIndex = 0;
    }
    idlOS::strcpy(sHandle->mTableInfomation.mSeqArray[sHandle->mTableInfomation.mSeqIndex].seqName, sName);
    idlOS::strcpy(sHandle->mTableInfomation.mSeqArray[sHandle->mTableInfomation.mSeqIndex].seqCol, cName);
    idlOS::strcpy(sHandle->mTableInfomation.mSeqArray[sHandle->mTableInfomation.mSeqIndex].seqVal, val);

    sHandle->mTableInfomation.mSeqIndex++;
}

iloTableNode::iloTableNode()
{
    m_NodeValue = NULL;
    m_pSon = NULL;
    m_pBrother = NULL;
    mSkipFlag = ILO_FALSE;
    mNoExpFlag = ILO_FALSE;
    mOutFileFlag = ILO_FALSE;   //PROJ-2030, CT_CASE-3020 CHAR outfile 지원 
    mIsQueue = SQL_FALSE;
    mPrecision = 0;
    m_Hint     = NULL;
    mScale = 0;
    m_Condition = NULL;
    m_DateFormat = NULL;
}

iloTableNode::iloTableNode(ETableNodeType eNodeType, SChar *szNodeValue,
                           iloTableNode *pSon, iloTableNode *pBrother,
                           SChar *szHint)
{
    m_NodeType = eNodeType;
    if (szNodeValue == NULL)
    {
        m_NodeValue = NULL;
    }
    else
    {
        m_NodeValue = new SChar[idlOS::strlen(szNodeValue) + 1];
        idlOS::strcpy(m_NodeValue, szNodeValue);
    }
    m_pSon = pSon;
    m_pBrother = pBrother;
    mSkipFlag = ILO_FALSE;
    mNoExpFlag = ILO_FALSE;
    mOutFileFlag = ILO_FALSE;  //PROJ-2030, CT_CASE-3020 CHAR outfile 지원 
    mIsQueue = SQL_FALSE;
    m_Hint = szHint;
    m_Condition = NULL;
    m_DateFormat = NULL;
    mPrecision = 0;
    mScale = 0;
}

iloTableNode::iloTableNode(ETableNodeType eNodeType, SChar *szNodeValue,
                           iloTableNode *pSon, iloTableNode *pBrother,
                           SInt aIsQueue, SChar *szHint)
{
    m_NodeType = eNodeType;
    if (szNodeValue == NULL)
    {
        m_NodeValue = NULL;
    }
    else
    {
        m_NodeValue = new SChar[idlOS::strlen(szNodeValue) + 1];
        idlOS::strcpy(m_NodeValue, szNodeValue);
    }
    m_pSon = pSon;
    m_pBrother = pBrother;
    mSkipFlag = ILO_FALSE;
    mNoExpFlag = ILO_FALSE;
    mOutFileFlag = ILO_FALSE;  //PROJ-2030, CT_CASE-3020 CHAR outfile 지원 
    mIsQueue = aIsQueue;
    m_Hint = szHint;
    m_Condition = NULL;
    m_DateFormat = NULL;
    mPrecision = 0;
    mScale = 0;
}

iloTableNode::~iloTableNode()
{
    if (this != NULL)
    {
        if ( m_pSon != NULL )
        {
            delete m_pSon;
            m_pSon = NULL;
        }
        if ( m_pBrother != NULL )
        {
            delete m_pBrother;
            m_pBrother = NULL;
        }
        if ( m_NodeValue != NULL )
        {
            delete [] m_NodeValue;
            m_NodeValue = NULL;
        }
        if (m_Condition != NULL)
        {
            delete m_Condition;
            m_Condition = NULL;
        }
    }
}

void iloTableNode::setPrecision( SInt aPrecision, SInt aScale )
{
    mPrecision = aPrecision;
    mScale = aScale;
}

/* Called with nDepth = 0 in First */
SInt iloTableNode::PrintNode(SInt nDepth)
{
    SInt i;

    if (nDepth > 0)
    {
        for (i=nDepth ; i > 0; i--)
            idlOS::printf("     ");
        idlOS::printf("|--");
    }

    switch (m_NodeType)
    {
    case TABLE_NODE :
        idlOS::printf("TABLE\n");
        break;
    case DOWN_NODE :
        idlOS::printf("Download condition [%s]\n", m_NodeValue);
        break;
    case TABLENAME_NODE :
        idlOS::printf("Table Name [%s]\n", m_NodeValue);
        break;
    case ATTR_NODE :
        idlOS::printf("Attribute\n");
        break;
    case SEQ_NODE :
        idlOS::printf("Attr Name [%s]\n", m_NodeValue);
        break;
    case ATTRNAME_NODE :
        idlOS::printf("Attr Name [%s]\n", m_NodeValue);
        break;
    case ATTRTYPE_NODE :
        idlOS::printf("Attr Type [%s]\n", m_NodeValue);
        break;
    }

    if (m_pSon != NULL)
    {
        m_pSon->PrintNode(nDepth + 1);
    }
    if (m_pBrother != NULL)
    {
        m_pBrother->PrintNode(nDepth);
    }

    return SQL_TRUE;
}

SInt iloTableTree::SetTreeRoot(iloTableNode *pRoot)
{
    if (m_Root != NULL)
    {
        delete m_Root;
    }
    m_Root = pRoot;

    return SQL_TRUE;
}

SInt iloTableTree::PrintTree()
{
    if (m_Root == NULL)
    {
        return SQL_TRUE;
    }
    else
    {
        return m_Root->PrintNode(0);
    }
}

iloTableInfo::iloTableInfo()
{
    (void)idlOS::memset(localSeqArray, 0, ID_SIZEOF(localSeqArray));
    m_TableName[0]      = '\0';
    m_bDownCond         = ILO_FALSE;
    m_QueryString[0]    = '\0';
    m_AttrCount         = 0;
    m_ReadCount         = NULL;
    (void)idlOS::memset(m_AttrName, 0, ID_SIZEOF(m_AttrName));
    (void)idlOS::memset(m_AttrType, 0, ID_SIZEOF(m_AttrType));
    mAttrCVal           = NULL;
    mAttrCValEltLen     = NULL;
    mAttrVal            = NULL;
    mAttrValEltLen      = NULL;
    // BUG-20569
    mAttrFail           = NULL;
    mAttrFailLen        = NULL;
    mAttrFailMaxLen     = NULL;
    mAttrInd            = NULL;
    mLOBPhyOffs         = NULL;
    mLOBPhyLen          = NULL;
    mLOBLen             = NULL;
    m_HintString[0]     = '\0';
    mStatusPtr          = NULL;
    (void)idlOS::memset(mSkipFlag, 0, ID_SIZEOF(mSkipFlag));
    (void)idlOS::memset(mNoExpFlag, 0, ID_SIZEOF(mNoExpFlag));
    (void)idlOS::memset(mOutFileFlag, 0, ID_SIZEOF(mOutFileFlag)); //PROJ-2030, CT_CASE-3020 CHAR outfile 지원 
    (void)idlOS::memset(mPrecision, 0, ID_SIZEOF(mPrecision));
    (void)idlOS::memset(mScale, 0, ID_SIZEOF(mScale));
    (void)idlOS::memset(mAttrDateFormat, 0, ID_SIZEOF(mAttrDateFormat));
    mIsQueue            = 0;
    mLOBColumnCount     = 0;            //BUG-24583
}

iloTableInfo::~iloTableInfo()
{
    FreeTableAttr();
    FreeDateFormat();
}

SInt iloTableInfo::GetTableInfo( ALTIBASE_ILOADER_HANDLE  aHandle,
                                 iloTableNode            *pTableNameNode)
{
    iloTableNode *pNode;
    SChar         szTmp[100];
    SChar        *sDateFormat;
    SInt          sLength;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    pNode = pTableNameNode->GetSon();

    if (pNode->m_Condition != NULL)
    {
        m_bDownCond = ILO_TRUE;
        idlOS::strcpy(m_QueryString, pNode->m_Condition->GetNodeValue());
        if (pNode->m_Condition->m_Hint != NULL)
        {
            idlOS::strcpy(m_HintString, pNode->m_Condition->m_Hint);
        }
        else
        {
            idlOS::strcpy(m_HintString, "");
        }
    }
    else
    {
        m_bDownCond = ILO_FALSE;
    }
    /* pNode->GetNodeValue() 는 TABLE_DEF */
    idlOS::strcpy(m_TableName, pNode->GetNodeValue());

    mIsQueue = pNode->getIsQueue();

    pNode = pNode->GetSon();

    for (m_AttrCount = 0; pNode != NULL; m_AttrCount++)
    {
        IDE_TEST_RAISE(m_AttrCount >= MAX_ATTR_COUNT, TooManyAttrError);

        idlOS::strcpy(m_AttrName[m_AttrCount], pNode->GetSon()->GetNodeValue());
        mSkipFlag[m_AttrCount] = pNode->mSkipFlag;
        mNoExpFlag[m_AttrCount] = pNode->mNoExpFlag;
        mOutFileFlag[m_AttrCount] = pNode->mOutFileFlag; //PROJ-2030, CT_CASE-3020 CHAR outfile 지원 
        mPrecision[m_AttrCount] = pNode->mPrecision;
        mScale[m_AttrCount] = pNode->mScale;
        if ( (sDateFormat = pNode->GetNodeValue()) != NULL )
        {
            sLength = idlOS::strlen(sDateFormat);
            if (mAttrDateFormat[m_AttrCount] != NULL)
            {
                free(mAttrDateFormat[m_AttrCount]);
            }
            mAttrDateFormat[m_AttrCount] = (SChar *)idlOS::malloc(sLength + 1);
            IDE_TEST_RAISE(mAttrDateFormat[m_AttrCount] == NULL, MAllocError);
            (void)idlOS::snprintf(mAttrDateFormat[m_AttrCount], sLength + 1,
                                  "%s", sDateFormat);
        }
        else
        {
            mAttrDateFormat[m_AttrCount] = NULL;
        }

        idlOS::strcpy(szTmp, pNode->GetSon()->GetBrother()->GetNodeValue());
        if (strcmp(szTmp, "char") == 0)
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_CHAR;
        }
        else if (strcmp(szTmp, "varchar") == 0)
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_VARCHAR;
        }
        else if (strcmp(szTmp, "clob") == 0)
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_CLOB;
        }
        else if (strcmp(szTmp, "integer") == 0)
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_INTEGER;
        }
        else if (strcmp(szTmp, "double") == 0 )
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_DOUBLE;
        }
        else if (strcmp(szTmp, "smallint") == 0 )
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_SMALLINT;
        }
        else if (strcmp(szTmp, "bigint") == 0 )
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_BIGINT;
        }
        else if (strcmp(szTmp, "decimal") == 0 )
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_DECIMAL;
        }
        else if (strcmp(szTmp, "float") == 0 )
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_FLOAT;
        }
        else if (strcmp(szTmp, "real") == 0 )
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_REAL;
        }
        else if (strcmp(szTmp, "inteval") == 0 )
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_INTEVAL;
        }
        else if (strcmp(szTmp, "boolean") == 0 )
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_BOOLEAN;
        }
        else if (strcmp(szTmp, "blob") == 0 )
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_BLOB;
        }
        else if (strcmp(szTmp, "nibble") == 0)
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_NIBBLE;
        }
        else if ((strcmp(szTmp, "bytes") == 0) ||
                 (strcmp(szTmp, "byte") == 0)) // BUG-35237
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_BYTES;
        }
        else if (strcmp(szTmp, "varbyte") == 0)
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_VARBYTE;
        }
        else if (strcmp(szTmp, "timestamp") == 0)
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_TIMESTAMP;
        }
        else if (strcmp(szTmp, "bit") == 0)
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_BIT;
        }
        else if (strcmp(szTmp, "varbit") == 0)
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_VARBIT;
        }
        else if (strcmp(szTmp, "numeric_long") == 0)
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_NUMERIC_LONG;
        }
        else if (strcmp(szTmp, "numeric_double") == 0)
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_NUMERIC_DOUBLE;
        }
        else if (strcmp(szTmp, "date") == 0)
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_DATE;
        }
        else if (strcmp(szTmp, "nextval") == 0)
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_DATE;
        }
        else if (strcmp(szTmp, "currval") == 0)
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_DATE;
        }
        else if (strcmp(szTmp, "nchar") == 0)
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_NCHAR;
        }
        else if (strcmp(szTmp, "nvarchar") == 0)
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_NVARCHAR;
        }
        /* BUG-24359 Geometry formfile */
        else if (strcmp(szTmp, "geometry") == 0)
        {
            m_AttrType[m_AttrCount] = ISP_ATTR_GEOMETRY;
        }
        else
        {
            IDE_RAISE(UnknownDataTypeError);
        }

        pNode = pNode->GetBrother();
    }

    return SQL_TRUE;

    IDE_EXCEPTION(TooManyAttrError);
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_too_many_Attr_Error,
                        MAX_ATTR_COUNT);
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
        
        Reset();
        m_AttrCount = 0;
    }
    IDE_EXCEPTION(MAllocError);
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_memory_error,
                        __FILE__, __LINE__);
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
        
        Reset();
        m_AttrCount = 0;
    }
    IDE_EXCEPTION(UnknownDataTypeError);
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Unkown_Datatype_Error, szTmp);
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
        
        Reset();
        m_AttrCount = 0;
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

SInt iloTableInfo::AllocTableAttr( ALTIBASE_ILOADER_HANDLE aHandle,
                                   SInt                    sArrayCount,
                                   SInt                    sExistBadLog)
{
    SInt i;
    SInt j;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    IDE_TEST(sArrayCount <= 0);

    m_ReadCount = (SInt *)idlOS::malloc((UInt)sArrayCount * ID_SIZEOF(SInt));
    IDE_TEST_RAISE(m_ReadCount == NULL, MAllocError);

    mAttrCVal = (SChar **)idlOS::malloc((UInt)m_AttrCount * ID_SIZEOF(SChar *));
    IDE_TEST_RAISE(mAttrCVal == NULL, MAllocError);
    (void)idlOS::memset(mAttrCVal, 0, (UInt)m_AttrCount * ID_SIZEOF(SChar *));

    mAttrCValEltLen = (UInt *)idlOS::malloc((UInt)m_AttrCount * ID_SIZEOF(UInt));
    IDE_TEST_RAISE(mAttrCValEltLen == NULL, MAllocError);

    mAttrVal = (void **)idlOS::malloc((UInt)m_AttrCount * ID_SIZEOF(void *));
    IDE_TEST_RAISE(mAttrVal == NULL, MAllocError);
    (void)idlOS::memset(mAttrVal, 0, (UInt)m_AttrCount * ID_SIZEOF(void *));

    mAttrValEltLen = (UInt *)idlOS::malloc((UInt)m_AttrCount * ID_SIZEOF(UInt));
    IDE_TEST_RAISE(mAttrValEltLen == NULL, MAllocError);

    mAttrInd = (SQLLEN **)idlOS::malloc((UInt)m_AttrCount * ID_SIZEOF(SQLLEN *));
    IDE_TEST_RAISE(mAttrInd == NULL, MAllocError);
    (void)idlOS::memset(mAttrInd, 0, (UInt)m_AttrCount * ID_SIZEOF(SQLLEN *));

    mLOBPhyOffs = (ULong **)idlOS::malloc(
                                       (UInt)m_AttrCount * ID_SIZEOF(ULong *));
    IDE_TEST_RAISE(mLOBPhyOffs == NULL, MAllocError);
    (void)idlOS::memset(mLOBPhyOffs, 0,
                                       (UInt)m_AttrCount * ID_SIZEOF(ULong *));

    mLOBPhyLen = (ULong **)idlOS::malloc(
                                       (UInt)m_AttrCount * ID_SIZEOF(ULong *));
    IDE_TEST_RAISE(mLOBPhyLen == NULL, MAllocError);
    (void)idlOS::memset(mLOBPhyLen, 0,
                                       (UInt)m_AttrCount * ID_SIZEOF(ULong *));

    mLOBLen = (ULong **)idlOS::malloc((UInt)m_AttrCount * ID_SIZEOF(ULong *));
    IDE_TEST_RAISE(mLOBLen == NULL, MAllocError);
    (void)idlOS::memset(mLOBLen, 0, (UInt)m_AttrCount * ID_SIZEOF(ULong *));

    /* BUG - 18804 */
    if( sExistBadLog == 1 )
    {
        mAttrFail = (SChar **)idlOS::malloc((UInt)m_AttrCount * ID_SIZEOF(SChar *));
        IDE_TEST_RAISE(mAttrFail == NULL, MAllocError);
        // BUG-28208
        mAttrFailMaxLen = (SInt *)idlOS::malloc(m_AttrCount * ID_SIZEOF(SInt));
        IDE_TEST_RAISE(mAttrFailMaxLen == NULL, MAllocError);
        for (j = 0; j < m_AttrCount; j++)
        {
            mAttrFail[ j ] = ( SChar * )idlOS::malloc ( MAX_VARCHAR_SIZE + 1  );
            IDE_TEST_RAISE( mAttrFail[ j ] == NULL, MAllocError );
            mAttrFail[ j ][ 0 ] = '\0';
            mAttrFailMaxLen[j] = MAX_VARCHAR_SIZE;
        }
        /* TASK-2657 */
        mAttrFailLen = (UInt *)idlOS::malloc((UInt)m_AttrCount * ID_SIZEOF(UInt));
        idlOS::memset(mAttrFailLen, 0x00, (UInt)m_AttrCount * ID_SIZEOF(UInt));
        IDE_TEST_RAISE( mAttrFailLen == NULL, MAllocError );
    }
    else
    {
        mAttrFail = NULL;
        mAttrFailLen = NULL;
    }

    for (i = 0; i < m_AttrCount; i++)
    {
        if (m_AttrType[i] == ISP_ATTR_TIMESTAMP && sHandle->mParser.mAddFlag == ILO_TRUE)
        {
            mAttrCValEltLen[i] = mAttrValEltLen[i] = 0;
            continue;
        }

        switch (m_AttrType[i])
        {
        case ISP_ATTR_CHAR:
        case ISP_ATTR_VARCHAR:
            /* BUG - 18804 */
            mAttrCValEltLen[i] = (UInt)mPrecision[i] + 1;
            mAttrValEltLen[i] = 0;
            break;
        //===========================================================
        // proj1778 nchar
        case ISP_ATTR_NCHAR:
        case ISP_ATTR_NVARCHAR:
            mAttrCValEltLen[i] = (UInt)mPrecision[i] * 4;
            mAttrValEltLen[i] = 0;
            break;
        case ISP_ATTR_CLOB:
        case ISP_ATTR_BLOB:
            mAttrCValEltLen[i] = 0;
            mAttrValEltLen[i] = ID_SIZEOF(SQLUBIGINT);

            /* mLOBPhyOffs[i], mLOBPhyLen[i], mLOBLen[i]는
             * i번째 컬럼 타입이 LOB일 때만 메모리 할당한다. */
            mLOBPhyOffs[i] = (ULong *)idlOS::malloc(
                                         (UInt)sArrayCount * ID_SIZEOF(ULong));
            IDE_TEST_RAISE(mLOBPhyOffs[i] == NULL, MAllocError);

            mLOBPhyLen[i] = (ULong *)idlOS::malloc(
                                         (UInt)sArrayCount * ID_SIZEOF(ULong));
            IDE_TEST_RAISE(mLOBPhyLen[i] == NULL, MAllocError);

            mLOBLen[i] = (ULong *)idlOS::malloc(
                                         (UInt)sArrayCount * ID_SIZEOF(ULong));
            IDE_TEST_RAISE(mLOBLen[i] == NULL, MAllocError);

            break;
        case ISP_ATTR_BYTES:
            /* BUG - 18804 */
            mAttrCValEltLen[i] = (UInt)mPrecision[i] * 2 + 1;
            mAttrValEltLen[i] = 0;
            break;
        case ISP_ATTR_VARBYTE:
            /* BUG - 18804 */
            mAttrCValEltLen[i] = (UInt)mPrecision[i] * 2 + 1;
            mAttrValEltLen[i] = 0;
            break;
        case ISP_ATTR_NIBBLE:
            /* BUG - 18804 */
            mAttrCValEltLen[i] = (UInt)mPrecision[i] + 1;
            mAttrValEltLen[i] = 0;
            break;
        /* DATE타입의 string 길이는 길어야 64*3+1 을 넘지 않습니다. */
        case ISP_ATTR_DATE:
        case ISP_ATTR_TIMESTAMP:
            /* BUG - 18804 */
            mAttrCValEltLen[i] = MAX_NUMBER_SIZE + 1;
            mAttrValEltLen[i] = 0;
            break;
        case ISP_ATTR_FLOAT:
        case ISP_ATTR_DECIMAL:
        case ISP_ATTR_NUMERIC_DOUBLE:
        case ISP_ATTR_NUMERIC_LONG:
        case ISP_ATTR_SMALLINT:
        case ISP_ATTR_INTEGER:
        case ISP_ATTR_BIGINT:
        case ISP_ATTR_REAL:
            /* BUG - 18804 */
            mAttrCValEltLen[i] = MAX_NUMBER_SIZE + 1 ; 
            mAttrValEltLen[i] = 0;
            break;    
        case ISP_ATTR_DOUBLE:
            /* BUG - 18804 */
            mAttrCValEltLen[i] = MAX_NUMBER_SIZE + 1 ;
            mAttrValEltLen[i] = ID_SIZEOF(double);
            break;
        case ISP_ATTR_BIT:
            mAttrCValEltLen[i] = 60704 + 1;
            if (mPrecision[i] == 0)
            {
                mAttrValEltLen[i] = ID_SIZEOF(UInt);
            }
            else
            {
                mAttrValEltLen[i] = ID_SIZEOF(UInt) +
                        (UInt)((mPrecision[i] - 1) / 8 + 1);
            }
            break;
        case ISP_ATTR_VARBIT:
            mAttrCValEltLen[i] = 131070 + 1;
            if (mPrecision[i] == 0)
            {
                mAttrValEltLen[i] = ID_SIZEOF(UInt);
            }
            else
            {
                mAttrValEltLen[i] = ID_SIZEOF(UInt) +
                        (UInt)((mPrecision[i] - 1) / 8 + 1);
            }
            break;
        /* BUG-24384 iloader geometry support */
        case ISP_ATTR_GEOMETRY:
            /* BUG-31404 */
            mAttrCValEltLen[i] = (UInt)mPrecision[i] + ILO_GEOHEAD_SIZE;
            mAttrValEltLen[i]  = (SInt)mPrecision[i] + ILO_GEOHEAD_SIZE;
            break;
            
        default:
            mAttrCValEltLen[i] = MAX_VARCHAR_SIZE + 1;
            mAttrValEltLen[i] = 0;
            break;
        }

        if (mAttrCValEltLen[i] > 0)
        {
            mAttrCVal[i] = (SChar *)idlOS::malloc((UInt)sArrayCount *
                                                  mAttrCValEltLen[i]);
            IDE_TEST_RAISE(mAttrCVal[i] == NULL, MAllocError);
        }
        if (mAttrValEltLen[i] > 0)
        {
            mAttrVal[i] = idlOS::malloc((UInt)sArrayCount * mAttrValEltLen[i]);
            IDE_TEST_RAISE(mAttrVal[i] == NULL, MAllocError);
        }
        mAttrInd[i] = (SQLLEN *)idlOS::malloc(
                                        (UInt)sArrayCount * ID_SIZEOF(SQLLEN));
        IDE_TEST_RAISE(mAttrInd[i] == NULL, MAllocError);
        (void)idlOS::memset(mAttrInd[i], 0, (UInt)sArrayCount * ID_SIZEOF(SQLLEN));
    }

    mStatusPtr = (SQLUSMALLINT *)idlOS::malloc(
                                  (UInt)sArrayCount * ID_SIZEOF(SQLUSMALLINT));
    IDE_TEST_RAISE(mStatusPtr == NULL, MAllocError);

    return SQL_TRUE;

    IDE_EXCEPTION(MAllocError);
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_memory_error,
                        __FILE__, __LINE__);
        if ( sHandle->mUseApi != SQL_TRUE )
        {        
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }

        FreeTableAttr();
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

SChar *iloTableInfo::GetAttrName(SInt nAttr)
{
    if ((nAttr >= 0) && (nAttr < m_AttrCount))
    {
        return m_AttrName[nAttr];
    }
    else
    {
        return NULL;
    }
}

/* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
SChar *iloTableInfo::GetTransAttrName(SInt nAttr, SChar *aName, UInt aLen)
{
    if ((nAttr >= 0) && (nAttr < m_AttrCount))
    {
        utString::makeNameInSQL( aName,
                                 aLen,
                                 m_AttrName[nAttr],
                                 idlOS::strlen(m_AttrName[nAttr]) );
        return aName;
    }
    else
    {
        return NULL;
    }
}

void iloTableInfo::CopyStruct( ALTIBASE_ILOADER_HANDLE aHandle )
{
    SInt i;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    for (i = 0; i < sHandle->mTableInfomation.mSeqIndex; i++)
    {
        idlOS::strcpy(localSeqArray[i].seqName, sHandle->mTableInfomation.mSeqArray[i].seqName);
        idlOS::strcpy(localSeqArray[i].seqCol, sHandle->mTableInfomation.mSeqArray[i].seqCol);
        idlOS::strcpy(localSeqArray[i].seqVal, sHandle->mTableInfomation.mSeqArray[i].seqVal);
     }
}

EispAttrType iloTableInfo::GetAttrType(SInt nAttr)
{
    if ((nAttr >= 0) && (nAttr < m_AttrCount))
    {
        return m_AttrType[nAttr];
    }
    else
    {
        return ISP_ATTR_NONE;
    }
}
//==============================================================
// proj1778 nchar
// short(2bytes) endian convert
// calling function:
//  upload  : SetAttrValue()
//  download: iloDataFile::PrintOneRecord
// conversion need when nchar/nvarchar and utf16=yes and little-endian
//==============================================================
void iloTableInfo::ConvertShortEndian(SChar* aUTF16Buf, int aLenBytes)
{
    UShort* sWidePtr = (UShort*)aUTF16Buf;
    int sWideLen = aLenBytes/2;
    int sIdx;
    UShort sVal, sHigh, sLow;

    for (sIdx = 0; sIdx < sWideLen; sIdx++, sWidePtr++)
    {
       sVal = *sWidePtr;
       sHigh = sVal >> 8;
       sLow = (sVal & 0x00ff) << 8;
       *sWidePtr = (sHigh | sLow);
    }
}

IDE_RC iloTableInfo::SetAttrValue( ALTIBASE_ILOADER_HANDLE  aHandle,
                                   SInt                     nAttr,
                                   SInt                     aArrayCount,
                                   SChar                   *szAttrValue, 
                                   UInt                     aLen, 
                                   SInt                     aMsSql,
                                   iloBool                   aNCharUTF16)
{
    SChar *sCVal;
    SChar *sUsecSep;
    void  *sVal;
    SInt  i = 0;
    SInt  j = (SInt)aLen -1;
    SInt  sUpZero = 0;
    SInt  sDownZero = 0;
    SChar sColName[MAX_OBJNAME_LEN];

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    IDE_TEST(nAttr < 0 || nAttr >= m_AttrCount || aArrayCount < 0)

    // ignore blob, clob
    if ((m_AttrType[nAttr] == ISP_ATTR_BLOB) ||
        (m_AttrType[nAttr] == ISP_ATTR_CLOB))
    {
        IDE_CONT(SET_ATTR_SUCCESS);
    }
    
    /* BUG - 18804 */
    /* type에 따른 size길이에 대하여 최대값 511byte(null제외)를 넘었을 경우에 최소한의 error체크를 합니다. */
    switch (m_AttrType[nAttr])
    {
        case ISP_ATTR_CHAR:
            // BUG-24610 load 시에 공백을 제거해 버립니다.
            IDE_TEST_RAISE( aLen >= mAttrCValEltLen[nAttr] , OverflowLengthError);
            break;
        case ISP_ATTR_FLOAT:
        case ISP_ATTR_DECIMAL:
        case ISP_ATTR_NUMERIC_DOUBLE:
        case ISP_ATTR_NUMERIC_LONG:
        case ISP_ATTR_SMALLINT:
        case ISP_ATTR_INTEGER:
        case ISP_ATTR_BIGINT:
        case ISP_ATTR_REAL:
            if ( aLen >= mAttrCValEltLen[nAttr] )
            {
                /* Truncate Heading Zeros & Trailing Zeros */
                if( *(szAttrValue + i) == '-' || *(szAttrValue + i) == '+' )
                {
                    i++;
                }
                while( *(szAttrValue + i) == '0' )
                {
                    i++;
                    sUpZero++;
                }
                if( idlOS::strchr( szAttrValue, '.') != NULL )
                {
                    while( *( szAttrValue + j ) == '0' )
                    {
                        j--;
                        sDownZero++;
                    }
                }
                IDE_TEST_RAISE(aLen - sUpZero - sDownZero >= mAttrCValEltLen[nAttr], OverflowLengthError);

                if ( i >= j )
                {
                    *( szAttrValue ) = '0';
                    *( szAttrValue + 1 ) = '\0';
                    aLen = 1;
                }
                else if( sUpZero > 0 || sDownZero > 0 )
                {
                    while( i <= j  )    
                    {
                        *( szAttrValue + i - sUpZero ) = *( szAttrValue + i );
                        i++;    
                    }
                    *( szAttrValue + i - sUpZero ) = '\0';
                    aLen = i - sUpZero - sDownZero;
                }
            }

            // BUG-26426 iloader에서 Integer형의 데이터에 공백이 있을 경우, 에러가 발생합니다.
            // 전부 공백일 경우만 null 로 처리합니다.
            for(i = 0; i < (SInt)aLen; i++)
            {
                if( *(szAttrValue + i) != ' ')
                {
                    break;
                }
            }

            if(i == (SInt)aLen)
            {
                *szAttrValue = '\0';
            }
            break;
        default:
            IDE_TEST_RAISE(aLen >= mAttrCValEltLen[nAttr], OverflowLengthError);
            break;
    }

    sCVal = mAttrCVal[nAttr] + (UInt)aArrayCount * mAttrCValEltLen[nAttr];
    /* TASK-2657 */
    idlOS::memcpy( sCVal, szAttrValue, aLen + 1 );


    //==========================================================
    // proj1778 nchar
    // cpu가 little-endian이고 nchar/nvarchar 컬럼이고, nchar_utf16=yes이면
    // little-endian으로 변환시킨다 (datafile에는 big-endian으로만 저장되어 있다)
    // 그러면 cli에서 다시 big-endian으로 변환시킬 것이다
#ifndef ENDIAN_IS_BIG_ENDIAN
    if (((m_AttrType[nAttr] == ISP_ATTR_NCHAR) ||
        (m_AttrType[nAttr] == ISP_ATTR_NVARCHAR)) &&
        (aNCharUTF16 == ILO_TRUE))
    {
        ConvertShortEndian(sCVal, aLen);
    }
#endif

    switch (m_AttrType[nAttr])
    {
        case ISP_ATTR_BIT:
        case ISP_ATTR_VARBIT:
            sVal = (SChar *)mAttrVal[nAttr]
                + (UInt)aArrayCount * mAttrValEltLen[nAttr];
            IDE_TEST(ConvCharToBit(sCVal, (UInt)mPrecision[nAttr],
                        (UChar *)sVal, &mAttrInd[nAttr][aArrayCount])
                        != IDE_SUCCESS);
            break;
        case ISP_ATTR_BYTES:
        case ISP_ATTR_VARBYTE:
        case ISP_ATTR_TIMESTAMP:
            mAttrInd[nAttr][aArrayCount] =
                (sCVal[0] != '\0')? (SQLLEN)aLen: SQL_NULL_DATA;
            IDE_TEST((SInt)aLen > mPrecision[nAttr] * 2);
            break;
        case ISP_ATTR_DATE:
            mAttrInd[nAttr][aArrayCount] =
            (sCVal[0] != '\0')? (SQLLEN)aLen: SQL_NULL_DATA;
            /* CASE-4061 */
            if (aMsSql == SQL_TRUE)
            {
                sUsecSep = idlOS::strrchr(sCVal, '.');
                if (sUsecSep != NULL && idlOS::strlen(sUsecSep + 1) == 3)
                {
                    IDE_TEST(aLen + 3 >= mAttrCValEltLen[nAttr]);
                    *sUsecSep = ':';
                    idlVA::appendFormat(sCVal, mAttrCValEltLen[nAttr], "000");
                    mAttrInd[nAttr][aArrayCount] += 3;
                }
            }
            break;
        case ISP_ATTR_DOUBLE:
            sVal = (SChar *)mAttrVal[nAttr]
                + (UInt)aArrayCount * mAttrValEltLen[nAttr];
            if (sCVal[0] != '\0')
            {
                mAttrInd[nAttr][aArrayCount] = SQL_NTS;
                IDE_TEST_RAISE(StrToD(sCVal, (double *)sVal) != IDE_SUCCESS,
                               StrToDFailed);
            }
            else
            {
                mAttrInd[nAttr][aArrayCount] = SQL_NULL_DATA;
                *(double *)sVal = 0.;
            }
            break;
        /* TASK-2657 */
        /* to insert binary data(0x00) in char, varchar types */
        case ISP_ATTR_CHAR:
        case ISP_ATTR_VARCHAR:
        case ISP_ATTR_NCHAR:
        case ISP_ATTR_NVARCHAR:
        /* BUG-24384 iloader geometry support */
        case ISP_ATTR_GEOMETRY:
            mAttrInd[nAttr][aArrayCount] = (SQLLEN)aLen;
            break;
        default:
            mAttrInd[nAttr][aArrayCount] =
                (sCVal[0] != '\0')? (SQLLEN)aLen: SQL_NULL_DATA;
            break;
    }
    
    IDE_EXCEPTION_CONT(SET_ATTR_SUCCESS);
    return IDE_SUCCESS;

    /* BUG-31395 */
    IDE_EXCEPTION ( StrToDFailed );
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Invalid_Value_errno,
                        errno);
        SetReadCount( nAttr, aArrayCount );
    }
    /* BUG - 18804 */
    IDE_EXCEPTION ( OverflowLengthError );
    {
        // BUG-24823 iloader 에서 파일라인을 에러메시지로 출력하고 있어서 diff 가 발생합니다.
        // 파일명과 라인수를 출력하는 부분을 제거합니다.
        // BUG-24898 iloader 파싱에러 상세화
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Token_Value_Range_Error,
                        (SInt)(mAttrCValEltLen[nAttr] - 1),
                        GetTransAttrName(nAttr, sColName, (UInt)MAX_OBJNAME_LEN),
                        szAttrValue);
        SetReadCount( nAttr, aArrayCount);
    }
    
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC iloTableInfo::StrToD(SChar *aStr, double *aDVal)
{
    SChar *sP = NULL;

    errno = 0;
    *aDVal = idlOS::strtod(aStr, &sP);
    IDE_TEST(sP == aStr || errno == ERANGE);
    for (; *sP; sP++)
    {
        IDE_TEST(!isspace(*sP));
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    *aDVal = 0.;

    return IDE_FAILURE;
}

/**
 * ConvCharToBit.
 *
 * 문자열 형태의 BIT 데이터를 BIT 타입 고유의 바이너리 형태로 변환한다.
 * BIT 타입 고유의 바이너리 형태란,
 * 선두 4바이트에 UInt 형태의 비트수가 저장되고
 * 그 이후에 16진수 형태로 비트 데이터가 저장되는 방식을 말한다.
 *
 * @param[in] aCVal
 *  문자열 형태 BIT 데이터.
 * @param[in] aPrecision
 *  BIT 타입의 컬럼폭.
 * @param[out] aRaw
 *  BIT 타입 고유의 바이너리 형태의 BIT 데이터.
 * @param[out] aRawLen
 *  aRaw 데이터의 길이(Bytes).
 */
IDE_RC iloTableInfo::ConvCharToBit(SChar *aCVal, UInt aPrecision, UChar *aRaw,
                                   SQLLEN *aRawLen)
{
    UChar *sBit;
    UInt   sBitLen;

    if (aCVal[0] == '\0')
    {
        sBitLen = 0;
        idlOS::memcpy(aRaw, &sBitLen, ID_SIZEOF(UInt));
        *aRawLen = SQL_NULL_DATA;

        return IDE_SUCCESS;
    }

    sBit = aRaw + ID_SIZEOF(UInt);

    for (; *aCVal; aCVal++)
    {
        if (!isspace(*aCVal))
        {
            break;
        }
    }

    for (sBitLen = 0; *aCVal && sBitLen < aPrecision; aCVal++, sBitLen++)
    {
        if (*aCVal == '0')
        {
            sBit[sBitLen / 8] &= (UChar)(~(1 << (7 - sBitLen % 8)));
        }
        else if (*aCVal == '1')
        {
            sBit[sBitLen / 8] |= (UChar)(1 << (7 - sBitLen % 8));
        }
        else if (isspace(*aCVal))
        {
            break;
        }
        else
        {
            IDE_TEST(1);
        }
    }
    IDE_TEST(sBitLen == 0);

    for (; *aCVal; aCVal++)
    {
        if (!isspace(*aCVal))
        {
            IDE_TEST(1);
        }
    }

    idlOS::memcpy(aRaw, &sBitLen, ID_SIZEOF(UInt));
    *aRawLen = (SQLLEN)(ID_SIZEOF(UInt) + (sBitLen - 1) / 8 + 1);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    sBitLen = 0;
    idlOS::memcpy(aRaw, &sBitLen, ID_SIZEOF(UInt));
    *aRawLen = SQL_NULL_DATA;

    return IDE_FAILURE;
}

SInt iloTableInfo::PrintTableInfo()
{
    SInt i;

    idlOS::printf("Table [%s]\n", m_TableName);

    if (m_bDownCond == ILO_TRUE)
    {
        idlOS::printf("Download Condition [%s]\n", m_QueryString);
    }

    for (i = 0; i < m_AttrCount; i++)
    {
        idlOS::printf("%s ", m_AttrName[i]);

        switch (m_AttrType[i])
        {
        case ISP_ATTR_INTEGER :
            idlOS::printf("(integer) ");
            break;
        case ISP_ATTR_NIBBLE :
            idlOS::printf("(nibble) ");
            break;
        case ISP_ATTR_BYTES :
            idlOS::printf("(byte) ");
            break;
        case ISP_ATTR_VARBYTE :
            idlOS::printf("(varbyte) ");
            break;
        case ISP_ATTR_TIMESTAMP :
            idlOS::printf("(timestamp) ");
            break;
        case ISP_ATTR_CHAR :
            idlOS::printf("(char) ");
            break;
        case ISP_ATTR_VARCHAR :
            idlOS::printf("(varchar) ");
            break;
        case ISP_ATTR_NCHAR :
            idlOS::printf("(nchar) ");
            break;
        case ISP_ATTR_NVARCHAR :
            idlOS::printf("(nvarchar) ");
            break;
        case ISP_ATTR_BIT :
            idlOS::printf("(bit) ");
            break;
        case ISP_ATTR_VARBIT :
            idlOS::printf("(varbit) ");
            break;
        case ISP_ATTR_NUMERIC_LONG :
            idlOS::printf("(numeric long) ");
            break;
        case ISP_ATTR_NUMERIC_DOUBLE :
            idlOS::printf("(numeric double) ");
            break;
        case ISP_ATTR_DATE :
            idlOS::printf("(date) ");
            break;
        case ISP_ATTR_BLOB :
            idlOS::printf("(blob) ");
            break;
        case ISP_ATTR_CLOB :
            idlOS::printf("(clob) ");
            break;
        default :
            idlOS::printf("(none) ");
            break;
        }
    }

    return SQL_TRUE;
}

SInt iloTableInfo::seqEqualChk( ALTIBASE_ILOADER_HANDLE aHandle, SInt index )
{
    SInt j;
    SChar sSeqCol[MAX_OBJNAME_LEN];
    SChar sAttrName[MAX_OBJNAME_LEN];

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    for (j = 0; j < seqCount(sHandle); j++)
    {
        /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
        // SEQUENCE 컬럼 이름 대소문자 구분.
        utString::makeNameInSQL( sSeqCol,
                                 MAX_OBJNAME_LEN,
                                 localSeqArray[j].seqCol,
                                 idlOS::strlen(localSeqArray[j].seqCol) );
        utString::makeNameInSQL( sAttrName,
                                 MAX_OBJNAME_LEN,
                                 GetAttrName(index),
                                 idlOS::strlen(GetAttrName(index)) );

        if (idlOS::strcmp(sAttrName, sSeqCol)
            == 0)
        {
            return j;
        }
    }

    return -1;
}


SInt iloTableInfo::seqColChk( ALTIBASE_ILOADER_HANDLE aHandle )
{
    SInt i;
    SInt j;
    SInt exist = 0;
    SChar sSeqCol[MAX_OBJNAME_LEN];
    SChar sAttrName[MAX_WORD_LEN];

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;    

    for (j = 0; j < seqCount(sHandle); j++)
    {
        exist = 0;
        for (i = 0; i < GetAttrCount(); i++)
        {
            /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
            // SEQUENCE 컬럼 이름 대소문자 구분.
            utString::makeNameInSQL( sSeqCol,
                                     MAX_OBJNAME_LEN,
                                     localSeqArray[j].seqCol,
                                     idlOS::strlen(localSeqArray[j].seqCol) );
            utString::makeNameInSQL( sAttrName,
                                     MAX_WORD_LEN,
                                     GetAttrName(i),
                                     idlOS::strlen(GetAttrName(i)) );

            // SEQUENCE 컬럼 이름이 table 컬럼이름에 존재하는지 확인함.
            if (idlOS::strcmp(sAttrName, sSeqCol)
                == 0)
            {
                exist = 1;
                break;
            }
        }
        IDE_TEST(exist == 0);
    }

    return SQL_TRUE;

    IDE_EXCEPTION_END;

    uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_No_Column_Sequence_Error,
                    localSeqArray[j].seqCol);
    
    if ( sHandle->mUseApi != SQL_TRUE )
    {
        utePrintfErrorCode(stdout, sHandle->mErrorMgr);
    }

    return SQL_FALSE;
}

SInt iloTableInfo::seqDupChk( ALTIBASE_ILOADER_HANDLE aHandle )
{
    SInt i;
    SInt j;
    SInt exist = 0;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    for (j = 0; j < seqCount(sHandle); j++)
    {
        exist = 0;
        for (i = 0; i < seqCount(sHandle); i++)
        {
            if (idlOS::strcasecmp(localSeqArray[i].seqCol, localSeqArray[j].seqCol)
                == 0)
            {
                exist++;
            }
        }
        IDE_TEST(exist > 1);
    }

    return SQL_TRUE;

    IDE_EXCEPTION_END;

    uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_Many_Column_Sequence_Error,
                    localSeqArray[j].seqCol, exist);

    if ( sHandle->mUseApi != SQL_TRUE )
    {
        utePrintfErrorCode(stdout, sHandle->mErrorMgr);
    }

    return SQL_FALSE;
}

SInt iloTableInfo::seqCount( ALTIBASE_ILOADER_HANDLE aHandle )
{
    SInt i;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    for (i = 0; i < sHandle->mTableInfomation.mSeqIndex; i++)
    {
        if (idlOS::strcasecmp(localSeqArray[i].seqName, "") == 0)
        {
            return i;
        }
    }

    return i;
}

/**
 * Reset.
 *
 * iloTableInfo 객체에 할당된 모든 메모리를 해제하고
 * 변수를 초기화한다.
 */
void iloTableInfo::Reset()
{
    FreeDateFormat();
    FreeTableAttr();

    (void)idlOS::memset(localSeqArray, 0, ID_SIZEOF(localSeqArray));
    m_TableName[0] = '\0';
    m_bDownCond = ILO_FALSE;
    m_QueryString[0] = '\0';
    m_AttrCount = 0;
    (void)idlOS::memset(m_AttrName, 0, ID_SIZEOF(m_AttrName));
    (void)idlOS::memset(m_AttrType, 0, ID_SIZEOF(m_AttrType));
    m_HintString[0] = '\0';
    (void)idlOS::memset(mSkipFlag, 0, ID_SIZEOF(mSkipFlag));
    (void)idlOS::memset(mNoExpFlag, 0, ID_SIZEOF(mNoExpFlag));
    (void)idlOS::memset(mOutFileFlag, 0, ID_SIZEOF(mOutFileFlag)); //PROJ-2030, CT_CASE-3020 CHAR outfile 지원 
    (void)idlOS::memset(mPrecision, 0, ID_SIZEOF(mPrecision));
    (void)idlOS::memset(mScale, 0, ID_SIZEOF(mScale));
    mIsQueue = 0;
}

void iloTableInfo::FreeTableAttr()
{
    SInt sI;

    if (m_ReadCount != NULL)
    {
        idlOS::free(m_ReadCount);
        m_ReadCount = NULL;
    }
    if (mAttrCVal != NULL)
    {
        for (sI = 0; sI < m_AttrCount; sI++)
        {
            idlOS::free(mAttrCVal[sI]);
        }
        idlOS::free(mAttrCVal);
        mAttrCVal = NULL;
    }
    if (mAttrCValEltLen != NULL)
    {
        idlOS::free(mAttrCValEltLen);
        mAttrCValEltLen = NULL;
    }
    if (mAttrVal != NULL)
    {
        for (sI = 0; sI < m_AttrCount; sI++)
        {
            idlOS::free(mAttrVal[sI]);
        }
        idlOS::free(mAttrVal);
        mAttrVal = NULL;
    }
    if (mAttrValEltLen != NULL)
    {
        idlOS::free(mAttrValEltLen);
        mAttrValEltLen = NULL;
    }
    if (mAttrInd != NULL)
    {
        for (sI = 0; sI < m_AttrCount; sI++)
        {
            idlOS::free(mAttrInd[sI]);
        }
        idlOS::free(mAttrInd);
        mAttrInd = NULL;
    }
    if (mLOBPhyOffs != NULL)
    {
        for (sI = 0; sI < m_AttrCount; sI++)
        {
            idlOS::free(mLOBPhyOffs[sI]);
        }
        idlOS::free(mLOBPhyOffs);
        mLOBPhyOffs = NULL;
    }
    if (mLOBPhyLen != NULL)
    {
        for (sI = 0; sI < m_AttrCount; sI++)
        {
            idlOS::free(mLOBPhyLen[sI]);
        }
        idlOS::free(mLOBPhyLen);
        mLOBPhyLen = NULL;
    }
    if (mLOBLen != NULL)
    {
        for (sI = 0; sI < m_AttrCount; sI++)
        {
            idlOS::free(mLOBLen[sI]);
        }
        idlOS::free(mLOBLen);
        mLOBLen = NULL;
    }
    if (mStatusPtr != NULL)
    {
        idlOS::free(mStatusPtr);
        mStatusPtr = NULL;
    }
    /* BUG - 18804 */
    if (mAttrFail != NULL)
    {
        for (sI = 0; sI < m_AttrCount; sI++)
        {
            idlOS::free(mAttrFail[sI]);
        }
        idlOS::free(mAttrFail);
        mAttrFail = NULL;
    }
    if (mAttrFailMaxLen != NULL)
    {
        idlOS::free(mAttrFailMaxLen);
        mAttrFailMaxLen = NULL;
    }
    if (mAttrFailLen != NULL )
    {
        idlOS::free(mAttrFailLen);
        mAttrFailLen = NULL;
    }
}

void iloTableInfo::FreeDateFormat()
{
    SInt sI;

    for (sI = 0; sI < m_AttrCount; sI++)
    {
        if (mAttrDateFormat[sI] != NULL)
        {
            idlOS::free(mAttrDateFormat[sI]);
            mAttrDateFormat[sI] = NULL;
        }
    }
}

/* BUG - 18804 */
SChar *iloTableInfo::GetAttrFail(SInt nAttr)
{
    if ( mAttrFail[ nAttr ] != NULL )
    {
        return ( SChar * ) mAttrFail[ nAttr ];
    }
    return NULL;
}

// BUG-28208
/**
 * 에러난 토큰을 mAttrFail[aIdx]에 복사.
 *
 * mAttrFail[aIdx]가 복사하려는 토큰 길이보다 작으면
 * mAttrFail[aIdx]을 다시 할당한 후 복사한다.
 *
 * @param [IN] aIdx      에러난 필드 인덱스
 * @param [IN] aToken    에러난 데이타
 * @param [IN] aTokenLen 에러난 데이타 길이
 * @return 잘 복사했으면 IDE_SUCCESS, 그렇지 않으면 IDE_FAILURE
 */
IDE_RC iloTableInfo::SetAttrFail( ALTIBASE_ILOADER_HANDLE  aHandle, 
                                  SInt                     aIdx,
                                  SChar                   *aToken,
                                  SInt                     aTokenLen)
{

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    if ( sHandle->mUseApi != SQL_TRUE )
    {
        // 내부에서만 사용되는 함수이므로 절대 이런 에러가 나서는 안된다.
        IDE_DASSERT( (0 <= aIdx) && (aIdx < m_AttrCount) );
        IDE_DASSERT( (aToken != NULL) && (aTokenLen >= 0) );
    }

    if (mAttrFailMaxLen[aIdx] < aTokenLen)
    {
        idlOS::free(mAttrFail[aIdx]);

        mAttrFail[aIdx] = (SChar *) idlOS::malloc(aTokenLen + 1);
        IDE_TEST_RAISE(mAttrFail[aIdx] == NULL, MAllocError);
        mAttrFailMaxLen[aIdx] = aTokenLen;
    }

    idlOS::memcpy(mAttrFail[aIdx], aToken, aTokenLen + 1);
    mAttrFailLen[aIdx] = aTokenLen;

    return IDE_SUCCESS;

    IDE_EXCEPTION(MAllocError);
    {
        mAttrFail[aIdx]       = NULL;
        mAttrFailMaxLen[aIdx] = 0;
        mAttrFailLen[aIdx]    = 0;

        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_memory_error, __FILE__, __LINE__);
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
SChar *iloTableInfo::GetTransTableName(SChar *aName, UInt aLen)
{
    utString::makeNameInSQL( aName,
                             aLen,
                             GetTableName(),
                             idlOS::strlen(GetTableName()) );
    return aName;
}
