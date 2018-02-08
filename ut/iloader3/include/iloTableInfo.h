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
 * $Id: iloTableInfo.h 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

#ifndef _O_ILO_TABLEINFO_H
#define _O_ILO_TABLEINFO_H

#include <iloApi.h>

void InsertSeq( ALTIBASE_ILOADER_HANDLE  aHandle,
                SChar                   *sName,
                SChar                   *cName,
                SChar                   *val);

class iloTableNode
{
public:
    iloTableNode();

    iloTableNode(ETableNodeType eNodeType,
                 SChar         *szNodeValue,
                 iloTableNode  *pSon,
                 iloTableNode  *pBrother,
                 SChar         *szHint = NULL);

    iloTableNode(ETableNodeType eNodeType,
                 SChar         *szNodeValue,
                 iloTableNode  *pSon,
                 iloTableNode  *pBrother,
                 SInt           aIsQueue,
                 SChar         *szHint = NULL);

    ~iloTableNode();

    ETableNodeType GetNodeType()             { return m_NodeType; }

    SChar *GetNodeValue()                    { return m_NodeValue; }

    iloTableNode *GetSon()                   { return m_pSon; }

    iloTableNode *GetBrother()               { return m_pBrother; }

    void SetSon(iloTableNode *pSon)          { m_pSon = pSon; }

    void SetBrother(iloTableNode *pBrother)  { m_pBrother = pBrother; }

    void setSkipFlag(SInt aSkipFlag)
    {
        mSkipFlag = (aSkipFlag != 0) ? ILO_TRUE : ILO_FALSE;
    }

    void setNoExpFlag( SInt aNoExpFlag )
    {
        mNoExpFlag = (aNoExpFlag != 0) ? ILO_TRUE : ILO_FALSE;
    }

    void setPrecision( SInt aPrecision, SInt aScale );

    SInt PrintNode(SInt nDepth);
    SInt getIsQueue()                        { return mIsQueue; }

    void setBinaryFlag(SInt aBinaryFlag)
    {
        mBinaryFlag = (aBinaryFlag != 0) ? ILO_TRUE : ILO_FALSE;
    }
    
    // PROJ-2030, CT_CASE-3020 CHAR outfile 지원
    void setOutFileFlag(SInt aOutFileFlag)    
    {
        mOutFileFlag = (aOutFileFlag != 0) ? ILO_TRUE : ILO_FALSE;    
    }

private:
    ETableNodeType  m_NodeType;
    SChar          *m_NodeValue;
    iloTableNode   *m_pSon;
    iloTableNode   *m_pBrother;
    SInt            mIsQueue;

public:
    iloBool          mSkipFlag;
    SInt            mPrecision;
    SInt            mScale;
    iloBool          mNoExpFlag;
    SChar          *m_Hint;
    iloTableNode   *m_Condition;
    iloTableNode   *m_DateFormat;
    iloBool          mBinaryFlag;
    iloBool          mOutFileFlag;  // PROJ-2030, CT_CASE-3020 CHAR outfile 지원
};

class iloTableTree
{
public:
    iloTableTree()                     { m_Root = NULL; }

    iloTableTree(iloTableNode *pRoot)  { m_Root = pRoot; }

    ~iloTableTree()                    { delete m_Root; }

    SInt SetTreeRoot(iloTableNode *pRoot);

    iloTableNode *GetTreeRoot()        { return m_Root; }

    void FreeTree()                    { delete m_Root; m_Root = NULL; }

    SInt PrintTree();

private:
    iloTableNode *m_Root;
};

class iloTableInfo
{
public:
    iloTableInfo();

    ~iloTableInfo();

    struct SeqInfo
    {
        SChar seqName[MAX_OBJNAME_LEN];
        SChar seqCol[MAX_OBJNAME_LEN];
        SChar seqVal[UT_MAX_SEQ_VAL_LEN];
    } localSeqArray[UT_MAX_SEQ_ARRAY_CNT];

    void Reset();

    SInt GetTableInfo( ALTIBASE_ILOADER_HANDLE aHandle, iloTableNode *pTableNameNode);
    // TABLE_NODE에 대한 포인터를 입력으로 받아 테이블에 대한 정보를 얻는다.
    // 이때 형제 테이블 노드가 있는지는 검색하지 않는다.
    iloBool ExistDownCond()               { return m_bDownCond; }

    SChar *GetQueryString()              { return m_QueryString; }

    SChar *GetTableName()                { return m_TableName; }

    /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
    SChar *GetTransTableName(SChar *aName, UInt aLen);

    SInt GetAttrCount()                  { return m_AttrCount; }

    SInt GetReadCount(SInt aIdx)         { return m_ReadCount[aIdx]; }

    void SetReadCount(SInt nReadCount, SInt aIdx)
                                         { m_ReadCount[aIdx] = nReadCount; }

    void CopyStruct( ALTIBASE_ILOADER_HANDLE aHandle );

    SChar *GetAttrName(SInt nAttr);

    /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
    SChar *GetTransAttrName(SInt nAttr, SChar *aName, UInt aLen);

    EispAttrType GetAttrType(SInt nAttr);

    void ConvertShortEndian(SChar* aUTF16Buf, int aLenBytes);

    IDE_RC SetAttrValue( ALTIBASE_ILOADER_HANDLE  aHandle,
                         SInt                     nAttr,
                         SInt                     aArrayCount,
                         SChar                   *szAttrValue,
                         UInt                     aLen,
                         SInt                     aMsSql,
                         iloBool                   aNCharUTF16);
        
    SChar *GetAttrCVal(SInt nAttr, SInt aArrayCnt = 0);

