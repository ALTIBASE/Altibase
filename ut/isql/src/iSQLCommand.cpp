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
 * $Id: iSQLCommand.cpp 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#include <ideErrorMgr.h>
#include <utString.h>
#include <iSQLProperty.h>
#include <iSQLCommandQueue.h>
#include <iSQLCommand.h>

extern iSQLProperty         gProperty;
extern iSQLCommandQueue   * gCommandQueue;

iSQLCommand::iSQLCommand()
{
    if ( (m_Query = (SChar*)idlOS::malloc(gProperty.GetCommandLen())) == NULL )
    {
        idlOS::fprintf(stderr, "Memory allocation error!!! --- (%d, %s)\n", __LINE__, __FILE__);
        Exit(0);
    }
    idlOS::memset(m_Query, 0x00, gProperty.GetCommandLen());

    if ( (m_CommandStr = (SChar*)idlOS::malloc(gProperty.GetCommandLen())) == NULL )
    {
        idlOS::fprintf(stderr, "Memory allocation error!!! --- (%d, %s)\n", __LINE__, __FILE__);
        Exit(0);
    }
    idlOS::memset(m_CommandStr, 0x00, gProperty.GetCommandLen());

    /* BUG-37166 isql does not consider double quotation
     * when it parses stored procedure's arguments */
    if ( (m_ArgList = (SChar*)idlOS::malloc(gProperty.GetCommandLen())) == NULL )
    {
        idlOS::fprintf(stderr, "Memory allocation error!!! --- (%d, %s)\n", __LINE__, __FILE__);
        Exit(0);
    }
    idlOS::memset(m_ArgList, 0x00, gProperty.GetCommandLen());

    reset();
}

iSQLCommand::~iSQLCommand()
{
    if ( m_Query != NULL )
    {
        idlOS::free(m_Query);
        m_Query = NULL;
    }

    if ( m_CommandStr != NULL )
    {
        idlOS::free(m_CommandStr);
        m_CommandStr = NULL;
    }
    
    /* BUG-37166 isql does not consider double quotation
     * when it parses stored procedure's arguments */
    if ( m_ArgList != NULL )
    {
        idlOS::free(m_ArgList);
        m_ArgList = NULL;
    }
}

void
iSQLCommand::reset()
{
    m_CommandKind     = NON_COM;
    m_HelpKind        = NON_COM;
    m_iSQLOptionKind  = iSQL_NON;
    m_ChangeKind      = NON_COMMAND;

    if (m_Query != NULL)
    {
        m_Query[0] = '\0';
    }
    if (m_CommandStr != NULL)
    {
        m_CommandStr[0] = '\0';
    }
    
    /* BUG-37166 isql does not consider double quotation
     * when it parses stored procedure's arguments */
    if (m_ArgList != NULL)
    {
        m_ArgList[0] = '\0';
    }
    idlOS::memset(m_ShellCommand, 0x00, ID_SIZEOF(m_ShellCommand));
    idlOS::memset(m_FileName, 0x00, ID_SIZEOF(m_FileName));
    idlOS::memset(m_UserName, 0x00, ID_SIZEOF(m_UserName));
    idlOS::memset(m_Passwd, 0x00, ID_SIZEOF(m_Passwd));
    idlOS::memset(m_NlsUse, 0x00, ID_SIZEOF(m_NlsUse));
    idlOS::memset(m_TableName, 0x00, ID_SIZEOF(m_TableName));
    idlOS::memset(m_ProcName, 0x00, ID_SIZEOF(m_ProcName));
    idlOS::memset(m_PkgName, 0x00, ID_SIZEOF(m_PkgName));           /* BUG-37002 */
    idlOS::memset(m_HostVarName, 0x00, ID_SIZEOF(m_HostVarName));
    idlOS::memset(m_OldStr, 0x00, ID_SIZEOF(m_OldStr));
    idlOS::memset(m_NewStr, 0x00, ID_SIZEOF(m_NewStr));
    idlOS::memset(mHostVarScale, 0x00, ID_SIZEOF(mHostVarScale));
    idlOS::memset(mHostVarValue, 0x00, ID_SIZEOF(mHostVarValue));

    m_Timescale    = iSQL_SEC;
    m_OnOff        = ID_FALSE;
    m_Colsize      = 0;
    m_Linesize     = 80;
    m_Pagesize     = 0;
    m_HistoryNo    = 0;
    m_ChangeNo     = 0;
    m_ReturnClause = ID_FALSE; /* PROJ-1584 DML Return Clause */
    mHostVarType   = iSQL_BAD;
    mHostVarPrecision = 0;
    mHostInOutType = 0;

    /* BUG-44613 */
    m_AsyncPrefetch = ASYNCPREFETCH_OFF;
    m_PrefetchRows  = 0;

    /* BUG-40246 column format */
    idlOS::memset(mColumnName, 0x00, ID_SIZEOF(mColumnName));
    idlOS::memset(mFormatStr, 0x00, ID_SIZEOF(mFormatStr));
}

