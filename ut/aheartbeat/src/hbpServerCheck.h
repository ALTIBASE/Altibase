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
 
#ifndef _HBP_SERVERCHECK_H_
#define _HBP_SERVERCHECK_H_ (1)

#include <hbp.h>


/* allocate one Env handle */
ACI_RC hbpAllocEnvHandle( DBCInfo  *aDBCInfo );
/* allocate one DBC handle */
ACI_RC hbpAllocDbcHandle( DBCInfo  *aDBCInfo );

ACI_RC hbpDBConnectIfClosed( DBCInfo *aDBCInfo );

/* connect one db with IP,UID,Passwd
 * aDbCInfo(in/output)
 */
ACI_RC hbpDbConnect( DBCInfo    *aDBCInfo );


ACI_RC hbpAllocStmt( DBCInfo    *aDBCInfo );

/* free one handle */
void hbpFreeHandle( DBCInfo *aDBCInfo );

/* execute Select 1+1 from dual with DB Connection
 * aDBCInfo(input)
 * it will return ACP_RC_SUCCESS when successfully get 2 as a result
 */
ACI_RC hbpExecuteSelect( DBCInfo *aDBCInfo );

/* initialize one DBCInfo
 * aDBCInfo(input)
 * aUserName(Input)
 * aPassWord(Input)
 * aHostInfo(Input)
 */
ACI_RC hbpInitDBCInfo( DBCInfo        *aDBCInfo,
                       acp_char_t     *aUserName,
                       acp_char_t     *aPassWord,
                       HostInfo        aHostInfo );
/*
 *
 */
ACI_RC hbpExecuteQuery( DBCInfo *aDBCInfo, acp_char_t *aQuery );
/*
 *
 */
ACI_RC hbpSetTimeOut( DBCInfo *aDBCInfo );

/* Initialize DBAInfo
 * aDBAInfo(input)  : DBAInfo
 * aUserName(input) : UID
 * aPassWord(input) : Password
 */
ACI_RC hbpInitializeDBAInfo( DBAInfo        *aDBAInfo,
                             acp_char_t     *aUserName,
                             acp_char_t     *aPassWord );

/* execute Select 1+1 from dual with DBAInfo Once
 * aDBAInfo(input)  : DBAInfo Array
 * It returns ACP_RC_APP_ERROR when select failed with all connection info.
 */
ACI_RC hbpSelectFromDBAInfoOnce( DBAInfo *aDBAInfo );

/* execute Select 1+1 from dual with DBAInfo.
 * it calls hbpSelectFromDBAInfoOnce.
 * aDBAInfo(input)  : DBAInfo
 */
ACI_RC hbpSelectFromDBAInfo( DBAInfo *aDBAInfo );

ACI_RC hbpAltibaseServerCheck( DBAInfo *aDBAInfo );

/* free DBAInfo
 * aDBAInfo(input)  : DBAInfo
 * aCount(input)    : the number of DBCInfo in DBAInfo
 */
void hbpFreeDBAInfo( DBAInfo *aDBAInfo, acp_uint32_t aCount );

ACI_RC hbpAltibaseConnectionCheck( DBAInfo *aDBAInfo );

/* get ALTIBASE Port No
 * aPort(output) : port No
 */
ACI_RC hbpGetAltibasePort( acp_uint32_t *aPort );

ACI_RC hbpGetHBPHome( acp_char_t **aHBPHome );
#endif
