/**
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ULP_LIB_INTERCOREFUNCA_H_
#define _ULP_LIB_INTERCOREFUNCA_H_ 1

#include <acp.h>
#include <aciTypes.h>
#include <aciErrorMgr.h>
#include <ulo.h>
#include <sqlcli.h>
#include <ulpLibInterCoreFuncB.h>
#include <ulpLibStmtCur.h>
#include <ulpLibConnection.h>
#include <ulpLibInterface.h>
#include <ulpLibOption.h>
#include <ulpLibError.h>
#include <ulpLibMacro.h>
#include <ulpTypes.h>
#include <mtcl.h>            /* mtlUTF16, mtlUTF8*/


/* library internal core functions */

ACI_RC ulpPrepareCore( ulpLibConnNode *aConnNode,
                       ulpLibStmtNode *aStmtNode,
                       acp_char_t     *aQuery,
                       ulpSqlca       *aSqlca,
                       ulpSqlcode     *aSqlcode,
                       ulpSqlstate    *aSqlstate );

ACI_RC ulpDynamicBindParameter( ulpLibConnNode *aConnNode,
                                ulpLibStmtNode *aStmtNode,
                                SQLHSTMT       *aHstmt,
                                ulpSqlstmt     *aSqlstmt,
                                SQLDA          *aBinda,
                                ulpSqlca       *aSqlca,
                                ulpSqlcode     *aSqlcode,
                                ulpSqlstate    *aSqlstate );

ACI_RC ulpDynamicBindCol( ulpLibConnNode *aConnNode,
                          ulpLibStmtNode *aStmtNode,
                          SQLHSTMT       *aHstmt,
                          ulpSqlstmt     *aSqlstmt,
                          SQLDA          *aBinda,
                          ulpSqlca       *aSqlca,
                          ulpSqlcode     *aSqlcode,
                          ulpSqlstate    *aSqlstate );

ACI_RC ulpBindHostVarCore ( ulpLibConnNode *aConnNode,
                            ulpLibStmtNode *aStmtNode,
                            SQLHSTMT       *aHstmt,
                            ulpSqlstmt     *aSqlstmt,
                            ulpSqlca       *aSqlca,
                            ulpSqlcode     *aSqlcode,
                            ulpSqlstate    *aSqlstate );

ACI_RC ulpExecuteCore( ulpLibConnNode *aConnNode,
                       ulpLibStmtNode *aStmtNode,
                       ulpSqlstmt     *aSqlstmt,
                       SQLHSTMT       *aHstmt );

ACI_RC ulpFetchCore( ulpLibConnNode *aConnNode,
                     ulpLibStmtNode *aStmtNode,
                     SQLHSTMT       *aHstmt,
                     ulpSqlstmt     *aSqlstmt,
                     ulpSqlca       *aSqlca,
                     ulpSqlcode     *aSqlcode,
                     ulpSqlstate    *aSqlstate );

ACI_RC ulpExecDirectCore( ulpLibConnNode *aConnNode,
                          ulpLibStmtNode *aStmtNode,
                          ulpSqlstmt     *aSqlstmt,
                          SQLHSTMT       *aHstmt,
                          acp_char_t     *aQuery );

ACI_RC ulpCloseStmtCore( ulpLibConnNode *aConnNode,
                         ulpLibStmtNode *aStmtNode,
                         SQLHSTMT       *aHstmt,
                         ulpSqlca       *aSqlca,
                         ulpSqlcode     *aSqlcode,
                         ulpSqlstate    *aSqlstate );

acp_bool_t ulpCheckNeedReBindAllCore( ulpLibStmtNode *aStmtNode,
                                      ulpSqlstmt     *aSqlstmt );

acp_bool_t ulpCheckNeedReBindParamCore( ulpLibStmtNode *aStmtNode,
                                        ulpSqlstmt     *aSqlstmt );

acp_bool_t ulpCheckNeedReBindColCore( ulpLibStmtNode *aStmtNode,
                                      ulpSqlstmt     *aSqlstmt );

acp_bool_t ulpCheckNeedReSetStmtAttrCore( ulpLibStmtNode *aStmtNode,
                                          ulpSqlstmt     *aSqlstmt );
#endif
