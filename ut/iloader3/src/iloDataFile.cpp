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
 * $Id: iloDataFile.cpp 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

#include <ilo.h>

#if (defined(VC_WIN32) || defined(VC_WIN64)) && _MSC_VER < 1400
extern "C" int _fseeki64(FILE *, __int64, int);
#endif

// BUG-24902: FilePath가 절대 경로인지 확인
#if defined(VC_WIN32)
#define IS_ABSOLUTE_PATH(P) ((P[0] == IDL_FILE_SEPARATOR) || ((strlen(P) > 3) && (P[1] == ':') && (P[2] == IDL_FILE_SEPARATOR)))
#else
#define IS_ABSOLUTE_PATH(P)  (P[0] == IDL_FILE_SEPARATOR)
#endif

iloLOB::iloLOB()
{
#if (SIZEOF_LONG == 8) || defined(HAVE_LONG_LONG) || defined(VC_WIN32)
    mLOBLoc = 0;
# else
    mLOBLoc.hiword = mLOBLoc.loword = 0;
#endif
    *(SShort *)&mLOBLocCType = 0;
    *(SShort *)&mValCType = 0;
    mLOBLen = 0;
    mPos = 0;
    mIsFetchError = ILO_FALSE;
    mLOBAccessMode = LOBAccessMode_RDONLY;
    /* BUG-21064 : CLOB type CSV up/download error */
    mIsBeginCLOBAppend  = ILO_FALSE;
    mSaveBeginCLOBEnc   = ILO_FALSE;
    mIsCSVCLOBAppend    = ILO_FALSE;
}

iloLOB::~iloLOB()
{

}

/**
 * Open.
 *
 * 조작할 LOB에 관한 정보를 설정한다.
 * LOB 읽기의 경우 서버로부터 LOB 길이를 얻는 일도 수행한다.
 *
 * @param[in] aLOBLocCType
 *  LOB locator의 C 데이터형. 다음 값 중 하나이다.
 *  SQL_C_CLOB_LOCATOR, SQL_C_BLOB_LOCATOR.
 * @param[in] aLOBLoc
 *  LOB locator
 * @param[in] aValCType
 *  값의 C 데이터형. 다음 값 중 하나이다.
 *  SQL_C_CHAR, SQL_C_BINARY.
 *  단, SQL_C_BINARY는 aLOBLocCType이
 *  SQL_C_BLOB_LOCATOR인 경우만 가능하다.
 * @param[in] aLOBAccessMode
 *  LOB 읽기 또는 쓰기를 지정한다. 다음 값 중 하나이다.
 *  iloLOB::LOBAccessMode_RDONLY,
 *  iloLOB::LOBAccessMode_WRONLY.
 */
