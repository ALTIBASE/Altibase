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
 * $Id: qrc.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#ifndef  _O_QRC_H_
#define _O_QRC_H_  1

#include <qc.h>
#include <qriParseTree.h>

class qrc
{
public:

    static IDE_RC validateCreate(qcStatement * aStatement);
    static IDE_RC validateOneReplItem(qcStatement * aStatement,
                                      qriReplItem  * aReplItem,
                                      SInt          aRole,
                                      idBool        aIsRecoveryOpt,
                                      SInt          aReplMode);
    static IDE_RC validateAlterAddTbl(qcStatement * aStatement);
    static IDE_RC validateAlterDropTbl(qcStatement * aStatement);
    static IDE_RC validateAlterAddHost(qcStatement * aStatement);
    static IDE_RC validateAlterDropHost(qcStatement * aStatement);
    static IDE_RC validateAlterSetHost(qcStatement * aStatement);
    static IDE_RC validateAlterSetMode(qcStatement * aStatement);
    static IDE_RC validateDrop(qcStatement * aStatement);
    static IDE_RC validateStart(qcStatement * aStatement);
    /* PROJ-1915 */
    static IDE_RC validateOfflineStart(qcStatement * aStatement);
    
    static IDE_RC validateQuickStart(qcStatement * aStatement);
    static IDE_RC validateSync(qcStatement * aStatement);
    static IDE_RC validateSyncTbl(qcStatement * aStatement);
    static IDE_RC validateReset(qcStatement * aStatement);
    static IDE_RC validateAlterSetRecovery(qcStatement * aStatement);
    static IDE_RC validateAlterSetOffline(qcStatement * aStatement);
    
    /* PROJ-1969 Gapless */
    static IDE_RC validateAlterSetGapless(qcStatement * aStatement);
    /* PROJ-1969 Parallel Receiver Apply */
    static IDE_RC validateAlterSetParallel(qcStatement * aStatement);
    /* PROJ-1969 Replicated Transaction Group */
    static IDE_RC validateAlterSetGrouping(qcStatement * aStatement);

    static IDE_RC executeCreate(qcStatement * aStatement);
    static IDE_RC executeAlterAddTbl(qcStatement * aStatement);
    static IDE_RC executeAlterDropTbl(qcStatement * aStatement);
    static IDE_RC executeAlterAddHost(qcStatement * aStatement);
    static IDE_RC executeAlterDropHost(qcStatement * aStatement);
    static IDE_RC executeAlterSetHost(qcStatement * aStatement);
    static IDE_RC executeAlterSetMode(qcStatement * aStatement);
    static IDE_RC executeDrop(qcStatement * aStatement);
    static IDE_RC executeStart(qcStatement * aStatement);
    static IDE_RC executeQuickStart(qcStatement * aStatement);
    static IDE_RC executeSync(qcStatement * aStatement);
    static IDE_RC executeStop(qcStatement * aStatement);
    static IDE_RC executeReset(qcStatement * aStatement);
    static IDE_RC executeFlush(qcStatement * aStatement);
    static IDE_RC executeAlterSetRecovery(qcStatement * aStatement);

    /* PROJ-1915 */
    static IDE_RC executeAlterSetOfflineEnable(qcStatement * aStatement);
    static IDE_RC executeAlterSetOfflineDisable(qcStatement * aStatement);

    /* PROJ-1969 Gapless */
    static IDE_RC executeAlterSetGapless(qcStatement * aStatement);
    /* PROJ-1969 Parallel Receiver Apply */
    static IDE_RC executeAlterSetParallel(qcStatement * aStatement);
    /* PROJ-1969 Replicated Transaction Group */
    static IDE_RC executeAlterSetGrouping(qcStatement * aStatement);

    static idBool isValidIPFormat(SChar * aIP);
};

#endif  // _O_QRC_H_