    /* BUG - 18804 */
    SChar *GetAttrFail( SInt nAttr );
    // BUG-28208
    IDE_RC SetAttrFail( ALTIBASE_ILOADER_HANDLE  aHandle,
                        SInt                     aIdx,
                        SChar                   *aToken,
                        SInt                     aTokenLen);

    void *GetAttrVal(SInt nAttr, SInt aArrayCnt = 0);

    SInt PrintTableInfo();

    SInt seqEqualChk( ALTIBASE_ILOADER_HANDLE aHandle, SInt index );

    SInt seqColChk( ALTIBASE_ILOADER_HANDLE aHandle );

    SInt seqDupChk( ALTIBASE_ILOADER_HANDLE aHandle );

    SInt seqCount( ALTIBASE_ILOADER_HANDLE aHandle );

    SInt AllocTableAttr( ALTIBASE_ILOADER_HANDLE aHandle, 
                         SInt                    aArrayCount,
                         SInt                    aExistBadLog);
    void FreeTableAttr();
    void FreeDateFormat();
   
private:
    SChar               m_TableName[MAX_OBJNAME_LEN];
    iloBool              m_bDownCond;
    /* BUG-32395 The condition at iloader should change the size of the string. */
    SChar               m_QueryString[MAX_WORD_LEN*64];
    SInt                m_AttrCount;
    SInt               *m_ReadCount;
    SChar               m_AttrName[MAX_ATTR_COUNT][MAX_OBJNAME_LEN];
    EispAttrType        m_AttrType[MAX_ATTR_COUNT];

    IDE_RC StrToD(SChar *aStr, double *aDVal);
    IDE_RC ConvCharToBit(SChar *aCVal, UInt aPrecision, UChar *aRaw,
                         SQLLEN *aRawLen);

public:
    /* * 이하 6 개의 변수는 iLoader의 in 명령 시 사용 * */
    /* 문자열 형태의 값이 저장되는 버퍼.
     * SQLBindParameter()에 의해 바인드될 수 있다. */
    SChar             **mAttrCVal;
    /* mAttrCVal의 각 원소(SChar *)가 가리키는 배열의 원소 크기 */
    UInt               *mAttrCValEltLen;
    /* 데이터형 고유 형태의 값이 저장되는 버퍼.
     * SQLBindParameter()에 의해 바인드될 수 있다. */
    void              **mAttrVal;
    /* mAttrVal의 각 원소(void *)가 가리키는 배열의 원소 크기 */
    UInt               *mAttrValEltLen;
    /* BUG - 18804 */
    /* ReadOneRecord()할 때 error난 필드들을 잠시 저장하는 버퍼.*/
    SChar             **mAttrFail;
    /* BUG-28208 */
    SInt               *mAttrFailMaxLen;
    /* TASK-2657 */
    UInt               *mAttrFailLen;
    /* SQLBindParameter()에 인자로 줄 지시자 변수 */
    SQLLEN            **mAttrInd;
    /* 데이터 파일 또는 LOB 파일 내에서 LOB 데이터의 시작 위치.
     * LOB 컬럼인 경우만 메모리 할당된다.
     * "물리적"의 의미는 Windows 플랫폼에서 "\n"이 "\r\n"으로 저장되는데,
     * LOB 데이터에 "\n"이 있을 경우 실제 저장되는 길이인 2로 카운트한다는 것이다.
     * use_lob_file=no 또는
     * use_lob_file=yes, use_separate_files=no인 경우 사용된다. */
    ULong             **mLOBPhyOffs;
    /* LOB 데이터의 물리적 길이.
     * LOB 컬럼인 경우만 메모리 할당된다.
     * use_lob_file=no 또는
     * use_lob_file=yes, use_separate_files=no인 경우 사용된다. */
    ULong             **mLOBPhyLen;
    /* LOB 데이터의 길이.
     * LOB 컬럼인 경우만 메모리 할당된다.
     * "물리적" 길이와는 달리 "\n"은 1바이트로 카운트한다.
     * use_lob_file=no인 경우 사용된다. */
    ULong             **mLOBLen; /* Used only when use_lob_file=no */
    SChar               m_HintString[MAX_WORD_LEN*3];
    SQLUSMALLINT       *mStatusPtr;
    iloBool              mSkipFlag[MAX_ATTR_COUNT];
    iloBool              mOutFileFlag[MAX_ATTR_COUNT];  // PROJ-2030, CT_CASE-3020 CHAR outfile 지원
    iloBool              mNoExpFlag[MAX_ATTR_COUNT];
    SInt                mPrecision[MAX_ATTR_COUNT];
    SInt                mScale[MAX_ATTR_COUNT];
    SChar              *mAttrDateFormat[MAX_ATTR_COUNT];
    SInt                mIsQueue;
    UInt                mLOBColumnCount;    //BUG-24583 Table에 포함된 LOB Column의 개수
};

inline SChar *iloTableInfo::GetAttrCVal(SInt nAttr, SInt aArrayCnt)
{
    IDE_TEST((nAttr < 0) || (nAttr >= m_AttrCount) || (aArrayCnt < 0));

    return mAttrCVal[nAttr] + (UInt)aArrayCnt * mAttrCValEltLen[nAttr];

    IDE_EXCEPTION_END;

    return NULL;
}

inline void *iloTableInfo::GetAttrVal(SInt nAttr, SInt aArrayCnt)
{
    IDE_TEST((nAttr < 0) || (nAttr >= m_AttrCount) || (aArrayCnt < 0));

    return (SChar *)mAttrVal[nAttr] + (UInt)aArrayCnt * mAttrValEltLen[nAttr];

    IDE_EXCEPTION_END;

    return NULL;
}
#endif /* _O_ILO_TABLEINFO_H */


