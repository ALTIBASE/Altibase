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
 * $Id: iSQLProperty.cpp 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#include <iSQL.h>
#include <utString.h>
#include <utISPApi.h>
#include <iSQLSpool.h>
#include <iSQLProperty.h>
#include <iSQLExecuteCommand.h>
#include <iSQLCommand.h>
#include <isqlFloat.h>

extern iSQLSpool          * gSpool;
extern iSQLCommand        * gCommand;
extern iSQLExecuteCommand * gExecuteCommand;
extern iSQLProperty         gProperty;

/* BUG-41163 SET SQLP{ROMPT} */
extern SInt lexSqlPrompt(const SChar *aSqlPrompt,
                         SChar       *aNewPromptBuf,
                         UInt        *aPromptRefreshFlag);

iSQLProperty::iSQLProperty()
{
    m_ColSize              = 0;
    m_LineSize             = 80;
    m_PageSize             = 0;
    m_Feedback             = 1;
    m_Term                 = ID_TRUE;
    m_Timing               = ID_FALSE;
    m_Vertical             = ID_FALSE; // BUG-22685
    m_Heading              = ID_TRUE;
    m_ShowCheckConstraints = ID_FALSE; /* PROJ-1107 Check Constraint 지원 */
    m_ShowForeignKeys      = ID_FALSE;
    m_ShowPartitions       = ID_FALSE; /* BUG-43516 */
    m_PlanCommit           = ID_FALSE;
    m_QueryLogging         = ID_FALSE;
    mIsSysDBA              = ID_FALSE;
    mIsConnToRealInstance  = ID_FALSE;
    m_TimeScale            = iSQL_SEC;
    mExplainPlan           = EXPLAIN_PLAN_OFF;
    m_LobOffset            = 0;
    m_LobSize              = 80;
    m_NumWidth             = 11;       // BUG-39213 Need to support SET NUMWIDTH in isql
    mEcho                  = ID_FALSE;
    mFullName              = ID_FALSE;
    m_Define               = ID_FALSE;
    m_Verify               = ID_TRUE; /* BUG-43599 */

    /* BUG-44613 */
    m_PrefetchRows         = 0;
    m_AsyncPrefetch        = ASYNCPREFETCH_OFF;

    idlOS::memset(m_UserName, 0x00, ID_SIZEOF(m_UserName));
    idlOS::memset(m_UserCert, 0x00, ID_SIZEOF(m_UserCert));
    idlOS::memset(m_UserKey, 0x00, ID_SIZEOF(m_UserKey));
    idlOS::memset(m_UserAID, 0x00, ID_SIZEOF(m_UserAID));
    idlOS::memset(m_UnixdomainFilepath, 0x00, ID_SIZEOF(m_UnixdomainFilepath));
    idlOS::memset(m_IpcFilepath, 0x00, ID_SIZEOF(m_IpcFilepath));
    idlOS::memset(mPasswd, 0x00, ID_SIZEOF(mPasswd));
    idlOS::memset(mCaseSensitivePasswd, 0x00, ID_SIZEOF(mCaseSensitivePasswd));
    idlOS::memset(m_Editor, 0x00, ID_SIZEOF(m_Editor));
    idlOS::memset(mConnTypeStr,  0x00, ID_SIZEOF(mConnTypeStr));

    /*BUG-41163 SET SQLP[ROMPT] */
    mSqlPrompt[0] = '\0';
    mPromptRefreshFlag = PROMPT_REFRESH_OFF;

    SetEnv();

    mColFmtList = NULL;

    mNumFormat[0] = '\0';
    mNumToken[0] = '\0';
}

iSQLProperty::~iSQLProperty()
{
    freeFmtList();
}

/* BUG-19097 */
void iSQLProperty::clearPlanProperty()
{
    mExplainPlan = EXPLAIN_PLAN_OFF;
}

/* ============================================
 * iSQL 관련 환경변수를 읽어서 세팅
 * ============================================ */
void
iSQLProperty::SetEnv()
{
    /* ============================================
     * ISQL_BUFFER_SIZE, default : 64K
     * ============================================ */
    if (idlOS::getenv(ENV_ISQL_BUFFER_SIZE))
    {
        m_CommandLen = idlOS::atoi(idlOS::getenv(ENV_ISQL_BUFFER_SIZE));
    }
    else
    {
        m_CommandLen = COMMAND_LEN;
    }

    /* ============================================
     * ISQL_EDITOR, default : /usr/bin/vi
     * ============================================ */
    if (idlOS::getenv(ENV_ISQL_EDITOR))
    {
        idlOS::strcpy(m_Editor, idlOS::getenv(ENV_ISQL_EDITOR));
    }
    else
    {
        idlOS::strcpy(m_Editor, ISQL_EDITOR);
    }

    /* ============================================
     * ISQL_CONNECTION, default : TCP
     * ============================================ */
    if (idlOS::getenv(ENV_ISQL_CONNECTION))
    {
        idlOS::strcpy(mConnTypeStr, idlOS::getenv(ENV_ISQL_CONNECTION));
    }
    else
    {
        idlOS::strcpy(mConnTypeStr, "TCP");
    }
}

/* ============================================
 * Set ColSize
 * Display 되는 한 컬럼(char,varchar 타입만 적용)의 길이
 * ============================================ */
void
iSQLProperty::SetColSize( SChar * a_CommandStr,
                          SInt    a_ColSize )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    if ( (a_ColSize < 0) || (a_ColSize > 32767) )
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Column_Size_Error, (UInt)0, (UInt)32767);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        gSpool->Print();
    }
    else
    {
        m_ColSize = a_ColSize;
    }
}

void
iSQLProperty::SetFeedback( SChar * a_CommandStr,
                           SInt    a_Feedback )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    m_Feedback = a_Feedback;
}

/* ============================================
 * Set LineSize
 * Display 되는 한 라인의 길이
 * ============================================ */
void
iSQLProperty::SetLineSize( SChar * a_CommandStr,
                           SInt    a_LineSize )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    if ( (a_LineSize < 10) || (a_LineSize > 32767) )
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Line_Size_Error, (UInt)10, (UInt)32767);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        gSpool->Print();
    }
    else
    {
        m_LineSize = a_LineSize;
    }
}

// BUG-39213 Need to support SET NUMWIDTH in isql
/* ============================================
 * Set NumWidth
 * Display 되는 한 컬럼(numeric, decimal, float 타입만 적용)의 길이
 * ============================================ */
void
iSQLProperty::SetNumWidth( SChar * a_CommandStr,
                           SInt    a_NumWidth )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    if ( (a_NumWidth < 11) || (a_NumWidth > 50) )
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Num_Width_Error, (UInt)11, (UInt)50);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        gSpool->Print();
    }
    else
    {
        m_NumWidth = a_NumWidth;
    }
}

/* ============================================
 * Set PageSize
 * 레코드를 몇 개 단위로 보여줄 것인가
 * ============================================ */
void
iSQLProperty::SetPageSize( SChar * a_CommandStr,
                           SInt    a_PageSize )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    if ( a_PageSize < 0 || a_PageSize > 50000 )
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Page_Size_Error, (UInt)0, (UInt)50000);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);

        gSpool->Print();
    }
    else
    {
        m_PageSize = a_PageSize;
    }
}