void
iSQLCommand::setAll( iSQLCommand * a_SrcCommand,
                     iSQLCommand * a_DesCommand )
{
    a_DesCommand->m_CommandKind    = a_SrcCommand->m_CommandKind;
    a_DesCommand->m_HelpKind       = a_SrcCommand->m_HelpKind;
    a_DesCommand->m_iSQLOptionKind = a_SrcCommand->m_iSQLOptionKind;

    idlOS::strcpy(a_DesCommand->m_Query, a_SrcCommand->m_Query);
    idlOS::strcpy(a_DesCommand->m_CommandStr, a_SrcCommand->m_CommandStr);
    
    /* BUG-37166 isql does not consider double quotation
     * when it parses stored procedure's arguments */
    idlOS::strcpy(a_DesCommand->m_ArgList, a_SrcCommand->m_ArgList);
    idlOS::strcpy(a_DesCommand->m_ShellCommand, a_SrcCommand->m_ShellCommand);

    a_DesCommand->m_OnOff = a_SrcCommand->m_OnOff;

    idlOS::strcpy(a_DesCommand->m_FileName, a_SrcCommand->m_FileName);
    idlOS::strcpy(a_DesCommand->m_UserName, a_SrcCommand->m_UserName);
    idlOS::strcpy(a_DesCommand->m_Passwd, a_SrcCommand->m_Passwd);
    idlOS::strcpy(a_DesCommand->m_NlsUse, a_SrcCommand->m_NlsUse);
    idlOS::strcpy(a_DesCommand->m_TableName, a_SrcCommand->m_TableName);

    a_DesCommand->m_Linesize  = a_SrcCommand->m_Linesize;
    a_DesCommand->m_Pagesize  = a_SrcCommand->m_Pagesize;
    a_DesCommand->m_HistoryNo = a_SrcCommand->m_HistoryNo;
    a_DesCommand->m_Timescale = a_SrcCommand->m_Timescale;

    idlOS::strcpy(a_DesCommand->m_OldStr, a_SrcCommand->m_OldStr);
    idlOS::strcpy(a_DesCommand->m_NewStr, a_SrcCommand->m_NewStr);
    a_DesCommand->m_ChangeNo   = a_SrcCommand->m_ChangeNo;
    a_DesCommand->m_ChangeKind = a_SrcCommand->m_ChangeKind;

    idlOS::strcpy(a_DesCommand->m_ProcName, a_SrcCommand->m_ProcName);
    idlOS::strcpy(a_DesCommand->m_PkgName, a_SrcCommand->m_PkgName);        /* BUG-37002*/
    idlOS::strcpy(a_DesCommand->m_HostVarName, a_SrcCommand->m_HostVarName);
}

IDE_RC
iSQLCommand::SetQuery( SChar * a_Query )
{
    SChar * tmp;

    IDE_TEST(a_Query == NULL);

    tmp = idlOS::strrchr(a_Query, ';');

    IDE_TEST(tmp == NULL);

    *tmp = '\0';
    idlOS::strcpy(m_Query, a_Query);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    m_Query[0] = '\0';

    return IDE_FAILURE;
}

void
iSQLCommand::SetQueryStr( const SChar * a_Query )
{
    idlOS::strcpy(m_Query, a_Query);
}
void
iSQLCommand::SetCommandStr( SChar * a_CommandStr )
{
    if (a_CommandStr != NULL)
    {
        idlOS::strcpy(m_CommandStr, a_CommandStr);
    }
    else
    {
        m_CommandStr[0] = '\0';
    }
}

