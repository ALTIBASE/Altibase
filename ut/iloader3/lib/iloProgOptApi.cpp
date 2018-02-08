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
 
#include <ilo.h>

SInt iloInitOpt_v1( SInt aVersion, void *aOptions )
{

    ALTIBASE_ILOADER_OPTIONS_V1 *sOptions = (ALTIBASE_ILOADER_OPTIONS_V1 *) aOptions;
    
    idlOS::memset( sOptions, 0x00, ID_SIZEOF(ALTIBASE_ILOADER_OPTIONS_V1 ));

    sOptions->version         = aVersion;
    sOptions->portNum         = UNKNOWN_PORT_NUM;
    sOptions->dataFileNum     = 0;
    sOptions->firstRow        = DEFAULT_FIRST_ROW;
    sOptions->lastRow         = DEFAULT_LAST_ROW;

    sOptions->useLobFile      = ILO_FALSE;
    sOptions->useSeparateFile = ILO_FALSE;

    idlOS::snprintf( sOptions->lobIndicator, ID_SIZEOF(sOptions->lobIndicator),
                     "%%%%");

    sOptions->replication      = ILO_TRUE;
    sOptions->loadModeType     = ILO_APPEND;

    sOptions->splitRowCount    = DEFAULT_SPLIT_ROW_COUNT;
    sOptions->errorCount       = DEFAULT_ERROR_COUNT;
    sOptions->arrayCount       = DEFAULT_ARRAY_COUNT;
    sOptions->commitUnit       = DEFAULT_COMMIT_UNIT;
    sOptions->atomic           = ILO_FALSE;
    sOptions->directLog        = ILO_DIRECT_NONE;
    sOptions->parallelCount    = DEFAULT_PARALLEL_COUNT;
    sOptions->readSize         = FILE_READ_SIZE_DEFAULT;
    sOptions->informix         = ILO_FALSE;
    sOptions->flock            = ILO_FALSE;
    /* BUG-30413 */
    sOptions->getTotalRowCount = ILO_FALSE;
    sOptions->setRowFrequency  = 0;

    return ALTIBASE_ILO_SUCCESS;
}