/* ============================================
 * Set Term
 * 콘솔 화면으로의 출력을 할 것인가 말 것인가
 * ============================================ */
void
iSQLProperty::SetTerm( SChar * a_CommandStr,
                       idBool  a_IsTerm )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    m_Term = a_IsTerm;
}

/* ============================================
 * Set Timing
 * 쿼리 수행 시간을 보여줄 것인가 말 것인가
 * ============================================ */
void
iSQLProperty::SetTiming( SChar * a_CommandStr,
                         idBool  a_IsTiming )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    m_Timing = a_IsTiming;
}

// BUG-22685
/* ============================================
 * Set Vertical
 * 질의 결과물을 세로로 보여줄 것인가
 * ============================================ */
void
iSQLProperty::SetVertical( SChar * a_CommandStr,
                           idBool  a_IsVertical )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    m_Vertical = a_IsVertical;
}

/* ============================================
 * Set Heading
 * 헤더(Column Name)를 보여줄 것인가 말 것인가
 * ============================================ */
void
iSQLProperty::SetHeading( SChar * a_CommandStr,
                          idBool  a_IsHeading )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    m_Heading = a_IsHeading;
}

/* ============================================
 * Set TimeScale
 * 쿼리 수행 시간의 단위
 * ============================================ */
void
iSQLProperty::SetTimeScale( SChar         * a_CommandStr,
                            iSQLTimeScale   a_Timescale )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    m_TimeScale = a_Timescale;
}

/* ============================================
 * Set User
 * Connect 할 때마다 현재의 유저를 세팅한다
 * ============================================ */
void
iSQLProperty::SetUserName( SChar * a_UserName )
{
    // To Fix BUG-17430
    utString::makeNameInCLI(m_UserName,
                            ID_SIZEOF(m_UserName),
                            a_UserName,
                            idlOS::strlen(a_UserName));
}

/* ============================================
 * Set User
 * 
 * ============================================ */
void
iSQLProperty::SetUserCert( SChar * a_UserCert )
{
    idlOS::strcpy(m_UserCert, a_UserCert);
}

/* ============================================
 * Set User
 * Connect?????????????????????????????????
 * ============================================ */
void
iSQLProperty::SetUserKey( SChar * a_UserKey )
{
    idlOS::strcpy(m_UserKey, a_UserKey);
}

/* ============================================
 * Set User
 * Connect?????????????????????????????????
 * ============================================ */
void
iSQLProperty::SetUserAID( SChar * a_UserAID )
{
    idlOS::strcpy(m_UserAID, a_UserAID);
}

/* ============================================
 * Set IpcFilepath
 * Connect?????????????????????????????????
 * ============================================ */
void
iSQLProperty::SetIpcFilepath( SChar * a_IpcFilepath )
{
    idlOS::strcpy(m_IpcFilepath, a_IpcFilepath);
}

/* ============================================
 * Set UnixdomainFilepath
 * Connect?????????????????????????????????
 * ============================================ */
void
iSQLProperty::SetUnixdomainFilepath( SChar * a_UnixdomainFilepath )
{
    idlOS::strcpy(m_UnixdomainFilepath, a_UnixdomainFilepath);
}

/* ============================================
 * Set Password.
 * ============================================ */
void iSQLProperty::SetPasswd(SChar * aPasswd)
{
    idlOS::snprintf(mCaseSensitivePasswd, ID_SIZEOF(mCaseSensitivePasswd), "%s", aPasswd);
    idlOS::snprintf(mPasswd, ID_SIZEOF(mPasswd), "%s", aPasswd);
}

// ============================================
// bug-19279 remote sysdba enable
// conntype string(tcp/unix...)을 재 설정한다.(화면 출력용)
// why? sysdba의 경우 초기값과 다를 수 있다.
void iSQLProperty::AdjustConnTypeStr(idBool aIsSysDBA, SChar* aServerName)
{
    SInt sConnType = GetConnType(aIsSysDBA, aServerName);

    switch (sConnType)
    {
        default:
        case ISQL_CONNTYPE_TCP:
            idlOS::strcpy(mConnTypeStr, "TCP");
            break;
        case ISQL_CONNTYPE_UNIX:
            idlOS::strcpy(mConnTypeStr, "UNIX");
            break;
        case ISQL_CONNTYPE_IPC:
            idlOS::strcpy(mConnTypeStr, "IPC");
            break;
        case ISQL_CONNTYPE_IPCDA:
            idlOS::strcpy(mConnTypeStr, "IPCDA");
            break;
        case ISQL_CONNTYPE_SSL:
            idlOS::strcpy(mConnTypeStr, "SSL");
            break;
    }
}

/* ============================================
 * Get connnection type number
 * from connection type string.
 * ============================================ */
SInt iSQLProperty::GetConnType(idBool aIsSysDBA, SChar* aServerName)
{
    SInt sConnType;

    if (aIsSysDBA == ID_FALSE)
    {
        if (idlOS::strcasecmp(mConnTypeStr, "TCP") == 0)
        {
            sConnType = ISQL_CONNTYPE_TCP;
        }
        else if (idlOS::strcasecmp(mConnTypeStr, "UNIX") == 0)
        {
#if !defined(VC_WIN32) && !defined(NTO_QNX)
            sConnType = ISQL_CONNTYPE_UNIX;
#else
            sConnType = ISQL_CONNTYPE_TCP;
#endif
        }
        else if (idlOS::strcasecmp(mConnTypeStr, "IPC") == 0)
        {
            sConnType = ISQL_CONNTYPE_IPC;
        }
        else if (idlOS::strcasecmp(mConnTypeStr, "IPCDA") == 0)
        {
            sConnType = ISQL_CONNTYPE_IPCDA;
        }
        else if (idlOS::strcasecmp(mConnTypeStr, "SSL") == 0)
        {
            sConnType = ISQL_CONNTYPE_SSL;
        }
        else
        {
            sConnType = ISQL_CONNTYPE_TCP;
        }
    }
    // =============================================================
    // bug-19279 remote sysdba enable
    // 통신 방식(tcp/ unix domain)을 다음에 의해 결정
    // windows               : tcp
    // localhost: unix domain socket
    // 그외                   : tcp socket (ipc는 몰라요)
    // why? local인 경우 unix domain이 더 안정적일것 같아서
    else
    {
#if defined(VC_WIN32) || defined(NTO_QNX)

        /* TASK-5894 Permit sysdba via IPC */
        if (idlOS::strcasecmp(mConnTypeStr, "IPC") == 0)
        {
            sConnType = ISQL_CONNTYPE_IPC;
        }
        else if (idlOS::strcasecmp(mConnTypeStr, "SSL") == 0)
        {
            sConnType = ISQL_CONNTYPE_SSL;
        }
        else
        {
            sConnType = ISQL_CONNTYPE_TCP;
        }

#else
        if ((idlOS::strcmp(aServerName, "127.0.0.1" ) == 0) ||
            (idlOS::strcmp(aServerName, "::1") == 0) ||
            (idlOS::strcmp(aServerName, "0:0:0:0:0:0:0:1") == 0) ||
            (idlOS::strcmp(aServerName, "localhost" ) == 0))
        {
            /* TASK-5894 Permit sysdba via IPC */
            if (idlOS::strcasecmp(mConnTypeStr, "IPC") == 0)
            {
                sConnType = ISQL_CONNTYPE_IPC;
            }
            else
            {
                sConnType = ISQL_CONNTYPE_UNIX;
            }
        }
        else
        {
            if (idlOS::strcasecmp(mConnTypeStr, "SSL") == 0)
            {
                sConnType = ISQL_CONNTYPE_SSL;
            }
            else
            {
                sConnType = ISQL_CONNTYPE_TCP;
            }
        }
#endif
    }

    return sConnType;
}

