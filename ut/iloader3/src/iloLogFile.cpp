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
 * $Id: iloLogFile.cpp 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

#include <ilo.h>

iloLogFile::iloLogFile()
{
    m_LogFp = NULL;
    m_UseLogFile = SQL_FALSE;
    mIsCSV  = ID_FALSE;
    idlOS::strcpy(m_FieldTerm, "^");
    idlOS::strcpy(m_RowTerm, "\n");
}

void iloLogFile::SetTerminator(SChar *szFiledTerm, SChar *szRowTerm)
{
    idlOS::strcpy(m_FieldTerm, szFiledTerm);
    idlOS::strcpy(m_RowTerm, szRowTerm);
}

SInt iloLogFile::OpenFile(SChar *szFileName)
{
    // bug-20391
    if( idlOS::strncmp(szFileName, "stderr", 6) == 0)
    {
        m_LogFp = stderr;
    }
    else if(idlOS::strncmp(szFileName, "stdout", 6) == 0)
    {
        m_LogFp = stdout;
    }
    else
    {
        m_LogFp = ilo_fopen(szFileName, "wb");      //BUG-24511 모든 Fopen은 binary type으로 설정해야함
    }
    IDE_TEST( m_LogFp == NULL );

    m_UseLogFile = SQL_TRUE;
    return SQL_TRUE;

    IDE_EXCEPTION_END;

    idlOS::printf("Log File [%s] open fail\n", szFileName);

    m_UseLogFile = SQL_FALSE;
    return SQL_FALSE;
}

