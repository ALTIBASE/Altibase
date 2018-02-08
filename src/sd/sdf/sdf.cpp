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
 * $Id$
 **********************************************************************/
#include <sdf.h>

extern mtfModule sdfCreateMetaModule;
extern mtfModule sdfSetNodeModule;
extern mtfModule sdfResetNodeModule;
extern mtfModule sdfUnsetNodeModule;
extern mtfModule sdfSetShardTableModule;
extern mtfModule sdfSetShardProcedureModule;
extern mtfModule sdfSetShardRangeModule;
extern mtfModule sdfSetShardCloneModule;
extern mtfModule sdfSetShardSoloModule;
extern mtfModule sdfUnsetShardTableModule;
extern mtfModule sdfUnsetShardTableByIDModule;
extern mtfModule sdfExecImmediateModule;
// PROJ-2638
extern mtfModule sdfShardNodeName;
extern mtfModule sdfShardKey;
extern mtfModule sdfShardConditionModule;

const mtfModule* sdf::extendedFunctionModules[] =
{
    &sdfCreateMetaModule,
    &sdfSetNodeModule,
    &sdfResetNodeModule,
    &sdfUnsetNodeModule,
    &sdfSetShardTableModule,
    &sdfSetShardProcedureModule,
    &sdfSetShardRangeModule,
    &sdfSetShardCloneModule,
    &sdfSetShardSoloModule,
    &sdfUnsetShardTableModule,
    &sdfUnsetShardTableByIDModule,
    &sdfExecImmediateModule,
    NULL
};

const mtfModule* sdf::extendedFunctionModules2[] =
{
    &sdfShardNodeName,
    &sdfShardKey,
    &sdfShardConditionModule,
    NULL
};