IDE_RC iloLOB::OpenForUp( ALTIBASE_ILOADER_HANDLE  /* aHandle */,
                          SQLSMALLINT              aLOBLocCType,
                          SQLUBIGINT               aLOBLoc,
                          SQLSMALLINT              aValCType,
                          LOBAccessMode            aLOBAccessMode,
                          iloSQLApi               *aISPApi )
{
    UInt sLOBLen;

    /* 1.Check validity of arguments. */
    IDE_TEST(aLOBLocCType != SQL_C_BLOB_LOCATOR &&
             aLOBLocCType != SQL_C_CLOB_LOCATOR);
#if (SIZEOF_LONG == 8) || defined(HAVE_LONG_LONG) || defined(VC_WIN32)
    IDE_TEST((ULong)aLOBLoc == ID_ULONG(0));
# else
    IDE_TEST(aLOBLoc.hiword == 0 && aLOBLoc.loword == 0);
#endif
    if (aLOBLocCType == SQL_C_BLOB_LOCATOR)
    {
        IDE_TEST((aValCType != SQL_C_BINARY) && (aValCType != SQL_C_CHAR));
    }
    else /* (aLOBLocCType == SQL_C_CLOB_LOCATOR) */
    {
        IDE_TEST(aValCType != SQL_C_CHAR);
    }

    /* 2.Get LOB length. */
    if (aLOBAccessMode == LOBAccessMode_RDONLY)
    {
        IDE_TEST_RAISE(aISPApi->GetLOBLength(aLOBLoc, aLOBLocCType, &sLOBLen)
                       != IDE_SUCCESS, GetLOBLengthError);
            mLOBLen = sLOBLen;
    }
    else
    {
        mLOBLen = 0;
    }

    /* 3.Initialize class member variables. */
    mLOBLocCType = aLOBLocCType;
    mLOBLoc = aLOBLoc;
    mValCType = aValCType;
    mLOBAccessMode = aLOBAccessMode;
    mPos = 0;
    mIsFetchError = ILO_FALSE;

    return IDE_SUCCESS;

    IDE_EXCEPTION(GetLOBLengthError);
    {
        /* BUG-42849 Error code and message should not be output to stdout.
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }*/
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


IDE_RC iloLOB::OpenForDown( ALTIBASE_ILOADER_HANDLE aHandle,
                            SQLSMALLINT             aLOBLocCType, 
                            SQLUBIGINT              aLOBLoc,
                            SQLSMALLINT             aValCType, 
                            LOBAccessMode           aLOBAccessMode)
{
    UInt sLOBLen;
    
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
        
    /* 1.Check validity of arguments. */
    IDE_TEST(aLOBLocCType != SQL_C_BLOB_LOCATOR &&
             aLOBLocCType != SQL_C_CLOB_LOCATOR);
#if (SIZEOF_LONG == 8) || defined(HAVE_LONG_LONG) || defined(VC_WIN32)
    IDE_TEST((ULong)aLOBLoc == ID_ULONG(0));
# else
    IDE_TEST(aLOBLoc.hiword == 0 && aLOBLoc.loword == 0);
#endif
    if (aLOBLocCType == SQL_C_BLOB_LOCATOR)
    {
        IDE_TEST((aValCType != SQL_C_BINARY) && (aValCType != SQL_C_CHAR));
    }
    else /* (aLOBLocCType == SQL_C_CLOB_LOCATOR) */
    {
        IDE_TEST(aValCType != SQL_C_CHAR);
    }

    /* 2.Get LOB length. */
    if (aLOBAccessMode == LOBAccessMode_RDONLY)
    {
        IDE_TEST_RAISE(sHandle->mSQLApi->GetLOBLength(aLOBLoc, aLOBLocCType, &sLOBLen)
                       != IDE_SUCCESS, GetLOBLengthError);
            mLOBLen = sLOBLen;
    }
    else
    {
        mLOBLen = 0;
    }

    /* 3.Initialize class member variables. */
    mLOBLocCType = aLOBLocCType;
    mLOBLoc = aLOBLoc;
    mValCType = aValCType;
    mLOBAccessMode = aLOBAccessMode;
    mPos = 0;
    mIsFetchError = ILO_FALSE;

    return IDE_SUCCESS;

    IDE_EXCEPTION(GetLOBLengthError);
    {
        /* BUG-42849 Error code and message should not be output to stdout.
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }*/
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
/**
 * Close.
 *
 * 조작하던 LOB 관련 자원을 해제한다.
 */
IDE_RC iloLOB::CloseForUp( ALTIBASE_ILOADER_HANDLE /* aHandle */, iloSQLApi *aISPApi )
{
#if (SIZEOF_LONG == 8) || defined(HAVE_LONG_LONG) || defined(VC_WIN32)
    if ((ULong)mLOBLoc != ID_ULONG(0))
#else
    if ((mLOBLoc.hiword != 0 || mLOBLoc.loword != 0))
#endif
    {
        /* 1.Free resources (for example, locks) related to LOB locator. */
        IDE_TEST_RAISE(aISPApi->FreeLOB(mLOBLoc) != IDE_SUCCESS,
                       FreeLOBError);

        /* 2.Reset LOB locator member variable. */
#if (SIZEOF_LONG == 8) || defined(HAVE_LONG_LONG) || defined(VC_WIN32)
        *(ULong *)&mLOBLoc = ID_ULONG(0);
# else
        mLOBLoc.hiword = mLOBLoc.loword = 0;
#endif
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(FreeLOBError);
    {
        /* BUG-42849 Error code and message should not be output to stdout.
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }*/
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


IDE_RC iloLOB::CloseForDown( ALTIBASE_ILOADER_HANDLE aHandle )
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
#if (SIZEOF_LONG == 8) || defined(HAVE_LONG_LONG) || defined(VC_WIN32)
    if ((ULong)mLOBLoc != ID_ULONG(0))
#else
    if ((mLOBLoc.hiword != 0 || mLOBLoc.loword != 0))
#endif
    {
        /* 1.Free resources (for example, locks) related to LOB locator. */
        IDE_TEST_RAISE( sHandle->mSQLApi->FreeLOB(mLOBLoc) != IDE_SUCCESS,
                       FreeLOBError);

        /* 2.Reset LOB locator member variable. */
#if (SIZEOF_LONG == 8) || defined(HAVE_LONG_LONG) || defined(VC_WIN32)
        *(ULong *)&mLOBLoc = ID_ULONG(0);
# else
        mLOBLoc.hiword = mLOBLoc.loword = 0;
#endif
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(FreeLOBError);
    {
        /* BUG-42849 Error code and message should not be output to stdout.
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }*/
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * Fetch.
 *
 * LOB 읽기 시 호출되며,
 * 서버로부터 일정 길이의 LOB 데이터를 읽어와 리턴한다.
 * 내부적으로 현재까지 읽은 위치를 기억하고 있다가
 * 다음 Fetch() 호출 시 이어지는 부분을 읽어서 리턴한다.
 *
 * @param[out] aVal
 *  서버로부터 읽어온 LOB 데이터가 저장되어있는 버퍼.
 *  데이터는 NULL로 종결되지 않는다.
 * @param[out] aStrLen
 *  버퍼에 저장되어있는 LOB 데이터의 길이.
 */
IDE_RC iloLOB::Fetch( ALTIBASE_ILOADER_HANDLE   aHandle,
                      void                    **aVal,
                      UInt                     *aStrLen)
{
    UInt         sForLength;
    UInt         sValueLength;
    SQLSMALLINT  sTargetCType;
    void        *sValue;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    /* 1.Check whether we have already fetched all. */
    IDE_TEST_RAISE(mLOBLen <= mPos, AllFetched);

    /* 2.Call SQLGetLob(). */
    // BUG-21837 LOB 버퍼 사이즈를 조절한다.
    if ((mLOBLen - mPos) > ILO_LOB_PIECE_SIZE)
    {
        sForLength = ILO_LOB_PIECE_SIZE;
    }
    else
    {
        sForLength = mLOBLen - mPos;
    }
    if (mLOBLocCType == SQL_C_BLOB_LOCATOR)
    {
        sTargetCType = SQL_C_BINARY;
        sValue = mBinBuf;
    }
    else /* (mLOBLocCType == SQL_C_CLOB_LOCATOR) */
    {
        sTargetCType = SQL_C_CHAR;
        /* BUG-21064 : CLOB type CSV up/download error */
        if( ( sHandle->mProgOption->mRule == csv ) &&
            ( sHandle->mProgOption->mUseLOBFile != ILO_TRUE ) )
        {
            sValue = mBinBuf;
        }
        else
        {
            sValue = mChBuf;
        }
    }
    IDE_TEST_RAISE( sHandle->mSQLApi->GetLOB( mLOBLocCType,
                                              mLOBLoc, 
                                              mPos,
                                              sForLength,
                                              sTargetCType,
                                              sValue,
                                              ILO_LOB_PIECE_SIZE,
                                              &sValueLength)
                                              != IDE_SUCCESS, GetLOBError);



    /* 3.Update current position in LOB. */
    mPos += sValueLength;

    /* 4.If column's type is BLOB and user buffer's type is char,
     * convert binary data to hexadecimal character string. */
    if (mLOBLocCType == SQL_C_BLOB_LOCATOR && mValCType == SQL_C_CHAR)
    {
        ConvertBinaryToChar(sValueLength);
        sValueLength *= 2;
    }
    /* BUG-21064 : CLOB type CSV up/download error */
    else
    {
        if ( CLOB4CSV && ( sHandle->mProgOption->mUseLOBFile != ILO_TRUE ) )
        {
            // 원본 CLOB data를 CSV형태로 변환한다.
             iloConvertCharToCSV( sHandle, &sValueLength);
        }
    }

    /* 5.Make return values. */
    if (mValCType == SQL_C_BINARY)
    {
        *(UChar **)aVal = mBinBuf;
    }
    else /* (mValCType == SQL_C_CHAR) */
    {
        *(SChar **)aVal = mChBuf;
    }
    *aStrLen = (UInt)sValueLength;

    mIsFetchError = ILO_FALSE;
    
   
    return IDE_SUCCESS;

    IDE_EXCEPTION(AllFetched);
    {
        IDE_TEST_RAISE( sHandle->mSQLApi->FreeLOB(mLOBLoc) != IDE_SUCCESS,
                        FreeLOBError);
 
        mIsFetchError = ILO_FALSE;
    }
    IDE_EXCEPTION(GetLOBError);
    {
        /* BUG-42849 Error code and message should not be output to stdout.
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }*/

        mIsFetchError = ILO_TRUE;
    }
    IDE_EXCEPTION(FreeLOBError);
    {
        /* BUG-42849 Error code and message should not be output to stdout.
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }*/
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * GetBuffer.
 *
 * LOB 쓰기 시 호출되며,
 * 서버로 전송할 LOB 데이터를 저장할 버퍼를 얻는다.
 *
 * @param[out] aVal
 *  서버로 전송할 LOB 데이터를 저장할 버퍼.
 * @param[out] aBufLen
 *  버퍼 길이.
 */
IDE_RC iloLOB::GetBuffer(void **aVal, UInt *aBufLen)
{
    if (mLOBLocCType == SQL_C_BLOB_LOCATOR)
    {
        if (mValCType == SQL_C_BINARY)
        {
            *(UChar **)aVal = mBinBuf;
            // BUG-21837 LOB 버퍼 사이즈를 조절한다.
            *aBufLen = ILO_LOB_PIECE_SIZE;
        }
        else /* (mValCType == SQL_C_CHAR) */
        {
            *(SChar **)aVal = mChBuf;
            *aBufLen = ILO_LOB_PIECE_SIZE * 2;
        }
    }
    else /* (mLOBLocCType == SQL_C_CLOB_LOCATOR) */
    {
        *(SChar **)aVal = mChBuf;
        *aBufLen = ILO_LOB_PIECE_SIZE;
    }

    return IDE_SUCCESS;
}

/**
 * Append.
 *
 * LOB 쓰기 시 호출되며,
 * GetBuffer()로 얻은 버퍼에 저장된 데이터를 서버로 전송하여
 * 해당 LOB의 뒷부분에 덧붙여 쓴다.
 *
 * @param[in] aStrLen
 *  버퍼에 저장되어있는 데이터의 길이.
 *  버퍼의 데이터는 NULL로 종결될 필요가 없으며,
 *  본 값 또한 NULL 종결 문자를 제외한 길이이다.
 */
IDE_RC iloLOB::Append( ALTIBASE_ILOADER_HANDLE  aHandle,
                       UInt                     aStrLen,
                       iloSQLApi               *aISPApi)
{
    SQLSMALLINT  sSourceCType;
    void        *sValue;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    /* 1.If column's type is BLOB and user buffer's type is char,
     * convert hexadecimal character string to binary data. */
    if (mLOBLocCType == SQL_C_BLOB_LOCATOR && mValCType == SQL_C_CHAR)
    {
        IDE_TEST(ConvertCharToBinary( sHandle, aStrLen) != IDE_SUCCESS);
        aStrLen /= 2;
    }
    /* BUG-21064 : CLOB type CSV up/download error */
    else
    {
        if ( CLOB4CSV && ( sHandle->mProgOption->mUseLOBFile != ILO_TRUE ) )
        {
            // 첫 CLOB data block이 CSV 포멧인지 아닌지 판별.
            if( (mIsBeginCLOBAppend == ILO_TRUE) &&
                (mChBuf[0] == sHandle->mProgOption->mCSVEnclosing) )
            {
                mIsCSVCLOBAppend   = ILO_TRUE;
            }

            if( mIsCSVCLOBAppend == ILO_TRUE )
            {
                IDE_TEST(iloConvertCSVToChar( sHandle, &aStrLen) != IDE_SUCCESS);
            }
            else
            {
                // CSV 포멧이 아닌 경우 이전 처리방법을 따름.
            }
        }
    }

    /* 2.Call SQLPutLob(). */
    if (mLOBLocCType == SQL_C_BLOB_LOCATOR)
    {
        sSourceCType = SQL_C_BINARY;
        sValue = mBinBuf;
    }
    else /* (mLOBLocCType == SQL_C_CLOB_LOCATOR) */
    {
        sSourceCType = SQL_C_CHAR;
        /* BUG-21064 : CLOB type CSV up/download error */
        if( mIsCSVCLOBAppend == ILO_TRUE )
        {
            sValue = mBinBuf;
        }
        else
        {
            sValue = mChBuf;
        }
    }
    IDE_TEST_RAISE(aISPApi->PutLOB(mLOBLocCType, mLOBLoc, (SInt)mPos, 0,
                                  sSourceCType, sValue, (SInt)aStrLen)
                   != IDE_SUCCESS, PutLOBError);

    /* 3.Update current position in LOB. */
    mPos += aStrLen;

    return IDE_SUCCESS;

    IDE_EXCEPTION(PutLOBError);
    {
        /* BUG-42849 Error code and message should not be output to stdout.
        if ( sHandle->mUseApi != SQL_TRUE )
        {    
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }*/
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * ConvertBinaryToChar.
 *
 * LOB 읽기 시 Fetch()에서 호출되며,
 * BLOB을 SQL_C_CHAR형 버퍼에 저장할 경우
 * 서버로부터 얻은 raw 이진 데이터를 16진수 스트링으로 변환한다.
 *
 * @param[in] aBinLen
 *  이진 데이터 버퍼에 저장되어있는 raw 이진 데이터의 길이.
 */
void iloLOB::ConvertBinaryToChar(UInt aBinLen)
{
    UInt sBinIdx;
    UInt sChIdx;
    UInt sNibble;

    for (sBinIdx = sChIdx = 0; sBinIdx < aBinLen; sBinIdx++)
    {
        sNibble = mBinBuf[sBinIdx] / 16;
        if (sNibble < 10)
        {
            mChBuf[sChIdx++] = (SChar)(sNibble + '0');
        }
        else
        {
            mChBuf[sChIdx++] = (SChar)(sNibble - 10 + 'A');
        }

        sNibble = mBinBuf[sBinIdx] % 16;
        if (sNibble < 10)
        {
            mChBuf[sChIdx++] = (SChar)(sNibble + '0');
        }
        else
        {
            mChBuf[sChIdx++] = (SChar)(sNibble - 10 + 'A');
        }
    }
}

/**
 * ConvertCharToBinary.
 *
 * LOB 쓰기 시 Append()에서 호출되며,
 * SQL_C_CHAR형 버퍼의 데이터를 BLOB에 저장할 경우
 * 16진수 스트링 형태의 데이터를 raw 이진 데이터로 변환한다.
 *
 * @param[in] aChLen
 *  Char 버퍼에 저장되어있는 16진수 스트링의 길이.
 */
IDE_RC iloLOB::ConvertCharToBinary( ALTIBASE_ILOADER_HANDLE aHandle, UInt aChLen)
{
    SChar sCh;
    UInt  sBinIdx;
    UInt  sChIdx;
    UInt  sHiNibble;
    UInt  sLoNibble;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    IDE_TEST_RAISE(aChLen % 2 == 1, IllegalChLenError);

    for (sChIdx = sBinIdx = 0; sChIdx < aChLen; sChIdx += 2)
    {
        sCh = mChBuf[sChIdx];
        if ('0' <= sCh && sCh <= '9')
        {
            sHiNibble = (UInt)(sCh - '0');
        }
        else if ('A' <= sCh && sCh <= 'F')
        {
            sHiNibble = (UInt)(sCh - 'A' + 10);
        }
        else
        {
            IDE_RAISE(IllegalHexCharError);
        }

        sCh = mChBuf[sChIdx + 1];
        if ('0' <= sCh && sCh <= '9')
        {
            sLoNibble = (UInt)(sCh - '0');
        }
        else if ('A' <= sCh && sCh <= 'F')
        {
            sLoNibble = (UInt)(sCh - 'A' + 10);
        }
        else
        {
            IDE_RAISE(IllegalHexCharError);
        }

        mBinBuf[sBinIdx++] = (UChar)(sHiNibble * 16 + sLoNibble);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(IllegalChLenError);
    {
        uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_Invalid_Value_Len);

        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
    }
    IDE_EXCEPTION(IllegalHexCharError);
    {
        uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_Invalid_Value);

        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


/* BUG-21064 : CLOB type CSV up/download error */
/**
 * iloConvertCharToCSV.
 *
 * CLOB 읽기 시 Fetch()에서 호출되며,
 * CLOB을 버퍼에 저장할 경우
 * 서버로부터 char 데이터를 CSV 형태로 변환한다.
 *
 * @param[in] aValueLength
 *  원본 CLOB 데이터의 길이.
 */
void iloLOB::iloConvertCharToCSV( ALTIBASE_ILOADER_HANDLE  aHandle,
                                  UInt                    *aValueLen )
{
    SChar sEnclosing;
    UInt  i;
    UInt  j;
    UInt  sIndex;
    UInt  sLen;
    
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    sEnclosing = sHandle->mProgOption->mCSVEnclosing;
    i = j = 0;
    sIndex = 0;
    sLen = *aValueLen;

    while ( i < sLen )
    {
        if( *(mBinBuf + i) == sEnclosing )
        {
            if( i == 0 )
            {
                mChBuf[sIndex++] = sEnclosing;
                // escape 문자가 추가되어 변환된 문자열길이가 1증가.
                (*aValueLen) ++;
            }
            else
            {
                idlOS::strncpy( mChBuf + sIndex, (const SChar *)mBinBuf + j, i - j );
                sIndex += i - j;
                mChBuf[sIndex++] = sEnclosing;
                // escape 문자가 추가되어 변환된 문자열길이가 1증가.
                (*aValueLen) ++;
            }
            j = i;
        }
        if( i == (sLen - 1) )
        {
            idlOS::strncpy( mChBuf + sIndex, (const SChar *)mBinBuf + j, i - j + 1 );
        }
        i++;
    }
}

/* BUG-21064 : CLOB type CSV up/download error */
/**
 * iloConvertCSVToChar.
 *
 * CLOB upload 시 Append()에서 호출되며,
 * datafile 에 저장된 CSV형태의 CLOB data를
 * 원본 data형태로 변환한다.
 *
 * @param[in] aStrLen
 *  CSV CLOB 데이터의 길이.
 */
IDE_RC iloLOB::iloConvertCSVToChar( ALTIBASE_ILOADER_HANDLE  aHandle,
                                    UInt                    *aStrLen )
{
    SChar sCh;
    UInt  sBinIdx;
    UInt  sChIdx;
    UInt  sLen;

    sBinIdx = 0;
    sChIdx  = 0;
    sLen    = *aStrLen;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    // CLOB 컬럼은 항상 "로 enclosing되어 있어, 첫문자 "은 무시해야한다.
    if( mIsBeginCLOBAppend  == ILO_TRUE )
    {
        sChIdx++;
        mIsBeginCLOBAppend = ILO_FALSE;
    }
    // 첫번째 블럭의 마지막 문자가 "일경우 다음 블럭 처음에 "가 오면 버리지 않고 저장해야한다.
    else if( mSaveBeginCLOBEnc == ILO_TRUE )
    {
        IDE_TEST( mChBuf[ sChIdx++ ] != sHandle->mProgOption->mCSVEnclosing );
        mBinBuf[sBinIdx++] = sHandle->mProgOption->mCSVEnclosing;
        mSaveBeginCLOBEnc = ILO_FALSE;
    }
    else
    {
        // do nothing
    }

    for( ; sChIdx < sLen ; sChIdx++ )
    {
        sCh = mChBuf[sChIdx];
        if ( sCh == sHandle->mProgOption->mCSVEnclosing )
        {
            if( sChIdx == (ILO_LOB_PIECE_SIZE - 1) )
            {
                if( mChBuf[sChIdx-1] != sHandle->mProgOption->mCSVEnclosing )
                {
                    mSaveBeginCLOBEnc = ILO_TRUE;
                }
            }
            else
            {
                if( sChIdx == (sLen - 1) )
                {
                    // do nothing
                }
                else
                {
                    IDE_TEST( mChBuf[ ++sChIdx ] != sHandle->mProgOption->mCSVEnclosing );
                    IDE_TEST( sBinIdx >= ILO_LOB_PIECE_SIZE );
                    mBinBuf[ sBinIdx++ ] = mChBuf[ sChIdx ];
                }
            }
        }
        else
        {
            IDE_TEST( sBinIdx >= ILO_LOB_PIECE_SIZE );
            mBinBuf[ sBinIdx++ ] = mChBuf[ sChIdx ];
        }
    }

    *aStrLen = sBinIdx;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


iloDataFile::iloDataFile( ALTIBASE_ILOADER_HANDLE aHandle )
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    idlOS::memset(mDataFilePath, 0, ID_SIZEOF(mDataFilePath));
    m_DataFp = NULL;
    mDataFileNo = -1;
    idlOS::strcpy(m_FieldTerm, "^");
    m_nFTLen = (SInt)idlOS::strlen(m_FieldTerm);
    mFTPhyLen = GetStrPhyLen(m_FieldTerm);
    mFTLexStateTransTbl = NULL;
    idlOS::strcpy(m_RowTerm, "\n");
    m_nRTLen = (SInt)idlOS::strlen(m_RowTerm);;
    mRTPhyLen = GetStrPhyLen(m_RowTerm);;
    mRTLexStateTransTbl = NULL;
    m_SetEnclosing = SQL_FALSE;
    idlOS::strcpy(m_Enclosing, "");
    mEnLexStateTransTbl = NULL;
    /* TASK-2657 */
    // BUG-27633: mErrorToken도 컬럼의 최대 크기로 할당
    mErrorToken = (SChar*) idlOS::malloc(MAX_TOKEN_VALUE_LEN + MAX_SEPERATOR_LEN);
    IDE_TEST_RAISE(mErrorToken == NULL, MAllocError);
    mErrorToken[0] = '\0';
    mErrorTokenMaxSize = (MAX_TOKEN_VALUE_LEN + MAX_SEPERATOR_LEN);
    m_SetNextToken = SQL_FALSE;
    mLOBColExist = ILO_FALSE;
    mLOB = NULL;
    mLOBFP = NULL;
    mLOBFileNo = 0;
    mLOBFilePos = ID_ULONG(0);
    mAccumLOBFilePos = ID_ULONG(0);
    mFileNameBody[0] = '\0';
    mDataFileNameExt[0] = '\0';
    // BUG-18803 readsize 옵션 추가
    // mDoubleBuff를 readsize 값에 맞추어서 동적 할당을 한다.
    mDoubleBuffPos = -1;              //BUG-22434
    mDoubleBuffSize = 0;  //BUG-22434
    mDoubleBuff     = NULL;
    /* BUG-24358 iloader Geometry Data */    
    m_TokenValue = (SChar*) idlOS::malloc(MAX_TOKEN_VALUE_LEN + MAX_SEPERATOR_LEN);
    IDE_TEST_RAISE(m_TokenValue == NULL, MAllocError);
    m_TokenValue[0] = '\0';
    mTokenMaxSize = (MAX_TOKEN_VALUE_LEN + MAX_SEPERATOR_LEN);
    mLOBFile = NULL;            //BUG-24583
    mLOBFileRowCount = 0;
    mLOBFileColumnNum = 0;      //BUG-24583
    mOutFileFP = NULL;          //PROJ-2030, CT_CASE-3020 CHAR outfile 지원

    return;

    IDE_EXCEPTION(MAllocError);
    {
        uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_memory_error, __FILE__, __LINE__);
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
    }
    IDE_EXCEPTION_END;

    if (mErrorToken != NULL)
    {
        idlOS::free(mErrorToken);
        mErrorToken = NULL;
    }

    if ( sHandle->mUseApi != SQL_TRUE )
    {
        // 객체를 초기화할 때 메모리 할당에 실패해서는 안된다.
        exit(1);
    }
}

iloDataFile::~iloDataFile()
{
    FreeAllLexStateTransTbl();
    if (mLOB != NULL)
    {
        delete mLOB;
    }
    /* BUG-24358 iloader Geometry Data */    
    if (m_TokenValue != NULL)
    {
        idlOS::free(m_TokenValue);
        mTokenMaxSize = 0;
    }
    // BUG-27633: mErrorToken도 컬럼의 최대 크기로 할당
    if (mErrorToken != NULL)
    {
        idlOS::free(mErrorToken);
        mErrorToken = NULL;
        mErrorTokenMaxSize = 0;
    }
    
    //BUG-24583
    if ( mLOBFile != NULL )
    {
        LOBFileInfoFree();
    }

    // BUG-18803 readsize 옵션 추가
    if(mDoubleBuff != NULL)
    {
        idlOS::free(mDoubleBuff);
    }
}

/**
 * 데이타 파일의 전체 경로로부터 경로만 추출해서 저장해둔다.
 *
 * @param [IN] aDataFileName
 *             데이타 파일의 전체 경로
 */
void iloDataFile::InitDataFilePath(SChar *aDataFileName)
{
    SChar *sPtr;
    SInt   sPos;

    sPtr = idlOS::strrchr(aDataFileName, IDL_FILE_SEPARATOR);
    if (sPtr != NULL)
    {
        sPos = (sPtr - aDataFileName) + 1;
        (void)idlOS::strncpy(mDataFilePath, aDataFileName, sPos);
        mDataFilePath[sPos] = '\0';
    }
}

SInt iloDataFile::OpenFileForUp( ALTIBASE_ILOADER_HANDLE  aHandle,
                                 SChar                   *aDataFileName,
                                 SInt                     aDataFileNo,
                                 iloBool                   aIsWr, 
                                 iloBool                   aLOBColExist )
{
    SChar  sFileName[MAX_FILEPATH_LEN];
    SChar  sMode[3];

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    /* 데이터 파일명으로부터 파일명 본체, 데이터 파일 확장자,
     * 데이터 파일 번호를 얻는다. */
    AnalDataFileName(aDataFileName, aIsWr);
    /* 데이터 파일에 쓰기인 경우 사용자가 인자로 준 데이터 파일 번호를 사용 */
    if (aIsWr == ILO_TRUE)
    {
        mDataFileNo = aDataFileNo;
    }
    mLOBColExist = aLOBColExist;

    if (mDataFileNo < 0)
    {
        (void)idlOS::snprintf(sFileName, ID_SIZEOF(sFileName), "%s%s",
                              mFileNameBody, mDataFileNameExt);
    }
    else
    {
        (void)idlOS::snprintf(sFileName, ID_SIZEOF(sFileName),
                              "%s%s%"ID_INT32_FMT, mFileNameBody,
                              mDataFileNameExt, mDataFileNo);
    }

    if (aIsWr == ILO_TRUE)
    {
        //BUG-24511 모든 Fopen은 binary type으로 설정해야함
        (void)idlOS::snprintf(sMode, ID_SIZEOF(sMode), "wb");
    }
    else
    {
        /* To Fix BUG-14940 한글이 깨진 데이터에 대해
         *                  iloader에서 import가 중간에 종료됨. */
        (void)idlOS::snprintf(sMode, ID_SIZEOF(sMode), "rb");
    }
    m_DataFp = iloFileOpen( sHandle, sFileName, O_CREAT | O_RDWR, sMode, LOCK_WR);
    IDE_TEST_RAISE(m_DataFp == NULL, OpenError);
    mDataFilePos = ID_ULONG(0);
    // BUG-24873 clob 데이터 로딩할때 [ERR-91044 : Error occurred during data file IO.] 발생
    mDataFileRead = ID_ULONG(0);

    if (aIsWr == ILO_FALSE)
    {
        m_SetNextToken = SQL_FALSE;
        IDE_TEST_RAISE(MakeAllLexStateTransTbl( sHandle ) != IDE_SUCCESS,
                       LexStateTransTblMakeFail);
    }

    if (mLOBColExist == ILO_TRUE)
    {
        IDE_TEST_RAISE( InitLOBProc(sHandle, aIsWr) != IDE_SUCCESS, LOBProcInitError);
    }

    // BUG-18803 readsize 옵션 추가
    // mDoubleBuff를 readsize 값에 맞추어서 동적 할당을 한다.
    if(mDoubleBuff != NULL)
    {
        idlOS::free(mDoubleBuff);
    }
    mDoubleBuffPos = -1;
    mDoubleBuffSize = sHandle->mProgOption->mReadSzie;

    mDoubleBuff = (SChar*)idlOS::malloc(mDoubleBuffSize +1);
    IDE_TEST_RAISE(mDoubleBuff == NULL, NO_MEMORY);

    idlOS::memset(mDoubleBuff, 0, mDoubleBuffSize +1);

    InitDataFilePath(sFileName);

    return SQL_TRUE;

    IDE_EXCEPTION(OpenError);
    {
        if (uteGetErrorCODE(sHandle->mErrorMgr) != 0x91100)
        {
            uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_openFileError,
                    sFileName);

            if ( sHandle->mUseApi != SQL_TRUE )
            {
                utePrintfErrorCode(stdout, sHandle->mErrorMgr);
            }
        }
        else
        {
            uteSetErrorCode( sHandle->mErrorMgr,utERR_ABORT_File_Lock_Error,
                    sFileName);

            if ( sHandle->mUseApi != SQL_TRUE )
            {
                utePrintfErrorCode(stdout, sHandle->mErrorMgr);
            }
        }
    }
    IDE_EXCEPTION(LexStateTransTblMakeFail)
    {
        if (idlOS::fclose(m_DataFp) == 0)
        {
            m_DataFp = NULL;
        }
    }
    IDE_EXCEPTION(LOBProcInitError);
    {
        if (idlOS::fclose(m_DataFp) == 0)
        {
            m_DataFp = NULL;
        }
        FreeAllLexStateTransTbl();
    }
    IDE_EXCEPTION(NO_MEMORY);
    {
        mDoubleBuffSize = 0;
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_memory_error, __FILE__, __LINE__);
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stderr, sHandle->mErrorMgr);
        }
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

FILE* iloDataFile::OpenFileForDown( ALTIBASE_ILOADER_HANDLE  aHandle,
                                    SChar                   *aDataFileName,
                                    SInt                     aDataFileNo, 
                                    iloBool                   aIsWr, 
                                    iloBool                   aLOBColExist )
{
    FILE   *spFile;
    SChar  sFileName[MAX_FILEPATH_LEN];
    SChar  sMode[3];
    
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    /* 데이터 파일명으로부터 파일명 본체, 데이터 파일 확장자,
     * 데이터 파일 번호를 얻는다. */
    AnalDataFileName(aDataFileName, aIsWr);
    /* 데이터 파일에 쓰기인 경우 사용자가 인자로 준 데이터 파일 번호를 사용 */
    if (aIsWr == ILO_TRUE)
    {
        mDataFileNo = aDataFileNo;
    }
    mLOBColExist = aLOBColExist;

    if (mDataFileNo < 0)
    {
        (void)idlOS::snprintf(sFileName, ID_SIZEOF(sFileName), "%s%s",
                              mFileNameBody, mDataFileNameExt);
    }
    else
    {
        (void)idlOS::snprintf(sFileName, ID_SIZEOF(sFileName),
                              "%s%s%"ID_INT32_FMT, mFileNameBody,
                              mDataFileNameExt, mDataFileNo);
    }

    if (aIsWr == ILO_TRUE)
    {
        //BUG-24511 모든 Fopen은 binary type으로 설정해야함
        (void)idlOS::snprintf(sMode, ID_SIZEOF(sMode), "wb");
    }
    else
    {
        /* To Fix BUG-14940 한글이 깨진 데이터에 대해
         *                  iloader에서 import가 중간에 종료됨. */
        (void)idlOS::snprintf(sMode, ID_SIZEOF(sMode), "rb");
    }
    spFile = iloFileOpen( sHandle, sFileName, O_CREAT | O_RDWR, sMode, LOCK_WR);
    IDE_TEST_RAISE(spFile == NULL, OpenError);
    
    //PROJ-1714
    //LOB Column이 있을 경우에는, LOB File을 처리하는 곳에서 사용할 수 있도록 한다.
    if(aLOBColExist == ILO_TRUE)
    {
        m_DataFp = spFile;
    }
    
    mDataFilePos = ID_ULONG(0);
    
    if (aIsWr == ILO_FALSE)
    {
        m_SetNextToken = SQL_FALSE;
        IDE_TEST_RAISE(MakeAllLexStateTransTbl( sHandle ) != IDE_SUCCESS,
                       LexStateTransTblMakeFail);
    }

    if (mLOBColExist == ILO_TRUE)
    {
        IDE_TEST_RAISE(InitLOBProc(sHandle, aIsWr) != IDE_SUCCESS, LOBProcInitError);
    }

    InitDataFilePath(sFileName);

    return spFile;

    IDE_EXCEPTION(OpenError);
    {
        /* BUG-43432 Error Message is printed out in the function which calls this function.
        if (uteGetErrorCODE( sHandle->mErrorMgr) != 0x91100) //utERR_ABORT_File_Lock_Error
        {
            if ( sHandle->mUseApi != SQL_TRUE )
            {
                (void)idlOS::printf("Data File [%s] open fail\n", sFileName);
            }
        }
        else
        {
            if ( sHandle->mUseApi != SQL_TRUE )
            {
                utePrintfErrorCode(stdout, sHandle->mErrorMgr);
            }
        }
        */
    }
    IDE_EXCEPTION(LexStateTransTblMakeFail)
    {
        if (idlOS::fclose(spFile) == 0)
        {
            spFile = NULL;
        }
    }
    IDE_EXCEPTION(LOBProcInitError);
    {
        if (idlOS::fclose(spFile) == 0)
        {
            spFile = NULL;
        }
        FreeAllLexStateTransTbl();
    }
    IDE_EXCEPTION_END;
    
    return NULL;
}

SInt iloDataFile::CloseFileForUp( ALTIBASE_ILOADER_HANDLE aHandle )
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    FreeAllLexStateTransTbl();
    if (mLOBColExist == ILO_TRUE)
    {
        IDE_TEST_RAISE(FinalLOBProc(sHandle) != IDE_SUCCESS, LOBProcFinalError);
    }
    IDE_TEST_RAISE(idlOS::fclose(m_DataFp) != 0, CloseError);
    m_DataFp = NULL;

    return SQL_TRUE;

    IDE_EXCEPTION(LOBProcFinalError);
    {
        if (idlOS::fclose(m_DataFp) == 0)
        {
            m_DataFp = NULL;
        }
    }
    IDE_EXCEPTION(CloseError);
    {
        uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_Data_File_IO_Error);
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        (void)idlOS::printf("Data file close fail\n");
        }
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

SInt iloDataFile::CloseFileForDown( ALTIBASE_ILOADER_HANDLE aHandle, FILE *fp)
{
    
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    if (mLOBColExist == ILO_TRUE)
    {
        IDE_TEST_RAISE(FinalLOBProc(sHandle) != IDE_SUCCESS, LOBProcFinalError);
    }
    IDE_TEST_RAISE(idlOS::fclose(fp) != 0, CloseError);
    fp = NULL;

    return SQL_TRUE;

    IDE_EXCEPTION(LOBProcFinalError);
    {
        if (idlOS::fclose(fp) == 0)
        {
            fp = NULL;
        }
    }
    IDE_EXCEPTION(CloseError);
    {
        uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_Data_File_IO_Error);
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
            (void)idlOS::printf("Data file close fail\n");
        }
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

void iloDataFile::SetTerminator(SChar *szFiledTerm, SChar *szRowTerm)
{
    idlOS::strcpy(m_FieldTerm, szFiledTerm);
    m_nFTLen = (SInt)idlOS::strlen(m_FieldTerm);
    mFTPhyLen = GetStrPhyLen(m_FieldTerm);
    idlOS::strcpy(m_RowTerm, szRowTerm);
    m_nRTLen = (SInt)idlOS::strlen(m_RowTerm);
    mRTPhyLen = GetStrPhyLen(m_RowTerm);
}

void iloDataFile::SetEnclosing(SInt bSetEnclosing, SChar *szEnclosing)
{
    m_SetEnclosing = bSetEnclosing;
    if (m_SetEnclosing)
    {
        idlOS::strcpy(m_Enclosing, szEnclosing);
        m_nEnLen = (SInt)idlOS::strlen(m_Enclosing);
        mEnPhyLen = GetStrPhyLen(m_Enclosing);
    }
}



SInt iloDataFile::PrintOneRecord( ALTIBASE_ILOADER_HANDLE  aHandle,
                                  SInt                     aRowNo, 
                                  iloColumns              *pCols,
                                  iloTableInfo            *pTableInfo, 
                                  FILE                    *aWriteFp, 
                                  SInt                     aArrayNum ) //PROJ-1714
{
    SChar  mMsSqlBuf[128];
    SChar *pos;
    SChar *microSecond;
    SChar  miliSecond[128];
    SInt   i;
    UInt   sWrittenLen;
    EispAttrType sAttrType;
    SChar *sValuePtr = NULL;
    UInt   sValueLen = 0;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    for (i=0; i < pCols->GetSize(); i++)
    {    
        sAttrType = pTableInfo->GetAttrType(i);
        if (m_SetEnclosing)
        {
            IDE_TEST_RAISE( idlOS::fwrite(m_Enclosing, m_nEnLen, 1, aWriteFp)
                            != (UInt)1, WriteError );
        }

        // switch문 실행전 공통으로 사용되는 buffer pointer 초기화
        sValuePtr = (SChar*)(pCols->m_Value[i])
                  + (aArrayNum * (SInt)pCols->mBufLen[i]);
        // 여기서 세팅한 길이값은 다시 변할 수 있으므로 밑에서 적절히 다시 세팅해야 한다
        sValueLen = (UInt)*(pCols->m_Len[i] + aArrayNum);
        
        switch (pCols->GetType(i))
        {
            case SQL_NUMERIC : /* SQLBindCol에서 SQL_C_CHAR로 지정함 */
            case SQL_DECIMAL :
            case SQL_FLOAT   :
                if (*(pCols->m_Len[i] + aArrayNum)  != SQL_NULL_DATA)
                {
                    // BUG-25827 iloader에 Default 로 noexp 옵션이 붙었으면 좋겠습니다.
                    /*
                    if (gProgOption.mNoExp != SQL_TRUE &&
                        pTableInfo->mNoExpFlag[i] != ID_TRUE)
                    {
                        ConvNumericNormToExp(pCols, i, aArrayNum);
                    }
                    */
                    // 잎전에서 길이값이 변할수 있으므로 여기서 길이값 변수를 세팅
                    sValueLen = (UInt)*(pCols->m_Len[i] + aArrayNum);
                    IDE_TEST_RAISE( idlOS::fwrite(sValuePtr, sValueLen, 1, aWriteFp)
                                    != (UInt)1, WriteError );
                }
                break;
            case SQL_CHAR :
            case SQL_WCHAR :
            case SQL_VARCHAR :
            case SQL_WVARCHAR :
            case SQL_SMALLINT:
            case SQL_INTEGER :
            case SQL_BIGINT  :
            case SQL_REAL   :
            case SQL_DOUBLE  :
            case SQL_NIBBLE :
            case SQL_BYTES :
            case SQL_VARBYTE :
            case SQL_DATE :
            case SQL_TIMESTAMP :
            case SQL_TYPE_DATE :
            case SQL_TYPE_TIMESTAMP:
                if (*(pCols->m_Len[i] + aArrayNum) == SQL_NULL_DATA)
                {
                    break;
                }

                // TruncTrailingSpaces()  안에서 length값이 변할 수 있으므로
                // 여기서 실제 길이값을 세팅
                sValueLen = (UInt)*(pCols->m_Len[i] + aArrayNum);
                /* fix CASE-4061 */
                if ( (sAttrType == ISP_ATTR_DATE) &&
                    ( sHandle->mProgOption->mMsSql == SQL_TRUE) )
                {
                    (void)idlOS::snprintf(mMsSqlBuf, ID_SIZEOF(mMsSqlBuf), "%s",
                                        sValuePtr);
                    pos = idlOS::strrchr(mMsSqlBuf, ':');
                    if ( pos != NULL )
                    {
                        *pos          = '\0';
                        microSecond   = pos+1;
                        if ( idlOS::strlen(microSecond) > 3 )
                        {
                            idlOS::strncpy(miliSecond, microSecond, 3);
                            miliSecond[3] ='\0';
                            sWrittenLen = (UInt)idlOS::fprintf(aWriteFp, "%s.%s",
                                                               mMsSqlBuf,
                                                               miliSecond);
                            IDE_TEST_RAISE(sWrittenLen
                                           < (UInt)idlOS::strlen(mMsSqlBuf) + 4,
                                           WriteError);
                        }
                        else
                        {
                            IDE_TEST_RAISE( idlOS::fwrite(sValuePtr, sValueLen, 1, aWriteFp)
                                            != (UInt)1, WriteError );
                        }
                    }
                    else
                    {
                        IDE_TEST_RAISE( idlOS::fwrite(sValuePtr, sValueLen, 1, aWriteFp)
                                        != (UInt)1, WriteError );
                    }
                }
                else if( (sAttrType == ISP_ATTR_DATE) ||
                         (sAttrType == ISP_ATTR_CHAR) ||
                         (sAttrType == ISP_ATTR_VARCHAR) ||
                         (sAttrType == ISP_ATTR_NCHAR) ||
                         (sAttrType == ISP_ATTR_NVARCHAR) )

                {
                    
                    // cpu가 little-endian이고 nchar/nvarchar 컬럼이고, nchar_utf16=yes이면
                    // big-endian으로 변환시킨다 (datafile에는 big-endian으로만 저장)
#ifndef ENDIAN_IS_BIG_ENDIAN
                    if (((sAttrType == ISP_ATTR_NCHAR) ||
                         (sAttrType == ISP_ATTR_NVARCHAR)) &&
                        ( sHandle->mProgOption->mNCharUTF16 == ILO_TRUE))
                    {
                        pTableInfo->ConvertShortEndian(sValuePtr, sValueLen);
                    }
#endif
                    /* TASK-2657 */
                    if ( sHandle->mProgOption->mRule == csv )
                    {
                        /* the date type might have ',' in the value string, so " " is forced */
                        if ( sAttrType == ISP_ATTR_DATE )
                        {
                            IDE_TEST_RAISE( idlOS::fwrite( &(sHandle->mProgOption->mCSVEnclosing), 1, 1, aWriteFp ) 
                                            != (UInt)1, WriteError);
                            IDE_TEST_RAISE( idlOS::fwrite(sValuePtr, sValueLen,
                                            1, aWriteFp ) != (UInt)1, WriteError);
                            IDE_TEST_RAISE( idlOS::fwrite( &(sHandle->mProgOption->mCSVEnclosing),
                                            1, 1, aWriteFp ) 
                                            != (UInt)1, WriteError);
                        }
                        else
                        {
                            IDE_TEST_RAISE( csvWrite ( sHandle, 
                                                       sValuePtr,
                                                       sValueLen,
                                                       aWriteFp )
                                                       != SQL_TRUE, WriteError);
                        }
                    }
                    else
                    {
                        IDE_TEST_RAISE( idlOS::fwrite(sValuePtr, sValueLen, 1, aWriteFp)
                                        != (UInt)1, WriteError );
                    }
                }
                else
                {
                    IDE_TEST_RAISE( idlOS::fwrite(sValuePtr, sValueLen, 1, aWriteFp)
                                    != (UInt)1, WriteError );
                }
                break;
            case SQL_BINARY :
            case SQL_BLOB :
            case SQL_BLOB_LOCATOR :
            case SQL_CLOB :
            case SQL_CLOB_LOCATOR :
                 /* BUG-24358 iloader Geometry Data */
                if (sAttrType == ISP_ATTR_GEOMETRY)
                {
                    // if null, skip.
                    if (*(pCols->m_Len[i] + aArrayNum) == SQL_NULL_DATA)
                    {
                        break;
                    }
                    sValueLen = (UInt)*(pCols->m_Len[i] + aArrayNum);

                    if ( sHandle->mProgOption->mRule == csv )
                    {
                        IDE_TEST_RAISE(
                             csvWrite ( sHandle, sValuePtr, sValueLen, aWriteFp )
                            != SQL_TRUE, WriteError);
                    }
                    else /* rule == csv */
                    {
                        IDE_TEST_RAISE(
                            idlOS::fwrite(sValuePtr, sValueLen,
                                          1, aWriteFp )
                            != (UInt)1, WriteError);
                    }
                }
                else // if sAttrType != ISP_ATTR_GEOMETRY
                {
                    /* LOB */
                    IDE_TEST(PrintOneLOBCol(sHandle,
                                            aRowNo, 
                                            pCols, 
                                            i, 
                                            aWriteFp, 
                                            pTableInfo)     //BUG-24583
                             != IDE_SUCCESS);   
                }
                break;
            default :
                IDE_RAISE(UnknownDataTypeError);
        }

        if (m_SetEnclosing)
        {
            IDE_TEST_RAISE( idlOS::fwrite(m_Enclosing, m_nEnLen, 1, aWriteFp)
                            != (UInt)1, WriteError );
        }
        if ( i == pCols->GetSize()-1 )
        {
            if ( sHandle->mProgOption->mInformix == SQL_TRUE )
            {
                /* TASK-2657 */
                if ( sHandle->mProgOption->mRule == csv )
                {
                    IDE_TEST_RAISE( idlOS::fwrite( &(sHandle->mProgOption->mCSVFieldTerm), 1, 1, aWriteFp ) 
                                    != 1, WriteError);
                }
                else
                {
                    IDE_TEST_RAISE( idlOS::fwrite(m_FieldTerm, m_nFTLen, 1, aWriteFp)
                                    != (UInt)1, WriteError );
                }
            }
            /* BUG-29779: csv의 rowterm을 \r\n으로 지정하는 기능 */
            IDE_TEST_RAISE( idlOS::fwrite(m_RowTerm, m_nRTLen, 1, aWriteFp)
                            != (UInt)1, WriteError );
        }
        else
        {
            /* TASK-2657 */
            if ( sHandle->mProgOption->mRule == csv )
            {
                IDE_TEST_RAISE( idlOS::fwrite( &(sHandle->mProgOption->mCSVFieldTerm), 1, 1, aWriteFp ) 
                                != 1, WriteError);
            }
            else
            {
                IDE_TEST_RAISE( idlOS::fwrite(m_FieldTerm, m_nFTLen, 1, aWriteFp)
                                != (UInt)1, WriteError );
            }
        }
    } //end of Column For Loop
    
    return SQL_TRUE;

    IDE_EXCEPTION(WriteError);
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Data_File_IO_Error);
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
            (void)idlOS::printf("Data file write fail\n");
        }
    }
    IDE_EXCEPTION(UnknownDataTypeError);
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Unkown_Datatype_Error, "");
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

/* TASK-2657 */
SInt iloDataFile::csvWrite ( ALTIBASE_ILOADER_HANDLE  aHandle,
                             SChar                   *aValue,
                             UInt                     aValueLen,
                             FILE                    *aWriteFp )
{
    SChar sEnclosing;
    UInt  i;
    UInt  j;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    IDE_TEST_RAISE((aValue == NULL) || (aWriteFp == NULL), WriteError);

    sEnclosing = sHandle->mProgOption->mCSVEnclosing;

    i = j = 0;


    /* if the type is char or varchar, " " is forced. */
    while ( i < aValueLen )
    {
       if( *(aValue + i) == sEnclosing )
       {
           if( i == 0 )
           {
               IDE_TEST_RAISE(idlOS::fwrite( &sEnclosing, 1, 1, aWriteFp ) 
                              != (UInt)1, WriteError);
           }
           else
           {
               if ( j == 0 )
               {
                   /* write heading quote, except for the i==0 case. */
                   IDE_TEST_RAISE(idlOS::fwrite( &sEnclosing, 1, 1, aWriteFp )
                                  != (UInt)1, WriteError);
               }
               IDE_TEST_RAISE( idlOS::fwrite( aValue + j, i - j, 1, aWriteFp )
                               != (UInt)1, WriteError);
               IDE_TEST_RAISE( idlOS::fwrite( &sEnclosing, 1, 1, aWriteFp )
                               != (UInt)1, WriteError);
           }
           j = i;
       }
       if( i == aValueLen - 1 )
       {
           if ( j == 0 )
           {
               IDE_TEST_RAISE( idlOS::fwrite( &sEnclosing, 1, 1, aWriteFp )
                               != (UInt)1, WriteError);
               IDE_TEST_RAISE( idlOS::fwrite( aValue, aValueLen, 1, aWriteFp )
                               != (UInt)1, WriteError);
               IDE_TEST_RAISE( idlOS::fwrite( &sEnclosing, 1, 1, aWriteFp )
                               != (UInt)1, WriteError);
           }
           else
           {
               IDE_TEST_RAISE( idlOS::fwrite( aValue + j, i - j + 1, 1, aWriteFp )
                               != (UInt)1, WriteError);
               IDE_TEST_RAISE( idlOS::fwrite( &sEnclosing, 1, 1, aWriteFp )
                               != (UInt)1, WriteError);
           }
       }
       i++;
    }
    return SQL_TRUE;

    IDE_EXCEPTION(WriteError);
    {
        // 이 함수를 호출하는 곳에서 에러를 처리하므로 여기서는 생략
    }
    IDE_EXCEPTION_END;
    return SQL_FALSE;
}

/**
 * ConvNumericNormToExp.
 *
 * 문자열 형태로 얻어진 NUMERIC 컬럼 값을
 * 비지수형([-]ddd.ddd)에서 지수형([-]d.dddE+dd)으로 변환한다.
 *
 * @param[in,out] aCols
 *  컬럼 값 및 컬럼 값의 길이가 저장되어있는 구조체.
 * @param[in] aColIdx
 *  aCols 구조체 내에서 몇 번째 컬럼인지를 지정하는 컬럼 인덱스.
 */
void iloDataFile::ConvNumericNormToExp(iloColumns *aCols, SInt aColIdx, SInt aArrayNum)
{
    SChar *sPeriodPtr;
    SChar *sStr;
    UInt   sDigitIdx;
    UInt   sExp;
    UInt   sRDigitIdx;
    UInt   sStrLen;

    /* 이미 지수형이면 변환 필요 없음. */
    
    if (idlOS::strchr((SChar*)(aCols->m_Value[aColIdx]) + (aArrayNum * (SInt)aCols->mBufLen[aColIdx]), 'E') != NULL)
    {
        return;
    }

    /* aCols->m_Value[aColIdx]에서 부호를 제외한 문자열을 sStr에 설정. */
    if (((SChar*)(aCols->m_Value[aColIdx]) + (aArrayNum * (SInt)aCols->mBufLen[aColIdx]))[0] != '-')
    {
        sStr = (SChar*)(aCols->m_Value[aColIdx]) + (aArrayNum * (SInt)aCols->mBufLen[aColIdx]);
        sStrLen = (UInt)*(aCols->m_Len[aColIdx] + aArrayNum);
    }
    else
    {
        sStr = (SChar*)(aCols->m_Value[aColIdx]) + (aArrayNum * (SInt)aCols->mBufLen[aColIdx]) + 1;
        sStrLen = (UInt)*(aCols->m_Len[aColIdx] + aArrayNum) - 1;
    }

    /* 정수부가 1자리이고 소수부가 없는 경우, 변환 필요 없음. */
    if (sStr[1] == '\0')
    {
        return;
    }

    /* 0.으로 시작하지 않는 숫자인 경우 */
    if (sStr[0] != '0')
    {
        /* 정수부가 1자리이고 소수부가 있는 경우, 변환 필요 없음. */
        if (sStr[1] == '.')
        {
            return;
        }

        sPeriodPtr = idlOS::strchr(sStr, '.');

        /* 소수점이 없는 숫자인 경우 */
        if (sPeriodPtr == NULL)
        {
            sExp = (UInt)(sStrLen - 1);

            for (sRDigitIdx = sStrLen - 1; sStr[sRDigitIdx] == '0';
                 sRDigitIdx--);

            /* 정수부의 가장 큰 자릿수를 제외한 모든 자릿수가 0인 경우 */
            if (sRDigitIdx == 0)
            {
                sStrLen = 1;
            }
            /* 정수부에 0이 아닌 자릿수가 두 개 이상인 경우 */
            else
            {
                /* 소수부 출력. */
                (void)idlOS::memmove(sStr + 2, sStr + 1, sRDigitIdx);

                /* 소수점 출력. */
                sStr[1] = '.';

                sStrLen = 2 + sRDigitIdx;
            }
        }
        /* 소수점이 있는 숫자인 경우 */
        else /* (sPeriodPtr != NULL) */
        {
            sExp = (UInt)((UInt)(sPeriodPtr - sStr) - 1);

            /* 소수부 출력. */
            (void)idlOS::memmove(sStr + 2, sStr + 1, sExp);

            /* 소수점 출력. */
            sStr[1] = '.';
        }

        /* 지수부 출력. */
        sStrLen += (UInt)idlOS::sprintf(sStr + sStrLen, "E+%"ID_UINT32_FMT,
                                        sExp);
    }
    /* 0.으로 시작하는 숫자인 경우 */
    else /* (sStr[0] == '0') */
    {
        sStr[1] = '0';

        // bug-20800
        sDigitIdx = 0;
        while(sStr[sDigitIdx] == '0')
        {
            ++sDigitIdx;
        }

        sExp = (UInt)(sDigitIdx - 1);

        /* 정수부 출력. */
        sStr[0] = sStr[sDigitIdx];

        /* 0이 아닌 숫자가 1자리뿐인 경우 */
        if (sStr[sDigitIdx + 1] == '\0')
        {
            sStrLen = 1;
        }
        else
        {
            /* 소수점 출력. */
            sStr[1] = '.';

            /* 소수부 출력. */
            (void)idlOS::memmove(sStr + 2, sStr + sDigitIdx + 1,
                                 sStrLen - sDigitIdx - 1);

            sStrLen = (UInt)(sStrLen - sDigitIdx + 1);
        }

        /* 지수부 출력. */
        sStrLen += (UInt)idlOS::sprintf(sStr + sStrLen, "E-%"ID_UINT32_FMT,
                                        sExp);
    }

    /* aCols->m_Len[aColIdx]를 변환된 문자열의 길이로 재설정. */
    if (((SChar *)aCols->m_Value[aColIdx])[0] != '-')
    {
        *(aCols->m_Len[aColIdx] + aArrayNum) = (SQLLEN)sStrLen;
    }
    else
    {
        *(aCols->m_Len[aColIdx] + aArrayNum) = (SQLLEN)(sStrLen + 1);
    }
  
}

/**
 * GetToken.
 *
 * 데이터 파일로부터 한 개의 토큰을 읽기 위한 함수이다.
 * 토큰의 종류에는 TEOF, TFIELD_TERM, TROW_TERM, TVALUE가 있으며,
 * 본 함수의 리턴값은 이 값들 중 하나가 된다.
 * 리턴값이 TVALUE일 경우 m_TokenValue, mTokenLen에 각각
 * 토큰 문자열과 문자열의 길이가 설정된다.
 */
EDataToken iloDataFile::GetTokenFromCBuff( ALTIBASE_ILOADER_HANDLE aHandle )
{
    SChar sChr;
    SInt  sRet = 0;
    UInt  sEnCnt = 0;
    UInt  sEnMatchLen = 0;
    UInt  sFTMatchLen = 0;
    UInt  sRTMatchLen = 0;
    SInt  sTmp = 0;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    mTokenLen = 0;

    IDE_TEST_RAISE(m_SetNextToken == SQL_TRUE, NextTokenExist);

    /* m_TokenValue 버퍼에는 값과 구분자가 동시에 저장될 수 있으므로,
     * m_TokenValue 버퍼 크기는 값의 최대 크기 + 구분자 최대 크기 및
     * NULL 종결문자를 위한 공간으로 구성된다. */
    while (mTokenLen < (mTokenMaxSize - 1) )
    {
        sRet = ReadDataFromCBuff( sHandle, &sChr );    //BUG-22434
        IDE_TEST_RAISE(sRet == -1, EOFReached);
        //BUG-22513
        sTmp = (UChar)sChr;
        
        mDataFileRead++;         //LOB 파일일 경우 사용되며, single Thread처리이므로 Mutex lock이 필요없음.. 

        m_TokenValue[mTokenLen++] = (SChar)sChr;

        if (sEnCnt == 0)
        {
            sFTMatchLen = mFTLexStateTransTbl[sFTMatchLen][sTmp];
            if (sFTMatchLen == (UInt)m_nFTLen)
            {
                if (mTokenLen == (UInt)m_nFTLen &&
                    m_SetEnclosing)
                {
                    return TFIELD_TERM;
                }
                else
                {
                    m_SetNextToken = SQL_TRUE;
                    mNextToken = TFIELD_TERM;
                    mTokenLen -= (UInt)m_nFTLen;
                    m_TokenValue[mTokenLen] = '\0';
                    return TVALUE;
                }
            }

            sRTMatchLen = mRTLexStateTransTbl[sRTMatchLen][sTmp];
            if (sRTMatchLen == (UInt)m_nRTLen)
            {
                if (mTokenLen == (UInt)m_nRTLen &&
                    (m_SetEnclosing || sHandle->mProgOption->mInformix))
                {
                    return TROW_TERM;
                }
                else
                {
                    m_SetNextToken = SQL_TRUE;
                    mNextToken = TROW_TERM;
                    mTokenLen -= (UInt)m_nRTLen;
                    m_TokenValue[mTokenLen] = '\0';
                    return TVALUE;
                }
            }
        }

        if (m_SetEnclosing && (sEnCnt == 1 || mTokenLen == sEnMatchLen + 1))
        {
            sEnMatchLen = mEnLexStateTransTbl[sEnMatchLen][sTmp];
            if (sEnMatchLen == (UInt)m_nEnLen)
            {
                if (sEnCnt == 0)
                {
                    sEnCnt++;
                    sEnMatchLen = 0;
                    mTokenLen = 0;
                }
                else
                {
                    mTokenLen -= (UInt)m_nEnLen;
                    m_TokenValue[mTokenLen] = '\0';
                    return TVALUE;
                }
            }
        }
    }

    IDE_RAISE(BufferOverflow);

    IDE_EXCEPTION(NextTokenExist);
    {
        m_SetNextToken = SQL_FALSE;
        return mNextToken;
    }
    IDE_EXCEPTION(EOFReached);
    {
        if (mTokenLen > 0)
        {
            m_SetNextToken = SQL_TRUE;
            mNextToken = TEOF;
            m_TokenValue[mTokenLen] = '\0';
            return TVALUE;
        }
        else
        {
            return TEOF;
        }
    }
    IDE_EXCEPTION(BufferOverflow);
    {
        m_TokenValue[mTokenLen] = '\0';
        return TVALUE;
    }
    IDE_EXCEPTION_END;

    return TEOF;
}


/* TASK-2657 ******************************************************
 * 
 * Description  : parse csv format data, and store the token value
 * Return value : the type of a token 
 *
 ******************************************************************/
EDataToken iloDataFile::GetCSVTokenFromCBuff( ALTIBASE_ILOADER_HANDLE  aHandle,
                                              SChar                   *aColName)
{

    SChar  sEnclosing;
    SChar  sFieldTerm;
    SChar  sChr;
    /* BUG-29779: csv의 rowterm을 \r\n으로 지정하는 기능 */
    SChar *sRowTerm;
    SChar  sRowTermBK[MAX_SEPERATOR_LEN];   //temparary space for ensuring rowterm.
    SInt   sRowTermInd;                     //sRowTermBK의 index
    SInt   sReadresult = 0;
    UInt   sMaxTokenSize;
    UInt   sMaxErrorTokenSize;
    SInt   sInQuotes;
    SInt   sBadorLog;
    SInt   sRowTermLen;
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    idBool sSkipReadData = ID_FALSE;
    eReadState sState;
    eReadState sPreState;                   // 이전상태저장

    sState    = stStart;
    sPreState = stError;
    /* BUG-24358 iloader Geometry Data */
    sMaxTokenSize = mTokenMaxSize;
    sMaxErrorTokenSize = mErrorTokenMaxSize;
    mTokenLen = 0;
    mErrorTokenLen = 0;
    sInQuotes = ILO_FALSE;

    sEnclosing = sHandle->mProgOption->mCSVEnclosing;
    sRowTerm   = sHandle->mProgOption->m_RowTerm;
    sFieldTerm = sHandle->mProgOption->mCSVFieldTerm;
    sBadorLog  = sHandle->mProgOption->m_bExist_bad || sHandle->mProgOption->m_bExist_log || ( sHandle->mLogCallback != NULL );

    sRowTermInd = 0;
    sRowTermLen = idlOS::strlen(sRowTerm);

    if ( m_SetNextToken == SQL_TRUE )
    {
        m_SetNextToken = SQL_FALSE;
        return mNextToken;
    }

    while ( sMaxTokenSize && sMaxErrorTokenSize )
    {
        /* BUG-29779: csv의 rowterm을 \r\n으로 지정하는 기능 */
        if( sSkipReadData == ID_TRUE )
        {
            sSkipReadData = ID_FALSE;
        }
        else
        {
            if ( (sReadresult = ReadDataFromCBuff(sHandle,&sChr)) < 0 )
            {   // case of EOF
                break;
            }

            //PROJ-1714
            //Buffer에서 읽은 Byte수.. LOB Column을 Upload할 경우에 사용됨.
            mDataFileRead++;
            /* if bad or log option is set, save an entire token value */
            if ( sBadorLog )
            {
                mErrorToken[ mErrorTokenLen++ ] = (SChar)sChr;
                sMaxErrorTokenSize--;
            }
        }

        switch ( sState )
        {
            /* stStart   : state of starting up reading a field  */
            case stStart :
                if ( sChr != '\n' && isspace(sChr) )
                {
                    break;
                }
                else if ( sChr == sEnclosing )
                {
                    sState = stCollect;
                    sInQuotes = ILO_TRUE;
                    break;
                }
                sState = stCollect;
                sSkipReadData = ID_TRUE;
                break;
            /* BUG-29779: csv의 rowterm을 \r\n으로 지정하는 기능 */
            // Rowterm이 string으로 바뀌어 rowterm이 맞는지를 처리하기 위한 상태를 추가함.
            case stRowTerm:
                if ( sRowTermInd < sRowTermLen && sChr == sRowTerm[sRowTermInd] )
                {
                    sRowTermBK[sRowTermInd++] = sChr;
                    if( sRowTermInd == sRowTermLen )
                    {   // match up with rowterm
                        switch( sPreState )
                        {
                            case stCollect:
                                while ( mTokenLen && m_TokenValue[ mTokenLen - 1] == ' ' )
                                {
                                    mTokenLen--;
                                }
                                m_SetNextToken = SQL_TRUE;
                                mNextToken = TROW_TERM;
                                m_TokenValue[ mTokenLen ] = '\0';
                                if ( sBadorLog )
                                {
                                    mErrorToken[ --mErrorTokenLen ] = '\0';
                                }
                                return TVALUE;
                                break;
                            case stTailSpace:
                            case stEndQuote:
                                m_SetNextToken = SQL_TRUE;
                                mNextToken = TROW_TERM;
                                m_TokenValue[ mTokenLen ] = '\0';
                                if ( sBadorLog )
                                {
                                    mErrorToken[ --mErrorTokenLen ] = '\0';
                                }
                                return TVALUE;
                                break;
                            case stError:
                                if ( sBadorLog )
                                {
                                    mErrorToken[ --mErrorTokenLen ] = '\0';
                                }
                                m_SetNextToken = SQL_TRUE;
                                mNextToken = TROW_TERM;
                                IDE_RAISE( wrong_CSVformat_error );
                                break;
                            default:
                                break;
                        }
                        // 사실 아래 두라인은 탈수 없음.
                        sState      = sPreState;
                        sRowTermInd = 0;
                    }
                }
                else
                {
                    IDE_TEST_RAISE( (sMaxTokenSize - sRowTermInd) <= 0,
                                    buffer_overflow_error);
                    idlOS::memcpy( (void*)(m_TokenValue + mTokenLen),
                                   (void*)sRowTermBK,
                                   sRowTermInd );
                    mTokenLen    += sRowTermInd;
                    sSkipReadData = ID_TRUE;
                    sState        = sPreState;
                    sRowTermInd   = 0;
                }
                break;
            /* state of in the middle of reading a field  */    
            case stCollect :
                if ( sInQuotes == ILO_TRUE )
                {
                    if ( sChr == sEnclosing )
                    {
                        sState = stEndQuote;
                        break;
                    }
                }
                else if ( sChr == sFieldTerm )
                {
                    while ( mTokenLen && m_TokenValue[ mTokenLen - 1] == ' ' )
                    {
                        mTokenLen--;
                    }
                    m_SetNextToken = SQL_TRUE;
                    mNextToken = TFIELD_TERM;
                    m_TokenValue[ mTokenLen ] = '\0';
                    if ( sBadorLog )
                    {
                        mErrorToken[ --mErrorTokenLen ] = '\0';
                    }
                    return TVALUE;
                }
                else if ( sChr == sRowTerm[sRowTermInd] )
                {
                    sPreState = sState;
                    sState    = stRowTerm;
                    sSkipReadData = ID_TRUE;
                    break;
                }
                else if ( sChr == sEnclosing )
                {
                    /* CSV format is wrong, so state must be changed to stError  */
                    sState = stError;
                    /* if buffer is sufficient, save the error character.
                     * this is just for showing error character on stdout. */
                    if( sMaxTokenSize > 1 )
                    {
                        m_TokenValue[ mTokenLen++] = sChr;
                    }
                    m_TokenValue[ mTokenLen ] = '\0';
                    break;
                }
                /* collect good(csv format) characters */
                m_TokenValue[ mTokenLen++ ] = sChr;
                sMaxTokenSize--;
                break;
            /* at tailing spaces out of quotes */    
            case stTailSpace :
            /* at tailing quote */
            case stEndQuote :
                /* In case of reading an escaped quote. */
                if ( sChr == sEnclosing && sState != stTailSpace )
                {
                    m_TokenValue[ mTokenLen++ ] = sChr;
                    sMaxTokenSize--;
                    sState = stCollect;
                    break;
                }
                else if ( sChr == sFieldTerm )
                {
                    m_SetNextToken = SQL_TRUE;
                    mNextToken = TFIELD_TERM;
                    m_TokenValue[ mTokenLen ] = '\0';
                    if ( sBadorLog )
                    {
                        mErrorToken[ --mErrorTokenLen ] = '\0';
                    }
                    return TVALUE;
                }
                else if ( sChr == sRowTerm[sRowTermInd] )
                {
                    sPreState = sState;
                    sState    = stRowTerm;
                    sSkipReadData = ID_TRUE;
                    break;
                }
                else if ( isspace(sChr) )
                {
                    sState = stTailSpace;
                    break;
                }

                if( sMaxTokenSize > 1 )
                {
                    m_TokenValue[ mTokenLen++] = sChr;
                }
                m_TokenValue[ mTokenLen ] = '\0';
                sState = stError;
                break;
            /* state of a wrong csv format is read */    
            case stError :
                if ( sChr == sFieldTerm )
                {
                    if ( sBadorLog )
                    {
                        mErrorToken[ --mErrorTokenLen ] = '\0';
                    }
                    m_SetNextToken = SQL_TRUE;
                    mNextToken = TFIELD_TERM;
                    IDE_RAISE( wrong_CSVformat_error );
                }
                else if ( sChr == sRowTerm[sRowTermInd] )
                {
                    sPreState = sState;
                    sState    = stRowTerm;
                    sSkipReadData = ID_TRUE;
                    break;
                }
                break;
            default:
                break;
        }
    }

    if ( (sMaxTokenSize == 0) || (sMaxErrorTokenSize == 0) )
    {
        IDE_RAISE( buffer_overflow_error );
    }

    if( sReadresult != 1 )
    {
        if( mTokenLen != 0 )
        {
            m_SetNextToken = SQL_TRUE;
            mNextToken = TEOF;
            m_TokenValue[ mTokenLen ] = '\0';
            if ( sBadorLog )
            {
                mErrorToken[ mErrorTokenLen ] = '\0';
            }
            return TVALUE;
        }
        else
        {
            return TEOF;
        }
    }

    IDE_RAISE( wrong_CSVformat_error );

    IDE_EXCEPTION( wrong_CSVformat_error );
    {
        // BUG-24898 iloader 파싱에러 상세화
        uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_Invalid_CSV_File_Format_Error,
                        aColName,
                        m_TokenValue);
    }
    IDE_EXCEPTION( buffer_overflow_error );
    {
        m_TokenValue[mTokenLen] = '\0';
        if ( sBadorLog )
        {
            // fix BUG-25539 [CodeSonar::TypeUnderrun]
            if (mErrorTokenLen > 0)
            {
                mErrorToken[ mErrorTokenLen - 1 ] = '\0';
                mErrorTokenLen--;
            }
        }
        return TVALUE;
    }
    IDE_EXCEPTION_END;

    return TERROR;
}


/**
 * GetLOBToken.
 *
 * 데이터 파일로부터 LOB 데이터를 읽기 위한 함수이다.
 * LOB은 크기가 크므로
 * 데이터 파일의 LOB 데이터를 메모리 버퍼로 읽어들이는 것이 아니라
 * LOB 데이터의 시작 위치, 길이만 구하여 리턴한다.
 * 알고리즘 자체는 GetToken()과 유사하다.
 * 리턴값이 TVALUE일 때만 출력 인자들이 설정된다.
 *
 * @param[out] aLOBPhyOffs
 *  데이터 파일내에서 LOB 데이터의 시작 위치.
 * @param[out] aLOBPhyLen
 *  데이터 파일내에서 LOB 데이터의 길이.
 *  "물리적"의 의미는 Windows 플랫폼의 경우
 *  "\n"이 파일에는 "\r\n"으로 저장되어 2바이트로 카운트됨을 의미한다.
 * @param[out] aLOBLen
 *  LOB 데이터의 길이(물리적 길이 아님).
 *  "\n"은 1바이트로 카운트된다.
 */
EDataToken iloDataFile::GetLOBToken( ALTIBASE_ILOADER_HANDLE   aHandle,
                                     ULong                    *aLOBPhyOffs,
                                     ULong                    *aLOBPhyLen,
                                     ULong                    *aLOBLen )
{
    SChar sChr;
    SInt  i;
    SInt  sRet;
    UInt  sEnCnt      = 0;
    UInt  sEnMatchLen = 0;
    UInt  sFTMatchLen = 0;
    UInt  sRTMatchLen = 0;
    SInt  sTmp;
    /* BUG-21064 : CLOB type CSV up/download error */
    /* BUG-30409 : CLOB type data를 upload시 csv 포멧변환이 옳바로 되지 않음. */
    iloBool        sCSVnoLOBFile;
    // 바로이전 "문자가 왔었는지를 알려주는 변수.
    iloBool        sPreSCVEnclose;
    // 현재 enclosing("...")안인지 밖인지를 알려주는 변수.
    iloBool        sCSVEnclose;
    iloaderHandle *sHandle;
    sHandle        = (iloaderHandle *) aHandle;
    sCSVEnclose    = ILO_FALSE;
    sPreSCVEnclose = ILO_FALSE;
    sCSVnoLOBFile  = (iloBool)(( sHandle->mProgOption->mRule == csv ) &&
                               ( sHandle->mProgOption->mUseLOBFile != ILO_TRUE ));

    (*aLOBPhyOffs) = mDataFileRead; //mDataFilePos;
    (*aLOBPhyLen)  = ID_ULONG(0);
    (*aLOBLen)     = ID_ULONG(0);

    IDE_TEST_RAISE(m_SetNextToken == SQL_TRUE, NextTokenExist);

    for(i=0 ; ; i++)
    {
        sRet = ReadDataFromCBuff( sHandle, &sChr);    //BUG-22434
        IDE_TEST_RAISE(sRet == -1, EOFReached);

        (*aLOBPhyLen)++;
        (*aLOBLen)++;

        /* BUG-30409 : CLOB type data를 upload시 csv 포멧변환이 옳바로 되지 않음. */
        // CSV 컬럼의 시작과 끝을 알아내야한다.
        // 1. 처음에 " 가 오면 시작이다.
        // 2. "안의 \n은 컬럼data일 뿐, row term은 아니다.
        // 3. "안에 "가 오면 다음에 어떤 문자가 오느냐에 따라 컬럼의 끝인지가 결정된다.
        //    - "안에 "가 온뒤 다시 "가오면 컬럼 data일 뿐이다.
        //    - "안에 "가 온뒤 "를 제외한 다른문자가오면 컬럼의 끝이다.
        if( sCSVnoLOBFile == ILO_TRUE )
        {
            if( sChr == sHandle->mProgOption->mCSVEnclosing )
            {
                if( i == 0 )
                {   // 처음에 " 가 오면 컬럼의 시작이다.
                    sCSVEnclose = ILO_TRUE;
                    continue;
                }
                else
                {
                    // enclosing되어있지 않은데 "가 왔다.
                    IDE_TEST_RAISE( sCSVEnclose != ILO_TRUE, WrongCSVFormat);

                    if( sPreSCVEnclose == ILO_TRUE )
                    {
                        // "안에 "가 온뒤 다시 "가오면 컬럼 data일 뿐이다.
                        sPreSCVEnclose = ILO_FALSE;
                        continue;
                    }
                    else
                    {
                        // "안에 "가 왔다
                        sPreSCVEnclose = ILO_TRUE;
                        continue;
                    }
                }
            }
            else
            {
                if( sPreSCVEnclose == ILO_TRUE )
                {
                    // "안에 "가 온뒤 "를 제외한 다른문자가오면 컬럼의 끝이다.
                    sCSVEnclose = ILO_FALSE;
                }
                else
                {
                    sPreSCVEnclose = ILO_FALSE;
                }
            }
        }

        //BUG-22513
        sTmp = (UChar)sChr;

        if (sEnCnt == 0)
        {
            if ( sCSVEnclose == ILO_FALSE )
            {
                sFTMatchLen = mFTLexStateTransTbl[sFTMatchLen][sTmp];
                if (sFTMatchLen == (UInt)m_nFTLen )
                {
                    mDataFileRead += (*aLOBPhyLen);
                    if ((*aLOBLen) == (ULong)m_nFTLen &&
                        m_SetEnclosing)
                    {
                        return TFIELD_TERM;
                    }
                    else
                    {
                        m_SetNextToken = SQL_TRUE;
                        mNextToken = TFIELD_TERM;
                        (*aLOBPhyLen) -= (ULong)mFTPhyLen;
                        (*aLOBLen) -= (ULong)m_nFTLen;
                        return TVALUE;
                    }
                }

                sRTMatchLen = mRTLexStateTransTbl[sRTMatchLen][sTmp];
                if (sRTMatchLen == (UInt)m_nRTLen)
                {
                    mDataFileRead += (*aLOBPhyLen);
                    if ((*aLOBLen) == (ULong)m_nRTLen &&
                          (m_SetEnclosing || sHandle->mProgOption->mInformix))
                    {
                        return TROW_TERM;
                    }
                    else
                    {
                        m_SetNextToken = SQL_TRUE;
                        mNextToken = TROW_TERM;
                        (*aLOBPhyLen) -= (ULong)mRTPhyLen;
                        (*aLOBLen) -= (ULong)m_nRTLen;
                        return TVALUE;
                    }
                }
            }
        }

        // csv 포멧일경우 아래 부분은 무시된다.
        if (m_SetEnclosing &&
            (sEnCnt == 1 || (*aLOBLen) == (ULong)sEnMatchLen + 1))
        {
            sEnMatchLen = mEnLexStateTransTbl[sEnMatchLen][sTmp];
            if (sEnMatchLen == (UInt)m_nEnLen)
            {
                mDataFileRead += (*aLOBPhyLen);
                if (sEnCnt == 0)
                {
                    sEnCnt++;
                    sEnMatchLen = 0;
                    (*aLOBPhyOffs) = mDataFileRead;
                    (*aLOBPhyLen) = ID_ULONG(0);
                    (*aLOBLen) = ID_ULONG(0);
                }
                else
                {
                    (*aLOBPhyLen) -= (ULong)mEnPhyLen;
                    (*aLOBLen) -= (ULong)m_nEnLen;
                    return TVALUE;
                }
            }
        }
    }

    return TVALUE;

    IDE_EXCEPTION(NextTokenExist);
    {
        m_SetNextToken = SQL_FALSE;
        return mNextToken;
    }
    IDE_EXCEPTION(EOFReached);
    {
        if ((*aLOBPhyLen) > ID_ULONG(0))
        {
            mDataFileRead += (*aLOBPhyLen);
            m_SetNextToken = SQL_TRUE;
            mNextToken = TEOF;
            return TVALUE;
        }
        else
        {
            return TEOF;
        }
    }
    IDE_EXCEPTION(WrongCSVFormat);
    {
        // csv format이 아니다.
        // 옳바로 data가 입력되지 않을 수 있다.
        return TERROR;
    }
    IDE_EXCEPTION_END;

    return TEOF;
}

void iloDataFile::rtrim()
{
    SInt   cnt = 0;
    SInt   len = 0;
    SChar *ptr;
    len = idlOS::strlen(m_TokenValue);
    ptr =  m_TokenValue + len;
    while (cnt < len)
    {
        if (*ptr != ' ') break;
        ptr--;
        cnt++;
    }
    if (cnt < len)
    {
        *ptr = '\0';
    }
}

/*
 *   Return Value
 *   READ_ERROR         0
 *   READ_SUCCESS       1
 *   END_OF_FILE        2
 */
 //PROJ-1714
SInt iloDataFile::ReadOneRecordFromCBuff( ALTIBASE_ILOADER_HANDLE  aHandle,
                                          iloTableInfo            *aTableInfo,
                                          SInt                     aArrayCount )
{

    EDataToken eToken;
    SInt i = 0;
    SInt j = 0;
    SInt sMsSql;
    SInt sRC = READ_SUCCESS;
    IDE_RC sRC_loadFromOutFile;   //PROJ-2030, CT_CASE-3020 CHAR outfile 지원
    IDE_RC sRC_SetAttrValue;
    SChar sColName[MAX_OBJNAME_LEN];
    
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    sMsSql             = sHandle->mProgOption->mMsSql;
    mLOBFileColumnNum  = 0;           //BUG-24583 Init 
    
    if ( i < aTableInfo->GetAttrCount() &&
         aTableInfo->GetAttrType(i) == ISP_ATTR_TIMESTAMP &&
         sHandle->mParser.mAddFlag == ILO_TRUE )
    {
        i++;
    }
    
    /* BUG - 18804 */
    if( sHandle->mProgOption->m_bExist_bad || sHandle->mProgOption->m_bExist_log || ( sHandle->mLogCallback != NULL))
    {
        for (; j < aTableInfo->GetAttrCount(); j++)
        {
            aTableInfo->mAttrFail[ j ][ 0 ] = '\0';
            aTableInfo->mAttrFailLen[j] = 0;
        }
    }
    
    if (i < aTableInfo->GetAttrCount())
    {
        /* 컬럼 값 읽음. */
        if ( ((aTableInfo->GetAttrType(i) != ISP_ATTR_CLOB) &&
              (aTableInfo->GetAttrType(i) != ISP_ATTR_BLOB))
            ||
             (mUseLOBFileOpt == ILO_TRUE) )
        {
            /* TASK-2657 */
            if( sHandle->mProgOption->mRule == csv )
            {
                eToken = GetCSVTokenFromCBuff( sHandle, aTableInfo->GetAttrName(i));
            }
            else
            {
                eToken = GetTokenFromCBuff(sHandle);
            }
        }
        else
        {
            eToken = GetLOBToken( sHandle,
                                  &aTableInfo->mLOBPhyOffs[i][aArrayCount],
                                  &aTableInfo->mLOBPhyLen[i][aArrayCount],
                                  &aTableInfo->mLOBLen[i][aArrayCount] );
        }
    }
    /* 컬럼 skip만으로 더 이상 읽어들일 컬럼이 없는 경우 */
    else /* (i == pTableInfo->GetAttrCount()) */
    {
        /* 데이터 파일의 이번 행에서 남은 데이터를 행 구분자가 나올 때까지 무시 */
        do
        {
            /* TASK-2657 */
            if( sHandle->mProgOption->mRule == csv )
            {
                eToken = GetCSVTokenFromCBuff( sHandle, aTableInfo->GetAttrName(i));
            }
            else
            {
                eToken = GetTokenFromCBuff(sHandle);
            }
        } 
        while (eToken != TEOF && eToken != TROW_TERM);

        if (eToken == TEOF)
        {
            aTableInfo->SetReadCount(i, aArrayCount);

            sRC = END_OF_FILE;
            IDE_CONT(RETURN_CODE);
        }
    }
    
    
    for ( ; i < aTableInfo->GetAttrCount(); i++)
    {
        if (eToken == TEOF)
        {
            aTableInfo->SetReadCount(i, aArrayCount);
            if (i == 0 ||
                (i == 1 && sHandle->mParser.mAddFlag == ILO_TRUE &&
                aTableInfo->GetAttrType(0) == ISP_ATTR_TIMESTAMP))
            {
                sRC = END_OF_FILE;
                IDE_CONT(RETURN_CODE);
            }
            else
            {
                sRC = READ_ERROR;
                IDE_CONT(RETURN_CODE);
            }
        }
        else if (eToken == TVALUE )
        {
            if ( (aTableInfo->GetAttrType(i) != ISP_ATTR_CLOB) &&
                 (aTableInfo->GetAttrType(i) != ISP_ATTR_BLOB) )
            {
                
                //PROJ-2030, CT_CASE-3020 CHAR outfile 지원
                //loadFromOutFile() 이 성공하는 경우에만 SetAttrValue() 를 실행한다.
                //loadFromOutFile() 또는 SetAttrValue가 실패하는 경우는 에러카운트하고, 
                //err화일에 기록한다.
                sRC_loadFromOutFile = IDE_SUCCESS;
                sRC_SetAttrValue = IDE_SUCCESS;

                if ( (mTokenLen > 0) && (aTableInfo->mOutFileFlag[i] == ILO_TRUE) )
                {
                    sRC_loadFromOutFile = loadFromOutFile(sHandle);
                }
                
                if (sRC_loadFromOutFile == IDE_SUCCESS)
                {
                    /* 값 저장해둠. */
                    sRC_SetAttrValue = aTableInfo->SetAttrValue( sHandle,
                                                                 i,
                                                                 aArrayCount,
                                                                 m_TokenValue,
                                                                 mTokenLen,
                                                                 sMsSql,
                                                                 sHandle->mProgOption->mNCharUTF16);
                }
            
                if ( (sRC_loadFromOutFile != IDE_SUCCESS) || (sRC_SetAttrValue != IDE_SUCCESS) )
                {
                    /* BUG - 18804 */
                    /* bad or log option이 명시 돼있을경우 fail된 필드값을 mAttrFail[ i ]에 저장함 */
                    if( sHandle->mProgOption->m_bExist_bad || 
                            sHandle->mProgOption->m_bExist_log || (sHandle->mLogCallback != NULL))
                    {
                        if( sHandle->mProgOption->mRule == csv )
                        {
                            // TASK-2657
                            IDE_TEST(aTableInfo->SetAttrFail( sHandle,
                                                              i,
                                                              mErrorToken,
                                                              mErrorTokenLen)
                                                              != IDE_SUCCESS);
                        }
                        // proj1778 nchar
                        // utf16 nchar data처리를 위해서
                        // no csv에서도, sprintf를 사용안하고 memcpy하도록 수정
                        // ref) no csv인 경우 mErrorToken 을 사용하지 않음
                        else
                        {
                            IDE_TEST(aTableInfo->SetAttrFail( sHandle,
                                                              i,
                                                              m_TokenValue,
                                                              mTokenLen)
                                                              != IDE_SUCCESS);
                        }
                    }
                    sRC = READ_ERROR;
                }
            }

            else if (mUseLOBFileOpt == ILO_TRUE)
            {
                /* LOB indicator를 분석하여 오프셋과 길이를 얻음. */
                if (AnalLOBIndicator(&aTableInfo->mLOBPhyOffs[i][aArrayCount],
                                     &aTableInfo->mLOBPhyLen[i][aArrayCount])
                    != IDE_SUCCESS)
                {
                    sRC = READ_ERROR;
                }
            }
            else
            {
                /* 컬럼 최대 길이 넘어서는지 검사. */
                if (aTableInfo->GetAttrType(i) == ISP_ATTR_CLOB)
                {
                    if (aTableInfo->mLOBLen[i][aArrayCount]
                        > ID_ULONG(0xFFFFFFFF))
                    {
                        // BUG-24823 iloader 에서 파일라인을 에러메시지로 출력하고 있어서 diff 가 발생합니다.
                        // 파일명과 라인수를 출력하는 부분을 제거합니다.
                        // BUG-24898 iloader 파싱에러 상세화
                        uteSetErrorCode( sHandle->mErrorMgr,
                                        utERR_ABORT_Token_Value_Range_Error,
                                        0,
                                        aTableInfo->GetTransAttrName(i,sColName,(UInt)MAX_OBJNAME_LEN),
                                        "[CLOB]");
                        aTableInfo->SetReadCount(i, aArrayCount);
                        sRC = READ_ERROR;
                    }
                }
                else /* (pTableInfo->GetAttrType(i) == ISP_ATTR_BLOB) */
                {
                    if (aTableInfo->mLOBLen[i][aArrayCount]
                        > ID_ULONG(0x1FFFFFFFE))
                    {
                        // BUG-24823 iloader 에서 파일라인을 에러메시지로 출력하고 있어서 diff 가 발생합니다.
                        // 파일명과 라인수를 출력하는 부분을 제거합니다.
                        // BUG-24898 iloader 파싱에러 상세화
                        uteSetErrorCode( sHandle->mErrorMgr,
                                        utERR_ABORT_Token_Value_Range_Error,
                                        0,
                                        aTableInfo->GetTransAttrName(i,sColName,(UInt)MAX_OBJNAME_LEN),
                                        "[BLOB]");
                        aTableInfo->SetReadCount(i, aArrayCount);
                        sRC = READ_ERROR;
                    }
                }
            }

            /* 컬럼 skip */
            if ( i+1 < aTableInfo->GetAttrCount() &&
                 aTableInfo->GetAttrType(i+1) == ISP_ATTR_TIMESTAMP &&
                 sHandle->mParser.mAddFlag == ILO_TRUE )
            {
                i++;
            }
            /* TASK-2657 */
            if( sHandle->mProgOption->mRule == csv )
            {
                eToken = GetCSVTokenFromCBuff( sHandle, aTableInfo->GetAttrName(i));
            }
            else
            {
                /* 값에 이어지는 필드 구분자, 행 구분자를 읽음. */
                eToken = GetTokenFromCBuff(sHandle);
            }

        }    
        /* TASK-2657 */
        /* this part for sticking the CSV module on previous messy code :( */
        else if ( eToken == TERROR )
        {
            if( sHandle->mProgOption->m_bExist_bad ||
                    sHandle->mProgOption->m_bExist_log || (sHandle->mLogCallback != NULL) )
            {
                IDE_TEST(aTableInfo->SetAttrFail( sHandle,
                                                  i, 
                                                  mErrorToken, 
                                                  mErrorTokenLen)
                                                  != IDE_SUCCESS);
            }
            sRC = READ_ERROR;
            if( sHandle->mProgOption->mRule == csv )
            {
                eToken = GetCSVTokenFromCBuff( sHandle, aTableInfo->GetAttrName(i));
                if( eToken == TEOF )
                {
                    aTableInfo->SetReadCount(i+1, aArrayCount);

                    sRC = READ_ERROR;
                    IDE_CONT(RETURN_CODE);
                }
            }
            else
            {
                eToken = GetTokenFromCBuff(sHandle);
            }
        }
        else /* !TEOF && !TVALUE */
        {
            aTableInfo->SetReadCount(i, aArrayCount);

            sRC = READ_ERROR;
            IDE_CONT(RETURN_CODE);
        }

        if ((i == aTableInfo->GetAttrCount()-1) && (eToken == TEOF))
        {
            aTableInfo->SetReadCount(i + 1, aArrayCount);

            IDE_CONT(RETURN_CODE);
        }

        if (sHandle->mProgOption->mInformix == SQL_TRUE &&
            (i == aTableInfo->GetAttrCount()-1) && (eToken == TFIELD_TERM))
        {
            /* TASK-2657 */
            if( sHandle->mProgOption->mRule == csv )
            {
                eToken = GetCSVTokenFromCBuff( sHandle, aTableInfo->GetAttrName(i));
            }
            else
            {
                eToken = GetTokenFromCBuff(sHandle);
            }
            if (eToken == TEOF)
            {
                aTableInfo->SetReadCount(i + 1, aArrayCount);

                IDE_CONT(RETURN_CODE);
            }
            else if (eToken != TROW_TERM)
            {
                aTableInfo->SetReadCount(i, aArrayCount);

                sRC = READ_ERROR;
                IDE_CONT(RETURN_CODE);
            }
        }
        else if ((i == aTableInfo->GetAttrCount()-1) && (eToken != TROW_TERM))
        {
            /* TASK-2657 */
            /* if we read columns more than what we expected */
            if( sHandle->mProgOption->mRule == csv && eToken == TFIELD_TERM )
            {
                aTableInfo->SetReadCount(i + 1, aArrayCount);
                do
                {
                    eToken = GetCSVTokenFromCBuff( sHandle, aTableInfo->GetAttrName(i));
                }
                while ( eToken != TROW_TERM && eToken != TEOF);
                if ( eToken == TEOF )
                {
                    sRC = END_OF_FILE;
                    IDE_CONT(RETURN_CODE);
                }
            }
            else
            {
                aTableInfo->SetReadCount(i, aArrayCount);
            }

            sRC = READ_ERROR;
            IDE_CONT(RETURN_CODE);
        }
        else if ( (i < aTableInfo->GetAttrCount()-1) &&
                  (eToken != TFIELD_TERM) )
        {
            //BUG-28584
            aTableInfo->SetReadCount( i + 1, aArrayCount);

            sRC = READ_ERROR;
            IDE_CONT(RETURN_CODE);
        }

        if (i < aTableInfo->GetAttrCount()-1)
        {
            /* 컬럼 값 읽음. */
            if ((aTableInfo->GetAttrType(i + 1) != ISP_ATTR_CLOB &&
                 aTableInfo->GetAttrType(i + 1) != ISP_ATTR_BLOB)
                ||
                mUseLOBFileOpt == ILO_TRUE)
            {
                /* TASK-2657 */
                if( sHandle->mProgOption->mRule == csv )
                {
                    eToken = GetCSVTokenFromCBuff( sHandle, aTableInfo->GetAttrName(i));
                }
                else
                {
                    eToken = GetTokenFromCBuff(sHandle);
                }
            }
            else
            {
                eToken = GetLOBToken( sHandle,
                                      &aTableInfo->mLOBPhyOffs[i + 1][aArrayCount],
                                      &aTableInfo->mLOBPhyLen[i + 1][aArrayCount],
                                      &aTableInfo->mLOBLen[i + 1][aArrayCount] );
            }
        }
    }

    aTableInfo->SetReadCount(i, aArrayCount);
    mLOBFileColumnNum = 0;          //BUG-24583 : 다음에서 실제 LOB File을 Open 하기위해 초기화 한다.

    // BUG-24898 iloader 파싱에러 상세화
    // 모든 return 은 여기로 모은다.
    // 에러코드가 설정되어 있지 않으면 데이타 파싱에러를 출력한다.
    IDE_EXCEPTION_CONT(RETURN_CODE);

    if(uteGetErrorCODE(sHandle->mErrorMgr) == 0)
    {
        if(sRC == READ_ERROR)
        {
            if(aTableInfo->GetAttrCount() > i)
            {
                uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Parsing_Error,
                                aTableInfo->GetTransAttrName(i,sColName,(UInt)MAX_OBJNAME_LEN)
                               );
            }
        }
        else if ( sHandle->mThreadErrExist == ILO_TRUE )
        {
            uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_THREAD_Error);
        }
    }

    return sRC;

    IDE_EXCEPTION_END;

    return SYS_ERROR;
}


SInt iloDataFile::strtonumCheck(SChar *p)
{
    SChar c;
    while((c = *p) == ' ' || (c = *p) == '\t' )
    {
        p++;
    }

    if (c == '-' || c == '+')
    {
        p++;
    }

    while ((c = *p++), isdigit(c))
        ;

    if (c=='.')
    {
        while ((c = *p++), isdigit(c)) ;
    }
    else if (c=='\0')
    {
        return 1;
    }
    else
    {
        return 0;
    }

    if ((c == 'E') || (c == 'e'))
    {
        if ((c= *p++) == '+')
            ;
        else if (c=='-')
           ;
        else
           --p;
        while ((c = *p++), isdigit(c))
          ;
    }

    if ( c == '\0' )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * AnalDataFileName.
 *
 * 데이터 파일명을 분석하여, 파일명 본체, 데이터 파일 확장자,
 * 데이터 파일 번호를 얻는다.
 * 분석 결과는 멤버변수 mFileNameBody, mDataFileNameExt,
 * mDataFileNo에 저장된다.
 *
 * @param[in] aDataFileName
 *  데이터 파일명.
 * @param[in] aIsWr
 *  데이터 파일 쓰기(ILO_TRUE)인지 읽기(ILO_FALSE)인지 여부.
 */
void iloDataFile::AnalDataFileName(const SChar *aDataFileName, iloBool aIsWr)
{
    SChar  sBk = 0;
    SChar *sFileNoPos;
    SChar *sPeriodPos;
    SInt   sI;

    (void)idlOS::snprintf(mFileNameBody, ID_SIZEOF(mFileNameBody), "%s",
                          aDataFileName);
    /* 파일 번호를 -1로 초기화한다. */
    mDataFileNo = -1;

    /* 파일명 뒷부분의 불필요한 공백을 제거한다. */
    for (sI = idlOS::strlen(mFileNameBody) - 1; sI >= 0; sI--)
    {
        if (mFileNameBody[sI] != ' ' && mFileNameBody[sI] != '\f' &&
            mFileNameBody[sI] != '\n' && mFileNameBody[sI] != '\r' &&
            mFileNameBody[sI] != '\t' && mFileNameBody[sI] != '\v')
        {
            break;
        }
    }
    mFileNameBody[sI + 1] = '\0';

    /* 데이터 파일 읽기인 경우,
     * 사용자가 입력한 데이터 파일명에 파일 번호가 존재할 수 있다. */
    if (aIsWr != ILO_TRUE)
    {
        /* 파일 번호의 시작 위치를 구한다. */
        for (; sI >= 0; sI--)
        {
            if (mFileNameBody[sI] < '0' || mFileNameBody[sI] > '9')
            {
                break;
            }
        }
        if (mFileNameBody[sI + 1] != '\0')
        {
            sFileNoPos = &mFileNameBody[sI + 1];
        }
        else
        {
            sFileNoPos = NULL;
        }
    }
    /* 데이터 파일 쓰기인 경우,
     * 사용자가 입력한 데이터 파일명에는 파일 번호가 없는 것으로 본다. */
    else /* (aIsWr == ILO_TRUE) */
    {
        sFileNoPos = NULL;
    }

    /* 확장자의 시작 위치를 구한다. */
    sPeriodPos = idlOS::strrchr(mFileNameBody, '.');
    if (sPeriodPos == mFileNameBody)
    {
        sPeriodPos = NULL;
    }

    /* 확장자가 존재하는 경우 */
    if (sPeriodPos != NULL)
    {
        /* 파일 번호를 제외한 순수한 확장자를 검사하기 위해,
         * 임시로 파일 번호 부분을 NULL로 덮어쓴다. */
        if (sFileNoPos != NULL)
        {
            sBk = *sFileNoPos;
            *sFileNoPos = '\0';
        }
        /* 확장자가 ".dat"인 경우 */
        if (idlOS::strcasecmp(sPeriodPos, ".dat") == 0)
        {
            /* 확장자는 파일 번호를 제외한 부분(".dat")으로 한다. */
            (void)idlOS::snprintf(mDataFileNameExt,
                                  ID_SIZEOF(mDataFileNameExt), "%s",
                                  sPeriodPos);
            /* 파일 번호가 존재하는 경우 */
            if (sFileNoPos != NULL)
            {
                /* 파일 번호를 얻는다. */
                *sFileNoPos = sBk;
                mDataFileNo = (SInt)idlOS::strtol(sFileNoPos, (SChar **)NULL,
                                                  10);
            }
        }
        /* 확장자가 ".dat"가 아닌 경우 */
        else
        {
            /* 확장자는 파일명 뒷부분의 숫자 부분까지 포함하도록 한다. */
            if (sFileNoPos != NULL)
            {
                *sFileNoPos = sBk;
            }
            (void)idlOS::snprintf(mDataFileNameExt,
                                  ID_SIZEOF(mDataFileNameExt), "%s",
                                  sPeriodPos);
        }
        /* 파일명 본체는 확장자 및 파일 번호를 제외한 부분으로 한다. */
        *sPeriodPos = '\0';
    }
    /* 확장자가 존재하지 않는 경우 */
    else /* (sPeriodPos == NULL) */
    {
        mDataFileNameExt[0] = '\0';

        /* 파일명 본체는 파일명 뒷부분의 숫자 부분까지 포함하게 된다. */
    }
}

/**
 * GetStrPhyLen.
 *
 * 인자로 받은 문자열이 텍스트 파일에 저장될 경우 가지게 될 길이를 구한다.
 *
 * @param[in] aStr
 *  길이를 구할 문자열.
 *  NULL로 종결되어야 한다.
 */
UInt iloDataFile::GetStrPhyLen(const SChar *aStr)
{
    UInt         sPhyLen;
#if defined(VC_WIN32) || defined(VC_WIN64) || defined(VC_WINCE)
    const SChar *sCh;

    for (sCh = aStr, sPhyLen = 0; *sCh != '\0'; sCh++, sPhyLen++)
    {
        if (*sCh == '\n')
        {
            sPhyLen++;
        }
    }
#else
    sPhyLen = (UInt)idlOS::strlen(aStr);
#endif

    return sPhyLen;
}

/**
 * MakeAllLexStateTransTbl.
 *
 * 필드 구분자, 행 구분자, 필드 encloser 매칭을 위한
 * 상태전이표를 작성한다.
 */
IDE_RC iloDataFile::MakeAllLexStateTransTbl( ALTIBASE_ILOADER_HANDLE aHandle )
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    IDE_TEST(MakeLexStateTransTbl( sHandle, m_FieldTerm, &mFTLexStateTransTbl)
             != IDE_SUCCESS);
    IDE_TEST(MakeLexStateTransTbl( sHandle, m_RowTerm, &mRTLexStateTransTbl)
             != IDE_SUCCESS);
    if (m_SetEnclosing)
    {
        IDE_TEST(MakeLexStateTransTbl( sHandle, m_Enclosing, &mEnLexStateTransTbl)
                 != IDE_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * FreeAllLexStateTransTbl.
 *
 * 필드 구분자, 행 구분자, 필드 encloser 매칭을 위한
 * 상태전이표를 메모리 해제한다.
 */
void iloDataFile::FreeAllLexStateTransTbl()
{
    if (mFTLexStateTransTbl != NULL)
    {
        FreeLexStateTransTbl(mFTLexStateTransTbl);
        mFTLexStateTransTbl = NULL;
    }
    if (mRTLexStateTransTbl != NULL)
    {
        FreeLexStateTransTbl(mRTLexStateTransTbl);
        mRTLexStateTransTbl = NULL;
    }
    if (mEnLexStateTransTbl != NULL)
    {
        FreeLexStateTransTbl(mEnLexStateTransTbl);
        mEnLexStateTransTbl = NULL;
    }
}

/**
 * MakeLexStateTransTbl.
 *
 * 구분자 매칭용 상태전이표를 작성한다.
 * 상태전이표는 8비트 integer의 2차원 배열이다.
 * 배열의 크기는 (aStr 문자열 길이)x(256)이다.
 * (정확히는 상태전이표 메모리 해제 시의 편의를 위해
 * aStr 문자열 길이보다 1 크게 할당한다.)
 * 배열의 1번 인덱스는 aStr 문자열의 선두로부터
 * 몇 개의 문자가 매칭된 상태인가를 나타낸다.
 * 배열의 2번 인덱스는 새로이 입력받은 문자를 나타낸다.
 * 배열의 원소 값은 위 1, 2번 인덱스의 조건일 때
 * aStr 문자열의 선두로부터 몇 개의 문자가 매칭된 상태로 되는가이다.
 * 예를 들어, (*aTbl)[1]['^']의 값은
 * aStr 문자열의 선두 1문자가 이미 매칭되어있는 상황에서
 * 이번에 새로이 '^' 문자를 입력받은 경우
 * aStr 문자열의 선두로부터 몇 개의 문자가 매칭된 상태로 되는가를 나타낸다.
 * 만약, aStr이 "^^"라면, (*aTbl)[1]['^']=2가 된다.
 *
 * @param[in] aStr
 *  구분자 매칭용 상태전이표를 작성할 구분자.
 *  NULL로 종결되어야 한다.
 * @param[out] aTbl
 *  구분자 매칭용 상태전이표.
 */
IDE_RC iloDataFile::MakeLexStateTransTbl( ALTIBASE_ILOADER_HANDLE  aHandle,
                                          SChar                  *aStr, 
                                          UChar                ***aTbl)
{
    iloBool  *sBfSubstLeftSubstMatch = NULL;
    UChar  **sTbl = NULL;
    UInt     sI, sJ, sK;
    UInt     sStrLen;

    // BUG-35099: [ux] Codesonar warning UX part -248480.2579709
    UInt     sState4Tbl   = 0;
    UInt     sState4Match = 0;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    sStrLen = (UInt)idlOS::strlen(aStr);

    /* 함수 내부적으로 사용하는 변수 메모리 할당. */
    if (sStrLen > 1)
    {
        sBfSubstLeftSubstMatch = (iloBool *)idlOS::malloc(
                                            (sStrLen - 1) * ID_SIZEOF(iloBool));
        IDE_TEST(sBfSubstLeftSubstMatch == NULL);
        sState4Match = 1;
    }

    /* 상태전이표 메모리 할당. */
    sTbl = (UChar **)idlOS::malloc((sStrLen + 1) * ID_SIZEOF(UChar *));
    IDE_TEST(sTbl == NULL);
    sState4Tbl = 1;

    idlOS::memset(sTbl, 0, (sStrLen + 1) * ID_SIZEOF(UChar *));

    for (sI = 0; sI < sStrLen; sI++)
    {
        sTbl[sI] = (UChar *)idlOS::malloc(256 * ID_SIZEOF(UChar));
        IDE_TEST(sTbl[sI] == NULL);
        sState4Tbl = 2;
    }

    /* 상태전이표 작성. */
    for (sI = 0; sI < sStrLen; sI++)
    {
        for (sK = sI; sK > 0; sK--)
        {
            if (sK == 1 ||
                idlOS::strncmp(&aStr[sI + 1 - sK], aStr, sK - 1) == 0)
            {
                sBfSubstLeftSubstMatch[sK - 1] = ILO_TRUE;
            }
            else
            {
                sBfSubstLeftSubstMatch[sK - 1] = ILO_FALSE;
            }
        }

        for (sJ = 0; sJ < 256; sJ++)
        {
            if ((UInt)aStr[sI] == sJ)
            {
                sTbl[sI][sJ] = (UChar)(sI + 1);
            }
            else
            {
                for (sK = sI; sK > 0; sK--)
                {
                    if (sBfSubstLeftSubstMatch[sK - 1] == ILO_TRUE &&
                        (UInt)aStr[sK - 1] == sJ)
                    {
                        break;
                    }
                }
                sTbl[sI][sJ] = (UChar)sK;
            }
        }
    }

    if( sState4Match == 1 )
    {
        idlOS::free(sBfSubstLeftSubstMatch);
    }

    /* 리턴. */
    if( sState4Tbl == 2 )
    {
        FreeLexStateTransTbl(*aTbl);
    }

    (*aTbl) = sTbl;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_memory_error, __FILE__, __LINE__);
    if ( sHandle->mUseApi != SQL_TRUE )
    {
        utePrintfErrorCode(stdout, sHandle->mErrorMgr);
    }

    switch( sState4Tbl )
    {
    case 2:
        for (sI = 0; sI < sStrLen; sI++)
        {
            if(sTbl[sI] != NULL)
            {
                (void)idlOS::free( sTbl[sI] );
            }
        }
        /* fall through */

    case 1:
        (void)idlOS::free( sTbl );
        break;

    default:
        break;
    }

    switch( sState4Match )
    {
    case 1:
        (void)idlOS::free( sBfSubstLeftSubstMatch );
    default:
        break;
    }

    return IDE_FAILURE;
}

/**
 * FreeLexStateTransTbl.
 *
 * 구분자 매칭용 상태전이표를 메모리 해제한다.
 *
 * @param[in] aTbl
 *  구분자 매칭용 상태전이표.
 */
void iloDataFile::FreeLexStateTransTbl(UChar **aTbl)
{
    UInt     sI;

    if (aTbl != NULL)
    {
        for (sI = 0; aTbl[sI] != NULL; sI++)
        {
            idlOS::free(aTbl[sI]);
        }
        idlOS::free(aTbl);
    }
}

/**
 * SetLOBOptions.
 *
 * 사용자로부터 입력받은 LOB 관련 옵션을 iloDataFile 객체에 설정한다.
 *
 * @param[in] aUseLOBFile
 *  use_lob_file 옵션.
 * @param[in] aLOBFileSize
 *  lob_file_size 옵션. 바이트 단위.
 * @param[in] aUseSeparateFiles
 *  use_separate_files 옵션.
 * @param[in] aLOBIndicator
 *  lob_indicator 옵션.
 *  %로 시작되는 이스케이프 문자(예:%n)는 '\n'과 같이 변환된 상태임.
 */
void iloDataFile::SetLOBOptions(iloBool aUseLOBFile, ULong aLOBFileSize,
                                iloBool aUseSeparateFiles,
                                const SChar *aLOBIndicator)
{
    mUseLOBFileOpt = aUseLOBFile;
    mLOBFileSizeOpt = aLOBFileSize;
    mUseSeparateFilesOpt = aUseSeparateFiles;
    (void)idlOS::snprintf(mLOBIndicatorOpt, ID_SIZEOF(mLOBIndicatorOpt), "%s",
                          aLOBIndicator);
    mLOBIndicatorOptLen = (UInt)idlOS::strlen(mLOBIndicatorOpt);
}

/**
 * InitLOBProc.
 *
 * LOB 처리를 초기화한다.
 * use_lob_file=yes, use_separate_files=no인 경우
 * 1번 LOB 파일을 여는 작업도 수행한다.
 *
 * @param[in] aIsWr
 *  LOB 파일에 쓰기(ILO_TRUE)인지 읽기(ILO_FALSE)인지 여부.
 */
IDE_RC iloDataFile::InitLOBProc( ALTIBASE_ILOADER_HANDLE aHandle, iloBool aIsWr)
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    /* LOB wrapper 객체 메모리 할당 */
    if (mLOB != NULL)
    {
        delete mLOB;
    }
    mLOB = new iloLOB;
    IDE_TEST_RAISE(mLOB == NULL, MAllocError);

    mLOBFileNo = 0;
    mLOBFilePos = ID_ULONG(0);
    mAccumLOBFilePos = ID_ULONG(0);

    if (aIsWr == ILO_TRUE)   /* Download ... */
    {
        if (mUseLOBFileOpt == ILO_TRUE && mUseSeparateFilesOpt == ILO_FALSE)
        {
            /* 1번 LOB 파일 열기 */
            IDE_TEST_RAISE(OpenLOBFile(sHandle, ILO_TRUE, 1, 0, ILO_TRUE)
                           != IDE_SUCCESS, OpenLOBFileError);
        }
    }
    else /* (aIsWr == ILO_FALSE) */ /* Upload ... */
    {
        mLOBFileSizeOpt = ID_ULONG(0);

        //BUG-24583 'use_separate_files=yes' 옵션을 사용할 경우에는 LOBFileSize를 얻지 않는다.
        if (mUseLOBFileOpt == ILO_TRUE && mUseSeparateFilesOpt == ILO_FALSE)  
        {
            /* 1번 LOB 파일 열기.
             * 읽기인 경우 use_separate_files 여부는
             * 실제 데이터 파일을 읽다가 %%separate_files가 나와야 알 수 있으므로
             * 파일 열기에 실패하더라도 에러로 처리하지는 않는다. */
            if (OpenLOBFile(sHandle,ILO_FALSE, 1, 0, ILO_FALSE) == IDE_SUCCESS)
            {
                /* lob_file_size 얻기 */
                IDE_TEST_RAISE(GetLOBFileSize( sHandle, &mLOBFileSizeOpt)
                                     != IDE_SUCCESS, OpenLOBFileError);
            }
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(MAllocError);
    {
        uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_memory_error,
                        __FILE__, __LINE__);
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
    }
    IDE_EXCEPTION(OpenLOBFileError);
    {
        delete mLOB;
        mLOB = NULL;
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * FinalLOBProc.
 *
 * LOB 처리를 마무리한다.
 * 열려있는 LOB 파일을 닫고, LOB wrapper 객체를 메모리 해제한다.
 */
IDE_RC iloDataFile::FinalLOBProc( ALTIBASE_ILOADER_HANDLE aHandle )
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    if (mLOB != NULL)
    {
        delete mLOB;
        mLOB = NULL;
    }
    if (mLOBFP != NULL)
    {
        IDE_TEST(CloseLOBFile(sHandle) != IDE_SUCCESS);
        mLOBFP = NULL;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * OpenLOBFile.
 *
 * LOB 파일을 연다.
 * 데이터 파일명으로부터 얻은 파일명 몸체와
 * 인자로 입력받은 값들에 의해 LOB 파일명을 만들어
 * 열기를 시도한다.
 *
 * @param[in] aIsWr
 *  LOB 파일에 쓰기(ILO_TRUE)인지 읽기(ILO_FALSE)인지 여부.
 * @param[in] aLOBFileNo_or_RowNo
 *  LOB 파일 번호(use_lob_file=yes, use_separate_files=no 경우)
 *  또는 행 번호(use_lob_file=yes, use_separate_files=yes 경우).
 *  본 인자를 사용하지 않는 경우 0을 주도록 한다.
 * @param[in] aColNo
 *  열 번호(use_lob_file=yes, use_separate_files=yes 경우).
 *  본 인자를 사용하지 않는 경우 0을 주도록 한다.
 * @param[in] aErrMsgPrint
 *  에러 발생 시 에러 메시지를 출력할지 여부(ILO_TRUE이면 출력).
 */
IDE_RC iloDataFile::OpenLOBFile( ALTIBASE_ILOADER_HANDLE  aHandle,
                                 iloBool                   aIsWr,
                                 UInt                     aLOBFileNo_or_RowNo,
                                 UInt                     /*aColNo*/,
                                 iloBool                   aErrMsgPrint, 
                                 SChar                   *aFilePath)
{
    SChar sFileName[MAX_FILEPATH_LEN] = { '\0', };
    SChar sMode[3];
    SInt  sPos;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    if (mLOBFP != NULL)
    {
        IDE_TEST(CloseLOBFile(sHandle) != IDE_SUCCESS);
    }

    if (mUseLOBFileOpt == ILO_TRUE)
    {
        if (mUseSeparateFilesOpt == ILO_TRUE)
        {
            /* BUG-24583 
             * use_separate_files=yes가 설정되어있을 경우,
             */
            if (aIsWr == ILO_TRUE)   /* Download */
            {
                (void)idlOS::snprintf(sFileName, ID_SIZEOF(sFileName), 
                                      "%s/%09"ID_UINT32_FMT".lob",
                                      aFilePath, aLOBFileNo_or_RowNo);
            }
            /* Upload 일 경우에는, DataFile에서 LOB File경로 및 이름을 읽어서 fileOpen을 수행한다. */
            else
            {
                IDE_TEST_RAISE(idlOS::strlen(mLOBFile[mLOBFileColumnNum]) == 0, OpenError);

                //BUG-24902: use_separate_files=yes일 경우 DataFile의 경로 고려
                /*
                | DataFile \ -d 옵션 |   상대경로    |   절대경로    |
                |:------------------:|:-------------:|:-------------:|
                |      절대경로      |    DataFile   |    DataFile   |
                |      상대경로      | -d + DataFile | -d + DataFile |

                * Datafile      : Datafile에 있는 FilePath를 사용
                * -d + Datafile : -d 옵션에 있는 FilePath있는 FilePath를 붙여서 사용
                */

                // DataFile에 있는 경로가 절대경로가 아닐경우, -d 옵션이 있으면 -d 옵션의 경로 복사
                if ((! IS_ABSOLUTE_PATH(mLOBFile[mLOBFileColumnNum]))
                 && ( sHandle->mProgOption->m_bExist_d == SQL_TRUE))
                {
                    (void)idlOS::strcpy(sFileName, mDataFilePath);
                }

                // DataFile에 있는 FilePath 앞의 "./"는 의미 없으므로 제거
                if ((idlOS::strlen(mLOBFile[mLOBFileColumnNum]) > 2)
                 && (mLOBFile[mLOBFileColumnNum][0] == '.')
                 && (mLOBFile[mLOBFileColumnNum][1] == IDL_FILE_SEPARATOR))
                {
                    sPos = 2;
                }
                else
                {
                    sPos = 0;
                }
                (void)idlOS::strcat( sFileName, mLOBFile[mLOBFileColumnNum] + sPos );

                mLOBFileColumnNum++; // BUG-24583 다음 Filename을 읽기하기 위해서...
            }
        }
        else /* (mUseSeparateFilesOpt == ILO_FALSE) */
        {
            if (mDataFileNo < 0)
            {
                (void)idlOS::snprintf(sFileName, ID_SIZEOF(sFileName),
                                      "%s_%09"ID_UINT32_FMT".lob",
                                      mFileNameBody, aLOBFileNo_or_RowNo);
            }
            else
            {
                (void)idlOS::snprintf(sFileName, ID_SIZEOF(sFileName),
                              "%s_%09"ID_UINT32_FMT".lob%"ID_INT32_FMT,
                              mFileNameBody, aLOBFileNo_or_RowNo, mDataFileNo);
            }
        }

        /* LOB 파일은 이진 모드로 연다. */
        if (aIsWr == ILO_TRUE)
        {
            (void)idlOS::snprintf(sMode, ID_SIZEOF(sMode), "wb");
        }
        else
        {
            (void)idlOS::snprintf(sMode, ID_SIZEOF(sMode), "rb");
        }
        
        mLOBFP = ilo_fopen(sFileName, sMode);
        IDE_TEST_RAISE(mLOBFP == NULL, OpenError);
        
        if (mUseSeparateFilesOpt == ILO_FALSE)
        {
            mLOBFileNo = aLOBFileNo_or_RowNo;
        }
        mLOBFilePos = ID_ULONG(0);
    }
    
    return IDE_SUCCESS;

    IDE_EXCEPTION(OpenError);
    {
        if (aErrMsgPrint == ILO_TRUE)
        {
            uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_LOB_File_IO_Error);
            
            if ( sHandle->mUseApi != SQL_TRUE )
            {
                utePrintfErrorCode(stdout, sHandle->mErrorMgr);
                idlOS::printf("LOB file [%s] open fail\n", sFileName);
            }
        }
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * CloseLOBFile.
 *
 * LOB 파일을 닫는다.
 */
IDE_RC iloDataFile::CloseLOBFile( ALTIBASE_ILOADER_HANDLE aHandle )
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    if (mLOBFP != NULL)
    {
        IDE_TEST_RAISE(idlOS::fclose(mLOBFP) != 0, CloseError);
        mLOBFP = NULL;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(CloseError);
    {
        uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_LOB_File_IO_Error);
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
            (void)idlOS::printf("LOB file close fail\n");
        }
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * PrintOneLOBCol.
 *
 * 한 개의 LOB 컬럼 값을 데이터 파일 또는 LOB 파일에 출력한다.
 *
 * @param[in] aRowNo
 *  출력할 LOB 컬럼이 소속된 행의 번호.
 * @param[in] aCols
 *  컬럼 정보(LOB locator 포함)들이 들어있는 자료구조.
 * @param[in] aColIdx
 *  aCols에서 몇 번째 컬럼이 출력할 컬럼인지를 지정.
 */
IDE_RC iloDataFile::PrintOneLOBCol( ALTIBASE_ILOADER_HANDLE  aHandle,
                                    SInt                     aRowNo, 
                                    iloColumns              *aCols, 
                                    SInt                     aColIdx, 
                                    FILE                    *aWriteFp, //PROJ-1714
                                    iloTableInfo            *pTableInfo ) //BUG-24583
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    if (mUseLOBFileOpt == ILO_TRUE)
    {
        if (mUseSeparateFilesOpt == ILO_TRUE)
        {
            IDE_TEST(PrintOneLOBColToSepLOBFile( sHandle,
                                                 aRowNo, 
                                                 aCols,
                                                 aColIdx,
                                                 aWriteFp,
                                                 pTableInfo)   //BUG-24583
                                                 != IDE_SUCCESS);
        }
        else
        {
            IDE_TEST(PrintOneLOBColToNonSepLOBFile( sHandle, aCols, aColIdx, aWriteFp)
                     != IDE_SUCCESS);
        }
    }
    else
    {
        /* BUG-21064 : CLOB type CSV up/download error */
        // download 시 컬럼이 type이 CLOB이라면 무조건 enclosing("")한다. 
        if( ( sHandle->mProgOption->mRule == csv ) &&
            ( aCols->GetType(aColIdx) == SQL_CLOB ) )
        {
            IDE_TEST( idlOS::fwrite( &sHandle->mProgOption->mCSVEnclosing,
                                     1, 1,
                                     aWriteFp ) != (UInt)1 );
        }
        IDE_TEST(PrintOneLOBColToDataFile( sHandle,
                                           aCols,
                                           aColIdx,
                                           aWriteFp) != IDE_SUCCESS);
        
        if( ( sHandle->mProgOption->mRule == csv ) &&
            ( aCols->GetType(aColIdx) == SQL_CLOB ) )
        {
            IDE_TEST( idlOS::fwrite( &sHandle->mProgOption->mCSVEnclosing,
                                     1, 1,
                                     aWriteFp ) != (UInt)1 );
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * PrintOneLOBToDataFile.
 *
 * use_lob_file=no인 경우 호출되며,
 * 한 개의 LOB 컬럼 값을 데이터 파일에 출력한다.
 * 인자 aCols에 저장되어있는 LOB locator와
 * LOB wrapper 객체(mLOB)를 사용하여
 * 서버로부터 LOB 데이터를 얻어와 데이터 파일에 쓴다.
 *
 * @param[in] aCols
 *  컬럼 정보(LOB locator 포함)들이 들어있는 자료구조.
 * @param[in] aColIdx
 *  aCols에서 몇 번째 컬럼이 출력할 컬럼인지를 지정.
 */
IDE_RC iloDataFile::PrintOneLOBColToDataFile( ALTIBASE_ILOADER_HANDLE  aHandle,
                                              iloColumns              *aCols, 
                                              SInt                     aColIdx, 
                                              FILE                    *aWriteFp )   //PROJ-1714
{
    SQLSMALLINT  sColDataType;
    SQLSMALLINT  sLOBLocCType;
    UInt         sStrLen;
    UInt         sWrittenLen;
    void        *sBuf;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    if (*aCols->m_Len[aColIdx] == SQL_NULL_DATA)
    {
        return IDE_SUCCESS;
    }

    sColDataType = aCols->GetType(aColIdx);
    if (sColDataType == SQL_CLOB || sColDataType == SQL_CLOB_LOCATOR)
    {
        sLOBLocCType = SQL_C_CLOB_LOCATOR;
    }
    else /* SQL_BINARY || SQL_BLOB || SQL_BLOB_LOCATOR */
    {
        sLOBLocCType = SQL_C_BLOB_LOCATOR;
    }
    IDE_TEST(mLOB->OpenForDown( sHandle,
                                sLOBLocCType,
                                *(SQLUBIGINT *)aCols->m_Value[aColIdx],
                                SQL_C_CHAR,
                                iloLOB::LOBAccessMode_RDONLY)
             != IDE_SUCCESS);

    /* 서버로부터 LOB 데이터를 읽음. */
    while (mLOB->Fetch( sHandle, &sBuf, &sStrLen) == IDE_SUCCESS)
    {
        /* LOB 데이터를 데이터 파일에 씀. */
        sWrittenLen = (UInt)idlOS::fwrite(sBuf, 1, sStrLen, aWriteFp);
        IDE_TEST_RAISE(sWrittenLen < sStrLen, WriteError);
    }
    /* 서버로부터 LOB 데이터를 얻는 과정에서 오류 발생했는지 검사. */
    IDE_TEST_RAISE(mLOB->IsFetchError() == ILO_TRUE, FetchError);

    return IDE_SUCCESS;

    IDE_EXCEPTION(FetchError);
    {
        (void)mLOB->CloseForDown(sHandle);
    }
    IDE_EXCEPTION(WriteError);
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Data_File_IO_Error);
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
            (void)idlOS::printf("LOB data write fail\n");
        }
        
        (void)mLOB->CloseForDown(sHandle);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * PrintOneLOBToNonSepLOBFile.
 *
 * use_lob_file=yes, use_separate_files=no인 경우 호출되며,
 * 한 개의 LOB 컬럼 값을 LOB 파일에 출력한다.
 * 또, 데이터 파일에는 LOB indicator를 출력한다.
 * 인자 aCols에 저장되어있는 LOB locator와
 * LOB wrapper 객체(mLOB)를 사용하여
 * 서버로부터 LOB 데이터를 얻어와 LOB 파일에 쓴다.
 * LOB 파일 크기가 옵션 lob_file_size를 넘게 되는지 검사하여
 * 자동적으로 다음 번호의 LOB 파일을 열어 이어서 쓴다.
 *
 * @param[in] aCols
 *  컬럼 정보(LOB locator 포함)들이 들어있는 자료구조.
 * @param[in] aColIdx
 *  aCols에서 몇 번째 컬럼이 출력할 컬럼인지를 지정.
 */
IDE_RC iloDataFile::PrintOneLOBColToNonSepLOBFile( ALTIBASE_ILOADER_HANDLE  aHandle,
                                                   iloColumns              *aCols,
                                                   SInt                     aColIdx, 
                                                   FILE                    *aWriteFp )  //PROJ-1714
{
    SChar        sLOBIndicator[52];
    SQLSMALLINT  sColDataType;
    SQLSMALLINT  sLOBLocCType;
    SQLSMALLINT  sValCType;
    UInt         sLOBLength;
    UInt         sStrLen;
    UInt         sToWriteLen;
    UInt         sWrittenLen;
    void        *sBuf;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    if (*aCols->m_Len[aColIdx] == SQL_NULL_DATA)
    {
        /* 데이터 파일에 LOB indicator 출력. */
        sWrittenLen = (UInt)idlOS::fprintf(aWriteFp, "%snull",
                                           mLOBIndicatorOpt);
        IDE_TEST_RAISE(sWrittenLen < mLOBIndicatorOptLen + 4,
                       DataFileWriteError);
        return IDE_SUCCESS;
    }

    sColDataType = aCols->GetType(aColIdx);
    if (sColDataType == SQL_CLOB || sColDataType == SQL_CLOB_LOCATOR)
    {
        sLOBLocCType = SQL_C_CLOB_LOCATOR;
        sValCType = SQL_C_CHAR;
    }
    else /* SQL_BINARY || SQL_BLOB || SQL_BLOB_LOCATOR */
    {
        sLOBLocCType = SQL_C_BLOB_LOCATOR;
        sValCType = SQL_C_BINARY;
    }
    IDE_TEST(mLOB->OpenForDown( sHandle,
                                sLOBLocCType,
                                *(SQLUBIGINT *)aCols->m_Value[aColIdx],
                                sValCType,
                                iloLOB::LOBAccessMode_RDONLY)
             != IDE_SUCCESS);

    sLOBLength = mLOB->GetLOBLength();
    /* LOB이 NULL이 아닌 경우 */
    if (sLOBLength > 0)
    {
        /* LOB 파일 크기 제한이 없는 경우 */
        if (mLOBFileSizeOpt == ID_ULONG(0))
        {
            /* 서버로부터 LOB 데이터를 읽음. */
            while (mLOB->Fetch( sHandle, &sBuf, &sStrLen) == IDE_SUCCESS)
            {
                /* LOB 데이터를 LOB 파일에 씀. */
                IDE_TEST_RAISE(idlOS::fwrite(sBuf, 1, sStrLen, mLOBFP) < sStrLen,
                               LOBFileWriteError);
                mLOBFilePos += (ULong)sStrLen;
                mAccumLOBFilePos += (ULong)sStrLen;
            }
            /* 서버로부터 LOB 데이터를 얻는 과정에서 오류 발생했는지 검사. */
            IDE_TEST_RAISE(mLOB->IsFetchError() == ILO_TRUE, FetchError);
        }
        /* LOB 파일 크기 제한이 있는 경우 */
        else /* (mLOBFileSizeOpt > ID_ULONG(0)) */
        {
            /* 서버로부터 LOB 데이터를 읽음. */
            while (mLOB->Fetch( sHandle, &sBuf, &sStrLen) == IDE_SUCCESS)
            {
                /* LOB 파일 크기 제한으로 LOB 데이터를
                 * 현재 열려있는 LOB 파일에 모두 출력할 수 없는 동안
                 * 계속 루프 */
                while (mLOBFileSizeOpt - mLOBFilePos < (ULong)sStrLen)
                {
                    sToWriteLen = (UInt)(mLOBFileSizeOpt - mLOBFilePos);
                    if (sToWriteLen > 0)
                    {
                        /* LOB 데이터를 LOB 파일에 씀. */
                        sWrittenLen = (UInt)idlOS::fwrite(sBuf, 1, sToWriteLen,
                                                          mLOBFP);
                        mLOBFilePos += (ULong)sWrittenLen;
                        mAccumLOBFilePos += (ULong)sWrittenLen;
                        IDE_TEST_RAISE(sWrittenLen < sToWriteLen,
                                       LOBFileWriteError);
                    }
                    IDE_TEST_RAISE(CloseLOBFile(sHandle) != IDE_SUCCESS,
                                   OpenOrCloseLOBFileError);
                    /* 다음 번호의 LOB 파일을 연다. */
                    IDE_TEST_RAISE(OpenLOBFile(sHandle, ILO_TRUE, mLOBFileNo + 1, 0,
                                               ILO_TRUE)
                                   != IDE_SUCCESS, OpenOrCloseLOBFileError);
                    sBuf = (void *)((SChar *)sBuf + sToWriteLen);
                    sStrLen -= sToWriteLen;
                }
                /* LOB 데이터를 LOB 파일에 씀. */
                sWrittenLen = (UInt)idlOS::fwrite(sBuf, 1, sStrLen, mLOBFP);
                mLOBFilePos += (ULong)sWrittenLen;
                mAccumLOBFilePos += (ULong)sWrittenLen;
                IDE_TEST_RAISE(sWrittenLen < sStrLen, LOBFileWriteError);
            }
            /* 서버로부터 LOB 데이터를 얻는 과정에서 오류 발생했는지 검사. */
            IDE_TEST_RAISE(mLOB->IsFetchError() == ILO_TRUE, FetchError);
        }

        /* 데이터 파일에 LOB indicator 출력. */
        sToWriteLen = (UInt)idlOS::snprintf(sLOBIndicator,
                                         ID_SIZEOF(sLOBIndicator),
                                         "%s%"ID_UINT64_FMT":%"ID_UINT32_FMT,
                                         mLOBIndicatorOpt,
                                         mAccumLOBFilePos - (ULong)sLOBLength,
                                         sLOBLength);
        IDE_TEST_RAISE( idlOS::fwrite(sLOBIndicator, sToWriteLen, 1, aWriteFp)
                        != (UInt)1, DataFileWriteError );
    }
    /* LOB이 NULL인 경우 */
    else /* (sLOBLength == 0) */
    {
        // BUG-31004
        IDE_TEST_RAISE( sHandle->mSQLApi->FreeLOB(mLOB->GetLOBLoc()) != IDE_SUCCESS,
                        FreeLOBError);

        /* 데이터 파일에 LOB indicator 출력. */
        sWrittenLen = (UInt)idlOS::fprintf(aWriteFp, "%snull",
                                           mLOBIndicatorOpt);
        IDE_TEST_RAISE(sWrittenLen < mLOBIndicatorOptLen + 4,
                       DataFileWriteError);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(OpenOrCloseLOBFileError);
    {
        (void)mLOB->CloseForDown(sHandle);
    }
    IDE_EXCEPTION(FetchError);
    {
        (void)mLOB->CloseForDown(sHandle);
    }
    IDE_EXCEPTION(LOBFileWriteError);
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_LOB_File_IO_Error);
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
            (void)idlOS::printf("LOB data write fail\n");
        }
    
        (void)mLOB->CloseForDown(sHandle);
    }
    IDE_EXCEPTION(DataFileWriteError);
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Data_File_IO_Error);
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
            (void)idlOS::printf("LOB indicator write fail\n");
        }
    }
    IDE_EXCEPTION(FreeLOBError);
    {
        /* BUG-42849 Error code and message should not be output to stdout.
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }*/
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * PrintOneLOBToSepLOBFile.
 *
 * use_lob_file=yes, use_separate_files=yes인 경우 호출되며,
 * 한 개의 LOB 컬럼 값을 독립된 LOB 파일에 출력한다.
 * 또, 데이터 파일에는 LOB indicator를 출력한다.
 * 인자 aCols에 저장되어있는 LOB locator와
 * LOB wrapper 객체(mLOB)를 사용하여
 * 서버로부터 LOB 데이터를 얻어와 LOB 파일에 쓴다.
 *
 * @param[in] aRowNo
 *  출력할 LOB 컬럼이 소속된 행의 번호.
 * @param[in] aCols
 *  컬럼 정보(LOB locator 포함)들이 들어있는 자료구조.
 * @param[in] aColIdx
 *  aCols에서 몇 번째 컬럼이 출력할 컬럼인지를 지정.
 */
IDE_RC iloDataFile::PrintOneLOBColToSepLOBFile( ALTIBASE_ILOADER_HANDLE  aHandle, 
                                                SInt                     aRowNo, 
                                                iloColumns              *aCols,
                                                SInt                     aColIdx, 
                                                FILE                    *aWriteFp,             //PROJ-1714
                                                iloTableInfo            *pTableInfo)   //BUG-24583
{
    SQLSMALLINT  sColDataType;
    SQLSMALLINT  sLOBLocCType;
    SQLSMALLINT  sValCType;
    UInt         sStrLen;
    void        *sBuf;
    SChar        sTmpFilePath[MAX_FILEPATH_LEN];
    SChar        sTableName[MAX_OBJNAME_LEN];
    SChar        sColName[MAX_OBJNAME_LEN];
   
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    if (*aCols->m_Len[aColIdx] == SQL_NULL_DATA)
    {
       /* BUG-24584
        * NULL 일 경우는 아무 처리도 하지 않음..
        */
        return IDE_SUCCESS;
    }

    sColDataType = aCols->GetType(aColIdx);
    if (sColDataType == SQL_CLOB || sColDataType == SQL_CLOB_LOCATOR)
    {
        sLOBLocCType = SQL_C_CLOB_LOCATOR;
        sValCType = SQL_C_CHAR;
    }
    else /* SQL_BINARY || SQL_BLOB || SQL_BLOB_LOCATOR */
    {
        sLOBLocCType = SQL_C_BLOB_LOCATOR;
        sValCType = SQL_C_BINARY;
    }
    IDE_TEST(mLOB->OpenForDown( sHandle,
                                sLOBLocCType,
                                *(SQLUBIGINT *)aCols->m_Value[aColIdx],
                                sValCType, 
                                iloLOB::LOBAccessMode_RDONLY)
             != IDE_SUCCESS);

    if (mLOB->GetLOBLength() > 0)
    {
        /* BUG-24583
         * -lob 'use_separate_files=yes'옵션을 사용할 경우에는 
         * 해당 폴더내의 파일에 데이터를 저장한다. 
         */
        (void)sprintf(sTmpFilePath, "%s/%s",
                      pTableInfo->GetTransTableName(sTableName,(UInt)MAX_OBJNAME_LEN),
                      aCols->GetTransName(aColIdx,sColName,(UInt)MAX_OBJNAME_LEN));
        IDE_TEST_RAISE(OpenLOBFile( sHandle,
                                    ILO_TRUE,
                                    aRowNo,
                                    aColIdx + 1,
                                    ILO_TRUE, 
                                    sTmpFilePath)
                       != IDE_SUCCESS, OpenOrCloseLOBFileError);

        /* 서버로부터 LOB 데이터를 읽음. */
        while (mLOB->Fetch( sHandle, &sBuf, &sStrLen) == IDE_SUCCESS)
        {
            /* LOB 데이터를 LOB 파일에 씀. */
            IDE_TEST_RAISE(idlOS::fwrite(sBuf, 1, sStrLen, mLOBFP) < sStrLen,
                        LOBFileWriteError);
        }
        /* 서버로부터 LOB 데이터를 얻는 과정에서 오류 발생했는지 검사. */
        IDE_TEST_RAISE(mLOB->IsFetchError() == ILO_TRUE, FetchError);

        /* LOB 파일을 닫는다. */
        IDE_TEST_RAISE(CloseLOBFile(sHandle) != IDE_SUCCESS, OpenOrCloseLOBFileError);

        /* 데이터 파일에 LOB indicator 출력. */
        (UInt)idlOS::fprintf(aWriteFp, "%s/%09"ID_UINT32_FMT".lob",       //BUG-24583
                             sTmpFilePath, aRowNo);
    }
    else /* (mLOB->GetLOBLength() == 0) */
    {
        // BUG-31004
        IDE_TEST_RAISE( sHandle->mSQLApi->FreeLOB(mLOB->GetLOBLoc()) != IDE_SUCCESS,
                        FreeLOBError);

        /* BUG-24583 
         * LOB Data가 없을 경우, %%NULL을 파일에 표시하지 않는다.
         * 아무 처리도 하지 않는다. 
         */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(OpenOrCloseLOBFileError);
    {
        (void)mLOB->CloseForDown(sHandle);
    }
    IDE_EXCEPTION(FetchError);
    {
        (void)CloseLOBFile(sHandle);
        (void)mLOB->CloseForDown(sHandle);
    }
    IDE_EXCEPTION(LOBFileWriteError);
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_LOB_File_IO_Error);
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr); 
            (void)idlOS::printf("LOB data write fail\n");
        }
        
        (void)CloseLOBFile(sHandle);
        (void)mLOB->CloseForDown(sHandle);
    }
    IDE_EXCEPTION(FreeLOBError);
    {
        /* BUG-42849 Error code and message should not be output to stdout.
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }*/
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * AnalLOBIndicator.
 *
 * 데이터 파일로부터 읽은 LOB indicator를 분석하여,
 * LOB indicator 구분자로 시작하는지 여부를 검사하고,
 * LOB 파일내에서 LOB 데이터의 시작 위치와 길이를 얻는다.
 * 분석할 LOB indicator는 m_TokenValue에 들어있다.
 *
 * @param[out] aLOBPhyOffs
 *  LOB 파일내에서 LOB 데이터의 시작 위치.
 * @param[out] aLOBPhyLen
 *  LOB 파일내에서 LOB 데이터의 길이(물리적 길이).
 */
IDE_RC iloDataFile::AnalLOBIndicator(ULong *aLOBPhyOffs, ULong *aLOBPhyLen)
{

    /* BUG-24583
     * use_separate_files=yes로 설정한 경우에는 LobIndicator를 사용하지 않고, 
     * FilePath+FileName 로 구성된다.
     */
    if (mUseSeparateFilesOpt == ILO_TRUE)
    {
        if ( idlOS::strcmp(m_TokenValue, "") != 0 )
        {
            // FilePath + FileName 추출.. 
            (void)idlOS::strcpy( mLOBFile[mLOBFileColumnNum], m_TokenValue);
            (*aLOBPhyLen) = ID_ULONG(1);        
            mLOBFileColumnNum++;        //다음 Filename을  저장하기 위해서...
        }
        else
        {
            //LOB Data가 없을 경우 (Datafile에 LOB File 경로가 없을 경우)
            (*aLOBPhyLen) = ID_ULONG(0);
        }
    }
    else /* (mUseSeparateFilesOpt == ILO_FALSE) */
    {
        /* LOB indicator 구분자로 시작하는지 검사 */
        IDE_TEST(idlOS::strncmp(m_TokenValue, mLOBIndicatorOpt,
                 mLOBIndicatorOptLen) != 0);    
    
        /* "%%null"인 경우 */
        if (idlOS::strcmp(&m_TokenValue[mLOBIndicatorOptLen], "null") == 0)
        {
            (*aLOBPhyLen) = ID_ULONG(0);
        }
        else if (mUseSeparateFilesOpt == ILO_FALSE)
        {
            /* "%%숫자:숫자"로부터 시작 위치와 길이를 얻기 시도 */
            IDE_TEST (sscanf(&m_TokenValue[mLOBIndicatorOptLen],
                      "%19"ID_UINT64_FMT":%19"ID_UINT64_FMT,
                      aLOBPhyOffs, aLOBPhyLen) < 2)
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    idlOS::printf("LOB indicator \"%s\" not found in token \"%s\"\n",
                  mLOBIndicatorOpt, m_TokenValue);

    return IDE_FAILURE;
}

/**
 * LoadOneRecordLOBCols.
 *
 * 한 레코드의 LOB 컬럼들에 대해 데이터를 파일로부터 읽어 서버로 전송한다.
 *
 * @param[in] aRowNo
 *  레코드의 행 번호.
 * @param[in] aTableInfo
 *  컬럼들의 정보가 들어있는 자료구조.
 *  컬럼 정보에는 파일내에서 LOB 데이터의 시작 위치와 길이,
 *  LOB locator가 포함된다.
 * @param[in] aArrIdx
 *  서버로의 데이터 전송에 array binding을 사용할 경우
 *  aTableInfo의 컬럼 값 배열에서 몇 번째 값이 사용할 값인지의 정보.
 */
IDE_RC iloDataFile::LoadOneRecordLOBCols( ALTIBASE_ILOADER_HANDLE  aHandle,
                                          SInt                     aRowNo, 
                                          iloTableInfo            *aTableInfo,
                                          SInt                     aArrIdx, 
                                          iloSQLApi               *aISPApi )
{
    iloBool sErrExist = ILO_FALSE;
    IDE_RC sRC;
    SInt   sColIdx;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    for (sColIdx = 0; sColIdx < aTableInfo->GetAttrCount(); sColIdx++)
    {
        /* 바인드되지 않은 컬럼 skip */
        if (aTableInfo->seqEqualChk(sHandle, sColIdx) >= 0 ||
            aTableInfo->mSkipFlag[sColIdx] == ILO_TRUE)
        {
            continue;
        }
        /* 바인드되지 않은 컬럼 skip */
        if (aTableInfo->GetAttrType(sColIdx) == ISP_ATTR_TIMESTAMP &&
            sHandle->mParser.mAddFlag == ILO_TRUE)
        {
            continue;
        }
        if (aTableInfo->GetAttrType(sColIdx) == ISP_ATTR_CLOB ||
            aTableInfo->GetAttrType(sColIdx) == ISP_ATTR_BLOB)
        {
            sRC = LoadOneLOBCol( sHandle, aRowNo,
                                 aTableInfo, sColIdx, aArrIdx, aISPApi);
            if (sRC != IDE_SUCCESS)
            {
                sErrExist = ILO_TRUE;
                /* 아래와 같은 에러의 경우
                 * 더 이상 LOB 컬럼을 읽어들이지 않고 즉시 함수 종료. */
                IDE_TEST(uteGetErrorCODE( sHandle->mErrorMgr) == 0x3b032 ||
                         uteGetErrorCODE( sHandle->mErrorMgr) == 0x5003b ||
                             /* buffer full */
                         uteGetErrorCODE( sHandle->mErrorMgr) == 0x51043 ||
                             /* 통신 장애 */
                         uteGetErrorCODE( sHandle->mErrorMgr) == 0x91044 ||
                             /* Data file IO error */
                         uteGetErrorCODE( sHandle->mErrorMgr) == 0x91045);
                             /* LOB file IO error */
            }
        }
    }

    IDE_TEST(sErrExist == ILO_TRUE);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * LoadOneLOBCol.
 *
 * 한 LOB 컬럼에 대해 데이터를 파일로부터 읽어 서버로 전송한다.
 *
 * @param[in] aRowNo
 *  레코드의 행 번호.
 * @param[in] aTableInfo
 *  컬럼들의 정보가 들어있는 자료구조.
 *  컬럼 정보에는 파일내에서 LOB 데이터의 시작 위치와 길이,
 *  LOB locator가 포함된다.
 * @param[in] aColIdx
 *  aTableInfo내에서 몇 번째 컬럼인지의 정보.
 * @param[in] aArrIdx
 *  서버로의 데이터 전송에 array binding을 사용할 경우
 *  aTableInfo의 컬럼 값 배열에서 몇 번째 값이 사용할 값인지의 정보.
 */
IDE_RC iloDataFile::LoadOneLOBCol( ALTIBASE_ILOADER_HANDLE  aHandle,
                                   SInt                     aRowNo,
                                   iloTableInfo            *aTableInfo,
                                   SInt                     aColIdx, 
                                   SInt                     aArrIdx, 
                                   iloSQLApi               *aISPApi)
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    if (mUseLOBFileOpt == ILO_TRUE)
    {
        if (mUseSeparateFilesOpt == ILO_TRUE)
        {
            IDE_TEST(LoadOneLOBColFromSepLOBFile( sHandle,
                                                  aRowNo,
                                                  aTableInfo,
                                                  aColIdx,
                                                  aArrIdx,
                                                  aISPApi) != IDE_SUCCESS);
        }
        else
        {
            IDE_TEST(LoadOneLOBColFromNonSepLOBFile( sHandle, 
                                                     aTableInfo,
                                                     aColIdx,
                                                     aArrIdx, 
                                                     aISPApi) != IDE_SUCCESS);
        }
    }
    else
    {
        IDE_TEST(LoadOneLOBColFromDataFile( sHandle, 
                                            aTableInfo, 
                                            aColIdx,
                                            aArrIdx,
                                            aISPApi) != IDE_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * LoadOneLOBColFromDataFile.
 *
 * use_lob_file=no인 경우 호출되며,
 * 한 LOB 컬럼에 대해 데이터를 데이터 파일로부터 읽어 서버로 전송한다.
 *
 * @param[in] aTableInfo
 *  컬럼들의 정보가 들어있는 자료구조.
 *  컬럼 정보에는 데이터 파일내에서 LOB 데이터의 시작 위치와 길이,
 *  LOB locator가 포함된다.
 * @param[in] aColIdx
 *  aTableInfo내에서 몇 번째 컬럼인지의 정보.
 * @param[in] aArrIdx
 *  서버로의 데이터 전송에 array binding을 사용할 경우
 *  aTableInfo의 컬럼 값 배열에서 몇 번째 값이 사용할 값인지의 정보.
 */
IDE_RC iloDataFile::LoadOneLOBColFromDataFile( ALTIBASE_ILOADER_HANDLE  aHandle,
                                               iloTableInfo             *aTableInfo,
                                               SInt                      aColIdx, 
                                               SInt                      aArrIdx, 
                                               iloSQLApi                *aISPApi )
{
    EispAttrType  sColDataType;
    SQLSMALLINT   sLOBLocCType;
    UInt          sBufLen;
    UInt          sReadLen;
    UInt          sStrLen;
    ULong         sAccumStrLen;
    void         *sBuf;
    void         *sAttrValPtr;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    IDE_ASSERT((mLOB != NULL) && (aTableInfo != NULL) && (aISPApi != NULL));

    sColDataType = aTableInfo->GetAttrType(aColIdx);
    if (sColDataType == ISP_ATTR_CLOB)
    {
        sLOBLocCType = SQL_C_CLOB_LOCATOR;
        /* BUG-21064 : CLOB type CSV up/download error */
        mLOB->mIsBeginCLOBAppend = ILO_TRUE;
    }
    else /* (sColDataType == ISP_ATTR_BLOB) */
    {
        sLOBLocCType = SQL_C_BLOB_LOCATOR;
    }

    sAttrValPtr = aTableInfo->GetAttrVal(aColIdx, aArrIdx);
    IDE_ASSERT(sAttrValPtr != NULL);

    IDE_TEST(mLOB->OpenForUp( sHandle,
                              sLOBLocCType,
                              *(SQLUBIGINT *)sAttrValPtr,
                              SQL_C_CHAR,
                              iloLOB::LOBAccessMode_WRONLY, aISPApi)
                              != IDE_SUCCESS);

    /* LOB 데이터의 길이가 0보다 큰 경우 */
    if (aTableInfo->mLOBPhyLen[aColIdx][aArrIdx] != ID_ULONG(0))
    {
        BackupDataFilePos();
        /* 데이터 파일내에서 LOB 데이터의 시작 위치로 이동. */
        IDE_TEST_RAISE(DataFileSeek( sHandle,
                                     aTableInfo->mLOBPhyOffs[aColIdx][aArrIdx])
                                     != IDE_SUCCESS, SeekError);

        (void)mLOB->GetBuffer(&sBuf, &sBufLen);

        for (sAccumStrLen = ID_ULONG(0);
             sAccumStrLen < aTableInfo->mLOBLen[aColIdx][aArrIdx];
             sAccumStrLen += (ULong)sStrLen)
        {
            if (aTableInfo->mLOBLen[aColIdx][aArrIdx] - sAccumStrLen
                > (ULong)sBufLen)
            {
                sStrLen = sBufLen;
            }
            else
            {
                sStrLen = (UInt)(aTableInfo->mLOBLen[aColIdx][aArrIdx]
                                 - sAccumStrLen);
            }
            /* 데이터 파일로부터 데이터 읽음. */
            // BUG-24873 clob 데이터 로딩할때 [ERR-91044 : Error occurred during data file IO.] 발생
            // 바이너리 모드로 읽으므로 윈도우즈를 따로 구별할 필요가 없다.
            sReadLen = (UInt)idlOS::fread(sBuf, 1, sStrLen, m_DataFp);
            mDataFilePos += sReadLen;

            IDE_TEST_RAISE(sReadLen < sStrLen, ReadError);
            /* 서버로 데이터 전송. */
            IDE_TEST_RAISE(mLOB->Append( sHandle, sStrLen, aISPApi)
                                         != IDE_SUCCESS, AppendError);
        }

        /* 데이터 파일내에서의 위치를 LOB 데이터 읽기 수행 전 위치로 복귀. */
        IDE_TEST_RAISE(RestoreDataFilePos(sHandle) != IDE_SUCCESS, SeekError);
    }

    /* BUG-21064 : CLOB type CSV up/download error */
    mLOB->mIsBeginCLOBAppend = ILO_FALSE;
    mLOB->mSaveBeginCLOBEnc  = ILO_FALSE;
    mLOB->mIsCSVCLOBAppend   = ILO_FALSE;

    return IDE_SUCCESS;

    IDE_EXCEPTION(SeekError);
    {
        (void)mLOB->CloseForUp( sHandle, aISPApi );
    }
    IDE_EXCEPTION(ReadError);
    {
        uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_Data_File_IO_Error);
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
            (void)idlOS::printf("LOB data read fail\n");
        }
        
        (void)RestoreDataFilePos(sHandle);
        (void)mLOB->CloseForUp( sHandle, aISPApi );
    }
    IDE_EXCEPTION(AppendError);
    {
        (void)RestoreDataFilePos(sHandle);
        (void)mLOB->CloseForUp( sHandle, aISPApi );
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * LoadOneLOBColFromNonSepLOBFile.
 *
 * use_lob_file=yes, use_separate_files=no인 경우 호출되며,
 * 한 LOB 컬럼에 대해 데이터를 LOB 파일로부터 읽어 서버로 전송한다.
 * LOB 데이터가 여러 LOB 파일에 걸쳐있을 경우
 * 자동적으로 해당 LOB 파일들을 열어 데이터를 읽어들인다.
 *
 * @param[in] aTableInfo
 *  컬럼들의 정보가 들어있는 자료구조.
 *  컬럼 정보에는 LOB 파일내에서 LOB 데이터의 시작 위치와 길이,
 *  LOB locator가 포함된다.
 * @param[in] aColIdx
 *  aTableInfo내에서 몇 번째 컬럼인지의 정보.
 * @param[in] aArrIdx
 *  서버로의 데이터 전송에 array binding을 사용할 경우
 *  aTableInfo의 컬럼 값 배열에서 몇 번째 값이 사용할 값인지의 정보.
 */
IDE_RC iloDataFile::LoadOneLOBColFromNonSepLOBFile( ALTIBASE_ILOADER_HANDLE  aHandle,
                                                    iloTableInfo            *aTableInfo,
                                                    SInt                      aColIdx, 
                                                    SInt                      aArrIdx,
                                                    iloSQLApi                *aISPApi )
{
    EispAttrType  sColDataType;
    SQLSMALLINT   sLOBLocCType;
    SQLSMALLINT   sValCType;
    UInt          sBufLen;
    UInt          sReadLen;
    UInt          sStrLen;
    ULong         sAccumStrLen;
    ULong         sAccumStrLenTo;
    void         *sBuf;
    void         *sAttrValPtr;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    IDE_ASSERT((mLOB != NULL) && (aTableInfo != NULL) && (aISPApi != NULL));

    sColDataType = aTableInfo->GetAttrType(aColIdx);
    if (sColDataType == ISP_ATTR_CLOB)
    {
        sLOBLocCType = SQL_C_CLOB_LOCATOR;
        sValCType = SQL_C_CHAR;
    }
    else /* (sColDataType == ISP_ATTR_BLOB) */
    {
        sLOBLocCType = SQL_C_BLOB_LOCATOR;
        sValCType = SQL_C_BINARY;
    }

    sAttrValPtr = aTableInfo->GetAttrVal(aColIdx, aArrIdx);
    IDE_ASSERT(sAttrValPtr != NULL);

    IDE_TEST(mLOB->OpenForUp( sHandle,
                              sLOBLocCType,
                              *(SQLUBIGINT *)sAttrValPtr,
                              sValCType, 
                              iloLOB::LOBAccessMode_WRONLY, 
                              aISPApi)
                              != IDE_SUCCESS);

    /* LOB 데이터의 길이가 0보다 큰 경우 */
    if (aTableInfo->mLOBPhyLen[aColIdx][aArrIdx] != ID_ULONG(0))
    {
        /* LOB 파일내에서 LOB 데이터의 시작 위치로 이동. */
        IDE_TEST_RAISE(LOBFileSeek( sHandle, aTableInfo->mLOBPhyOffs[aColIdx][aArrIdx])
                       != IDE_SUCCESS, SeekError);
        (void)mLOB->GetBuffer(&sBuf, &sBufLen);

        sAccumStrLen = ID_ULONG(0);
        /* 현재 열려있는 LOB 파일에서
         * 필요한 모든 LOB 데이터를 읽을 수 없는 동안 계속 루프 */
        while (mLOBFilePos + aTableInfo->mLOBPhyLen[aColIdx][aArrIdx]
               - sAccumStrLen > mLOBFileSizeOpt)
        {
            sAccumStrLenTo = sAccumStrLen + mLOBFileSizeOpt - mLOBFilePos;
            for (; sAccumStrLen < sAccumStrLenTo;
                 sAccumStrLen += (ULong)sStrLen)
            {
                if (sAccumStrLenTo - sAccumStrLen > (ULong)sBufLen)
                {
                    sStrLen = sBufLen;
                }
                else
                {
                    sStrLen = (UInt)(sAccumStrLenTo - sAccumStrLen);
                }
                /* LOB 파일로부터 데이터 읽음. */
                sReadLen = (UInt)idlOS::fread(sBuf, 1, sStrLen, mLOBFP);
                mLOBFilePos += (ULong)sReadLen;
                mAccumLOBFilePos += (ULong)sReadLen;
                IDE_TEST_RAISE(sReadLen < sStrLen, ReadError);
                /* 서버로 데이터 전송. */
                IDE_TEST_RAISE(mLOB->Append( sHandle, sStrLen, aISPApi ) 
                                             != IDE_SUCCESS, AppendError);
            }

            IDE_TEST_RAISE(CloseLOBFile(sHandle) != IDE_SUCCESS, SeekError);
            /* 다음 번호의 LOB 파일을 연다. */
            IDE_TEST_RAISE(OpenLOBFile( sHandle, ILO_FALSE, mLOBFileNo + 1, 0, ILO_TRUE)
                           != IDE_SUCCESS, SeekError);
        }

        sAccumStrLenTo = aTableInfo->mLOBPhyLen[aColIdx][aArrIdx];
        for (; sAccumStrLen < sAccumStrLenTo; sAccumStrLen += (ULong)sStrLen)
        {
            if (sAccumStrLenTo - sAccumStrLen > (ULong)sBufLen)
            {
                sStrLen = sBufLen;
            }
            else
            {
                sStrLen = (UInt)(sAccumStrLenTo - sAccumStrLen);
            }
            /* LOB 파일로부터 데이터 읽음. */
            sReadLen = (UInt)idlOS::fread(sBuf, 1, sStrLen, mLOBFP);
            mLOBFilePos += (ULong)sReadLen;
            mAccumLOBFilePos += (ULong)sReadLen;
            IDE_TEST_RAISE(sReadLen < sStrLen, ReadError);
            /* 서버로 데이터 전송. */
            IDE_TEST_RAISE(mLOB->Append( sHandle, sStrLen, aISPApi )
                                        != IDE_SUCCESS, AppendError);
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(SeekError);
    {
        (void)mLOB->CloseForUp( sHandle, aISPApi );
    }
    IDE_EXCEPTION(ReadError);
    {
        uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_LOB_File_IO_Error);
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout,  sHandle->mErrorMgr);
            (void)idlOS::printf("LOB data read fail\n");
        }
    
        (void)mLOB->CloseForUp( sHandle, aISPApi );
    }
    IDE_EXCEPTION(AppendError);
    {
        (void)mLOB->CloseForUp( sHandle, aISPApi );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * LoadOneLOBColFromSepLOBFile.
 *
 * use_lob_file=yes, use_separate_files=yes인 경우 호출되며,
 * 한 LOB 컬럼에 대해
 * 데이터를 독립된 LOB 파일로부터 읽어 서버로 전송한다.
 *
 * @param[in] aRowNo
 *  레코드의 행 번호.
 * @param[in] aTableInfo
 *  컬럼들의 정보가 들어있는 자료구조.
 *  컬럼 정보에는 null인 LOB인지 여부, LOB locator가 포함된다.
 * @param[in] aColIdx
 *  aTableInfo내에서 몇 번째 컬럼인지의 정보.
 * @param[in] aArrIdx
 *  서버로의 데이터 전송에 array binding을 사용할 경우
 *  aTableInfo의 컬럼 값 배열에서 몇 번째 값이 사용할 값인지의 정보.
 */
IDE_RC iloDataFile::LoadOneLOBColFromSepLOBFile( ALTIBASE_ILOADER_HANDLE  aHandle,
                                                 SInt                     aRowNo,
                                                 iloTableInfo            *aTableInfo,
                                                 SInt                     aColIdx, 
                                                 SInt                     aArrIdx, 
                                                 iloSQLApi               *aISPApi )
{
    EispAttrType  sColDataType;
    SQLSMALLINT   sLOBLocCType;
    SQLSMALLINT   sValCType;
    UInt          sBufLen;
    UInt          sStrLen;
    void         *sBuf;
    void         *sAttrValPtr;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    IDE_ASSERT((mLOB != NULL) && (aTableInfo != NULL) && (aISPApi != NULL));

    sColDataType = aTableInfo->GetAttrType(aColIdx);
    if (sColDataType == ISP_ATTR_CLOB)
    {
        sLOBLocCType = SQL_C_CLOB_LOCATOR;
        sValCType = SQL_C_CHAR;
    }
    else /* (sColDataType == ISP_ATTR_BLOB) */
    {
        sLOBLocCType = SQL_C_BLOB_LOCATOR;
        sValCType = SQL_C_BINARY;
    }

    sAttrValPtr = aTableInfo->GetAttrVal(aColIdx, aArrIdx);
    IDE_ASSERT(sAttrValPtr != NULL);

    IDE_TEST(mLOB->OpenForUp( sHandle,
                              sLOBLocCType,
                              *(SQLUBIGINT *)sAttrValPtr,
                              sValCType, 
                              iloLOB::LOBAccessMode_WRONLY, 
                              aISPApi)
                              != IDE_SUCCESS);
    
    if (aTableInfo->mLOBPhyLen[aColIdx][aArrIdx] != ID_ULONG(0))
    {
        /* 행 번호와 열 번호를 가지고 독립된 LOB 파일을 연다. */
        IDE_TEST_RAISE(OpenLOBFile(sHandle, ILO_FALSE, aRowNo, aColIdx + 1, ILO_TRUE)
                       != IDE_SUCCESS, OpenOrCloseLOBFileError);
        (void)mLOB->GetBuffer(&sBuf, &sBufLen);

        /* LOB 파일로부터 데이터 읽음. */
        sStrLen = (UInt)idlOS::fread(sBuf, 1, sBufLen, mLOBFP);
        while (sStrLen == sBufLen)
        {
            /* 서버로 데이터 전송. */
            IDE_TEST_RAISE(mLOB->Append( sHandle, sStrLen, aISPApi)
                                       != IDE_SUCCESS, AppendError);
            /* LOB 파일로부터 데이터 읽음. */
            sStrLen = (UInt)idlOS::fread(sBuf, 1, sBufLen, mLOBFP);
        }
        if (sStrLen > 0)
        {
            /* 서버로 데이터 전송. */
            IDE_TEST_RAISE(mLOB->Append( sHandle, sStrLen, aISPApi )
                                       != IDE_SUCCESS, AppendError);
        }
        /* LOB 파일 읽기 과정에서 I/O 에러 발생 여부 검사. */
        IDE_TEST_RAISE(ferror(mLOBFP), ReadError);

        /* LOB 파일을 닫는다. */
        IDE_TEST_RAISE(CloseLOBFile(sHandle) != IDE_SUCCESS, OpenOrCloseLOBFileError);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(OpenOrCloseLOBFileError);
    {
        (void)mLOB->CloseForUp( sHandle, aISPApi );
    }
    IDE_EXCEPTION(ReadError);
    {
        uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_LOB_File_IO_Error);
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout,  sHandle->mErrorMgr);
            (void)idlOS::printf("LOB data read fail\n");
        }
        
        (void)CloseLOBFile(sHandle);
        (void)mLOB->CloseForUp( sHandle, aISPApi );
    }
    IDE_EXCEPTION(AppendError);
    {
        (void)CloseLOBFile(sHandle);
        (void)mLOB->CloseForUp( sHandle, aISPApi );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * DataFileSeek.
 *
 * 데이터 파일내에서의 현재 위치를 인자 aDestPos 위치로 옮긴다.
 *
 * @param[in] aDestPos
 *  데이터 파일내에서 이동하고자 하는 새로운 위치.
 */
IDE_RC iloDataFile::DataFileSeek( ALTIBASE_ILOADER_HANDLE aHandle,
                                  ULong                   aDestPos )
{
    ULong   sDiff;
#if defined(VC_WIN32) || defined(VC_WIN64)
    __int64 sOffs;
#else
    ULong   sI;
    ULong   sTo;
    long    sOffs;
#endif

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    if (aDestPos > mDataFilePos)
    {
        sDiff = aDestPos - mDataFilePos;
    }
    else
    {
        sDiff = mDataFilePos - aDestPos;
    }

#if defined(VC_WIN32) || defined(VC_WIN64)
    sOffs = (aDestPos > mDataFilePos)? (__int64)sDiff : -(__int64)sDiff;
    IDE_TEST(::_fseeki64(m_DataFp, sOffs, SEEK_CUR) < 0);
#else
    if (ID_SIZEOF(long) >= 8 || sDiff <= ID_ULONG(0x7FFFFFFF))
    {
        sOffs = (aDestPos > mDataFilePos)? (long)sDiff : -(long)sDiff;
        IDE_TEST(idlOS::fseek(m_DataFp, sOffs, SEEK_CUR) < 0);
    }
    /* long이 4바이트이고 이동량이 long의 범위를 벗어날 경우,
     * 여러 번의 fseek()가 필요하다.
     *
     * BUGBUG :
     * 대부분의 경우 fseek()에서
     * 파일의 위치가 LONG_MAX를 넘어서게 될 것 같으면,
     * fseek()는 이러한 이동을 허용하지 않은 채 에러를 리턴할 것이다. */
    else
    {
        sTo = sDiff / ID_ULONG(0x7FFFFFFF);
        sOffs = (aDestPos > mDataFilePos)? 0x7FFFFFFF : -0x7FFFFFFF;
        for (sI = ID_ULONG(0); sI < sTo; sI++)
        {
            IDE_TEST(idlOS::fseek(m_DataFp, sOffs, SEEK_CUR) < 0);
        }
        if (aDestPos > mDataFilePos)
        {
            sOffs = (long)(sDiff % ID_ULONG(0x7FFFFFFF));
        }
        else
        {
            sOffs = -(long)(sDiff % ID_ULONG(0x7FFFFFFF));
        }
        if (sOffs != 0)
        {
            IDE_TEST(idlOS::fseek(m_DataFp, sOffs, SEEK_CUR) < 0);
        }
    }
#endif /* defined(VC_WIN32) || defined(VC_WIN64) */

    mDataFilePos = aDestPos;  //PROJ-1714

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_Data_File_IO_Error);
    
    if ( sHandle->mUseApi != SQL_TRUE )
    {
        utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        (void)idlOS::printf("Data file seek fail\n");
    }

    return IDE_FAILURE;
}

/**
 * GetLOBFileSize.
 *
 * 현재 열려있는 LOB 파일의 크기를 구한다.
 *
 * @param[out] aLOBFileSize
 *  LOB 파일 크기.
 */
IDE_RC iloDataFile::GetLOBFileSize( ALTIBASE_ILOADER_HANDLE  aHandle, 
                                    ULong                   *aLOBFileSize)
{
#if defined(VC_WIN32) || defined(VC_WIN64)
    __int64 sOffs;
#else
    SChar  *sBuf;
    UInt    sReadLen;
    ULong   sI;
    ULong   sTo;
    long    sOffs;
#endif

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

/* 1.Windows인 경우 */
#if defined(VC_WIN32) || defined(VC_WIN64)
    /* 1.1 파일 크기를 얻는다. */
    IDE_TEST(::_fseeki64(mLOBFP, 0, SEEK_END) < 0);
    sOffs = ::_ftelli64(mLOBFP);
    IDE_TEST(sOffs < 0);
    /* 1.2 파일 커서를 원위치로 복귀한다. */
    IDE_TEST(::_fseeki64(mLOBFP, (__int64)mLOBFilePos, SEEK_SET) < 0);

    (*aLOBFileSize) = (ULong)sOffs;
    return IDE_SUCCESS;
#else /* defined(VC_WIN32) || defined(VC_WIN64) */
    /* 2.long이 8바이트 이상인 경우 */
    if (ID_SIZEOF(long) >= 8)
    {
        /* 2.1 파일 크기를 얻는다. */
        IDE_TEST(idlOS::fseek(mLOBFP, 0, SEEK_END) < 0);
        sOffs = idlOS::ftell(mLOBFP);
        IDE_TEST(sOffs < 0);
        /* 2.2 파일 커서를 원위치로 복귀한다. */
        IDE_TEST(idlOS::fseek(mLOBFP, (long)mLOBFilePos, SEEK_SET) < 0);

        (*aLOBFileSize) = (ULong)sOffs;
        return IDE_SUCCESS;
    }
    else /* (ID_SIZEOF(long) == 4) */
    {
        /* 3.long이 4바이트이고 파일 크기가 LONG_MAX 이하인 경우 */
        /* 3.1 파일 크기를 얻는다. */
        if (idlOS::fseek(mLOBFP, 0, SEEK_END) == 0)
        {
            sOffs = idlOS::ftell(mLOBFP);
            if (sOffs >= 0)
            {
                /* 3.2 파일 커서를 원위치로 복귀한다. */
                IDE_TEST(idlOS::fseek(mLOBFP, (long)mLOBFilePos, SEEK_SET)
                         < 0);

                (*aLOBFileSize) = (ULong)sOffs;
                return IDE_SUCCESS;
            }
        }

        /* 4.long이 4바이트이고 파일 크기가 LONG_MAX보다 큰 경우
         *
         * BUGBUG :
         * 대부분의 경우 fread()에서
         * 파일의 위치가 LONG_MAX를 넘어서게 될 것 같으면,
         * fread()는 더 이상의 파일 위치 전진을 허용하지 않은 채
         * 에러를 리턴할 것이다. */
        /* 4.1 파일 크기를 얻는다. */
        IDE_TEST(idlOS::fseek(mLOBFP, 0x7FFFFE00, SEEK_SET) < 0);
        (*aLOBFileSize) = ID_ULONG(0x7FFFFE00);
        sBuf = (SChar *)idlOS::malloc(32768);
        IDE_TEST(sBuf == NULL);

        while ((sReadLen = (UInt)idlOS::fread(sBuf, 1, 32768, mLOBFP))
               == 32768)
        {
            (*aLOBFileSize) += ID_ULONG(32768);
        }
        (*aLOBFileSize) += (ULong)sReadLen;

        idlOS::free(sBuf);
        sBuf = NULL;
        IDE_TEST(ferror(mLOBFP));

        /* 4.2 파일 커서를 원위치로 복귀한다. */
        if (mLOBFilePos <= ID_ULONG(0x7FFFFFFF))
        {
            IDE_TEST(idlOS::fseek(mLOBFP, (long)mLOBFilePos, SEEK_SET) < 0);
        }
        else
        {
            IDE_TEST(idlOS::fseek(mLOBFP, 0, SEEK_SET) < 0);
            sTo = mLOBFilePos / ID_ULONG(0x7FFFFFFF);
            for (sI = ID_ULONG(0); sI < sTo; sI++)
            {
                IDE_TEST(idlOS::fseek(mLOBFP, 0x7FFFFFFF, SEEK_CUR) < 0);
            }
            sOffs = (long)(mLOBFilePos % ID_ULONG(0x7FFFFFFF));
            if (sOffs != 0)
            {
                IDE_TEST(idlOS::fseek(m_DataFp, sOffs, SEEK_CUR) < 0);
            }
        }

        return IDE_SUCCESS;
    }
#endif /* defined(VC_WIN32) || defined(VC_WIN64) */

    IDE_EXCEPTION_END;
 
    uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_LOB_File_IO_Error);
    
    if ( sHandle->mUseApi != SQL_TRUE )
    {
        utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        (void)idlOS::printf("LOB file size get fail\n");
    }
    
    return IDE_FAILURE;
}

/**
 * LOBFileSeek.
 *
 * LOB 파일들에서의 현재 위치를 인자 aAccumDestPos 위치로 옮긴다.
 * 인자 aAccumDestPos는 LOB 파일들에서의 누적 위치이다.
 * 누적 위치란 어떤 LOB 파일보다 작은 파일 번호를 가진
 * LOB 파일들의 크기가 누적된 것을 말한다.
 *
 * @param[in] aAccumDestPos
 *  LOB 파일들에서 이동하고자 하는 새로운 위치.
 */
IDE_RC iloDataFile::LOBFileSeek( ALTIBASE_ILOADER_HANDLE aHandle, ULong aAccumDestPos)
{
    UInt    sDestFileNo;
    ULong   sDestPos;
    ULong   sDiff;
#if defined(VC_WIN32) || defined(VC_WIN64)
    __int64 sOffs;
#else
    ULong   sI;
    ULong   sTo;
    long    sOffs;
#endif

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    /* LOB 파일이 존재하지 않는 경우:
     * InitLOBProc()의 lob_file_size 얻는 부분에서는
     * LOB 파일 열기에 실패하더라도
     * mLOBFileSizeOpt를 0으로 둔 채 정상 진행하도록 되어 있다.
     * 이렇게 한 이유는 InitLOBProc()에서는
     * use_separate_files 여부를 알 수 없기 때문이다.
     * 그러나, 본 라인까지 프로그램이 진행된 시점에서는
     * use_separate_files=no임을 알고 있는 상태이다.
     * 따라서, 본 라인에서 mLOBFileSizeOpt가 0인 경우
     * 파일 열기 실패 오류를 출력해야 한다. */
    if (mLOBFileSizeOpt == ID_ULONG(0))
    {
        /* 1번 LOB 파일 열기 시도.
         * 파일 열기는 무조건 실패하게 되어 있다.
         * 파일 열기가 실패할 줄 알고도 파일 열기를 시도하는 이유는
         * 파일 열기를 실패했다는 오류 메시지 출력을 위해서다. */
        (void)OpenLOBFile( sHandle, ILO_FALSE, 1, 0, ILO_TRUE);
        IDE_TEST(1);
    }

    sDestFileNo = (UInt)(aAccumDestPos / mLOBFileSizeOpt) + 1;
    sDestPos = aAccumDestPos % mLOBFileSizeOpt;

    if (sDestFileNo == mLOBFileNo && sDestPos == mLOBFilePos)
    {
        IDE_TEST_RAISE(mLOBFP == NULL, LOBFileIOError);
        mAccumLOBFilePos = aAccumDestPos;
        return IDE_SUCCESS;
    }

    if (sDestFileNo == mLOBFileNo)
    {
        IDE_TEST_RAISE(mLOBFP == NULL, LOBFileIOError);
    }
    /* 새로운 위치가 현재 열려있는 LOB 파일이 아닌
     * 다른 LOB 파일에 위치하는 경우 */
    else
    {
        IDE_TEST(CloseLOBFile(sHandle) != IDE_SUCCESS);
        /* 이동할 목표지점이 위치하는 LOB 파일을 연다. */
        IDE_TEST(OpenLOBFile(sHandle, ILO_FALSE, sDestFileNo, 0, ILO_TRUE)
                 != IDE_SUCCESS);
    }

    if (sDestPos > mLOBFilePos)
    {
        sDiff = sDestPos - mLOBFilePos;
    }
    else
    {
        sDiff = mLOBFilePos - sDestPos;
    }

#if defined(VC_WIN32) || defined(VC_WIN64)
    sOffs = (sDestPos > mLOBFilePos)? (__int64)sDiff : -(__int64)sDiff;
    IDE_TEST_RAISE(::_fseeki64(mLOBFP, sOffs, SEEK_CUR) < 0, LOBFileIOError);
#else
    if (ID_SIZEOF(long) >= 8 || sDiff <= ID_ULONG(0x7FFFFFFF))
    {
        sOffs = (sDestPos > mLOBFilePos)? (long)sDiff : -(long)sDiff;
        IDE_TEST_RAISE(idlOS::fseek(mLOBFP, sOffs, SEEK_CUR) < 0,
                       LOBFileIOError);
    }
    /* long이 4바이트이고 이동량이 long의 범위를 벗어날 경우,
     * 여러 번의 fseek()가 필요하다.
     *
     * BUGBUG :
     * 대부분의 경우 fseek()에서
     * 파일의 위치가 LONG_MAX를 넘어서게 될 것 같으면,
     * fseek()는 이러한 이동을 허용하지 않은 채 에러를 리턴할 것이다. */
    else
    {
        sTo = sDiff / ID_ULONG(0x7FFFFFFF);
        sOffs = (sDestPos > mLOBFilePos)? 0x7FFFFFFF : -0x7FFFFFFF;
        for (sI = ID_ULONG(0); sI < sTo; sI++)
        {
            IDE_TEST_RAISE(idlOS::fseek(mLOBFP, sOffs, SEEK_CUR) < 0,
                           LOBFileIOError);
        }
        if (sDestPos > mLOBFilePos)
        {
            sOffs = (long)(sDiff % ID_ULONG(0x7FFFFFFF));
        }
        else
        {
            sOffs = -(long)(sDiff % ID_ULONG(0x7FFFFFFF));
        }
        if (sOffs != 0)
        {
            IDE_TEST_RAISE(idlOS::fseek(mLOBFP, sOffs, SEEK_CUR) < 0,
                           LOBFileIOError);
        }
    }
#endif /* defined(VC_WIN32) || defined(VC_WIN64) */

    mLOBFilePos = sDestPos;
    mAccumLOBFilePos = aAccumDestPos;

    return IDE_SUCCESS;

    IDE_EXCEPTION(LOBFileIOError);
    {
        uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_LOB_File_IO_Error);
        utePrintfErrorCode(stdout,  sHandle->mErrorMgr);
        (void)idlOS::printf("LOB file seek fail\n");
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


/* PROJ-1714
 * 특정 Size만큼 File에서 읽음
 */
// BUG-18803 readsize 옵션 추가
// getc -> fread 함수로 변경
SInt iloDataFile::ReadFromFile(SInt aSize, SChar* aResult)
{
    UInt    sResultSize = 0;

    sResultSize = idlOS::fread(aResult, 1, aSize, m_DataFp);
    mDataFilePos += sResultSize;
    aResult[sResultSize] = '\0';

    return sResultSize;
}

/* PROJ-1714
 * 입력 받은 데이터를 Size 만큼 원형 버퍼에 삽입한다.
 */

SInt iloDataFile::WriteDataToCBuff( ALTIBASE_ILOADER_HANDLE aHandle,
                                    SChar* pBuf,
                                    SInt nSize)
{
    SInt sWriteResult;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    sWriteResult = mCirBuf.WriteBuf( sHandle, pBuf, nSize );
    
    return sWriteResult;
}

/* BUG-22434
 * 원형버퍼에 데이터를 읽은 후 Double Buffer에 값을 넣는다.
 * 실제 데이터의 Parsing과정은 Double Buffer를 이용하고,
 * 원형 버퍼는 File I/O를 처리하게 된다. 
 */
SInt iloDataFile::ReadDataFromCBuff( ALTIBASE_ILOADER_HANDLE aHandle, SChar* aResult )
{
    SInt  sRet       = 0;
    SInt  sReadSize  = 1;
        
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    //Double Buffer가 비었거나, 다 읽었을 경우.
    if( (mDoubleBuffPos == -1) || (mDoubleBuffPos == mDoubleBuffSize) )
    {
        // BUG-18803 readsize 옵션 추가
        sRet = mCirBuf.ReadBuf( sHandle, 
                                mDoubleBuff,
                                sHandle->mProgOption->mReadSzie);
        
        if ( sRet == -1 ) //EOF 일 경우
        {
            return -1;
        }
        
        mDoubleBuffSize  = sRet;    //Buffer Data Size      
        mDoubleBuffPos   = 0;       //Buffer Position 초기화
    }
    
    idlOS::memcpy(aResult, &mDoubleBuff[mDoubleBuffPos] , sReadSize);
    mDoubleBuffPos++;
    
    return sReadSize;
}

/* BUG-24358 iloader Geometry Data */
IDE_RC iloDataFile::ResizeTokenLen( ALTIBASE_ILOADER_HANDLE aHandle, UInt aLen )
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    if (mTokenMaxSize < aLen)
    {
        if (m_TokenValue != NULL)
        {
            idlOS::free(m_TokenValue);
        }
        m_TokenValue = (SChar*) idlOS::malloc(aLen);
        IDE_TEST_RAISE(m_TokenValue == NULL, MAllocError);
        m_TokenValue[0] = '\0';
        mTokenMaxSize = aLen;

        // BUG-27633: mErrorToken도 컬럼의 최대 크기로 할당
        if (mErrorToken != NULL)
        {
            idlOS::free(mErrorToken);
        }
        mErrorToken = (SChar*) idlOS::malloc(aLen);
        IDE_TEST_RAISE(mErrorToken == NULL, MAllocError);
        mErrorToken[0] = '\0';
        mErrorTokenMaxSize = aLen;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(MAllocError);
    {
        uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_memory_error, __FILE__, __LINE__);
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* BUG-24583 LOB FilePath+FileName 버퍼 할당 */
IDE_RC iloDataFile::LOBFileInfoAlloc( ALTIBASE_ILOADER_HANDLE aHandle, 
                                      UInt                    aRowCount )
{
    SInt sI;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    if ( aRowCount != 0 )
    {
        LOBFileInfoFree();

        mLOBFile = (SChar**)idlOS::calloc(aRowCount, ID_SIZEOF(SChar*));
        IDE_TEST_RAISE(mLOBFile == NULL, MAllocError);
        mLOBFileRowCount = aRowCount;

        for (sI = 0; sI < mLOBFileRowCount; sI++)
        {
            mLOBFile[sI] = (SChar*)idlOS::malloc(MAX_FILEPATH_LEN * ID_SIZEOF(SChar));
            IDE_TEST_RAISE(mLOBFile[sI] == NULL, MAllocError);

            mLOBFile[sI][0] = '\0';
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(MAllocError);
    {
        uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_memory_error, __FILE__, __LINE__);
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
    }
    IDE_EXCEPTION_END;

    if (mLOBFile != NULL)
    {
        LOBFileInfoFree();
    }

    return IDE_FAILURE;
}

void iloDataFile::LOBFileInfoFree()
{
    SInt sI;

    if (mLOBFile != NULL)
    {
        for (sI = 0; sI < mLOBFileRowCount; sI++)
        {
            if (mLOBFile[sI] != NULL)
            {
                idlOS::free(mLOBFile[sI]);
                mLOBFile[sI] = NULL;
            }
        }

        idlOS::free(mLOBFile);
        mLOBFile = NULL;
        mLOBFileRowCount = 0;
    }
}

/**
 * loadFromOutFile.  //PROJ-2030, CT_CASE-3020 CHAR outfile 지원
 *
 * Fmt화일의 CHAR, VARCHAR, NCHAR, NVARCHAR 컬럼에 outfile 옵션이 설정된 경우 호출된다.
 * m_TokenValue에 화일이름이 있는 상태에서, m_TokenValue에 화일의 내용을 채운다.
 */
IDE_RC iloDataFile::loadFromOutFile( ALTIBASE_ILOADER_HANDLE aHandle )
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    if (mOutFileFP != NULL)
    {
        IDE_TEST_RAISE(idlOS::fclose(mOutFileFP) != 0, CloseError);
        mOutFileFP = NULL;
    }

    mOutFileFP = ilo_fopen(m_TokenValue, "rb");
    IDE_TEST_RAISE(mOutFileFP == NULL, OpenError);

    mTokenLen = (UInt)idlOS::fread(m_TokenValue, 1, mTokenMaxSize, mOutFileFP);
    IDE_TEST_RAISE(ferror(mOutFileFP), ReadError);

    IDE_TEST_RAISE(idlOS::fclose(mOutFileFP) != 0, CloseError);
    mOutFileFP = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION(CloseError);
    {
        uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_OutFile_IO_Error);
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
            (void)idlOS::printf("Outfile close fail\n");
        }
    }
    IDE_EXCEPTION(OpenError);
    {
        uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_OutFile_IO_Error);
    
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
            idlOS::printf("Outfile [%s] open fail\n", m_TokenValue);
        }
    }
    IDE_EXCEPTION(ReadError);
    {
        uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_OutFile_IO_Error);
    
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout,  sHandle->mErrorMgr);
            (void)idlOS::printf("Outfile data read fail\n");
        }
        
        idlOS::fclose(mOutFileFP);
        mTokenLen = 0;
        m_TokenValue[0] = '\0';
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

