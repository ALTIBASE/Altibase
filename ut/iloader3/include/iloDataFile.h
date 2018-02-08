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
 * $Id: iloDataFile.h 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

#ifndef _O_ILO_DATAFILE_H
#define _O_ILO_DATAFILE_H

/* 토큰의 최대 길이 = VARBIT 문자열 표현의 최대 길이 */
#define MAX_TOKEN_VALUE_LEN 131070
#define MAX_SEPERATOR_LEN   11
// BUG-21837
/* LOB_PIECE_SIZE : 서버에서 LOB 데이타를 쪼개서 보내는 단위 */
#define ILO_LOB_PIECE_SIZE  (32000)

/**
 * class iloLOB.
 *
 * LOB locator를 사용한 데이터 읽고 쓰기를 편리하게 하기 위한
 * wrapper class이다.
 */
class iloLOB
{
public:
    enum LOBAccessMode
    {
        LOBAccessMode_RDONLY = 0,
        LOBAccessMode_WRONLY = 1
    };

    iloLOB();

    ~iloLOB();
    
    //PROJ-1714 Parallel iLoader
    // Parallel 기법을 적용하기 위해서 Data Upload 시에는, Upload Connection이 여러개가 생성되고,
    // Data Download시에는 Download File이 여러개 생성된다.
    // 따라서, 각 기능에서 사용되는 Open, Close 등의 함수를 구분하였다.
    // iloDataFile의 Member 함수 또한 같은 이유로 함수를 구분하였다.

    IDE_RC OpenForUp( ALTIBASE_ILOADER_HANDLE  aHandle,
                      SQLSMALLINT              aLOBLocCType,
                      SQLUBIGINT               aLOBLoc,
                      SQLSMALLINT              aValCType,
                      LOBAccessMode            aLOBAccessMode, 
                      iloSQLApi               *aISPApi);        //PROJ-1714

    IDE_RC CloseForUp( ALTIBASE_ILOADER_HANDLE aHandle, iloSQLApi *aISPApi );
    
    IDE_RC OpenForDown( ALTIBASE_ILOADER_HANDLE aHandle,
                        SQLSMALLINT             aLOBLocCType, 
                        SQLUBIGINT              aLOBLoc,
                        SQLSMALLINT             aValCType,
                        LOBAccessMode           aLOBAccessMode );

    IDE_RC CloseForDown( ALTIBASE_ILOADER_HANDLE aHandle );

    UInt GetLOBLength() { return mLOBLen; }

    // BUG-31004
    SQLUBIGINT GetLOBLoc() { return mLOBLoc; }

    IDE_RC Fetch( ALTIBASE_ILOADER_HANDLE   aHandle,
                  void                    **aVal,
                  UInt                     *aStrLen);

    iloBool IsFetchError() { return mIsFetchError; }

    IDE_RC GetBuffer(void **aVal, UInt *aBufLen);

    //For Upload
    IDE_RC Append( ALTIBASE_ILOADER_HANDLE  aHandle,
                   UInt                     aStrLen, 
                   iloSQLApi               *aISPApi);

    /* BUG-21064 : CLOB type CSV up/download error */
    // 32000 byte 이상의 CLOB data의 경우 필요한 변수들.
    // data 처음 블럭 부분이냐?
    iloBool mIsBeginCLOBAppend;
    // CLOB data가 CSV 포맷이냐?
    iloBool mIsCSVCLOBAppend;
    // 다음 블럭 처음에 enclosing문자가 저장되어야 하는지 여부.
    iloBool mSaveBeginCLOBEnc;

private:
    /* LOB locator */
    SQLUBIGINT    mLOBLoc;

    /* LOB locator's type: SQL_C_BLOB_LOCATOR or SQL_C_CLOB_LOCATOR */
    SQLSMALLINT   mLOBLocCType;

    /* User buffer's type: SQL_C_BINARY or SQL_C_CHAR */
    SQLSMALLINT   mValCType;

    /* LOB length */
    UInt          mLOBLen;

    /* Current position in LOB */
    UInt          mPos;