/* ============================================
 * Set CheckConstraints
 * desc 결과에 Check Constraint 정보를 보여줄 것인지
 * ============================================ */
void
iSQLProperty::SetCheckConstraints( SChar  * a_CommandStr,
                                   idBool   a_ShowCheckConstraints )
{
    idlOS::sprintf( gSpool->m_Buf, "%s", a_CommandStr );
    gSpool->PrintCommand();

    m_ShowCheckConstraints = a_ShowCheckConstraints;
}

/* ============================================
 * Set ForeignKeys
 * desc 결과에 foreign key 정보를 보여줄 것인지
 * ============================================ */
void
iSQLProperty::SetForeignKeys( SChar * a_CommandStr,
                              idBool  a_ShowForeignKeys )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    m_ShowForeignKeys = a_ShowForeignKeys;
}

/* ============================================
 * BUG-43516 Set Partitions
 * desc 결과에 partition 정보를 보여줄 것인지
 * ============================================ */
void
iSQLProperty::SetPartitions( SChar * a_CommandStr,
                             idBool  a_ShowPartitions )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    m_ShowPartitions = a_ShowPartitions;
}

/* ============================================
 * Set PlanCommit
 * autocommit mode false 인 세션에서 explain plan 을
 * on 또는 only 로 했을 때, desc, select * From tab;
 * 같은 명령어를 사용하게 되면, 이전에 수행중인
 * 트랜잭션이 존재할 경우에 에러가 발생하게 된다.
 * error -> The transaction is already active.
 * 이를 방지하기 위해서 수행전에 commit 을 자동으로
 * 수행하도록 하는 옵션을 줄 수 있다.
 * ============================================ */
void
iSQLProperty::SetPlanCommit( SChar * a_CommandStr,
                             idBool  a_Commit )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    m_PlanCommit = a_Commit;
}

void
iSQLProperty::SetQueryLogging( SChar * a_CommandStr,
                               idBool  a_Logging )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    m_QueryLogging = a_Logging;
}

void
iSQLProperty::SetExplainPlan(SChar           * aCmdStr,
                             iSQLSessionKind   aExplainPlan)
{
    idlOS::snprintf(gSpool->m_Buf, GetCommandLen(), "%s", aCmdStr);
    gSpool->PrintCommand();

    mExplainPlan = aExplainPlan;

    if (idlOS::strncasecmp(aCmdStr, "ALTER", 5) == 0)
    {
        idlOS::snprintf(gSpool->m_Buf, GetCommandLen(), "Alter success.\n");
        gSpool->Print();

        if (GetTiming() == ID_TRUE)
        {
            idlOS::snprintf(gSpool->m_Buf, GetCommandLen(),
                            "elapsed time : 0.00\n");
            gSpool->Print();
        }
    }
}
/* ============================================
 * Set LobSize
 * Display 되는 한 컬럼(clob 타입만 적용)의 길이
 * ============================================ */
void
iSQLProperty::SetLobOffset( SChar * a_CommandStr,
                            SInt    a_LobOffset )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    if ( (a_LobOffset < 0) || (a_LobOffset > 32767) )
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Column_Size_Error, (UInt)0, (UInt)32767);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        gSpool->Print();
    }
    else
    {
        m_LobOffset = a_LobOffset;
    }
}

void
iSQLProperty::SetLobSize( SChar * a_CommandStr,
                          SInt    a_LobSize )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    if ( (a_LobSize < 0) || (a_LobSize > 32767) )
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Column_Size_Error, (UInt)0, (UInt)32767);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        gSpool->Print();
    }
    else
    {
        m_LobSize = a_LobSize;
    }
}

/* ============================================
 * 현재의 iSQL Option을 보여준다.
 * ============================================ */