void
iSQLCommand::SetCommandStr( SChar * a_CommandStr1,
                            SChar * a_CommandStr2 )
{
    if (a_CommandStr1 != NULL)
    {
        idlOS::strcpy(m_CommandStr, a_CommandStr1);
    }
    else
    {
        m_CommandStr[0] = '\0';
    }
    if (a_CommandStr2 != NULL)
    {
        idlOS::strcat(m_CommandStr, " ");
        idlOS::strcat(m_CommandStr, a_CommandStr2);
    }
}

/* BUG-37166 isql does not consider double quotation
 * when it parses stored procedure's arguments */
void
iSQLCommand::SetArgList( SChar * a_ArgList )
{
    if (a_ArgList != NULL)
    {
        idlOS::strcpy(m_ArgList, a_ArgList);
    }
    else
    {
        m_ArgList[0] = '\0';
    }
}


void
iSQLCommand::SetOtherCommand( SChar * a_OtherCommandStr )
{
    SInt i;
    SInt nLen;

    nLen = idlOS::strlen(a_OtherCommandStr);

    for (i=0; i<nLen; i++)
    {
        if (a_OtherCommandStr[i] == ';')
        {
            m_CommandStr[i] = '\0';
            SetQuery(m_CommandStr);
            m_CommandStr[i] = a_OtherCommandStr[i];
            m_CommandStr[i+1] = '\0';
            m_CommandKind = OTHER_COM;
            return;
        }
        m_CommandStr[i] = a_OtherCommandStr[i];
    }

    m_CommandKind = NON_COM;
}

void
iSQLCommand::SetOnOff( const SChar * a_OnOff )
{
    if (a_OnOff != NULL)
    {
        if ( idlOS::strcasecmp(a_OnOff, "on") == 0 )
        {
            m_OnOff = ID_TRUE;
        }
        else
        {
            m_OnOff = ID_FALSE;
        }
    }
}

void
iSQLCommand::SetPathType( iSQLPathType   a_PathType )
{
    m_PathType = a_PathType;
}

void
iSQLCommand::SetFileName( SChar        * a_FileName,
                          iSQLPathType   a_PathType )
{
    if (a_FileName != NULL)
    {
        utString::eraseWhiteSpace(a_FileName);
        idlOS::strcpy(m_FileName, a_FileName);
    }
    else
    {
        m_FileName[0] = '\0';
    }
    m_PathType = a_PathType;
}

/* BUG-34502: handling quoted identifiers */
void
iSQLCommand::SetQuotedFileName( SChar        * a_FileName,
                                iSQLPathType   a_PathType )
{
    SInt sFileLen = 0;

    if (a_FileName != NULL)
    {
        idlOS::strcpy(m_FileName, a_FileName + 1);

        // quotation marks 제거한 길이만큼 ..
        sFileLen = idlOS::strlen(a_FileName) - 2;
        m_FileName[sFileLen] = '\0';
    }
    else
    {
        m_FileName[0] = '\0';
    }
    m_PathType = a_PathType;
}

void
iSQLCommand::setUserName( SChar * aUserName )
{
    if (aUserName != NULL)
    {
        utString::eraseWhiteSpace(aUserName);

        // To Fix BUG-17430
        utString::makeNameInCLI( m_UserName,
                                 ID_SIZEOF(m_UserName),
                                 aUserName,
                                 idlOS::strlen(aUserName) );
    }
    else
    {
        m_UserName[0] = '\0';
    }
}

void
iSQLCommand::SetUserCert( SChar * a_UserCert )
{
    SInt len;
    
    if (a_UserCert != NULL)
    {
        // BUG-9020
        if (a_UserCert[0] == '"')
        {
            len = idlOS::strlen(a_UserCert);
            idlOS::strcpy(m_UserCert, a_UserCert+1);
            m_UserCert[len - 2] = '\0';
        }
        else
        {
            idlOS::strcpy(m_UserCert, a_UserCert);
        }
    }
    else
    {
        m_UserCert[0] = '\0';
    }
}

