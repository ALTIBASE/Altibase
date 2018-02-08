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
 * $Id: tsm_test.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <tsm.h>
#include <smi.h>

extern UInt   gCursorFlag;

/****************************************************
CREATE TABLE 1.TABLE1 (
    COLUMN1 UINT,
    COLUMN2 STRING,
    COLUMN3 VARCHAR
);

CREATE TABLE 2.TABLE2 (
    COLUMN1 UINT,
    COLUMN2 STRING,
    COLUMN3 VARCHAR,
    COLUMN4 UINT,
    COLUMN5 STRING,
    COLUMN6 VARCHAR
);

CREATE TABLE 3.TABLE3 (
    COLUMN1 UINT,
    COLUMN2 STRING,
    COLUMN3 VARCHAR,
    COLUMN4 UINT,
    COLUMN5 STRING,
    COLUMN6 VARCHAR,
    COLUMN7 UINT,
    COLUMN8 STRING,
    COLUMN9 VARCHAR
);
*****************************************************/

int main()
{
    smiTrans         sTrans;
    smiStatement *spRootStmt;
    smiStatement  sStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    smiTableCursor   sCursor;
    tsmCursorData    sCursorData;
    SChar            sTableName[1024] ="Table1";
///////////////////////////////////////////////
    UInt            sUInt[3];
    SChar           sString[3][4096];
    SChar           sVarchar[3][4096];
    smiValue        sValue[9] = {
        { 4, &sUInt[0]    },
        { 0, &sString[0]  },
        { 0, &sVarchar[0] },
        { 4, &sUInt[1]    },
        { 0, &sString[1]  },
        { 0, &sVarchar[1] },
        { 4, &sUInt[2]    },
        { 0, &sString[2]  },
        { 0, &sVarchar[2] }
    };
    UInt            sUIntValues[19] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000,
        0xFFFFFFFF
    };
    UInt            sCount[3];
    const void*     sTable;
    void          * sDummyRow;
    scGRID          sDummyGRID;
///////////////////////////////////////////////

    gVerbose = ID_TRUE;


    if( tsmInit() != IDE_SUCCESS )
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmInit Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmInit Success!" );
    idlOS::fflush( TSM_OUTPUT );
    
    if( smiStartup(SMI_STARTUP_PRE_PROCESS,
                   SMI_STARTUP_NOACTION,
                   &gTsmGlobalCallBackList)
             != IDE_SUCCESS)
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiInit Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    if( smiStartup(SMI_STARTUP_PROCESS,
                   SMI_STARTUP_NOACTION,
                   &gTsmGlobalCallBackList)
             != IDE_SUCCESS)
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiInit Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    if( smiStartup(SMI_STARTUP_CONTROL,
                   SMI_STARTUP_NOACTION,
                   &gTsmGlobalCallBackList)
             != IDE_SUCCESS)
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiInit Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    if( smiStartup(SMI_STARTUP_META,
                   SMI_STARTUP_NOACTION,
                   &gTsmGlobalCallBackList)
             != IDE_SUCCESS)
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiInit Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }

    tsmCreateTable( 1, sTableName, TSM_TABLE1 );
    tsmCreateIndex( 1, sTableName, TSM_TABLE1_INDEX_UINT );
    tsmCreateIndex( 1, sTableName, TSM_TABLE1_INDEX_STRING );
    tsmCreateIndex( 1, sTableName, TSM_TABLE1_INDEX_VARCHAR );


    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag) != IDE_SUCCESS );
    
    tsmOpenCursor(   &sStmt,
                     1,
                     sTableName,
                     TSM_TABLE1_INDEX_NONE,
                     SMI_LOCK_WRITE|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE,
                     SMI_INSERT_CURSOR,
                     & sCursor,
                     & sCursorData );

    for( sCount[0] = 0; sCount[0] < 19; sCount[0]++ )
    {
        sUInt[0] = sUIntValues[sCount[0]];
        if( sUIntValues[sCount[0]] != 0xFFFFFFFF )
        {
            idlOS::sprintf( sString[0], "%09u", sUIntValues[sCount[0]] );
            sValue[1].length = idlOS::strlen( sString[0] ) + 1;
            idlOS::sprintf( sVarchar[0], "%09u", sUIntValues[sCount[0]] );
            sValue[2].length = idlOS::strlen( sVarchar[0] ) + 1;
        }
        else
        {
             idlOS::strcpy( sString[0], "" );
            sValue[1].length = 1;
            sValue[2].length = 0;
        }
        IDE_TEST( sCursor.insertRow( sValue,
                                     &sDummyRow,
                                     &sDummyGRID )
                  != IDE_SUCCESS );
    }
    
    IDE_TEST( sCursor.close() != IDE_SUCCESS );

    IDE_TEST( qcxSearchTable ( &sStmt,
                               &sTable,
                               1,
                               sTableName )
              != IDE_SUCCESS );
    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)sTableName,
                            sTable,
                            TSM_TABLE1_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );
    
    tsmDropIndex( 1, sTableName, TSM_TABLE1_INDEX_UINT );
    tsmDropIndex( 1, sTableName, TSM_TABLE1_INDEX_STRING );
    tsmDropIndex( 1, sTableName, TSM_TABLE1_INDEX_VARCHAR );
    tsmDropTable( 1, sTableName );

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    IDE_TEST( sTrans.commit(&sDummySCN) );

    IDE_TEST( sTrans.destroy() );
    
    if( smiStartup( SMI_STARTUP_SHUTDOWN,
                    SMI_STARTUP_NOACTION,
                    &gTsmGlobalCallBackList )
        != IDE_SUCCESS )
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiFinal Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiFinal Success!" );
    idlOS::fflush( TSM_OUTPUT );

    if( tsmFinal() != IDE_SUCCESS )
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmFinal Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmFinal Success!" );
    idlOS::fflush( TSM_OUTPUT );

    return EXIT_SUCCESS;
    
    failure:
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "== TSM CURSOR TEST END WITH FAILURE ==" );    
    idlOS::fflush( TSM_OUTPUT );
    
    IDE_EXCEPTION_END;

    ideDump();
    
    return EXIT_FAILURE;

    return 0;
}