void
iSQLProperty::ShowStmt( SChar          * a_CommandStr,
                        iSQLOptionKind   a_iSQLOptionKind )
{
    SChar tmp[20];

    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    switch(a_iSQLOptionKind)
    {
    case iSQL_SHOW_ALL :
        idlOS::sprintf(gSpool->m_Buf, "User      : %s\n", m_UserName);
        gSpool->Print();
        idlOS::sprintf(gSpool->m_Buf, "ColSize   : %d\n", m_ColSize);
        gSpool->Print(); 
        idlOS::sprintf(gSpool->m_Buf, "LobOffset : %d\n", m_LobOffset);
        gSpool->Print();
        idlOS::sprintf(gSpool->m_Buf, "LineSize  : %d\n", m_LineSize);
        gSpool->Print();
        idlOS::sprintf(gSpool->m_Buf, "LobSize   : %d\n", m_LobSize);
        gSpool->Print();
        idlOS::sprintf(gSpool->m_Buf, "NumWidth  : %d\n", m_NumWidth);
        gSpool->Print();
        idlOS::sprintf(gSpool->m_Buf, "NumFormat : \"%s\"\n", mNumFormat);
        gSpool->Print();
        idlOS::sprintf(gSpool->m_Buf, "PageSize  : %d\n", m_PageSize);
        gSpool->Print();

        if (m_TimeScale == iSQL_SEC)
            idlOS::strcpy(tmp, "Second");
        else if (m_TimeScale == iSQL_MILSEC)
            idlOS::strcpy(tmp, "MilliSecond");
        else if (m_TimeScale == iSQL_MICSEC)
            idlOS::strcpy(tmp, "MicroSecond");
        else if (m_TimeScale == iSQL_NANSEC)
            idlOS::strcpy(tmp, "NanoSecond");
        idlOS::sprintf(gSpool->m_Buf, "TimeScale : %s\n", tmp);
        gSpool->Print();

        if (m_Heading == ID_TRUE)
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Heading   : On\n");
        else
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Heading   : Off\n");
        gSpool->Print();

        if (m_Timing == ID_TRUE)
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Timing    : On\n");
        else
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Timing    : Off\n");
        gSpool->Print();

        if (m_Vertical == ID_TRUE)
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Vertical  : On\n");
        else
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Vertical  : Off\n");
        gSpool->Print();

        /* PROJ-1107 Check Constraint 지원 */
        if ( m_ShowCheckConstraints == ID_TRUE )
        {
            idlOS::sprintf( gSpool->m_Buf, "%s",
                            (SChar*)"ChkConstraints : On\n" );
        }
        else
        {
            idlOS::sprintf( gSpool->m_Buf, "%s",
                            (SChar*)"ChkConstraints : Off\n" );
        }
        gSpool->Print();

        if (m_ShowForeignKeys == ID_TRUE)
        {
            idlOS::sprintf(gSpool->m_Buf, "%s",
                           (SChar*)"ForeignKeys : On\n");
        }
        else
        {
            idlOS::sprintf(gSpool->m_Buf, "%s",
                           (SChar*)"ForeignKeys : Off\n");
        }
        gSpool->Print();

        /* BUG-43516 */
        if (m_ShowPartitions == ID_TRUE)
        {
            idlOS::sprintf(gSpool->m_Buf, "%s",
                           (SChar*)"Partitions : On\n");
        }
        else
        {
            idlOS::sprintf(gSpool->m_Buf, "%s",
                           (SChar*)"Partitions : Off\n");
        }
        gSpool->Print();

        if (m_PlanCommit == ID_TRUE)
        {
            idlOS::sprintf(gSpool->m_Buf, "%s",
                           (SChar*)"PlanCommit : On\n");
        }
        else
        {
            idlOS::sprintf(gSpool->m_Buf, "%s",
                           (SChar*)"PlanCommit : Off\n");
        }
        gSpool->Print();

        if (m_QueryLogging == ID_TRUE)
        {
            idlOS::sprintf(gSpool->m_Buf, "%s",
                           (SChar*)"QueryLogging : On\n");
        }
        else
        {
            idlOS::sprintf(gSpool->m_Buf, "%s",
                           (SChar*)"QueryLogging : Off\n");
        }
        gSpool->Print();
        if (m_Term == ID_TRUE)
        {
            idlOS::sprintf(gSpool->m_Buf, "%s",
                           (SChar*)"Term : On\n");
        }
        else
        {
            idlOS::sprintf(gSpool->m_Buf, "%s",
                           (SChar*)"Term : Off\n");
        }
        gSpool->Print();
        if ( GetEcho() == ID_TRUE )
        {
            idlOS::sprintf( gSpool->m_Buf, "%s",
                            (SChar*)"Echo : On\n" );
        }
        else
        {
            idlOS::sprintf( gSpool->m_Buf, "%s",
                            (SChar*)"Echo : Off\n" );
        }
        gSpool->Print();
        idlOS::sprintf(gSpool->m_Buf, "Feedback : %d\n", m_Feedback );
        gSpool->Print();
        if ( mFullName == ID_TRUE )
        {
            idlOS::sprintf( gSpool->m_Buf, "%s",
                            (SChar*)"Fullname : On\n" );
        }
        else
        {
            idlOS::sprintf( gSpool->m_Buf, "%s",
                            (SChar*)"Fullname : Off\n" );
        }
        gSpool->Print();

        /*BUG-41163 SET SQLP[ROMPT] */
        if (mCustPrompt[0] != '\0')
        {
            idlOS::sprintf(gSpool->m_Buf, "Sqlprompt : \"%s\"\n", mCustPrompt);
        }
        else
        {
            idlOS::sprintf(gSpool->m_Buf, "Sqlprompt : \"%s\"\n", mSqlPrompt);
        }
        gSpool->Print();

        /* BUG-43537 */
        if ( GetDefine() == ID_TRUE )
        {
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Define : On\n");
        }
        else
        {
          idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Define : Off\n");
        }
        gSpool->Print();

        /* BUG-43599 SET VERIFY ON|OFF */
        if ( GetVerify() == ID_TRUE )
        {
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Verify : On\n");
        }
        else
        {
          idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Verify : Off\n");
        }
        gSpool->Print();

        /* BUG-44613 Set PrefetchRows */
        idlOS::sprintf(gSpool->m_Buf, "PrefetchRows : %d\n", m_PrefetchRows);
        gSpool->Print();

        /* BUG-44613 Set AsyncPrefetch On|Auto|Off */
        if ( GetAsyncPrefetch() == ASYNCPREFETCH_ON )
        {
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"AsyncPrefetch : On\n");
        }
        else if ( GetAsyncPrefetch() == ASYNCPREFETCH_OFF )
        {
          idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"AsyncPrefetch : Off\n");
        }
        else if ( GetAsyncPrefetch() == ASYNCPREFETCH_AUTO_TUNING )
        {
          idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"AsyncPrefetch : Auto\n");
        }
        else
        {
            /* Do nothing */
        }
        gSpool->Print();

        break;

    case iSQL_HEADING :
        if (m_Heading == ID_TRUE)
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Heading : On\n");
        else
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Heading : Off\n");
        gSpool->Print();
        break;
    case iSQL_COLSIZE :
        idlOS::sprintf(gSpool->m_Buf, "ColSize  : %d\n", m_ColSize);
        gSpool->Print();
        break;
    case iSQL_LINESIZE :
        idlOS::sprintf(gSpool->m_Buf, "LineSize : %d\n", m_LineSize);
        gSpool->Print();
        break;
    case iSQL_LOBOFFSET :
        idlOS::sprintf(gSpool->m_Buf, "LobOffset: %d\n", m_LobOffset);
        gSpool->Print();
        break;
    case iSQL_LOBSIZE :
        idlOS::sprintf(gSpool->m_Buf, "LobSize  : %d\n", m_LobSize);
        gSpool->Print();
        break;
    case iSQL_NUMWIDTH :
        idlOS::sprintf(gSpool->m_Buf, "NumWidth  : %d\n", m_NumWidth);
        gSpool->Print();
        break;
    case iSQL_NUMFORMAT :
        idlOS::sprintf(gSpool->m_Buf, "NumFormat : \"%s\"\n", mNumFormat);
        gSpool->Print();
        break;
    case iSQL_PAGESIZE :
        idlOS::sprintf(gSpool->m_Buf, "PageSize : %d\n", m_PageSize);
        gSpool->Print();
        break;
    case iSQL_SQLPROMPT :
        if (mCustPrompt[0] != '\0')
        {
            idlOS::sprintf(gSpool->m_Buf, "Sqlprompt : \"%s\"\n", mCustPrompt);
        }
        else
        {
            idlOS::sprintf(gSpool->m_Buf, "Sqlprompt : \"%s\"\n", mSqlPrompt);
        }
        gSpool->Print();
        break;
    case iSQL_TERM :
        if (m_Term == ID_TRUE)
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Term : On\n");
        else
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Term : Off\n");
        gSpool->Print();
        break;
    case iSQL_TIMING :
        if (m_Timing == ID_TRUE)
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Timing : On\n");
        else
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Timing : Off\n");
        gSpool->Print();
        break;
    case iSQL_VERTICAL :
        if (m_Vertical == ID_TRUE)
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Vertical  : On\n");
        else
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Vertical  : Off\n");
        gSpool->Print();
        break;
    case iSQL_CHECKCONSTRAINTS : /* PROJ-1107 Check Constraint 지원 */
        if ( m_ShowCheckConstraints == ID_TRUE )
        {
            idlOS::sprintf( gSpool->m_Buf, "%s",
                            (SChar*)"ChkConstraints : On\n" );
        }
        else
        {
            idlOS::sprintf( gSpool->m_Buf, "%s",
                            (SChar*)"ChkConstraints : Off\n" );
        }
        gSpool->Print();
        break;
    case iSQL_FOREIGNKEYS :
        if (m_ShowForeignKeys == ID_TRUE)
        {
            idlOS::sprintf(gSpool->m_Buf, "%s",
                           (SChar*)"ForeignKeys : On\n");
        }
        else
        {
            idlOS::sprintf(gSpool->m_Buf, "%s",
                           (SChar*)"ForeignKeys : Off\n");
        }
        gSpool->Print();
        break;
    case iSQL_PARTITIONS : /* BUG-43516 */
        if (m_ShowPartitions == ID_TRUE)
        {
            idlOS::sprintf(gSpool->m_Buf, "%s",
                           (SChar*)"Partitions : On\n");
        }
        else
        {
            idlOS::sprintf(gSpool->m_Buf, "%s",
                           (SChar*)"Partitions : Off\n");
        }
        gSpool->Print();
        break;
    case iSQL_PLANCOMMIT :
        if (m_PlanCommit == ID_TRUE)
        {
            idlOS::sprintf(gSpool->m_Buf, "%s",
                           (SChar*)"PlanCommit : On\n");
        }
        else
        {
            idlOS::sprintf(gSpool->m_Buf, "%s",
                           (SChar*)"PlanCommit : Off\n");
        }
        gSpool->Print();
        break;
    case iSQL_QUERYLOGGING :
        if (m_QueryLogging  == ID_TRUE)
        {
            idlOS::sprintf(gSpool->m_Buf, "%s",
                           (SChar*)"QueryLogging : On\n");
        }
        else
        {
            idlOS::sprintf(gSpool->m_Buf, "%s",
                           (SChar*)"QueryLogging : Off\n");
        }
        gSpool->Print();
        break;
    case iSQL_USER :
        idlOS::sprintf(gSpool->m_Buf, "User : %s\n", m_UserName);
        gSpool->Print();
        break;
    case iSQL_TIMESCALE :
        if (m_TimeScale == iSQL_SEC)
            idlOS::strcpy(tmp, "Second");
        else if (m_TimeScale == iSQL_MILSEC)
            idlOS::strcpy(tmp, "MilliSecond");
        else if (m_TimeScale == iSQL_MICSEC)
            idlOS::strcpy(tmp, "MicroSecond");
        else if (m_TimeScale == iSQL_NANSEC)
            idlOS::strcpy(tmp, "NanoSecond");
        idlOS::sprintf(gSpool->m_Buf, "TimeScale : %s\n", tmp);
        gSpool->Print();
        break;
    case iSQL_FEEDBACK:
        idlOS::sprintf(gSpool->m_Buf, "Feedback : %d\n",  m_Feedback );
        gSpool->Print();
        break;
    /* BUG-37772 */
    case iSQL_ECHO:
        if ( GetEcho() == ID_TRUE )
        {
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Echo : On\n");
        }
        else
        {
          idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Echo : Off\n");
        }
        gSpool->Print();
        break;
    /* BUG-39620 */
    case iSQL_FULLNAME:
        if ( GetFullName() == ID_TRUE )
        {
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"FullName : On\n");
        }
        else
        {
          idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"FullName : Off\n");
        }
        gSpool->Print();
        break;
    /* BUG-43537 */
    case iSQL_DEFINE:
        if ( GetDefine() == ID_TRUE )
        {
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Define : On\n");
        }
        else
        {
          idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Define : Off\n");
        }
        gSpool->Print();
        break;

    /* BUG-43599 SET VERIFY ON|OFF */
    case iSQL_VERIFY:
        if ( GetVerify() == ID_TRUE )
        {
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Verify : On\n");
        }
        else
        {
          idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"Verify : Off\n");
        }
        gSpool->Print();
        break;

    /* BUG-44613 Set PrefetchRows */
    case iSQL_PREFETCHROWS:
        idlOS::sprintf(gSpool->m_Buf, "PrefetchRows : %d\n", m_PrefetchRows);
        gSpool->Print();
        break;

    /* BUG-44613 Set AsyncPrefetch On|Auto|Off */
    case iSQL_ASYNCPREFETCH:
        if ( GetAsyncPrefetch() == ASYNCPREFETCH_ON )
        {
            idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"AsyncPrefetch : On\n");
        }
        else if ( GetAsyncPrefetch() == ASYNCPREFETCH_OFF )
        {
          idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"AsyncPrefetch : Off\n");
        }
        else if ( GetAsyncPrefetch() == ASYNCPREFETCH_AUTO_TUNING )
        {
          idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"AsyncPrefetch : Auto\n");
        }
        else
        {
            /* Do nothing */
        }
        gSpool->Print();
        break;

    default :
	// BUG-32613 The English grammar of the iSQL message "Have no saved command." needs to be corrected.
        idlOS::sprintf(gSpool->m_Buf, "%s", (SChar*)"No information to show.\n");
        gSpool->Print();
        break;
    }
}