SInt iloSetGenOpt_v1( SInt                     aVersion,
                      ALTIBASE_ILOADER_HANDLE  aHandle,
                      void                    *aOptions )
{
    SInt                        i;
    iloaderHandle               *sHandle;

    ALTIBASE_ILOADER_OPTIONS_V1 *sOptions = (ALTIBASE_ILOADER_OPTIONS_V1 *) aOptions;
    
    IDE_DASSERT(aHandle != ALTIBASE_ILOADER_NULL_HANDLE);
    IDE_DASSERT(aOptions != NULL);

    sHandle = (iloaderHandle *) aHandle;

    /* lib version check */
    IDE_TEST_RAISE( CHECK_MAX_VER , err_version );

    /* Options Version check*/
    IDE_TEST_RAISE( sOptions->version != aVersion, err_options);

    /* Options Setting */

    // -s
    IDE_TEST_RAISE( sOptions->loginID[0] == '\0', err_server_error );
    sHandle->mProgOption->m_bExist_S = SQL_TRUE;
    idlOS::strcpy( sHandle->mProgOption->m_LoginID, sOptions->loginID );
    sHandle->mProgOption->StrToUpper(sHandle->mProgOption->m_LoginID);

    // -p
    IDE_TEST_RAISE( sOptions->password[0] == '\0', err_passwd );
    sHandle->mProgOption->m_bExist_P = SQL_TRUE;
    idlOS::strcpy( sHandle->mProgOption->m_Password, sOptions->password );

    // -u
    IDE_TEST_RAISE( sOptions->serverName[0] == '\0', err_user );
    sHandle->mProgOption->m_bExist_U = SQL_TRUE;
    idlOS::strcpy( sHandle->mProgOption->m_ServerName, sOptions->serverName );

    // -port
    if ( sOptions->portNum != UNKNOWN_PORT_NUM )
    {
        sHandle->mProgOption->m_bExist_PORT = SQL_TRUE;
        sHandle->mProgOption->m_PortNum = sOptions->portNum;
    }

    // lns_use
    if ( sOptions->NLS[0] != '\0' )
    {
        sHandle->mProgOption->m_bExist_NLS = ILO_TRUE;
        idlOS::strncpy( sHandle->mProgOption->m_NLS, sOptions->NLS,
                ID_SIZEOF(sHandle->mProgOption->m_NLS));
    }

    idlOS::strcpy( sHandle->mProgOption->m_DataNLS,
            sHandle->mProgOption->m_NLS);

    // DBName
     idlOS::strncpy( sHandle->mProgOption->m_DBName, sOptions->DBName,
                     ID_SIZEOF(sOptions->DBName));

    // -T
    if ( sHandle->mProgOption->m_CommandType == FORM_OUT )
    {
        IDE_TEST_RAISE( sOptions->tableName[0] == '\0', err_tableName );
        sHandle->mProgOption->m_bExist_T = SQL_TRUE;
        sHandle->mProgOption->m_nTableCount++;
        if (sOptions->tableOwner[0] != '\0')
        {
            sHandle->mProgOption->m_bExist_TabOwner = SQL_TRUE;
            idlOS::strcpy( sHandle->mProgOption->m_TableOwner[0], sOptions->tableOwner );
        }
        idlOS::strcpy( sHandle->mProgOption->m_TableName[0], sOptions->tableName );
    }

    // -f
    IDE_TEST_RAISE( sOptions->formFile[0] == '\0', err_formFile );
    sHandle->mProgOption->m_bExist_f = SQL_TRUE;
    idlOS::strcpy( sHandle->mProgOption->m_FormFile, sOptions->formFile );

    // -d
    if ( sHandle->mProgOption->m_CommandType != FORM_OUT )
    {
        IDE_TEST_RAISE( sOptions->dataFile[0][0] == '\0', err_dataFile );
        IDE_TEST_RAISE( sOptions->dataFileNum > MAX_PARALLEL_COUNT,
                err_dataFile_overflow );

        sHandle->mProgOption->m_bExist_d = SQL_TRUE;
        sHandle->mProgOption->m_DataFileNum = sOptions->dataFileNum;

        for ( i =0; i <  sHandle->mProgOption->m_DataFileNum; i++ )
        {
            idlOS::strcpy( sHandle->mProgOption->m_DataFile[i], sOptions->dataFile[i] );
        }
    }

    // -firstrow
    if ( sOptions->firstRow != DEFAULT_FIRST_ROW )
    {
         sHandle->mProgOption->m_bExist_F = SQL_TRUE;
         sHandle->mProgOption->m_FirstRow = sOptions->firstRow;

         IDE_TEST_RAISE( sHandle->mProgOption->m_FirstRow < 0, err_firstRow )
     }

    // -lastrow
    if ( sOptions->lastRow != DEFAULT_LAST_ROW )
    {
        sHandle->mProgOption->m_bExist_L = SQL_TRUE;
        sHandle->mProgOption->m_LastRow = sOptions->lastRow;

        IDE_TEST_RAISE( sHandle->mProgOption->m_LastRow < 0, err_lastRow )
    }

    // -t
    if ( sOptions->fieldTerm[0] != '\0' )
    {
        IDE_TEST_RAISE( idlOS::strlen(sOptions->fieldTerm) > 10, err_fieldTrem_overflow );

        sHandle->mProgOption->mRule = old;
        sHandle->mProgOption->m_bExist_t = SQL_TRUE;
        idlOS::strcpy(sHandle->mProgOption->m_FieldTerm, sOptions->fieldTerm);
    }

    // -r
    if ( sOptions->rowTerm[0] != '\0' )
    {
        IDE_TEST_RAISE( idlOS::strlen(sOptions->rowTerm) > 10, err_rowRerm_overflow );

        sHandle->mProgOption->mRule = old;
        sHandle->mProgOption->m_bExist_r = SQL_TRUE;
        idlOS::strcpy(sHandle->mProgOption->m_RowTerm, sOptions->rowTerm);
    }

    // -e
    if ( sOptions->enclosingChar[0] != '\0' )
    {
        IDE_TEST_RAISE( idlOS::strlen(sOptions->enclosingChar) > 10, err_enclosingChar_overflow );

        sHandle->mProgOption->mRule = old;
        sHandle->mProgOption->m_bExist_e = SQL_TRUE;
        idlOS::strcpy( sHandle->mProgOption->m_EnclosingChar,
                sOptions->enclosingChar );
    }

    // -lob user_lob_file
    if ( sOptions->useLobFile != ILO_FALSE )
    {
        sHandle->mProgOption->mExistUseLOBFile = ILO_TRUE;
        sHandle->mProgOption->mUseLOBFile = ILO_TRUE;
    }

    // -lob use_separate_files
    if ( sOptions->useSeparateFile != ILO_FALSE )
    {
          sHandle->mProgOption->mUseLOBFile = ILO_TRUE;
          sHandle->mProgOption->mExistUseSeparateFiles = ILO_TRUE;
          sHandle->mProgOption->mUseSeparateFiles = ILO_TRUE;
    }

    // -lob lob_file_size
    if ( sOptions->lobFileSize[0] != '\0' )
    {
        sHandle->mProgOption->mExistLOBFileSize = ILO_TRUE;

        getLobFileSize( sHandle, sOptions->lobFileSize );
    }

    // -lob lob_indicator
    if( idlOS::strcmp(sOptions->lobIndicator, "%%") != 0 )
    {

        IDE_TEST_RAISE( idlOS::strlen(sOptions->lobIndicator) > 10,
                        err_overFlow_lobIndicator );

        sHandle->mProgOption->mExistLOBIndicator = ILO_TRUE;
        idlOS::strcpy( sHandle->mProgOption->mLOBIndicator,
                sOptions->lobIndicator );
    }

    // -replication
    if ( sOptions->replication != ILO_TRUE )
    {
        sHandle->mProgOption->mReplication = ILO_FALSE;
    }

    // -mode
    if ( sOptions->loadModeType != ILO_APPEND )
    {
        sHandle->mProgOption->m_bExist_mode = SQL_TRUE;

        if ( sOptions->loadModeType == ILO_REPLACE )
        {
            sHandle->mProgOption->m_LoadMode = ILO_REPLACE;
        }
        else if ( sOptions->loadModeType == ILO_TRUNCATE )
        {
            sHandle->mProgOption->m_LoadMode = ILO_TRUNCATE;
        }
    }

    // -bad
    if ( sOptions->bad[0] != '\0' )
    {
        sHandle->mProgOption->m_bExist_bad = SQL_TRUE;
        idlOS::strcpy( sHandle->mProgOption->m_BadFile, sOptions->bad );
    }

    // -log
    if ( sOptions->log[0] != '\0' )
    {
        sHandle->mProgOption->m_bExist_log = SQL_TRUE;
        idlOS::strcpy( sHandle->mProgOption->m_LogFile, sOptions->log );
    }

    // -split
    if ( sOptions->splitRowCount != DEFAULT_SPLIT_ROW_COUNT )
    {
        sHandle->mProgOption->m_bExist_split = SQL_TRUE;
        sHandle->mProgOption->m_SplitRowCount = sOptions->splitRowCount;

        IDE_TEST_RAISE( sHandle->mProgOption->m_SplitRowCount <= 0, err_splitRowCount );
    }

    // -errors
    if ( sOptions->errorCount != DEFAULT_ERROR_COUNT )
    {
        sHandle->mProgOption->m_ErrorCount = sOptions->errorCount;

        IDE_TEST_RAISE( sHandle->mProgOption->m_ErrorCount < 0, err_errorCount );
    }

    // -informix
    if ( sOptions->informix != ILO_FALSE )
    {
        sHandle->mProgOption->mInformix = ILO_TRUE;
    }

    // -flock
    if ( sOptions->flock != ILO_FALSE )
    {
        sHandle->mProgOption->mFlockFlag = ILO_TRUE;
    }

    // -mssql
    if ( sOptions->mssql != ILO_FALSE )
    {
        sHandle->mProgOption->mMsSql = SQL_TRUE;
    }

    /* BUG-30413 data file total row */
    if ( sOptions->getTotalRowCount != ILO_FALSE )
    {
        sHandle->mProgOption->mGetTotalRowCount = ILO_TRUE;
        sHandle->mProgOption->mSetRowFrequency = sOptions->setRowFrequency; 

        /* row total count 를 구하는 옵션을 ILO_FALSE 설정
         * DATAIN 수행 하기 위해서 필요
         */
        sOptions->getTotalRowCount =  ILO_FALSE;
    }
    else 
    {
        sHandle->mProgOption->mGetTotalRowCount = ILO_FALSE;
    }

    /* BUG-30413 row frequency */
    if (  sOptions->setRowFrequency != 0 )
    {
        sHandle->mProgOption->mSetRowFrequency = sOptions->setRowFrequency; 
    }

    return ALTIBASE_ILO_SUCCESS;
    
    IDE_EXCEPTION( err_version );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr,
                        utERR_ABORT_LIB_VERSION_Error);
    }

    IDE_EXCEPTION( err_options );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr,
                        utERR_ABORT_OPTION_VERSION_Error);
    }
    IDE_EXCEPTION( err_user );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Dup_Option_Error, "-U");
    }
    IDE_EXCEPTION( err_passwd );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Dup_Option_Error, "-P");
    }
    IDE_EXCEPTION( err_server_error );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Dup_Option_Error, "-S");
    }
    IDE_EXCEPTION( err_tableName );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Dup_Option_Error,
                        "tableName");
    }
    IDE_EXCEPTION( err_formFile );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Option_Incorrect_Error,
                        "variable", "form file name");
    }
    IDE_EXCEPTION( err_dataFile );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Option_Incorrect_Error,
                        "variable", "data file name");
    }
    IDE_EXCEPTION( err_dataFile_overflow );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr,
                        utERR_ABORT_Option_Value_Overflow_Error,
                        "Number of DataFile", (UInt)32);
    }
    IDE_EXCEPTION( err_firstRow );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr,
                        utERR_ABORT_Option_Value_Range_Error,
                        "firstRow", (UInt)0);
    }
    IDE_EXCEPTION( err_lastRow );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr,
                        utERR_ABORT_Option_Value_Range_Error,
                        "lastRow", (UInt)0);
    }
    IDE_EXCEPTION( err_overFlow_lobIndicator );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr,
                        utERR_ABORT_Option_Value_Overflow_Error,
                        "lobIndicator", (UInt)10);
    }
    IDE_EXCEPTION( err_fieldTrem_overflow );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr,
                        utERR_ABORT_Option_Value_Overflow_Error,
                        "fieldTerm", (UInt)10);
    }
    IDE_EXCEPTION( err_rowRerm_overflow );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr,
                        utERR_ABORT_Option_Value_Overflow_Error,
                        "rowTerm", (UInt)10);
    }
    IDE_EXCEPTION( err_enclosingChar_overflow );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr,
                        utERR_ABORT_Option_Value_Overflow_Error,
                        "enclosingChar", (UInt)10);
    }
    IDE_EXCEPTION( err_splitRowCount );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr,
                        utERR_ABORT_Option_Value_Range_Error,
                        "splitRowCount", (UInt)0);
    }
    IDE_EXCEPTION( err_errorCount );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr,
                        utERR_ABORT_Option_Value_Range_Error,
                        "errorCount", (UInt)0);
    }
    IDE_EXCEPTION_END;

    return ALTIBASE_ILO_ERROR;
}