SInt iloLogFile::CloseFile()
{
    IDE_TEST_CONT( m_UseLogFile == SQL_FALSE, err_ignore );

    // BUG-20525
    if((m_LogFp != stderr) && (m_LogFp != stdout))
    {
        IDE_TEST( idlOS::fclose(m_LogFp) != 0 );
    }

    m_UseLogFile = SQL_FALSE;

    IDE_EXCEPTION_CONT( err_ignore );

    return SQL_TRUE;

    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

void iloLogFile::PrintLogMsg(const SChar *szMsg)
{
    UInt sMsgLen;

    IDE_TEST(m_UseLogFile == SQL_FALSE);

    idlOS::fprintf(m_LogFp, "%s", szMsg);
    sMsgLen = idlOS::strlen(szMsg);
    if (sMsgLen > 0 && szMsg[sMsgLen - 1] != '\n')
    {
        idlOS::fprintf(m_LogFp, "\n");
    }

    IDE_EXCEPTION_END;

    return;
}

// BUG-21640 iloader에서 에러메시지를 알아보기 편하게 출력하기
// 기존 에러메시지와 동일한 형식으로 출력하는 함수추가
void iloLogFile::PrintLogErr(uteErrorMgr *aMgr)
{
    IDE_TEST(m_UseLogFile == SQL_FALSE);

    utePrintfErrorCode(m_LogFp, aMgr);

    IDE_EXCEPTION_END;

    return;
}

void iloLogFile::PrintTime(const SChar *szPrnStr)
{
    time_t lTime;
    SChar  *szTime;

    IDE_TEST(m_UseLogFile == SQL_FALSE);

    time(&lTime);
    szTime = ctime(&lTime);

    idlOS::fprintf(m_LogFp, "%s : %s", szPrnStr, szTime);

    IDE_EXCEPTION_END;

    return;
}
/* TASK-2657 */
void iloLogFile::setIsCSV ( SInt aIsCSV )
{
    mIsCSV = aIsCSV;
}

// BUG-21640 iloader에서 에러메시지를 알아보기 편하게 출력하기
SInt iloLogFile::PrintOneRecord( ALTIBASE_ILOADER_HANDLE  aHandle,
                                 iloTableInfo            *pTableInfo,
                                 SInt                     aReadRecCount,
                                 SInt                     aArrayCount)
{
    SInt    i;
    SChar*  sDataPtr  = NULL;
    SInt    sDataLen  = 0;
    SInt    sColCount = pTableInfo->GetReadCount(aArrayCount);
    static SChar*  sBlob     = (SChar *)"[BLOB]"; // BUG-31130
    static SChar*  sClob     = (SChar *)"[CLOB]";
    static SChar*  sGeo      = (SChar *)"[GEOMETRY]";
    static SChar*  sUnknown  = (SChar *)"unknown type";
 

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    if(( sHandle->mUseApi == SQL_TRUE ) && ( sHandle->mLogCallback != NULL ))
    {
        
        if (sHandle->mDataBufMaxColCount < sColCount)
        {
            if (sHandle->mDataBuf != NULL)
            {
                idlOS::free(sHandle->mDataBuf);
            }
            sHandle->mDataBuf = (SChar**)idlOS::calloc(sColCount,
                                                       ID_SIZEOF(SChar*));
            IDE_TEST_RAISE( sHandle->mDataBuf == NULL, mallocError );

            sHandle->mDataBufMaxColCount = sColCount;
        }

        sHandle->mLog.recordData = sHandle->mDataBuf;
        sHandle->mLog.recordColCount =  sColCount;

        sHandle->mLog.record = aReadRecCount;
    }
    
    if ( sHandle->mUseApi != SQL_TRUE )
    {
        IDE_TEST_CONT(m_UseLogFile == SQL_FALSE, err_ignore);
    }
    else
    {
        IDE_TEST_CONT( ((m_UseLogFile == SQL_FALSE) && (sHandle->mLogCallback == NULL)), err_ignore);
    }

    if ( m_UseLogFile == SQL_TRUE )
    {
        // log 파일에 rowcount 를 출력한다.
        idlOS::fprintf(m_LogFp, "Record %d : ", aReadRecCount);
    }

    for (i=0; i < pTableInfo->GetReadCount(aArrayCount); i++)
    {
        if ( pTableInfo->GetAttrType(i) == ISP_ATTR_TIMESTAMP &&
             sHandle->mParser.mAddFlag == ILO_TRUE )
        {
            i++;
        }
        switch (pTableInfo->GetAttrType(i))
        {
        case ISP_ATTR_CHAR :
        case ISP_ATTR_VARCHAR :
        case ISP_ATTR_NCHAR :
        case ISP_ATTR_NVARCHAR :

            // TASK-2657
            //=======================================================
            // proj1778 nchar
            // 기존에는 csv이면 fwrite를 사용하고 아니면 fprintf(%s)를 사용했는데 utf16
            // nchar가 추가되면서 null종료자가 문자안에 있을수 있으므로 fwrite로 통일시킨다
            // mAttrFailLen[i] 가 0이 아니라면 read file시의 failed column이 들어있다는 얘기
            if (pTableInfo->mAttrFailLen[i] != 0)
            {
                sDataPtr = pTableInfo->GetAttrFail( i );
                sDataLen = (SInt)(pTableInfo->mAttrFailLen[i]);
            }
            else
            {
                sDataPtr = pTableInfo->GetAttrCVal(i, aArrayCount);
                sDataLen = (SInt)(pTableInfo->mAttrInd[i][aArrayCount]);
            }
    
            /* data file에 공백인 칼럼이 있는 경우 */ 
            if (( sHandle->mUseApi == SQL_TRUE ) &&
                    ( sHandle->mLogCallback != NULL ))      
            {
                // BUG-40202: insure++ warning READ_OVERFLOW
                // if ( idlOS::strcmp(sDataPtr, "") == 0 )
                /* BUG-43351 Null Pointer Dereference */
                if ( sDataPtr != NULL && sDataPtr[0] == '\0' )
                {
                    sHandle->mDataBuf[i] = sDataPtr;
                }
            }
           
            if (sDataLen > 0)
            {
                if (( sHandle->mUseApi == SQL_TRUE ) && ( sHandle->mLogCallback != NULL ))      
                {
                    if (sDataPtr != NULL)
                    {
                        sDataPtr[sDataLen]='\0';
                    }
        
                    sHandle->mDataBuf[i] = sDataPtr;
                }

                if ( m_UseLogFile == SQL_TRUE )
                {
                    // BUG-28069: log, bad에도 csv 형식으로 기록
                    if ((mIsCSV == ID_TRUE) && (pTableInfo->mAttrFailLen[i] == 0))
                    {
                        IDE_TEST_RAISE( iloDataFile::csvWrite( sHandle, sDataPtr, sDataLen, m_LogFp )
                                != SQL_TRUE, WriteError);
                    }
                    else
                    {
                        IDE_TEST_RAISE( idlOS::fwrite(sDataPtr, (size_t)sDataLen, 1, m_LogFp)
                                != 1, WriteError);
                    }
                }
            }
            break;
        case ISP_ATTR_INTEGER :
        case ISP_ATTR_DOUBLE :
        case ISP_ATTR_SMALLINT:
        case ISP_ATTR_BIGINT:
        case ISP_ATTR_DECIMAL:
        case ISP_ATTR_FLOAT:
        case ISP_ATTR_REAL:
        case ISP_ATTR_NIBBLE :
        case ISP_ATTR_BYTES :
        case ISP_ATTR_VARBYTE :
        case ISP_ATTR_NUMERIC_LONG :
        case ISP_ATTR_NUMERIC_DOUBLE :
        case ISP_ATTR_DATE :
        case ISP_ATTR_TIMESTAMP :
        case ISP_ATTR_BIT:
        case ISP_ATTR_VARBIT:

            // BUG - 18804
            // mAttrFail에 fail난 필드가 있는지 확인한 후 없으면 mAttrCVal에서 가져옴.
            if (pTableInfo->mAttrFailLen[i] != 0)
            {
                sDataPtr = pTableInfo->GetAttrFail( i );
            }
            else
            {
                sDataPtr = pTableInfo->GetAttrCVal(i, aArrayCount);
            }
            
            if (( sHandle->mUseApi == SQL_TRUE ) && ( sHandle->mLogCallback != NULL ))      
            {
                sHandle->mDataBuf[i] = sDataPtr;
            }

            if ( m_UseLogFile == SQL_TRUE )
            {
                IDE_TEST_RAISE( idlOS::fprintf(m_LogFp, "%s", sDataPtr )
                        < 0, WriteError);
            }
            break;
        case ISP_ATTR_BLOB :
            if ( m_UseLogFile == SQL_TRUE )
            {
                IDE_TEST_RAISE( idlOS::fprintf(m_LogFp, "%s",sBlob )
                        < 0, WriteError);
            }
            else
            {
                 sHandle->mDataBuf[i] = sBlob;
            }
            break;
        case ISP_ATTR_CLOB :
            if ( m_UseLogFile == SQL_TRUE )
            {
                IDE_TEST_RAISE( idlOS::fprintf(m_LogFp, "%s",sClob )
                        < 0, WriteError);
            }
            else
            {
                 sHandle->mDataBuf[i] = sClob;
            }
            break;
        // BUG-24358 iloader geometry type support            
        case ISP_ATTR_GEOMETRY :
            if ( m_UseLogFile == SQL_TRUE )
            {
                IDE_TEST_RAISE( idlOS::fprintf(m_LogFp, "%s",sGeo )
                        < 0, WriteError);
            }
            else
            {
                 sHandle->mDataBuf[i] = sGeo;
            }
            break;
        default :
            if ( m_UseLogFile == SQL_TRUE )
            {
                IDE_TEST_RAISE( idlOS::fprintf(m_LogFp, "%s",sUnknown )
                        < 0, WriteError);
            }
            else
            {
                 sHandle->mDataBuf[i] = sUnknown;
            }
            break;
        }

        if ( m_UseLogFile == SQL_TRUE )
        {
            /* TASK-2657 */
            // BUG-24898 iloader 파싱에러 상세화
            // log 파일에 출력시 데이타 출력후 반드시 엔터가 들어갈수 있도록한다.
            // rowterm 은 출력할 필요없이 데이타를 출력후 마지막에 엔터를 넣는다.
            if( mIsCSV == ID_TRUE )
            {
                if ( i != pTableInfo->GetAttrCount()-1 )
                {
                    IDE_TEST_RAISE( idlOS::fprintf(m_LogFp, "%c", ',' )
                            < 0, WriteError);
                }
            }
            else
            {
                if ( i != pTableInfo->GetAttrCount()-1 )
                {
                    IDE_TEST_RAISE( idlOS::fprintf(m_LogFp, "%s", m_FieldTerm )
                            < 0, WriteError);
                }
            }
        }
    }

    if ( m_UseLogFile == SQL_TRUE )
    {
        IDE_TEST_RAISE( idlOS::fprintf(m_LogFp, "%c", '\n' )
                < 0, WriteError);
    }

    IDE_EXCEPTION_CONT(err_ignore);

    return SQL_TRUE;

    IDE_EXCEPTION(mallocError)
    {   
        if (sHandle->mDataBuf != NULL)
        {
            idlOS::free(sHandle->mDataBuf);
        }
    }  
    IDE_EXCEPTION(WriteError);
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Log_File_IO_Error);
        if ( sHandle->mUseApi != SQL_TRUE)
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
            (void)idlOS::printf("Log file write fail\n");
        }
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