/* BUG-37772 */
void iSQLProperty::SetEcho( SChar * aCommandStr,
                            idBool  aIsEcho )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", aCommandStr);
    gSpool->PrintCommand();

    mEcho = aIsEcho;
}

idBool iSQLProperty::GetEcho( void )
{
    return mEcho;
}

/* ============================================
 * Set FULLNAME
 * 40 bytes 이상 길이의 객체 이름을 잘라서 또는
 * 모두 디스플레이할 것인지 결정
 * ============================================ */
void
iSQLProperty::SetFullName( SChar * aCommandStr,
                           idBool  aIsFullName )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", aCommandStr);
    gSpool->PrintCommand();

    mFullName = aIsFullName;
    if (aIsFullName == ID_TRUE)
    {
        gExecuteCommand->SetObjectDispLen(QP_MAX_NAME_LEN);
    }
    else
    {
        /* BUG-39620: 40 is old display length for meta */
        gExecuteCommand->SetObjectDispLen(40);
    }
}

idBool iSQLProperty::GetFullName( void )
{
    return mFullName;
}

/* ============================================
 * BUG-41163: SET SQLP[ROMPT]
 * iSQL command prompt 초기화
 * ============================================ */
void iSQLProperty::InitSqlPrompt()
{
    if (mIsSysDBA == ID_TRUE)
    {
        idlOS::strcpy(mSqlPrompt, ISQL_PROMPT_SYSDBA_STR);
    }
    else
    {
        idlOS::strcpy(mSqlPrompt, ISQL_PROMPT_DEFAULT_STR);
    }
    mCustPrompt[0] = '\0';
}

void
iSQLProperty::ResetSqlPrompt()
{
    if (mCustPrompt[0] == '\0')
    {
        InitSqlPrompt();
    }
}

/* ============================================
 * BUG-41163: SET SQLP[ROMPT]
 * iSQL command prompt 설정
 * ============================================ */
void
iSQLProperty::SetSqlPrompt( SChar * aCommandStr,
                            SChar * aSqlPrompt )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", aCommandStr);
    gSpool->PrintCommand();

    parseSqlPrompt(aSqlPrompt);
}