    /* Whether error is occurred in SQLGetLOB() */
    iloBool        mIsFetchError;

    /* LOB access mode: LOBAccessMode_RDONLY or LOBAccessMode_WRONLY */
    LOBAccessMode mLOBAccessMode;

    // BUG-21837 LOB 버퍼 사이즈를 조절한다.
    /* User buffer used when user buffer's type is SQL_C_BINARY */
    UChar         mBinBuf[ILO_LOB_PIECE_SIZE];

    /* User buffer used when user buffer's type is SQL_C_CHAR */
    SChar         mChBuf[ILO_LOB_PIECE_SIZE * 2];

    void ConvertBinaryToChar(UInt aBinLen);

    IDE_RC ConvertCharToBinary( ALTIBASE_ILOADER_HANDLE aHandle, UInt aChLen);

    /* BUG-21064 : CLOB type CSV up/download error */
    void iloConvertCharToCSV( ALTIBASE_ILOADER_HANDLE  aHandle,
                              UInt                    *aValueLen );

    IDE_RC iloConvertCSVToChar( ALTIBASE_ILOADER_HANDLE aHandle, UInt *aStrLen );
};

/* TASK-2657 */
/* enum type for iloDataFile::getCSVToken() */
enum eReadState
{
    /*******************************************************
     * stStart   : state of starting up reading a field
     * stCollect : state of in the middle of reading a field
     * stTailSpace : at tailing spaces out of quotes
     * stEndQuote  : at tailing quote 
     * stError     : state of a wrong csv format is read
     * stRowTerm   : ensuring the row term state
     *******************************************************/
    stStart= 0 , stCollect, stTailSpace, stEndQuote, stError, stRowTerm
};

class iloDataFile
{
public:

    iloDataFile( ALTIBASE_ILOADER_HANDLE aHandle );
    
    ~iloDataFile();

    /* BUG-24358 iloader Geometry Data */
    IDE_RC ResizeTokenLen( ALTIBASE_ILOADER_HANDLE aHandle, UInt aLen );
    
    SInt OpenFileForUp( ALTIBASE_ILOADER_HANDLE  aHandle,
                        SChar                   *aDataFileName,
                        SInt                     aDataFileNo, 
                        iloBool                   aIsWr,
                        iloBool                    aLOBColExist );    //Upload 전용 함수
    
    FILE* OpenFileForDown( ALTIBASE_ILOADER_HANDLE  aHandle,
                           SChar                   *aDataFileName, 
                           SInt                     aDataFileNo,
                           iloBool                   aIsWr,
                           iloBool                   aLOBColExist );   //Download전용 함수 

    SInt CloseFileForUp( ALTIBASE_ILOADER_HANDLE aHandle );             //Upload 전용 함수
    
    SInt CloseFileForDown( ALTIBASE_ILOADER_HANDLE aHandle, FILE *fp);     //Download 전용 함수

    void SetTerminator(SChar *szFiledTerm, SChar *szRowTerm);

    void SetEnclosing(SInt bSetEnclosing, SChar *szEnclosing);

    
    SInt PrintOneRecord( ALTIBASE_ILOADER_HANDLE  aHandle,
                         SInt                     aRowNo,
                         iloColumns              *pCols,
                         iloTableInfo            *pTableInfo, 
                         FILE                    *aWriteFp,
                         SInt                     aArrayNum );  //PROJ-1714
    /* TASK-2657 */
    // BUG-28069: log, bad에도 csv 형식으로 기록
    static SInt csvWrite( ALTIBASE_ILOADER_HANDLE  aHandle,
                          SChar                   *aValue,
                          UInt                     aValueLen,
                          FILE                    *aWriteFp );

    EDataToken GetLOBToken( ALTIBASE_ILOADER_HANDLE  aHandle,
                            ULong                   *aLOBPhyOffs,
                            ULong                   *aLOBPhyLen,
                            ULong                   *aLOBLen );

    void rtrim();

    SInt strtonumCheck(SChar *p);

