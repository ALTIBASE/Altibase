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
 * $Id: stm.cpp 16545 2006-06-07 01:06:51Z laufer $
 **********************************************************************/

#include <stm.h>

const void * gStmColumns;
const void * gStmUserColumns;
const void * gStmSRS;
const void * gStmProjCS;
const void * gStmProjections;
const void * gStmGeogCS;
const void * gStmGeocCS;
const void * gStmDatums;
const void * gStmEllipsoids;
const void * gStmPrimems;

const void * gStmColumnsIndex       [STM_MAX_META_INDICES];
const void * gStmUserColumnsIndex   [STM_MAX_META_INDICES];
const void * gStmSRSIndex           [STM_MAX_META_INDICES];
const void * gStmProjCSIndex        [STM_MAX_META_INDICES];
const void * gStmProjectionsIndex   [STM_MAX_META_INDICES];
const void * gStmGeogCSIndex        [STM_MAX_META_INDICES];
const void * gStmGeocCSIndex        [STM_MAX_META_INDICES];
const void * gStmDatumsIndex        [STM_MAX_META_INDICES];
const void * gStmEllipsoidsIndex    [STM_MAX_META_INDICES];
const void * gStmPrimemsIndex       [STM_MAX_META_INDICES];


IDE_RC
stm::initializeGlobalVariables( void )
{
    SInt              i;

    gStmColumns     = NULL;
    gStmUserColumns = NULL;
    gStmSRS         = NULL;
    gStmProjCS      = NULL;
    gStmProjections = NULL;
    gStmGeogCS      = NULL;
    gStmGeocCS      = NULL;
    gStmDatums      = NULL;
    gStmEllipsoids  = NULL;
    gStmPrimems     = NULL;

    for (i = 0; i < STM_MAX_META_INDICES; i ++)
    {
        gStmColumnsIndex[i]     = NULL;
        gStmUserColumnsIndex[i] = NULL;
        gStmSRSIndex[i]         = NULL;
        gStmProjCSIndex[i]      = NULL;
        gStmProjectionsIndex[i] = NULL;
        gStmGeogCSIndex[i]      = NULL;
        gStmGeocCSIndex[i]      = NULL;
        gStmDatumsIndex[i]      = NULL;
        gStmEllipsoidsIndex[i]  = NULL;
        gStmPrimemsIndex[i]     = NULL;
    }

    return IDE_SUCCESS;
}

IDE_RC
stm::initMetaHandles( smiStatement * aSmiStmt )
{
    /**************************************************************
                        Get Table Handles
    **************************************************************/
    
    IDE_TEST( qcm::getMetaTable( STM_STO_COLUMNS,
                                 & gStmColumns,
                                 aSmiStmt ) != IDE_SUCCESS );

    IDE_TEST( qcm::getMetaTable( STM_STO_USER_COLUMNS,
                                 & gStmUserColumns,
                                 aSmiStmt ) != IDE_SUCCESS );

    IDE_TEST( qcm::getMetaTable( STM_STO_SRS,
                                 & gStmSRS,
                                 aSmiStmt ) != IDE_SUCCESS );

    IDE_TEST( qcm::getMetaTable( STM_STO_PROJCS,
                                 & gStmProjCS,
                                 aSmiStmt ) != IDE_SUCCESS );

    IDE_TEST( qcm::getMetaTable( STM_STO_PROJECTIONS,
                                 & gStmProjections,
                                 aSmiStmt ) != IDE_SUCCESS );

    IDE_TEST( qcm::getMetaTable( STM_STO_GEOGCS,
                                 & gStmGeogCS,
                                 aSmiStmt) != IDE_SUCCESS );

    IDE_TEST( qcm::getMetaTable( STM_STO_GEOCCS,
                                 & gStmGeocCS,
                                 aSmiStmt ) != IDE_SUCCESS );

    IDE_TEST( qcm::getMetaTable( STM_STO_DATUMS,
                                 & gStmDatums,
                                 aSmiStmt) != IDE_SUCCESS );

    IDE_TEST( qcm::getMetaTable( STM_STO_ELLIPSOIDS,
                                 & gStmEllipsoids,
                                 aSmiStmt ) != IDE_SUCCESS );

    IDE_TEST( qcm::getMetaTable( STM_STO_PRIMEMS,
                                 & gStmPrimems,
                                 aSmiStmt ) != IDE_SUCCESS );

    /**************************************************************
                        Get Index Handles
    **************************************************************/

    /*
    IDE_TEST( qcm::getMetaIndex( gStmColumnsIndex,
                            gStmColumns ) != IDE_SUCCESS );

    IDE_TEST( qcm::getMetaIndex( gStmUserColumnsIndex,
                            gStmUserColumns ) != IDE_SUCCESS );

    IDE_TEST( qcm::getMetaIndex( gStmSRSIndex,
                            gStmSRS ) != IDE_SUCCESS );

    IDE_TEST( qcm::getMetaIndex( gStmProjCSIndex,
                            gStmProjCS ) != IDE_SUCCESS );

    IDE_TEST( qcm::getMetaIndex( gStmProjectionsIndex,
                            gStmProjections ) != IDE_SUCCESS );

    IDE_TEST( qcm::getMetaIndex( gStmGeogCSIndex,
                            gStmGeogCS ) != IDE_SUCCESS );

    IDE_TEST( qcm::getMetaIndex( gStmGeocCSIndex,
                            gStmGeocCS ) != IDE_SUCCESS );

    IDE_TEST( qcm::getMetaIndex( gStmDatumsIndex,
                            gStmDatums ) != IDE_SUCCESS );

    IDE_TEST( qcm::getMetaIndex( gStmEllipsoidsIndex,
                            gStmEllipsoids ) != IDE_SUCCESS);

    IDE_TEST( qcm::getMetaIndex( gStmPrimemsIndex,
                            gStmPrimems ) != IDE_SUCCESS );
    */
                            
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC stm::makeAndSetStmColumnInfo( void * /* aSmiStmt */,
                                     void * /* aTableInfo */,
                                     UInt /* aIndex */)
{
    return IDE_SUCCESS;

    // IDE_EXCEPTION_END;

    // return IDE_FAILURE;
}                                     