/*
 * _PRIVILEGE 또는 _USER 변수가 포함된 prompt의 경우에는
 * 매번 파싱하지 않고 CONNECT 명령어를 사용할 때만 파싱한다.
 * 즉 PROMPT_VARIABLE_ON, PROMPT_RECONNECT_ON 이 모두 설정된 경우에만
 */
SChar * iSQLProperty::GetSqlPrompt( void )
{
    if ( mPromptRefreshFlag == PROMPT_REFRESH_ON )
    {
        parseSqlPrompt(mCustPrompt);
    }
    else
    {
        /* do nothing. */
    }
    return mSqlPrompt;
}

/*
 * CONNECT 명령어가 수행된 경우 PROMPT_RECONNECT_ON bit를 설정하기 위해,
 * 이 함수가 호출된다.
 */
void iSQLProperty::SetPromptRefreshFlag(UInt aFlag)
{
    mPromptRefreshFlag = mPromptRefreshFlag | aFlag;
}

void
iSQLProperty::parseSqlPrompt( SChar * aSqlPrompt )
{
    SInt   sRet;
    UInt   sPromptRefreshFlag = PROMPT_REFRESH_OFF;
    SChar  sNewPrompt[WORD_LEN];

    sNewPrompt[0] ='\0';
    sRet = lexSqlPrompt(aSqlPrompt, sNewPrompt, &sPromptRefreshFlag);

    if (sRet == IDE_SUCCESS)
    {
        idlOS::strcpy(mSqlPrompt, sNewPrompt);
        idlOS::strcpy(mCustPrompt, aSqlPrompt);
        mPromptRefreshFlag = sPromptRefreshFlag;
    }
    else
    {
        uteSprintfErrorCode(gSpool->m_Buf,
                            gProperty.GetCommandLen(),
                            &gErrorMgr);
        gSpool->Print();
    }
}

/* ============================================
 * BUG-41173
 * Set Define
 * turns substitution on/off
 * ============================================ */
void
iSQLProperty::SetDefine( SChar * a_CommandStr,
                         idBool  a_IsDefine )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    m_Define = a_IsDefine;
}

/* ============================================
 * BUG-40246
 * COLUMN column CLEAR
 * ============================================ */
IDE_RC iSQLProperty::ClearFormat()
{
    SChar *sColumnName = NULL;

    sColumnName = gCommand->GetColumnName();

    IDE_TEST(deleteNode(sColumnName) != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    uteSetErrorCode(&gErrorMgr, utERR_ABORT_Column_Not_Defined,
                    sColumnName);
    uteSprintfErrorCode(gSpool->m_Buf,
                        gProperty.GetCommandLen(),
                        &gErrorMgr);
    gSpool->Print();

    return IDE_FAILURE;
}

/* ============================================
 * BUG-40246
 * COLUMN column ON|OFF
 * ============================================ */
IDE_RC iSQLProperty::TurnColumnOnOff()
{
    idBool       sOnOff;
    SChar       *sKey = NULL;
    SChar       *sColumnName = NULL;
    isqlFmtNode *sNode = NULL;

    sColumnName = gCommand->GetColumnName();
    sOnOff = gCommand->GetOnOff();

    sNode = getNode(sColumnName);

    if (sNode == NULL)
    {
        sKey = (SChar*)idlOS::malloc(idlOS::strlen(sColumnName)+1);
        IDE_TEST_RAISE(sKey == NULL, malloc_error);

        idlOS::strcpy(sKey, sColumnName);

        IDE_TEST(insertNode(sKey,        // Column Name
                            FMT_UNKNOWN, // Type
                            NULL,        // Format
                            0,           // Column Size
                            NULL,        // Token for Number format
                            sOnOff) != IDE_SUCCESS);
    }
    else
    {
        sNode->mOnOff = sOnOff;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(malloc_error);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_memory_error,
                        __FILE__, (UInt)__LINE__);
        utePrintfErrorCode(stderr, &gErrorMgr);
        Exit(0);
    }
    IDE_EXCEPTION_END;

    if (sKey != NULL)
    {
        idlOS::free(sKey);
    }
    return IDE_FAILURE;
}

/* ============================================
 * BUG-40246
 * COLUMN 
 * ============================================ */
IDE_RC
iSQLProperty::ListAllColumns()
{
    UInt         sCnt = 0;
    isqlFmtNode *sNode = NULL;

    sNode = mColFmtList;
    while (sNode != NULL)
    {
        if (sCnt > 0)
        {
            idlOS::sprintf(gSpool->m_Buf, "\n");
            gSpool->Print();
        }
        printNode(sNode);
        sNode = sNode->mNext;
        sCnt++;
    }
    return IDE_SUCCESS;
}

/* ============================================
 * BUG-40246
 * COLUMN column
 * ============================================ */
IDE_RC
iSQLProperty::ListColumn()
{
    SChar       *sColumnName = NULL;
    isqlFmtNode *sNode = NULL;

    sColumnName = gCommand->GetColumnName();

    sNode = getNode(sColumnName);

    IDE_TEST(sNode == NULL);

    printNode(sNode);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    uteSetErrorCode(&gErrorMgr, utERR_ABORT_Column_Not_Defined,
                    sColumnName);
    uteSprintfErrorCode(gSpool->m_Buf,
                        gProperty.GetCommandLen(),
                        &gErrorMgr);
    gSpool->Print();

    return IDE_FAILURE;
}

/* ============================================
 * BUG-40246
 * COLUMN char_column FORMAT fmt
 * ============================================ */
void
iSQLProperty::SetFormat4Char()
{
    SInt   sSize;
    SChar *sFmt = NULL;
    SChar *sKey = NULL;
    SChar *sValue = NULL;
    SChar *sColumnName = NULL;
    isqlFmtNode *sNode = NULL;

    idlOS::sprintf(gSpool->m_Buf, "%s", gCommand->GetCommandStr());
    gSpool->PrintCommand();

    sFmt = gCommand->GetFormatStr();
    sSize = idlOS::atoi(sFmt + 1);

    IDE_TEST_RAISE(sSize == 0 || sSize > 32767, illegal_format);

    sColumnName = gCommand->GetColumnName();

    sKey = (SChar*)idlOS::malloc(idlOS::strlen(sColumnName)+1);
    IDE_TEST_RAISE(sKey == NULL, malloc_error);

    idlOS::strcpy(sKey, sColumnName);

    sValue = (SChar*)idlOS::malloc(idlOS::strlen(sFmt)+1);
    IDE_TEST_RAISE(sValue == NULL, malloc_error);

    idlOS::strcpy(sValue, sFmt);

    sNode = getNode(sColumnName);

    if (sNode == NULL)
    {
        IDE_TEST(insertNode(sKey,        // Column Name
                            FMT_CHR,     // Type
                            sValue,      // Format
                            sSize,       // Column Size
                            NULL,        // Token for only Number format
                            ID_TRUE) != IDE_SUCCESS);
    }
    else
    {
        changeNode(sNode,
                   sKey,
                   FMT_CHR,
                   sValue,
                   sSize,
                   NULL);
    }

    return;

    IDE_EXCEPTION(illegal_format);
    {
        uteSetErrorCode(&gErrorMgr,
                        utERR_ABORT_Illegal_Format_String,
                        sFmt);
        uteSprintfErrorCode(gSpool->m_Buf,
                            gProperty.GetCommandLen(),
                            &gErrorMgr);
        gSpool->Print();
    }
    IDE_EXCEPTION(malloc_error);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_memory_error,
                        __FILE__, (UInt)__LINE__);
        utePrintfErrorCode(stderr, &gErrorMgr);
        Exit(0);
    }
    IDE_EXCEPTION_END;

    if (sKey != NULL)
    {
        idlOS::free(sKey);
    }
    if (sValue != NULL)
    {
        idlOS::free(sValue);
    }
    return;
}