    void SetLOBOptions(iloBool aUseLOBFile, ULong aLOBFileSize,
                       iloBool aUseSeparateFiles, const SChar *aLOBIndicator);

    IDE_RC LoadOneRecordLOBCols( ALTIBASE_ILOADER_HANDLE  aHandle,
                                 SInt                     aRowNo,
                                 iloTableInfo            *aTableInfo,
                                 SInt                     aArrIdx, 
                                 iloSQLApi               *aISPApi);
    
    //PROJ-1714
    SInt        ReadFromFile(SInt aSize, SChar* aResult);
    SInt        ReadOneRecordFromCBuff( ALTIBASE_ILOADER_HANDLE  aHandle,
                                        iloTableInfo            *aTableInfo,
                                        SInt                     aArrayCount );
    SInt        ReadDataFromCBuff( ALTIBASE_ILOADER_HANDLE  aHandle,
                                   SChar                   *aResult );    //BUG-22434 : Double Buffer에서 값을 읽는다.
    /* TASK-2657 */
    EDataToken  GetCSVTokenFromCBuff( ALTIBASE_ILOADER_HANDLE  aHandle, 
                                      SChar                   *aColName);
    
    EDataToken  GetTokenFromCBuff( ALTIBASE_ILOADER_HANDLE aHandle );
    
    SInt        WriteDataToCBuff( ALTIBASE_ILOADER_HANDLE aHandle,
                                  SChar* pBuf,
                                  SInt nSize);

    /* BUG-24583 Lob FilePath & FileName */
    IDE_RC      LOBFileInfoAlloc( ALTIBASE_ILOADER_HANDLE aHandle, UInt aRowCount );
    void        LOBFileInfoFree();

private:
    SChar      mDataFilePath[MAX_FILEPATH_LEN]; // BUG-24902: 데이타 파일의 경로 저장
    FILE      *m_DataFp;
    UInt       mTokenMaxSize;
    /* 현재 데이터 파일 번호 */
    SInt       mDataFileNo;
    /* 데이터 파일 내에서 현재 위치. 데이터 파일 읽기 시 사용. */
    ULong      mDataFilePos;        //원형 버퍼에 넣기 위해 읽은 Byte 수..
    /* mDataFilePos의 임시 백업용 변수 */
    ULong      mDataFileRead;       //PROJ-1714 : 데이터를 처리하기 위해 원형버퍼에서 읽은 Byte수.. (For LOB)
    SInt       mDoubleBuffPos;      //BUG-22434 : Double Buffer에서 읽은 포인트..
    SInt       mDoubleBuffSize;     //BUG-22434 : Double Buffer에 쌓인 데이터 Size
    ULong      mDataFilePosBk;
    SInt       m_SetEnclosing;
    SChar      m_FieldTerm[MAX_SEPERATOR_LEN];
    SChar      m_RowTerm[MAX_SEPERATOR_LEN];
    SChar      m_Enclosing[MAX_SEPERATOR_LEN];
    SInt       m_nFTLen;
    /* 필드 구분자 물리적 길이.
     * Windows는 \n이 \r\n으로 저장되는데 \r을 포함한 것이 물리적 길이. */
    UInt       mFTPhyLen;
    /* 데이터 파일 읽기 시 필드 구분자 매칭용 상태전이표 */
    UChar    **mFTLexStateTransTbl;
    SInt       m_nRTLen;
    /* 행 구분자 물리적 길이 */
    UInt       mRTPhyLen;
    /* 데이터 파일 읽기 시 행 구분자 매칭용 상태전이표 */
    UChar    **mRTLexStateTransTbl;
    SInt       m_nEnLen;
    /* 필드 encloser 물리적 길이 */
    UInt       mEnPhyLen;
    /* 데이터 파일 읽기 시 필드 encloser 매칭용 상태전이표 */
    UChar    **mEnLexStateTransTbl;

    /* BUG-24358 iloader Geometry Data */    
    SChar *m_TokenValue;
    
