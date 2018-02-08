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
 * $Id: iloBadFile.cpp 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

#include <ilo.h>

iloBadFile::iloBadFile()
{
    m_BadFp = NULL;
    m_UseBadFile = SQL_FALSE;
    mIsCSV = ILO_FALSE;
    idlOS::strcpy(m_FieldTerm, "^");
    idlOS::strcpy(m_RowTerm, "\n");
}

void iloBadFile::SetTerminator(SChar *szFiledTerm, SChar *szRowTerm)
{
    idlOS::strcpy(m_FieldTerm, szFiledTerm);
    idlOS::strcpy(m_RowTerm, szRowTerm);
}

SInt iloBadFile::OpenFile(SChar *szFileName)
{
    // bug-20391
    if( idlOS::strncmp(szFileName, "stderr", 6) == 0)
    {
        m_BadFp = stderr;
    }
    else if(idlOS::strncmp(szFileName, "stdout", 6) == 0)
    {
        m_BadFp = stdout;
    }
    else
    {
        m_BadFp = ilo_fopen(szFileName, "wb");      //BUG-24511 모든 Fopen은 binary type으로 설정해야함
    }
    IDE_TEST( m_BadFp == NULL );

    m_UseBadFile = SQL_TRUE;
    return SQL_TRUE;

    IDE_EXCEPTION_END;

    idlOS::printf("Bad File [%s] open fail\n", szFileName);

    m_UseBadFile = SQL_FALSE;
    return SQL_FALSE;
}

SInt iloBadFile::CloseFile()
{
    IDE_TEST_RAISE( m_UseBadFile != SQL_TRUE, err_ignore );

    // BUG-20525
    if((m_BadFp != stderr) && (m_BadFp != stdout))
    {
        IDE_TEST( idlOS::fclose( m_BadFp ) != 0 );
    }

    m_UseBadFile = SQL_FALSE;
    return SQL_TRUE;

    IDE_EXCEPTION( err_ignore );
    {
        return SQL_TRUE;
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

/* TASK-2657 */
void iloBadFile::setIsCSV ( SInt aIsCSV )
{
    mIsCSV = aIsCSV;
}

/*
// proj1778 nchar: for debug
static void dump(SChar* aBuf, SInt aLen)
{
    UChar* sPtr = (UChar*) aBuf;
    SInt i;
    for (i = 0; i < aLen; i++)
    {
        idlOS::printf("%2x ", *sPtr);
        sPtr++;
    }
    idlOS::printf("\n");
}
*/


SInt iloBadFile::PrintOneRecord( ALTIBASE_ILOADER_HANDLE   aHandle,
                                 iloTableInfo             *pTableInfo,
                                 SInt                      aArrayCount,
                                 SChar                   **aFile, 
                                 iloBool                    aIsOpt )
{
    SInt i;
    SChar*  sDataPtr = NULL;
    SInt    sDataLen = 0;
    SInt    sFileNum = 0;       //BUG-24583

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    

    IDE_TEST_CONT( m_UseBadFile != SQL_TRUE, err_ignore );

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
        // BUG-27633: geometry 데이타도 bad 파일에 기록
        case ISP_ATTR_GEOMETRY :

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

            if (sDataLen > 0)
            {
                // BUG-28069: log, bad에도 csv 형식으로 기록
                if ((mIsCSV == ILO_TRUE) && (pTableInfo->mAttrFailLen[i] == 0))
                {
                    IDE_TEST_RAISE( iloDataFile::csvWrite( sHandle,
                                                           sDataPtr,
                                                           sDataLen,
                                                           m_BadFp)
                                                      != SQL_TRUE, WriteError);
                }
                else
                {
                    IDE_TEST_RAISE( idlOS::fwrite(sDataPtr, (size_t)sDataLen, 1, m_BadFp)
                                    != 1, WriteError);
                }
            }

            break;
        case ISP_ATTR_INTEGER :
        case ISP_ATTR_DOUBLE:
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
            IDE_TEST_RAISE( idlOS::fprintf(m_BadFp, "%s", sDataPtr )
                            < 0, WriteError);

            break;
        /* BUG-24583 
         * BAD File을 이용해 BLOB, CLOB 데이터를 다시 업로드 할 수 있도록 한다. 
         */
        case ISP_ATTR_BLOB :
            if ( aIsOpt == ILO_TRUE )
            {
                IDE_TEST_RAISE( idlOS::fprintf(m_BadFp, "%s", aFile[sFileNum])
                                < 0, WriteError);
                sFileNum++;
            }
            else
            {
                IDE_TEST_RAISE( idlOS::fprintf(m_BadFp, "[BLOB]")
                                < 0, WriteError);
            }
            break;
        case ISP_ATTR_CLOB :
            if ( aIsOpt == ILO_TRUE )
            {
                IDE_TEST_RAISE( idlOS::fprintf(m_BadFp, "%s", aFile[sFileNum])
                                < 0, WriteError);
                sFileNum++;
            }
            else
            {
                IDE_TEST_RAISE( idlOS::fprintf(m_BadFp, "[CLOB]")
                                < 0, WriteError);
            }
            break;        
        default :
            IDE_TEST_RAISE( idlOS::fprintf(m_BadFp, "unknown type")
                            < 0, WriteError);
            break;
        }
        /* TASK-2657 */
        if( mIsCSV == ILO_TRUE )
        {
            if ( i == pTableInfo->GetAttrCount()-1 )
            {
                IDE_TEST_RAISE( idlOS::fprintf(m_BadFp, "%c", '\n' )
                                < 0, WriteError);
            }
            else 
            {
                /* if there are columns less than what we expected, print '\n' */
                IDE_TEST_RAISE( idlOS::fprintf(m_BadFp, "%c",
                                    ( i == pTableInfo->GetReadCount(aArrayCount) -1 ) ? '\n' : ',' )
                                < 0, WriteError);
            }
        }
        else
        {
            IDE_TEST_RAISE( idlOS::fprintf(m_BadFp, "%s",
                                (i == pTableInfo->GetAttrCount()-1) ? m_RowTerm : m_FieldTerm)
                            < 0, WriteError);
        }
    }

    IDE_EXCEPTION_CONT(err_ignore);

    return SQL_TRUE;

    IDE_EXCEPTION(WriteError);
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Bad_File_IO_Error);
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        
            (void)idlOS::printf("Bad file write fail\n");
        }
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}