void
iSQLCommand::SetUnixdomainFilepath( SChar * a_UnixdomainFilepath )
{
    SInt len;
    
    if (a_UnixdomainFilepath != NULL)
    {
        // BUG-9020
        if (a_UnixdomainFilepath[0] == '"')
        {
            len = idlOS::strlen(a_UnixdomainFilepath);
            idlOS::strcpy(m_UnixdomainFilepath, a_UnixdomainFilepath+1);
            m_UnixdomainFilepath[len - 2] = '\0';
        }
        else
        {
            idlOS::strcpy(m_UnixdomainFilepath, a_UnixdomainFilepath);
        }
    }
    else
    {
        m_UnixdomainFilepath[0] = '\0';
    }
}

void
iSQLCommand::SetIpcFilepath( SChar * a_IpcFilepath )
{
    SInt len;

    if (a_IpcFilepath != NULL)
    {
        // BUG-9020
        if (a_IpcFilepath[0] == '"')
        {
            len = idlOS::strlen(a_IpcFilepath);
            idlOS::strcpy(m_IpcFilepath, a_IpcFilepath+1);
            m_IpcFilepath[len - 2] = '\0';
        }
        else
        {
            idlOS::strcpy(m_IpcFilepath, a_IpcFilepath);
        }
    }
    else
    {
        m_IpcFilepath[0] = '\0';
    }
}

void
iSQLCommand::SetUserKey( SChar * a_UserKey )
{
    SInt len;
    
    if (a_UserKey != NULL)
    {
        // BUG-9020
        if (a_UserKey[0] == '"')
        {
            len = idlOS::strlen(a_UserKey);
            idlOS::strcpy(m_UserKey, a_UserKey+1);
            m_UserKey[len - 2] = '\0';
        }
        else
        {
            idlOS::strcpy(m_UserKey, a_UserKey);
        }
    }
    else
    {
        m_UserKey[0] = '\0';
    }
}

void
iSQLCommand::SetUserAID( SChar * a_UserAID )
{
    SInt len;
    
    if (a_UserAID != NULL)
    {
        // BUG-9020
        if (a_UserAID[0] == '"')
        {
            len = idlOS::strlen(a_UserAID);
            idlOS::strcpy(m_UserAID, a_UserAID+1);
            m_UserAID[len - 2] = '\0';
        }
        else
        {
            idlOS::strcpy(m_UserAID, a_UserAID);
        }
    }
    else
    {
        m_UserAID[0] = '\0';
    }
}

void
iSQLCommand::SetPasswd( SChar * a_Passwd )
{
    if (a_Passwd != NULL)
    {
        idlOS::snprintf(m_CaseSensitivePasswd, ID_SIZEOF(m_CaseSensitivePasswd), "%s", a_Passwd);
        idlOS::strcpy(m_Passwd, a_Passwd);
    }
    else
    {
        m_Passwd[0] = '\0';
    }
}

/* BUG-27155 */
void
iSQLCommand::SetNlsUse( SChar * aNlsUse )
{
    if (aNlsUse != NULL)
    {
        idlOS::snprintf(m_NlsUse, ID_SIZEOF(m_NlsUse), "%s", aNlsUse);
    }
    else
    {
        m_NlsUse[0] = '\0';
    }
}

void
iSQLCommand::setTableName( SChar * aTableName )
{
    if (aTableName != NULL)
    {
        utString::eraseWhiteSpace( aTableName );

        // To Fix BUG-17430
        utString::makeNameInCLI( m_TableName,
                                 ID_SIZEOF(m_TableName),
                                 aTableName,
                                 idlOS::strlen(aTableName) );
    }
    else
    {
        m_TableName[0] = '\0';
    }
}

void
iSQLCommand::SetChangeCommand( SChar * a_ChangeCommand )
{
    SChar  tmp[WORD_LEN];
    SChar *pos1 = '\0';
    SChar *pos2 = '\0';

    idlOS::strcpy(tmp, a_ChangeCommand);
    pos1 = idlOS::strchr(tmp, '/');
    if (pos1 != NULL)
    {
        *pos1 = '\0';
        SetChangeNo(tmp);
        pos2 = idlOS::strchr(pos1+1, '/');
    }

    if (pos2 != NULL)
    {
        *pos2 = '\0';
        SetNewStr(pos2+1);
        SetOldStr(pos1+1);
    }
    else
    {
        SetNewStr(pos2);
        SetOldStr(pos1+1);
    }
}