    /* TASK-2657 */
    // BUG-27633: mErrorToken도 컬럼의 최대 크기로 할당
    SChar     *mErrorToken;
    UInt       mErrorTokenMaxSize;
    /* m_TokenValue에 저장된 문자열의 길이 */
    UInt       mTokenLen;
    UInt       mErrorTokenLen;
    SInt       m_SetNextToken;
    EDataToken mNextToken;

    /* use_lob_file 옵션 */
    iloBool     mUseLOBFileOpt;
    /* lob_file_size 옵션. 바이트 단위. */
    ULong      mLOBFileSizeOpt;
    /* use_separate_files 옵션 */
    iloBool     mUseSeparateFilesOpt;
    /* lob_indicator 옵션 */
    SChar      mLOBIndicatorOpt[MAX_SEPERATOR_LEN];
    /* lob_indicator 문자열 길이. 물리적 길이 아님. */
    UInt       mLOBIndicatorOptLen;

    /* LOB 컬럼의 존재 여부 */
    iloBool     mLOBColExist;
    /* LOB wrapper 객체 */
    iloLOB    *mLOB;
    /* LOB 파일 포인터 */
    FILE      *mLOBFP;
    /* LOB 파일 번호.
     * use_lob_file=yes, use_separate_files=no일 때 사용. */
    UInt       mLOBFileNo;
    /* LOB 파일 내에서 현재 위치.
     * use_lob_file=yes, use_separate_files=no일 때 사용. */
    ULong      mLOBFilePos;
    /* LOB 파일들 내에서 현재 위치.
     * 현재 LOB 파일 번호보다 작은 번호를 가진 LOB 파일들의 크기가 누적된 값.
     * use_lob_file=yes, use_separate_files=no일 때 사용. */
    ULong      mAccumLOBFilePos;

    /* 데이터 및 LOB 파일 이름에서 공통된 몸체 부분 */
    SChar      mFileNameBody[MAX_FILEPATH_LEN];
    /* 데이터 파일 확장자 */
    SChar      mDataFileNameExt[MAX_FILEPATH_LEN];

    FILE      *mOutFileFP; // PROJ-2030, CT_CASE-3020 CHAR outfile 지원
    
    void AnalDataFileName(const SChar *aDataFileName, iloBool aIsWr);

    UInt GetStrPhyLen(const SChar *aStr);

    IDE_RC MakeAllLexStateTransTbl( ALTIBASE_ILOADER_HANDLE aHandle );

    void FreeAllLexStateTransTbl();

    IDE_RC MakeLexStateTransTbl( ALTIBASE_ILOADER_HANDLE  aHandle, 
                                 SChar                   *aStr,
                                 UChar                   ***aTbl);

    void FreeLexStateTransTbl(UChar **aTbl);

    IDE_RC InitLOBProc( ALTIBASE_ILOADER_HANDLE aHandle, iloBool aIsWr);

    IDE_RC FinalLOBProc( ALTIBASE_ILOADER_HANDLE aHandle );

    // BUG-24902: 데이타 파일의 전체 경로에서 경로만 저장해둔다.
    void InitDataFilePath(SChar *aDataFileName);

    IDE_RC OpenLOBFile( ALTIBASE_ILOADER_HANDLE  aHandle,
                        iloBool                   aIsWr, 
                        UInt                     aLOBFileNo_or_RowNo,
                        UInt                     aColNo,
                        iloBool                   aErrMsgPrint, 
                        SChar                   *aFilePath = NULL);        //BUG-24583

    IDE_RC CloseLOBFile( ALTIBASE_ILOADER_HANDLE aHandle );

    void ConvNumericNormToExp(iloColumns *aCols, SInt aColIdx, SInt aArrayNum);

    IDE_RC PrintOneLOBCol( ALTIBASE_ILOADER_HANDLE  aHandle,
                           SInt                     aRowNo, 
                           iloColumns              *aCols,
                           SInt                     aColNo, 
                           FILE                    *aWriteFp, 
                           iloTableInfo            *pTableInfo);        //BUG-24583

    IDE_RC PrintOneLOBColToDataFile( ALTIBASE_ILOADER_HANDLE  aHandle,
                                     iloColumns              *aCols, 
                                     SInt                     aColIdx, 
                                     FILE                    *aWriteFp);