SInt iloSetPerfOpt_v1( SInt                     aVersion,
                       ALTIBASE_ILOADER_HANDLE  aHandle,
                       void                    *aOptions )
{
    iloaderHandle               *sHandle;

    ALTIBASE_ILOADER_OPTIONS_V1 *sOptions = (ALTIBASE_ILOADER_OPTIONS_V1 *) aOptions;
    
    IDE_DASSERT(aHandle != ALTIBASE_ILOADER_NULL_HANDLE);
    IDE_DASSERT(aOptions != NULL);

    sHandle = (iloaderHandle *) aHandle;

    /* Options Version check*/
    IDE_TEST_RAISE( sOptions->version != aVersion, err_options);

    /* Performance Options Setting */

    // -array
    if ( sOptions->arrayCount != DEFAULT_ARRAY_COUNT )
    {
        /* BUG-30413 */
        if (( sHandle->mProgOption->mGetTotalRowCount == ILO_TRUE ) &&
            (sHandle->mProgOption->m_CommandType != DATA_OUT))
        {
            sHandle->mProgOption->m_bExist_array = SQL_FALSE;
            sHandle->mProgOption->m_ArrayCount = DEFAULT_ARRAY_COUNT;
        }
        else
        {
            sHandle->mProgOption->m_bExist_array = SQL_TRUE;
            sHandle->mProgOption->m_ArrayCount = sOptions->arrayCount;

            IDE_TEST_RAISE( sHandle->mProgOption->m_ArrayCount <= 0,
                    err_overFlow_arrayCount );
        }
    }

    // -commit
    if ( sOptions->commitUnit != DEFAULT_COMMIT_UNIT )
    {
        sHandle->mProgOption->m_bExist_commit = SQL_TRUE;
        sHandle->mProgOption->m_CommitUnit = sOptions->commitUnit;

        IDE_TEST_RAISE( sHandle->mProgOption->m_CommitUnit < 0,
                        err_overFlow_commitUnit );
    }

    // -atomic
    if ( sOptions->atomic != ILO_FALSE )
    {
        sHandle->mProgOption->m_bExist_atomic   = SQL_TRUE;
    }

    // -direct
    if ( sOptions->directLog == ILO_DIRECT_LOG )
    {
        sHandle->mProgOption->m_bExist_direct = SQL_TRUE;
        sHandle->mProgOption->m_directLogging = SQL_TRUE;
    }
    else if ( sOptions->directLog == ILO_DIRECT_NOLOG )
    {
        sHandle->mProgOption->m_bExist_direct = SQL_TRUE;
        sHandle->mProgOption->m_directLogging = SQL_FALSE;
    }
   
    // -parallel
    if ( sOptions->parallelCount != DEFAULT_PARALLEL_COUNT )
    {
        /* BUG-30413 */
        if (( sHandle->mProgOption->mGetTotalRowCount == ILO_TRUE ) &&
            (sHandle->mProgOption->m_CommandType != DATA_OUT))
        {
            sHandle->mProgOption->m_bExist_parallel = SQL_FALSE;
            sHandle->mProgOption->m_ParallelCount = DEFAULT_PARALLEL_COUNT;
        }
        else
        {
            sHandle->mProgOption->m_bExist_parallel = SQL_TRUE;
            sHandle->mProgOption->m_ParallelCount = sOptions->parallelCount;

            IDE_TEST_RAISE( sHandle->mProgOption->m_ParallelCount <= 0 ||
                    sHandle->mProgOption->m_ParallelCount > MAX_PARALLEL_COUNT,
                    err_overFlow_parallelCount );
        }
    }

    // -readsize
    if ( sOptions->readSize != FILE_READ_SIZE_DEFAULT )
    {
        sHandle->mProgOption->mReadSzie = sOptions->readSize;

        IDE_TEST_RAISE( sHandle->mProgOption->mReadSzie <= 0,
                        err_overFlow_readSize );
    }

    return ALTIBASE_ILO_SUCCESS;
    
    IDE_EXCEPTION( err_options );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr,
                        utERR_ABORT_OPTION_VERSION_Error);
    }
    IDE_EXCEPTION( err_overFlow_arrayCount );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr,
                        utERR_ABORT_Option_Value_Range_Error,
                        "arrayCount", (UInt)0);
    }
    IDE_EXCEPTION( err_overFlow_commitUnit );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr,
                        utERR_ABORT_Option_Value_Range_Error,
                        "commitUnit", (UInt)0);
    }
    IDE_EXCEPTION( err_overFlow_parallelCount );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr,
                        utERR_ABORT_Option_Value_Overflow_Error,
                        "parallelCount", (UInt)32);
    }
    IDE_EXCEPTION( err_overFlow_readSize );
    {
        sHandle->mProgOption->m_bErrorExist = SQL_TRUE;
        uteSetErrorCode(sHandle->mErrorMgr,
                        utERR_ABORT_Option_Value_Range_Error,
                        "-readsize", (UInt)0);
    }
    IDE_EXCEPTION_END;

    return ALTIBASE_ILO_ERROR;
}