void
iSQLCommand::SetOldStr( SChar * a_OldStr )
{
    if (a_OldStr != NULL)
    {
        if ( idlOS::strcasecmp(a_OldStr, "^") == 0 )
        {
            SetChangeKind(FIRST_ADD_COMMAND);
        }
        else if ( idlOS::strcasecmp(a_OldStr, "$") == 0 )
        {
            SetChangeKind(LAST_ADD_COMMAND);
        }
        else
        {
            idlOS::strcpy(m_OldStr, a_OldStr);
        }
    }
    else
    {
        m_OldStr[0] = '\0';
    }
}

void
iSQLCommand::SetNewStr( SChar * a_NewStr )
{
    SChar *pos;

    if (a_NewStr != NULL)
    {
        pos = idlOS::strrchr(a_NewStr, '\n');
        if (pos)
        {
            *pos = '\0';
        }
        idlOS::strcpy(m_NewStr, a_NewStr);
        SetChangeKind(CHANGE_COMMAND);
    }
    else
    {
        SetChangeKind(DELETE_COMMAND);
        m_NewStr[0] = '\0';
    }
}

void
iSQLCommand::SetChangeNo( SChar * a_ChangeNo )
{
    if (a_ChangeNo != NULL)
    {
        if ( idlOS::strcasecmp(a_ChangeNo, "C") == 0 )
        {
            m_ChangeNo = gCommandQueue->GetCurHisNum()-1;
        }
        else
        {
            m_ChangeNo = atoi(a_ChangeNo);
        }
    }
}

void
iSQLCommand::SetShellCommand( SChar * a_ShellCommand )
{
    if (a_ShellCommand != NULL)
    {
        idlOS::strcpy(m_ShellCommand, a_ShellCommand);
    }
    else
    {
        m_ShellCommand[0] = '\0';
    }
}

void
iSQLCommand::SetLoboffset( SChar * a_Loboffset )
{
    if (a_Loboffset != NULL)
    {
        m_Loboffset = atoi(a_Loboffset);
    }
}

void
iSQLCommand::SetLobsize( SChar * a_Lobsize )
{
    if (a_Lobsize != NULL)
    {
        m_Lobsize = atoi(a_Lobsize);
    }
}

void
iSQLCommand::SetColsize( SChar * a_Colsize )
{
    if (a_Colsize != NULL)
    {
        m_Colsize = atoi(a_Colsize);
    }
}

void
iSQLCommand::SetFeedback( SChar * aFeedback )
{
    if (aFeedback != NULL)
    {
        if (idlOS::strCaselessMatch("ON", 2, aFeedback, 2) == 0)
        {
            m_Feedback = 1;
        }
        else if (idlOS::strCaselessMatch("OFF", 3, aFeedback, 3) == 0)
        {
            m_Feedback = 0;
        }
        else
        {
            m_Feedback = atoi(aFeedback);
        }
    }
    else
    {
        m_Feedback = 1;
    }
}

void
iSQLCommand::SetLinesize( SChar * a_Linesize )
{
    if (a_Linesize != NULL)
    {
        m_Linesize = atoi(a_Linesize);
    }
}

/* BUG-39213 Need to support SET NUMWIDTH in isql */
void
iSQLCommand::SetNumWidth( SChar * a_NumWidth )
{
    if (a_NumWidth != NULL)
    {
        m_NumWidth = atoi(a_NumWidth);
    }
}

void
iSQLCommand::SetPagesize( SChar * a_Pagesize )
{
    if (a_Pagesize != NULL)
    {
        m_Pagesize = atoi(a_Pagesize);
    }
}

void
iSQLCommand::SetHistoryNo( SChar * a_HistoryNo )
{
    if (a_HistoryNo != NULL)
    {
        m_HistoryNo = atoi(a_HistoryNo);
    }
}

/* BUG-37002 isql cannot parse package as a assigned variable */
void
iSQLCommand::SetPkgName( SChar * aPkgName )
{
    if (aPkgName != NULL)
    {
        utString::eraseWhiteSpace(aPkgName);
        utString::makeNameInCLI( m_PkgName,
                                 ID_SIZEOF(m_PkgName),
                                 aPkgName,
                                 idlOS::strlen(aPkgName));
    }
    else
    {
        m_PkgName[0] = '\0';
    }
}