    IDE_RC PrintOneLOBColToNonSepLOBFile( ALTIBASE_ILOADER_HANDLE  aHandle,
                                          iloColumns              *aCols, 
                                          SInt                     aColIdx, 
                                          FILE                    *aWriteFp );

    IDE_RC PrintOneLOBColToSepLOBFile( ALTIBASE_ILOADER_HANDLE  aHandle,
                                       SInt                     aRowNo,
                                       iloColumns              *aCols,
                                       SInt                     aColIdx,
                                       FILE                    *aWriteFp,
                                       iloTableInfo            *pTableInfo);        //BUG-24583

    IDE_RC AnalLOBIndicator(ULong *aLOBPhyOffs, ULong *aLOBPhyLen);

    IDE_RC LoadOneLOBCol( ALTIBASE_ILOADER_HANDLE  aHandle,
                          SInt                     aRowNo,
                          iloTableInfo            *aTableInfo,
                          SInt                     aColIdx,
                          SInt                     aArrIdx,
                          iloSQLApi               *aISPApi);

    IDE_RC LoadOneLOBColFromDataFile( ALTIBASE_ILOADER_HANDLE  aHandle,
                                      iloTableInfo            *aTableInfo,
                                      SInt                     aColIdx,
                                      SInt                     aArrIdx, 
                                      iloSQLApi               *aISPApi);

    IDE_RC LoadOneLOBColFromNonSepLOBFile( ALTIBASE_ILOADER_HANDLE  aHandle, 
                                           iloTableInfo            *aTableInfo,
                                           SInt                     aColIdx, 
                                           SInt                     aArrIdx, 
                                           iloSQLApi               *aISPApi);

    IDE_RC LoadOneLOBColFromSepLOBFile( ALTIBASE_ILOADER_HANDLE  aHandle, 
                                        SInt                     aRowNo,
                                        iloTableInfo            *aTableInfo,
                                        SInt                     aColIdx, 
                                        SInt                     aArrIdx, 
                                        iloSQLApi               *aISPApi);

    IDE_RC DataFileSeek( ALTIBASE_ILOADER_HANDLE aHandle,
                         ULong                   aDestPos );

    void BackupDataFilePos() { mDataFilePosBk = mDataFilePos; }

    IDE_RC RestoreDataFilePos( ALTIBASE_ILOADER_HANDLE aHandle )
    { return DataFileSeek( aHandle,  mDataFilePosBk); }

    IDE_RC GetLOBFileSize( ALTIBASE_ILOADER_HANDLE aHandle, ULong *aLOBFileSize);

    IDE_RC LOBFileSeek( ALTIBASE_ILOADER_HANDLE aHandle, ULong aAccumDestPos);

    IDE_RC loadFromOutFile( ALTIBASE_ILOADER_HANDLE aHandle ); // PROJ-2030, CT_CASE-3020 CHAR outfile 지원
    
public:
    CCircularBuf mCirBuf;            //PROJ-1714 Data Upload시 사용하는 원형 버퍼
    SChar       *mDoubleBuff;        //BUG-22434 Double Buffer 객체
    
    /* BUG-24583 Lob FilePath & FileName */
    SChar     **mLOBFile;
    SInt        mLOBFileRowCount;
    SInt        mLOBFileColumnNum;             //LOB Column이 여러개 일때, 해당 LOB file이 어느 Column에 해당하는지를 구분
    
public:
    void SetEOF( ALTIBASE_ILOADER_HANDLE aHandle, iloBool aVal )
    {
        mCirBuf.SetEOF( aHandle, aVal ); 
    }
    void InitializeCBuff( ALTIBASE_ILOADER_HANDLE aHandle )
    { mCirBuf.Initialize(aHandle); }
    void FinalizeCBuff( ALTIBASE_ILOADER_HANDLE aHandle)
    { mCirBuf.Finalize(aHandle); }
    
};

#endif /* _O_ILO_DATAFILE_H */


