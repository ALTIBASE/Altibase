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
 * $Id: iSQLProgOption.h 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#ifndef _O_ISQLPROGOPTION_H_
#define _O_ISQLPROGOPTION_H_ 1

#include <idnCharSet.h>
#include <utISPApi.h>
#include <iSQL.h>

class iSQLProgOption
{
public:
    iSQLProgOption();

    IDE_RC ParsingCommandLine(SInt aArgc, SChar **aArgv);
    IDE_RC ReadProgOptionInteractive();
    // BUG-26287: 옵션 처리방법 통일
    IDE_RC ReadEnvironment();
    void   ReadServerProperties();

    /* BUG-31387: ConnType을 보정하고 경우에 따라 경고 출력 */
    void   AdjustConnType();

    SChar * GetServerName() { return m_ServerName; }
    SChar * GetLoginID()    { return m_LoginID; }
    SChar * GetPassword()   { return m_Password; }
    SChar * GetNLS_USE()    { return m_NLS_USE; }
    UInt    GetNLS_REPLACE(){ return m_NLS_REPLACE; }
    SInt    GetPortNo()     { return m_PortNo; }
    SChar * GetInFileName() { return m_InFileName; }

    idBool  IsPortNo()      { return m_bExist_PORT; }
    idBool  IsInFile()      { return m_bExist_F; }
    idBool  IsOutFile()     { return m_bExist_O; }
    idBool  IsSilent()      { return m_bExist_SILENT; }
    idBool  IsSysdba()      { return m_bExist_SYSDBA; }
    idBool  IsATC()         { return m_bExist_ATC; }
    idBool  IsATAF()        { return m_bExist_ATAF; }
    idBool  IsNoEditing()   { return m_bExist_NOEDITING; }
    idBool  IsNoPrompt()    { return m_bExist_NOPROMPT; } /* BUG-29760 */
    idBool  IsPreferIPv6()  { return mPreferIPv6; } /* BUG-29915 */
    idBool  IsServPropsLoaded() { return m_bServPropsLoaded; } /* BUG-27966 */
    SChar * getTimezone()   { return m_TimezoneString; }
    idBool  IsNOLOG()       { return m_bExist_NOLOG; } /* BUG-41476 */

    /* BUG-41281 SSL */
    SChar * GetSslCa()      { return m_SslCa; }
    SChar * GetSslCapath()  { return m_SslCapath; }
    SChar * GetSslCert()    { return m_SslCert; }
    SChar * GetSslKey()     { return m_SslKey; }
    SChar * GetSslCipher()  { return m_SslCipher; }
    SChar * GetSslVerify()  { return m_SslVerify; }

    /* BUG-43352 */
    SInt    GetConnectRetryMax() { return m_ConnectRetryMax; }

    /* bug-20046 */
    idBool  IsPortNoLogin() { return m_bExist_PORT_login; }

    idBool  UseLineEditing() {
        return ((IsATC()       != ID_TRUE) &&
                (IsInFile()    != ID_TRUE) &&
                (IsNoEditing() != ID_TRUE)) ? ID_TRUE : ID_FALSE;
    }
    
    /* BUG-36059 [ux-isql] Need to handle empty envirionment variables gracefully at iSQL */
    idBool HasValidEnvValue(SChar *aEnvValue ) {
    	return ( (aEnvValue != NULL) &&
    			 (aEnvValue[0] != '\0'))? ID_TRUE:ID_FALSE;
    }

public:
    FILE   * m_OutFile;

private:
    idBool   m_bExist_S;
    SChar    m_ServerName[WORD_LEN];
    idBool   m_bExist_U;
    SChar    m_LoginID[WORD_LEN];
    idBool   m_bExist_UserCert;
    SChar    m_UserCert[WORD_LEN];
    idBool   m_bExist_UserKey;
    SChar    m_UserKey[WORD_LEN];
    idBool   m_bExist_UserAID;
    SChar    m_UserAID[WORD_LEN];
    idBool   m_bExist_UnixdomainFilepath;
    SChar    m_UnixdomainFilepath[WORD_LEN];
    idBool   m_bExist_IpcFilepath;
    SChar    m_IpcFilepath[WORD_LEN];
    idBool   m_bExist_P;
    SChar    m_Password[WORD_LEN];
    idBool   m_bExist_F;
    SChar    m_InFileName[WORD_LEN];
    idBool   m_bExist_PORT;
    SInt     m_PortNo;
    idBool   m_bExist_O;
    SChar    m_OutFileName[WORD_LEN];
    idBool   m_bExist_SILENT;    /* silent mode */
    idBool   m_bExist_ATC;       /* for atc */
    idBool   m_bExist_ATAF;       /* for ataf */
    idBool   m_bExist_SYSDBA;    /* connect to db as sysdba */
    idBool   m_bExist_NOEDITING;
    idBool   m_bExistNLS_USE;
    SChar    m_NLS_USE[IDN_MAX_CHAR_SET_LEN];
    /* bug-20046 */
    idBool   m_bExist_PORT_login;
    idBool   m_bExistNLS_REPLACE;  // PROJ-1579 NCHAR
    SInt     m_NLS_REPLACE;        // PROJ-1579 NCHAR

    idBool   m_bExist_NOPROMPT; /* BUG-29760 */
    idBool   mPreferIPv6; /* BUG-29915 */
    idBool   m_bServPropsLoaded; /* BUG-27966 */

    idBool   m_bExist_TIME_ZONE;
    SChar    m_TimezoneString[TIMEZONE_STRING_LEN +1];

    /* BUG-41281 SSL */
    idBool   m_bExist_SslCa;
    SChar    m_SslCa[WORD_LEN];
    idBool   m_bExist_SslCapath;
    SChar    m_SslCapath[WORD_LEN];
    idBool   m_bExist_SslCert;
    SChar    m_SslCert[WORD_LEN];
    idBool   m_bExist_SslKey;
    SChar    m_SslKey[WORD_LEN];
    idBool   m_bExist_SslCipher;
    SChar    m_SslCipher[WORD_LEN];
    idBool   m_bExist_SslVerify;
    SChar    m_SslVerify[5];

    idBool   m_bExist_NOLOG; /* BUG-41476 */

    SInt     m_ConnectRetryMax; /* BUG-43352 */
};

#endif // _O_ISQLPROGOPTION_H_

 