SInt iSQLProperty::GetCharSize(SChar *aColumnName)
{
    isqlFmtNode *sNode = NULL;

    sNode = getNode(aColumnName);

    IDE_TEST(sNode == NULL);

    IDE_TEST(sNode->mOnOff != ID_TRUE);

    return sNode->mColSize;

    IDE_EXCEPTION_END;

    return 0;
}

/* ============================================
 * BUG-34447
 * COLUMN numeric_column FORMAT fmt
 * ============================================ */
void
iSQLProperty::SetFormat4Num()
{
    SChar *sFmt = NULL;
    SChar *sKey = NULL;
    SChar *sValue = NULL;
    UChar *sToken = NULL;
    SChar *sColumnName = NULL;
    UInt   sFmtLen;
    UChar  sTokenBuf[MTD_NUMBER_MAX];
    isqlFmtNode *sNode = NULL;

    idlOS::sprintf(gSpool->m_Buf, "%s", gCommand->GetCommandStr());
    gSpool->PrintCommand();

    IDE_TEST_RAISE( gExecuteCommand->SetNlsCurrency() != IDE_SUCCESS,
                    nls_error );

    sFmt = gCommand->GetFormatStr();
    sFmtLen = idlOS::strlen(sFmt);

    IDE_TEST_RAISE( sFmtLen > MTC_TO_CHAR_MAX_PRECISION,
                    exceed_max_length );

    IDE_TEST_RAISE( isqlFloat::CheckFormat( (UChar*)sFmt,
                                            sFmtLen,
                                            sTokenBuf )
                    != IDE_SUCCESS, illegal_format );

    sColumnName = gCommand->GetColumnName();

    sKey = (SChar*)idlOS::malloc(idlOS::strlen(sColumnName)+1);
    IDE_TEST_RAISE(sKey == NULL, malloc_error);

    idlOS::strcpy(sKey, sColumnName);

    sValue = (SChar*)idlOS::malloc(idlOS::strlen(sFmt)+1);
    IDE_TEST_RAISE(sValue == NULL, malloc_error);

    sToken = (UChar*)idlOS::malloc(MTD_NUMBER_MAX);
    IDE_TEST_RAISE(sToken == NULL, malloc_error);

    idlOS::strcpy(sValue, sFmt);
    idlOS::memcpy(sToken, sTokenBuf, MTD_NUMBER_MAX);

    sNode = getNode(sColumnName);

    if (sNode == NULL)
    {
        IDE_TEST(insertNode(sKey,        // Column Name
                            FMT_NUM,     // Type
                            sValue,      // Format
                            0,           // Column Size
                            sToken,      // Token for Number format
                            ID_TRUE) != IDE_SUCCESS);
    }
    else
    {
        changeNode( sNode,
                    sKey,
                    FMT_NUM,
                    sValue,
                    0,
                    sToken );
    }

    return;

    IDE_EXCEPTION(exceed_max_length);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Exceed_Max_String,
                        MTC_TO_CHAR_MAX_PRECISION);
        uteSprintfErrorCode(gSpool->m_Buf,
                            gProperty.GetCommandLen(),
                            &gErrorMgr);
        gSpool->Print();
    }
    IDE_EXCEPTION(illegal_format);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Illegal_Format_String,
                        sFmt);
        uteSprintfErrorCode(gSpool->m_Buf,
                            gProperty.GetCommandLen(),
                            &gErrorMgr);
        gSpool->Print();
    }
    IDE_EXCEPTION(malloc_error);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_memory_error,
                        __FILE__, (UInt)__LINE__);

        utePrintfErrorCode(stderr, &gErrorMgr);
        Exit(0);
    }
    IDE_EXCEPTION(nls_error);
    {
        uteSprintfErrorCode(gSpool->m_Buf, GetCommandLen(),
                            &gErrorMgr);
        gSpool->Print();
    }
    IDE_EXCEPTION_END;

    if (sKey != NULL)
    {
        idlOS::free(sKey);
    }
    if (sValue != NULL)
    {
        idlOS::free(sValue);
    }
    if (sToken != NULL)
    {
        idlOS::free(sToken);
    }
    return;
}

SChar* iSQLProperty::GetColumnFormat(SChar          *aColumnName,
                                     isqlFmtType     aType, /* BUG-43351 */
                                     mtlCurrency    *aCurrency,
                                     SInt           *aColSize,
                                     UChar          **aToken)
{
    isqlFmtNode *sNode = NULL;

    *aColSize = 0; /* BUG-43351 Uninitialized Variable */
    *aToken = NULL;
    sNode = getNode(aColumnName);

    IDE_TEST(sNode == NULL);

    /* BUG-43351 CodeSonar warning: Uninitialized Variable */
    IDE_TEST(sNode->mType != aType);

    IDE_TEST(sNode->mOnOff != ID_TRUE);

    IDE_TEST_CONT(sNode->mToken == NULL, ret_code);

    *aColSize = isqlFloat::GetColSize( aCurrency, sNode->mFmt, sNode->mToken );

    *aToken = sNode->mToken;

    IDE_EXCEPTION_CONT( ret_code );

    return sNode->mFmt;

    IDE_EXCEPTION_END;

    return NULL;
}

/* ============================================
 * BUG-40246
 * Operations For FmtNodeList
 *  - getNode
 *  - insertNode
 *  - deleteNode
 * ============================================ */
isqlFmtNode* iSQLProperty::getNode(SChar *aColumnName)
{
    isqlFmtNode *sTmpNode  = NULL;
    isqlFmtNode *sNode  = NULL;

    sTmpNode = mColFmtList;
    while (sTmpNode != NULL)
    {
        if ( idlOS::strcasecmp(sTmpNode->mKey, aColumnName) == 0 )
        {
            sNode = sTmpNode;
            break;
        }
        else
        {
            sTmpNode = sTmpNode->mNext;
        }
    }
    return sNode;
}