void
iSQLCommand::setProcName( SChar * aProcName )
{
    if (aProcName != NULL)
    {
        utString::eraseWhiteSpace(aProcName);
        
        // To Fix BUG-17430
        utString::makeNameInCLI( m_ProcName,
                                 ID_SIZEOF(m_ProcName),
                                 aProcName,
                                 idlOS::strlen(aProcName));
    }
    else
    {
        m_ProcName[0] = '\0';
    }
}

void
iSQLCommand::SetHostVarName( SChar * a_HostVarName )
{
    if (a_HostVarName != NULL)
    {
        utString::eraseWhiteSpace(a_HostVarName);
        utString::toUpper(a_HostVarName);
        idlOS::strcpy(m_HostVarName, a_HostVarName);
    }
    else
    {
        m_HostVarName[0] = '\0';
    }
}

/* BUG-41163 SET SQLP[ROMPT] */
IDE_RC
iSQLCommand::SetSqlPrompt( SChar * aSqlPrompt )
{
    UInt sLen;

    sLen = idlOS::strlen(aSqlPrompt);
    IDE_TEST(sLen > SQL_PROMPT_MAX);

    idlOS::strcpy(mSqlPrompt, aSqlPrompt);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    uteSetErrorCode(&gErrorMgr,
                    utERR_ABORT_Exceed_Max_String,
                    SQL_PROMPT_MAX);

    return IDE_FAILURE;
}

/* BUG-41817 For Host Variables */
void
iSQLCommand::SetHostVarScale( SChar * aHostVarScale )
{
    if (aHostVarScale != NULL)
    {
        idlOS::strcpy(mHostVarScale, aHostVarScale);
    }
    else
    {
        mHostVarScale[0] = '\0';
    }
}

void
iSQLCommand::SetHostVarValue( SChar * aHostVarValue )
{
    if (aHostVarValue != NULL)
    {
        idlOS::strcpy(mHostVarValue, aHostVarValue);
    }
    else
    {
        mHostVarValue[0] = '\0';
    }
}

/* BUG-40246 column format */
void
iSQLCommand::SetColumnName( SChar *aName )
{
    if (aName != NULL)
    {
        idlOS::strcpy(mColumnName, aName);
    }
    else
    {
        mColumnName[0] = '\0';
    }
}

void
iSQLCommand::SetFormatStr( SChar *aFmt )
{
    if (aFmt != NULL)
    {
        idlOS::strcpy(mFormatStr, aFmt);
    }
    else
    {
        mFormatStr[0] = '\0';
    }
}

void
iSQLCommand::EnableColumn( SChar *aName )
{
    if (aName != NULL)
    {
        idlOS::strcpy(mColumnName, aName);
        m_OnOff = ID_TRUE;
    }
    else
    {
        mColumnName[0] = '\0';
    }
}

void
iSQLCommand::DisableColumn( SChar *aName )
{
    if (aName != NULL)
    {
        idlOS::strcpy(mColumnName, aName);
        m_OnOff = ID_FALSE;
    }
    else
    {
        mColumnName[0] = '\0';
    }
}

/* BUG-44613 Set PrefetchRows */
IDE_RC
iSQLCommand::SetPrefetchRows( SChar * a_PrefetchRows )
{
    SLong sPrefetchRows = 0;

    if (a_PrefetchRows != NULL)
    {
        sPrefetchRows = idlOS::strtol(a_PrefetchRows,
                                       (SChar **)NULL,
                                       10);

        IDE_TEST_RAISE( sPrefetchRows < 0 ||
                        sPrefetchRows > ACP_SINT32_MAX, range_error );

        m_PrefetchRows = sPrefetchRows;
    }
    return IDE_SUCCESS;

    IDE_EXCEPTION(range_error);
    {
        uteSetErrorCode(&gErrorMgr,
                        utERR_ABORT_PrefetchRows_Error,
                        (UInt)0, ACP_SINT32_MAX);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* BUG-44613 Set AsyncPrefetch On|Auto|Off */
void
iSQLCommand::SetAsyncPrefetch( AsyncPrefetchType aType )
{
    m_AsyncPrefetch = aType;
}
