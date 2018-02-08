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
 * $Id: iloFormDown.cpp 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

#include <ilo.h>
#include <iloApi.h>

iloFormDown::iloFormDown()
{
    m_pProgOption = NULL;
    m_pISPApi = NULL;
    m_fpForm = NULL;
}

SInt iloFormDown::FormDown( ALTIBASE_ILOADER_HANDLE aHandle )
{
    iloBool sIsQueue = ILO_FALSE;
    SInt    i = 0;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    IDE_TEST_RAISE(m_pISPApi->CheckIsQueue( sHandle,
                                            m_pProgOption->m_TableName[0],
                                            m_pProgOption->m_TableOwner[0],
                                            &sIsQueue)
                                            != SQL_TRUE, SchemaGetError);

    IDE_TEST_RAISE(m_pISPApi->Columns( sHandle,
                                       m_pProgOption->m_TableName[0],
                                       m_pProgOption->m_TableOwner[0])
                                       != SQL_TRUE, SchemaGetError);

    /* BUG-30467 */
    if ( m_pProgOption->mPartition == ILO_TRUE )
    {
        IDE_TEST_RAISE(m_pISPApi->PartitionInfo( sHandle, m_pProgOption->m_TableName[0],
                                                 sHandle->mProgOption->GetLoginID())
                                                 != SQL_TRUE, SchemaGetError);
    }

    (void)m_pISPApi->EndTran(ILO_TRUE);
    
    IDE_TEST_RAISE(OpenFormFile(sHandle) != SQL_TRUE, OpenError);

    if (sIsQueue == ILO_FALSE)
    {
        /* table */
        
        /* BUG-30467 */
        if ( m_pProgOption->mPartition == ILO_TRUE )
        {
            for ( i = 0; i < m_pISPApi->m_Column.m_PartitionCount ; i++ )
            {
               m_fpForm = m_ArrayFpForm[i];

               IDE_TEST_RAISE(WriteColumns(i) != SQL_TRUE, WriteError);
            }
        }
        else
        {

            IDE_TEST_RAISE(WriteColumns(i) != SQL_TRUE, WriteError);
        }
    }
    else
    {      
        /* queue */
        IDE_TEST_RAISE(WriteMessageSize() != SQL_TRUE, WriteError);
    }
    
    CloseFormFile();

    (void)m_pISPApi->m_Column.AllFree();

    return SQL_TRUE;

    IDE_EXCEPTION(SchemaGetError);
    {
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
        
        (void)m_pISPApi->EndTran(ILO_FALSE);
    }
    IDE_EXCEPTION(OpenError);
    {
        if (uteGetErrorCODE(sHandle->mErrorMgr) != 0x91100) //utERR_ABORT_File_Lock_Error
        {
            uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_openFileError,
                            m_pProgOption->m_FormFile);
        }
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
        
        (void)m_pISPApi->m_Column.AllFree();
    }
    IDE_EXCEPTION(WriteError);
    {
        CloseFormFile();
        (void)m_pISPApi->m_Column.AllFree();
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

SInt iloFormDown::WriteMessageSize()
{
    SChar *pName;
    SInt   i;
    SInt   sDateFlag = 0;

    if ( m_pProgOption->m_bExist_TabOwner == SQL_FALSE )
    {
        idlOS::fprintf(m_fpForm, "queue %s\n", m_pProgOption->m_TableName[0]);
    }
    else
    {
        idlOS::fprintf(m_fpForm, "queue %s.%s\n",
                       m_pProgOption->m_TableOwner[0],
                       m_pProgOption->m_TableName[0]);
    }
    idlOS::fprintf(m_fpForm, "{\n");
    for (i=1; i < m_pISPApi->m_Column.GetSize(); i++)
    { /* to skip messageid column. i start with 1 */
        pName = (SChar *)m_pISPApi->m_Column.GetName(i);
        idlOS::fprintf(m_fpForm, "%s ", pName);

        switch (m_pISPApi->m_Column.GetType(i))
        {
        case SQL_DOUBLE :
            idlOS::fprintf(m_fpForm, "double;\n");
            break;
        case SQL_NUMERIC :
            if (m_pISPApi->m_Column.GetScale(i) != 0)
            {
                idlOS::fprintf(m_fpForm, "numeric (%"ID_UINT32_FMT", %"ID_INT32_FMT");\n",
                               m_pISPApi->m_Column.GetPrecision(i),
                               (SInt)m_pISPApi->m_Column.GetScale(i));
            }
            else if (m_pISPApi->m_Column.GetPrecision(i) == 38)
            {
                idlOS::fprintf(m_fpForm, "numeric;\n");
            }
            else
            {
                idlOS::fprintf(m_fpForm, "numeric (%"ID_UINT32_FMT");\n",
                               m_pISPApi->m_Column.GetPrecision(i));
            }
            break;
        case SQL_DECIMAL :
            if (m_pISPApi->m_Column.GetScale(i) != 0)
            {
                idlOS::fprintf(m_fpForm, "decimal (%"ID_UINT32_FMT", %"ID_INT32_FMT");\n",
                               m_pISPApi->m_Column.GetPrecision(i),
                               (SInt)m_pISPApi->m_Column.GetScale(i));
            }
            else if (m_pISPApi->m_Column.GetPrecision(i) == 38)
            {
                idlOS::fprintf(m_fpForm, "decimal;\n");
            }
            else
            {
                idlOS::fprintf(m_fpForm, "decimal (%"ID_UINT32_FMT");\n",
                               m_pISPApi->m_Column.GetPrecision(i));
            }
            break;
        case SQL_FLOAT :
            if (m_pISPApi->m_Column.GetPrecision(i) == 38)
            {
                idlOS::fprintf(m_fpForm, "float;\n");
            }
            else
            {
                idlOS::fprintf(m_fpForm, "float (%"ID_UINT32_FMT");\n",
                               m_pISPApi->m_Column.GetPrecision(i));
            }
            break;
        case SQL_VARCHAR :
            idlOS::fprintf(m_fpForm, "varchar (%"ID_UINT32_FMT");\n",
                           m_pISPApi->m_Column.GetPrecision(i));
            break;
        case SQL_SMALLINT :
            idlOS::fprintf(m_fpForm, "smallint;\n");
            break;
        case SQL_BIGINT :
            idlOS::fprintf(m_fpForm, "bigint;\n");
            break;
        case SQL_REAL :
            idlOS::fprintf(m_fpForm, "real;\n");
            break;
        case SQL_INTEGER :
            idlOS::fprintf(m_fpForm, "integer;\n");
            break;
        case SQL_NIBBLE :
            idlOS::fprintf(m_fpForm, "nibble (%"ID_UINT32_FMT");\n",
                           m_pISPApi->m_Column.GetPrecision(i));
            break;
        case SQL_CHAR :
            idlOS::fprintf(m_fpForm, "char (%"ID_UINT32_FMT");\n",
                           m_pISPApi->m_Column.GetPrecision(i));
            break;
        case SQL_CLOB :
        case SQL_CLOB_LOCATOR :
            idlOS::fprintf(m_fpForm, "clob;\n");
            break;
        case SQL_BYTES :
            idlOS::fprintf(m_fpForm, "byte (%"ID_UINT32_FMT");\n",
                           m_pISPApi->m_Column.GetPrecision(i));
            break;
        case SQL_VARBYTE :
            idlOS::fprintf(m_fpForm, "varbyte (%"ID_UINT32_FMT");\n",
                           m_pISPApi->m_Column.GetPrecision(i));
            break;
        case SQL_BINARY :
        case SQL_BLOB :
        case SQL_BLOB_LOCATOR :
            idlOS::fprintf(m_fpForm, "blob;\n");
            break;
        case SQL_DATE :
        case SQL_TIMESTAMP :
            idlOS::fprintf(m_fpForm, "date;\n");
            sDateFlag = 1;
            break;
        case SQL_NATIVE_TIMESTAMP :
            idlOS::fprintf(m_fpForm, "timestamp;\n");
            sDateFlag = 1;
            break;
        case SQL_BIT:
            idlOS::fprintf(m_fpForm, "bit (%"ID_UINT32_FMT");\n",
                           m_pISPApi->m_Column.GetPrecision(i));
            break;
        case SQL_VARBIT:
            idlOS::fprintf(m_fpForm, "varbit (%"ID_UINT32_FMT");\n",
                           m_pISPApi->m_Column.GetPrecision(i));
            break;
        default :
            idlOS::printf("%s [Unknown Attribute Type]\n", pName);
            return SQL_FALSE;
        }
    }
    idlOS::fprintf(m_fpForm, "}\n");

    // fix BUG-17605
    if ( sDateFlag == 1)
    {
        // fix CASE-4061
        /* BUG-32617 DATEFORM should be modified to YYYY/MM/DD HH:MI:SS:SSSSSS */
        idlOS::fprintf( m_fpForm, "DATEFORM YYYY/MM/DD HH:MI:SS:SSSSSS\n" );
    }

    /* BUG-40413 DATA_NLS_USE needs to be contained in form file for Queue objects */
    idlOS::fprintf(m_fpForm, "DATA_NLS_USE=%s\n", m_pProgOption->GetNLS());
    
    return SQL_TRUE;
}

SInt iloFormDown::FormDownAsStruct( ALTIBASE_ILOADER_HANDLE aHandle )
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    IDE_TEST_RAISE(m_pISPApi->Columns( sHandle,
                                       m_pProgOption->m_TableName[0],
                                       m_pProgOption->m_TableOwner[0])
                                       != SQL_TRUE, SchemaGetError);

    (void)m_pISPApi->EndTran(ILO_TRUE);

    IDE_TEST_RAISE(OpenFormFile(sHandle) != SQL_TRUE, OpenError);
    IDE_TEST_RAISE(WriteColumnsAsStruct() != SQL_TRUE, WriteError);
    CloseFormFile();

    (void)m_pISPApi->m_Column.AllFree();

    return SQL_TRUE;

    IDE_EXCEPTION(SchemaGetError);
    {
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
        
        (void)m_pISPApi->EndTran(ILO_FALSE);
    }
    IDE_EXCEPTION(OpenError);
    {
        (void)m_pISPApi->m_Column.AllFree();
    }
    IDE_EXCEPTION(WriteError);
    {
        CloseFormFile();
        (void)m_pISPApi->m_Column.AllFree();
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

SInt iloFormDown::OpenFormFile( ALTIBASE_ILOADER_HANDLE aHandle )
{ 
    SInt  i;
    SChar sNum[MAX_OBJNAME_LEN] = { '\0', };
    SInt sFormNameLen = 0;
    
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    sFormNameLen = idlOS::strlen(m_pProgOption->m_FormFile);

    /* BUG-30467 */
    if ( sHandle->mProgOption->mPartition == ILO_TRUE )
    {
        m_ArrayFpForm = (FILE **)idlOS::calloc( sHandle->mSQLApi->m_Column.m_PartitionCount,
                                         ID_SIZEOF(FILE*));
        IDE_TEST( m_ArrayFpForm == NULL );

        for ( i =0 ; i < sHandle->mSQLApi->m_Column.m_PartitionCount; i++)
        {
            
            sprintf(sNum,".%s", m_pISPApi->m_Column.m_ArrayPartName[i]);
                
            idlOS::sprintf( m_pProgOption->m_FormFile + sFormNameLen,"%s", sNum );
            
            m_ArrayFpForm[i] = iloFileOpen( sHandle, m_pProgOption->m_FormFile, 
                               O_CREAT | O_RDWR, (SChar*)"wt", LOCK_WR);
            IDE_TEST( m_ArrayFpForm[i] == NULL );
        }
        
    }
    else
    {
        m_NoArrayFpForm = iloFileOpen( sHandle, m_pProgOption->m_FormFile, 
                                       O_CREAT | O_RDWR, (SChar*)"wt", LOCK_WR);
        IDE_TEST( m_NoArrayFpForm == NULL );

        m_fpForm = m_NoArrayFpForm;        

    }

    return SQL_TRUE;

    IDE_EXCEPTION_END;

     /* BUG-30467 */    
    if ( m_ArrayFpForm != NULL )
    {
        idlOS::free(m_ArrayFpForm);
        m_ArrayFpForm = NULL;
    }

    return SQL_FALSE;
}

SInt iloFormDown::WriteColumns( SInt aPartitionCnt )
{
    SChar *pName;
    SInt   i;
    SInt   sDateFlag = 0;
    SInt   sNCharFlag = 0;

    if ( m_pProgOption->m_bExist_TabOwner == SQL_FALSE )
    {
        idlOS::fprintf(m_fpForm, "table %s\n", m_pProgOption->m_TableName[0]);
    }
    else
    {
        idlOS::fprintf(m_fpForm, "table %s.%s\n",
                       m_pProgOption->m_TableOwner[0],
                       m_pProgOption->m_TableName[0]);
    }
    idlOS::fprintf(m_fpForm, "{\n");
    for (i=0; i < m_pISPApi->m_Column.GetSize(); i++)
    {
        pName = (SChar *)m_pISPApi->m_Column.GetName(i);
        idlOS::fprintf(m_fpForm, "%s ", pName);

        switch (m_pISPApi->m_Column.GetType(i))
        {
        case SQL_DOUBLE :
            idlOS::fprintf(m_fpForm, "double;\n");
            break;
        case SQL_NUMERIC :
            if (m_pISPApi->m_Column.GetScale(i) != 0)
            {
                idlOS::fprintf(m_fpForm, "numeric (%"ID_UINT32_FMT", %"ID_INT32_FMT");\n",
                               m_pISPApi->m_Column.GetPrecision(i),
                               (SInt)m_pISPApi->m_Column.GetScale(i));
            }
            else if (m_pISPApi->m_Column.GetPrecision(i) == 38)
            {
                idlOS::fprintf(m_fpForm, "numeric;\n");
            }
            else
            {
                idlOS::fprintf(m_fpForm, "numeric (%"ID_UINT32_FMT");\n",
                               m_pISPApi->m_Column.GetPrecision(i));
            }
            break;
        case SQL_DECIMAL :
            if (m_pISPApi->m_Column.GetScale(i) != 0)
            {
                idlOS::fprintf(m_fpForm, "decimal (%"ID_UINT32_FMT", %"ID_INT32_FMT");\n",
                               m_pISPApi->m_Column.GetPrecision(i),
                               (SInt)m_pISPApi->m_Column.GetScale(i));
            }
            else if (m_pISPApi->m_Column.GetPrecision(i) == 38)
            {
                idlOS::fprintf(m_fpForm, "decimal;\n");
            }
            else
            {
                idlOS::fprintf(m_fpForm, "decimal (%"ID_UINT32_FMT");\n",
                               m_pISPApi->m_Column.GetPrecision(i));
            }
            break;
        case SQL_FLOAT :
            if (m_pISPApi->m_Column.GetPrecision(i) == 38)
            {
                idlOS::fprintf(m_fpForm, "float;\n");
            }
            else
            {
                idlOS::fprintf(m_fpForm, "float (%"ID_UINT32_FMT");\n",
                               m_pISPApi->m_Column.GetPrecision(i));
            }
            break;
        case SQL_VARCHAR :
            idlOS::fprintf(m_fpForm, "varchar (%"ID_UINT32_FMT");\n",
                           m_pISPApi->m_Column.GetPrecision(i));
            break;
        case SQL_SMALLINT :
            idlOS::fprintf(m_fpForm, "smallint;\n");
            break;
        case SQL_BIGINT :
            idlOS::fprintf(m_fpForm, "bigint;\n");
            break;
        case SQL_REAL :
            idlOS::fprintf(m_fpForm, "real;\n");
            break;
        case SQL_INTEGER :
            idlOS::fprintf(m_fpForm, "integer;\n");
            break;
        case SQL_NIBBLE :
            idlOS::fprintf(m_fpForm, "nibble (%"ID_UINT32_FMT");\n",
                           m_pISPApi->m_Column.GetPrecision(i));
            break;
        case SQL_CHAR :
            idlOS::fprintf(m_fpForm, "char (%"ID_UINT32_FMT");\n",
                           m_pISPApi->m_Column.GetPrecision(i));
            break;
        case SQL_CLOB :
        case SQL_CLOB_LOCATOR :
            idlOS::fprintf(m_fpForm, "clob;\n");
            break;
        case SQL_BYTES :
            idlOS::fprintf(m_fpForm, "byte (%"ID_UINT32_FMT");\n",
                           m_pISPApi->m_Column.GetPrecision(i));
            break;
        case SQL_VARBYTE :
            idlOS::fprintf(m_fpForm, "varbyte (%"ID_UINT32_FMT");\n",
                           m_pISPApi->m_Column.GetPrecision(i));
            break;
        case SQL_BINARY :
        case SQL_BLOB :
        case SQL_BLOB_LOCATOR :
            idlOS::fprintf(m_fpForm, "blob;\n");
            break;
        case SQL_DATE :
        case SQL_TIMESTAMP :
            idlOS::fprintf(m_fpForm, "date;\n");
            sDateFlag = 1;
            break;
        case SQL_NATIVE_TIMESTAMP :
            idlOS::fprintf(m_fpForm, "timestamp;\n");
            sDateFlag = 1;
            break;
        case SQL_BIT:
            idlOS::fprintf(m_fpForm, "bit (%"ID_UINT32_FMT");\n",
                           m_pISPApi->m_Column.GetPrecision(i));
            break;
        case SQL_VARBIT:
            idlOS::fprintf(m_fpForm, "varbit (%"ID_UINT32_FMT");\n",
                           m_pISPApi->m_Column.GetPrecision(i));
            break;
        case SQL_WCHAR :
            idlOS::fprintf(m_fpForm, "nchar (%"ID_UINT32_FMT");\n",
                           m_pISPApi->m_Column.GetPrecision(i));
            sNCharFlag = 1;
            break;
        case SQL_WVARCHAR :
            idlOS::fprintf(m_fpForm, "nvarchar (%"ID_UINT32_FMT");\n",
                           m_pISPApi->m_Column.GetPrecision(i));
            sNCharFlag = 1;
            break;
            /* BUG-24359 Geometry Formfile */
        case SQL_GEOMETRY:
            idlOS::fprintf(m_fpForm, "geometry (%"ID_UINT32_FMT");\n",
                           m_pISPApi->m_Column.GetPrecision(i));
            break;

        default :
            idlOS::printf("%s [Unknown Attribute Type]\n", pName);
            return SQL_FALSE;
        }
    }
    idlOS::fprintf(m_fpForm, "}\n");

    if ( sDateFlag == 1)
    {
        // fix CASE-4061
        /* BUG-32617 DATEFORM should be modified to YYYY/MM/DD HH:MI:SS:SSSSSS */
        idlOS::fprintf( m_fpForm, "DATEFORM YYYY/MM/DD HH:MI:SS:SSSSSS\n" );
    }

    //===============================================================
    // proj1778 nchar
    // syntax 정의에 의해 formout 파일 맨 뒤에 기록한다
    idlOS::fprintf(m_fpForm, "DATA_NLS_USE=%s\n", m_pProgOption->GetNLS());
    // if nchar type column exist, then write this
    if (sNCharFlag == 1)
    {
        idlOS::fprintf(m_fpForm, "NCHAR_UTF16=YES\n");
    }
    //===============================================================

    /* BUG-30467 */
    if ( m_pProgOption->mPartition == ILO_TRUE )
    {
        idlOS::fprintf( m_fpForm, "PARTITION %s\n", m_pISPApi->m_Column.m_ArrayPartName[aPartitionCnt]);
    }

    return SQL_TRUE;
}

SInt iloFormDown::WriteColumnsAsStruct()
{
    SChar *pName;
    SChar  sTableName[MAX_OBJNAME_LEN];
    SChar  sColName[MAX_OBJNAME_LEN];
    SInt   i;
    UInt   sBitPartSize;

    /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
    /* structout 하여 form파일을 만들경우, 대소문자를 구분하여 변수이름을 지정해준다.
     * ex) create table "t1" (i1 integer, "i2" integer);
     * structout form file  ==>   typedef struct t1_STRUCT
                                  {
                                     SQLINTEGER I1;
                                     SQLINTEGER i2;
                                  } T1_STRUCT;
     */
    utString::makeNameInSQL( sTableName,
                             MAX_OBJNAME_LEN,
                             m_pProgOption->m_TableName[0],
                             idlOS::strlen(m_pProgOption->m_TableName[0]) );
    idlOS::fprintf(m_fpForm, "typedef struct %s_STRUCT\n",
                   sTableName);
    idlOS::fprintf(m_fpForm, "{\n");
    for (i=0; i < m_pISPApi->m_Column.GetSize(); i++)
    {
        m_pISPApi->m_Column.GetTransName(i, sColName, (UInt)MAX_OBJNAME_LEN);

        pName = sColName;

        switch (m_pISPApi->m_Column.GetType(i))
        {
        case SQL_DOUBLE :
        case SQL_FLOAT :
            idlOS::fprintf(m_fpForm, "double %s;\n", pName);
            break;
        case SQL_NUMERIC :
        case SQL_DECIMAL :
            idlOS::fprintf(m_fpForm, "double %s;\n", pName);
            break;
        case SQL_VARCHAR :
        case SQL_CHAR :
            idlOS::fprintf(m_fpForm, "char %s[%"ID_UINT32_FMT"+1];\n",
                           pName,
                           m_pISPApi->m_Column.GetPrecision(i));
            break;
        case SQL_SMALLINT :
            idlOS::fprintf(m_fpForm, "SQLSMALLINT %s;\n", pName);
            break;
        case SQL_BIGINT :
            idlOS::fprintf(m_fpForm, "SQLBIGINT %s;\n", pName);
            break;
        case SQL_REAL :
            idlOS::fprintf(m_fpForm, "float %s;\n", pName);
            break;
        case SQL_INTEGER :
            idlOS::fprintf(m_fpForm, "SQLINTEGER %s;\n", pName);
            break;
        case SQL_NIBBLE :
            idlOS::fprintf(m_fpForm, "unsigned char %s[%"ID_UINT32_FMT"]; "
                           "/* use SES_NIBBLE %s[%"ID_UINT32_FMT"] for SESC */\n",
                           pName,
                           m_pISPApi->m_Column.GetPrecision(i),
                           pName,
                           m_pISPApi->m_Column.GetPrecision(i));
            break;
        case SQL_BYTES :
        case SQL_VARBYTE :
        case SQL_NATIVE_TIMESTAMP :
            idlOS::fprintf(m_fpForm, "unsigned char %s[%"ID_UINT32_FMT"]; "
                           "/* use SES_BYTES %s[%"ID_UINT32_FMT"] for SESC */\n",
                           pName,
                           m_pISPApi->m_Column.GetPrecision(i),
                           pName,
                           m_pISPApi->m_Column.GetPrecision(i));
            break;
        case SQL_BINARY :
        case SQL_BLOB :
        case SQL_BLOB_LOCATOR :
            idlOS::fprintf(m_fpForm, "unsigned char *%s; "
                           "/* use SES_BLOB *%s for SESC */\n", pName, pName);
            break;
        case SQL_CLOB :
        case SQL_CLOB_LOCATOR :
            idlOS::fprintf(m_fpForm, "char *%s; "
                           "/* use SES_CLOB *%s for SESC */\n", pName, pName);
            break;
        case SQL_DATE :
        case SQL_TIMESTAMP :
            idlOS::fprintf(m_fpForm, "SQL_TIMESTAMP_STRUCT %s;\n",
                           pName);
            break;
        case SQL_BIT:
        case SQL_VARBIT:
            if (m_pISPApi->m_Column.GetPrecision(i) == 0)
            {
                sBitPartSize = 0;
            }
            else
            {
                sBitPartSize = (m_pISPApi->m_Column.GetPrecision(i) - 1) / 8 + 1;
            }
            idlOS::fprintf(m_fpForm, "unsigned char %s[4+%"ID_UINT32_FMT"]; "
                          "/* use SES_BIT %s[4+%"ID_UINT32_FMT"] for SESC */\n",
                           pName, sBitPartSize, pName, sBitPartSize);
            break;
            /* BUB-24359  Geometry Formfile */
        case SQL_GEOMETRY:
            idlOS::fprintf(m_fpForm, "unsigned char *%s; "
                           "/* Geometry type */\n", pName);
            break;

        default :
            idlOS::printf("%s [Unknown Attribute Type]\n", pName);
            return SQL_FALSE;
        }
    }
    idlOS::fprintf(m_fpForm, "} %s_STRUCT;\n",
                   sTableName);

    return SQL_TRUE;
}