IDE_RC iSQLProperty::insertNode(SChar       *aColumnName,
                                isqlFmtType  aType,
                                SChar       *aFmt,
                                SInt         aColSize,
                                UChar       *aToken,
                                idBool       aOnOff /* = ID_TRUE */)
{
    isqlFmtNode *sNode = NULL;

    sNode = (isqlFmtNode *)idlOS::malloc(ID_SIZEOF(isqlFmtNode));
    IDE_TEST_RAISE(sNode == NULL, malloc_error);

    sNode->mType = aType;
    sNode->mOnOff = aOnOff;
    sNode->mKey = aColumnName;
    sNode->mFmt = aFmt;
    sNode->mColSize = aColSize;
    sNode->mToken = aToken;
    sNode->mNext = mColFmtList;
    mColFmtList = sNode;

    return IDE_SUCCESS;

    IDE_EXCEPTION(malloc_error);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_memory_error,
                        __FILE__, (UInt)__LINE__);
        utePrintfErrorCode(stderr, &gErrorMgr);
        Exit(0);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC iSQLProperty::deleteNode(SChar *aColumnName)
{
    isqlFmtNode *sNode = NULL;
    isqlFmtNode *sPrevNode = NULL;

    sNode = mColFmtList;
    while (sNode != NULL)
    {
        if (idlOS::strcasecmp(aColumnName, sNode->mKey) == 0)
        {
            if (sNode == mColFmtList)
            {
                mColFmtList = sNode->mNext;
                IDE_CONT(node_found);
            }
            else
            {
                sPrevNode->mNext = sNode->mNext;
                IDE_CONT(node_found);
            }
        }
        else
        {
            sPrevNode = sNode;
            sNode = sNode->mNext;
        }
    }

    IDE_RAISE(node_not_found);

    IDE_EXCEPTION_CONT(node_found);

    if (sNode->mFmt != NULL)
    {
        idlOS::free(sNode->mFmt);
    }
    if (sNode->mToken != NULL)
    {
        idlOS::free(sNode->mToken);
    }
    idlOS::free(sNode->mKey);
    idlOS::free(sNode);

    return IDE_SUCCESS;

    IDE_EXCEPTION(node_not_found);
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void iSQLProperty::changeNode(isqlFmtNode *aNode,
                              SChar       *aColumnName,
                              isqlFmtType  aType,
                              SChar       *aFmt,
                              SInt         aColSize,
                              UChar       *aToken)
{
    if (aNode->mFmt != NULL)
    {
        idlOS::free(aNode->mFmt);
    }
    if (aNode->mToken != NULL)
    {
        idlOS::free(aNode->mToken);
    }
    idlOS::free(aNode->mKey);

    aNode->mKey = aColumnName;
    aNode->mType = aType;
    aNode->mFmt = aFmt;
    aNode->mColSize = aColSize;
    aNode->mToken = aToken;
}

void iSQLProperty::freeFmtList()
{
    isqlFmtNode *sNode = NULL;
    isqlFmtNode *sTmpNode = NULL;

    sNode = mColFmtList;
    while (sNode != NULL)
    {
        sTmpNode = sNode->mNext;
        if (sNode->mFmt != NULL)
        {
            idlOS::free(sNode->mFmt);
        }
        if (sNode->mToken != NULL)
        {
            idlOS::free(sNode->mToken);
        }
        idlOS::free(sNode->mKey);
        idlOS::free(sNode);
        sNode = sTmpNode;
    }
    mColFmtList = NULL;
}

void iSQLProperty::printNode(isqlFmtNode *aNode)
{
    idlOS::sprintf(gSpool->m_Buf, "COLUMN   %s %s\n",
                   aNode->mKey,
                   (aNode->mOnOff == ID_TRUE)? "ON":"OFF");
    gSpool->Print();

    if (aNode->mFmt != NULL)
    {
        idlOS::sprintf(gSpool->m_Buf, "FORMAT   %s\n", aNode->mFmt);
        gSpool->Print();
    }
}

/* BUG-34447 SET NUMFORMAT */
IDE_RC iSQLProperty::SetNumFormat()
{
    SChar *sFmt = NULL;
    UInt   sFmtLen;
    UChar  sToken[MTD_NUMBER_MAX];

    idlOS::sprintf(gSpool->m_Buf, "%s", gCommand->GetCommandStr());
    gSpool->PrintCommand();

    IDE_TEST_RAISE( gExecuteCommand->SetNlsCurrency() != IDE_SUCCESS,
                    nls_error );

    sFmt = gCommand->GetFormatStr();
    sFmtLen = idlOS::strlen(sFmt);

    IDE_TEST_RAISE( isqlFloat::CheckFormat( (UChar*)sFmt,
                                            sFmtLen,
                                            sToken )
                    != IDE_SUCCESS, illegal_format );

    idlOS::strcpy(mNumFormat, sFmt);
    idlOS::memcpy(mNumToken, sToken, MTD_NUMBER_MAX);

    return IDE_SUCCESS;

    IDE_EXCEPTION(illegal_format);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Illegal_Format_String,
                        sFmt);
        uteSprintfErrorCode(gSpool->m_Buf,
                            gProperty.GetCommandLen(),
                            &gErrorMgr);
        gSpool->Print();
    }
    IDE_EXCEPTION(nls_error);
    {
        uteSprintfErrorCode(gSpool->m_Buf, GetCommandLen(),
                            &gErrorMgr);
        gSpool->Print();
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

SChar * iSQLProperty::GetNumFormat(mtlCurrency  *aCurrency,
                                   SInt         *aColSize,
                                   UChar       **aToken)
{
    *aToken = NULL;

    IDE_TEST_CONT( mNumFormat == NULL, ret_code );

    *aColSize = isqlFloat::GetColSize( aCurrency, mNumFormat, mNumToken );

    *aToken = mNumToken;

    IDE_EXCEPTION_CONT( ret_code );

    return mNumFormat;
}

/* ============================================
 * BUG-40246
 * CLEAR
 * ============================================ */
IDE_RC iSQLProperty::ClearAllFormats()
{
    freeFmtList();

    return IDE_SUCCESS;
}

/* BUG-43599 SET VERIFY ON|OFF */
void
iSQLProperty::SetVerify( SChar * a_CommandStr,
                         idBool  a_IsVerify )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    m_Verify = a_IsVerify;
}

/* BUG-44613 Set PrefetchRows */
void
iSQLProperty::SetPrefetchRows( SChar * a_CommandStr,
                               SInt    a_PrefetchRows )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    IDE_TEST_RAISE(
                gExecuteCommand->SetPrefetchRows(a_PrefetchRows)
                != IDE_SUCCESS, set_error );

    m_PrefetchRows = a_PrefetchRows;

    return;

    IDE_EXCEPTION(set_error);
    {
        uteSprintfErrorCode(gSpool->m_Buf,
                            GetCommandLen(),
                            &gErrorMgr);
        gSpool->Print();
    }
    IDE_EXCEPTION_END;

    return;
}

/* BUG-44613 Set AsyncPrefetch On|Auto|Off */
void
iSQLProperty::SetAsyncPrefetch( SChar             *a_CommandStr,
                                AsyncPrefetchType  a_Type )
{
    idlOS::sprintf(gSpool->m_Buf, "%s", a_CommandStr);
    gSpool->PrintCommand();

    IDE_TEST_RAISE( gExecuteCommand->SetAsyncPrefetch(a_Type)
                    != IDE_SUCCESS, set_error );

    m_AsyncPrefetch = a_Type;

    return;

    IDE_EXCEPTION(set_error);
    {
        uteSprintfErrorCode(gSpool->m_Buf, GetCommandLen(),
                            &gErrorMgr);
        gSpool->Print();
    }
    IDE_EXCEPTION_END;

    return;
}
