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
 
#include <idl.h>
#include <time.h>
#include <sqlcli.h>
#include <idl.h>
#include <ide.h>

#define DISPLAY_WIDTH  700
#define DISPLAY_HEIGHT 700

void printHeader();
void printFooter();
void printRectangle( SQLDOUBLE aMinX,
                     SQLDOUBLE aMinY,
                     SQLDOUBLE aWidth,
                     SQLDOUBLE aHeight );

void printText( SQLDOUBLE    aMinX,
                SQLDOUBLE    aMinY,
                SQLDOUBLE    aWidth,
                SQLDOUBLE    aHeight,
                SQLUBIGINT   aPageSeq,
                SQLSMALLINT  aSlotSeq );

IDE_RC getTreeMBR( SQLHENV   aEnv,
                   SQLHDBC   aCon,
                   SDouble * aTreeMinX,
                   SDouble * aTreeMinY,
                   SDouble * aTreeMaxX,
                   SDouble * aTreeMaxY );

void initGlobalVariable();

IDE_RC parseArgs( int &aArgc, char **&aArgv );

void usage();

UChar * gIndexName;
UChar * gFilter;
UChar * gUID = (UChar*)"SYS";
UChar * gDNS = (UChar*)"127.0.0.1";
UChar * gPasswd = (UChar*)"MANAGER";
UInt    gPortNo = 20300;
idBool  gDisplayText;
UInt    gDisplayWidth;
UInt    gDisplayHeight;

int main (int argc, char ** argv)
{
    SQLHENV      sEnv;
    SQLHDBC      sCon;
    SQLHSTMT     sSelectStmt;
    
    SQLRETURN    sSqlRC;
    SQLCHAR      sErrODBC[6];
    SQLINTEGER   sErrNo;
    SQLCHAR      sErrMsg[256];
    SQLSMALLINT  sMsgLength;

    SQLDOUBLE    sMinX;
    SQLDOUBLE    sMinY;
    SQLDOUBLE    sMaxX;
    SQLDOUBLE    sMaxY;
    SQLDOUBLE    sTreeMinX;
    SQLDOUBLE    sTreeMinY;
    SQLDOUBLE    sTreeMaxX;
    SQLDOUBLE    sTreeMaxY;
    SQLDOUBLE    sScaleWidth;
    SQLDOUBLE    sScaleHeight;
    SQLUBIGINT   sPageSeq;
    SQLSMALLINT  sSlotSeq;
    SQLLEN       sLen = ID_SIZEOF(SQLDOUBLE);
    SQLCHAR      sQuery[1024];
    SQLCHAR      sConnectString[1024];

    initGlobalVariable();

    IDE_TEST( parseArgs( argc, argv ) != IDE_SUCCESS );
    
    sSqlRC = SQLAllocEnv(&sEnv);
    IDE_TEST_RAISE( sSqlRC != SQL_SUCCESS, ERR_ALLOC_ENV );
    
    sSqlRC = SQLAllocConnect(sEnv, &sCon);
    IDE_TEST_RAISE( sSqlRC != SQL_SUCCESS, ERR_ALLOC_CONNECT );

    idlOS::sprintf( (SChar*)sConnectString,
                    "DSN={%s};UID={%s};PWD={%s};PORT_NO=%"ID_INT32_FMT,
                    gDNS,
                    gUID,
                    gPasswd,
                    gPortNo );

    sSqlRC = SQLDriverConnect( sCon, NULL,
                               sConnectString,
                               SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT );
    IDE_TEST_RAISE( sSqlRC != SQL_SUCCESS, ERR_DRIVER_CONNECT );

    IDE_TEST( getTreeMBR( sEnv,
                          sCon,
                          &sTreeMinX,
                          &sTreeMinY,
                          &sTreeMaxX,
                          &sTreeMaxY ) != IDE_SUCCESS );
    
    sSqlRC = SQLAllocStmt( sCon, &sSelectStmt );
    IDE_TEST_RAISE( sSqlRC != SQL_SUCCESS, ERR_ALLOC_STMT );

    if( gFilter != NULL )
    {
        idlOS::sprintf( (SChar*)sQuery,
                        "SELECT PAGE_SEQ, NTH_SLOT, MIN_X, MIN_Y, MAX_X, MAX_Y "
                        "FROM D$DISK_INDEX_RTREE_KEY( %s ) "
                        "WHERE %s;",
                        gIndexName,
                        gFilter );
    }
    else
    {
        idlOS::sprintf( (SChar*)sQuery,
                        "SELECT PAGE_SEQ, NTH_SLOT, MIN_X, MIN_Y, MAX_X, MAX_Y "
                        "FROM D$DISK_INDEX_RTREE_KEY( %s );",
                        gIndexName );
    }
                    
    sSqlRC = SQLExecDirect( sSelectStmt, sQuery, SQL_NTS );
    IDE_TEST_RAISE( sSqlRC != SQL_SUCCESS, ERR_EXEC_DIRECT );

    sLen = ID_SIZEOF(SQLUBIGINT);
    if( SQL_SUCCESS != SQLBindCol( sSelectStmt,
                                   1,
                                   SQL_C_UBIGINT,
                                   &sPageSeq,
                                   ID_SIZEOF( sPageSeq ),
                                   &sLen ) )
    {
        IDE_RAISE( ERR_BIND_COL );
    }
    
    sLen = ID_SIZEOF(SQLSMALLINT);
    if( SQL_SUCCESS != SQLBindCol( sSelectStmt,
                                   2,
                                   SQL_C_SSHORT,
                                   &sSlotSeq,
                                   ID_SIZEOF( sSlotSeq ),
                                   &sLen ) )
    {
        IDE_RAISE( ERR_BIND_COL );
    }

    sLen = ID_SIZEOF(SQLDOUBLE);
    if( SQL_SUCCESS != SQLBindCol( sSelectStmt,
                                   3,
                                   SQL_C_DOUBLE,
                                   &sMinX,
                                   ID_SIZEOF( sMinX ),
                                   &sLen ) )
    {
        IDE_RAISE( ERR_BIND_COL );
    }

    if( SQL_SUCCESS != SQLBindCol( sSelectStmt,
                                   4,
                                   SQL_C_DOUBLE,
                                   &sMinY,
                                   ID_SIZEOF( sMinY ),
                                   &sLen ) )
    {
        IDE_RAISE( ERR_BIND_COL );
    }

    if( SQL_SUCCESS != SQLBindCol( sSelectStmt,
                                   5,
                                   SQL_C_DOUBLE,
                                   &sMaxX,
                                   ID_SIZEOF( sMaxX ),
                                   &sLen ) )
    {
        IDE_RAISE( ERR_BIND_COL );
    }

    if( SQL_SUCCESS != SQLBindCol( sSelectStmt,
                                   6,
                                   SQL_C_DOUBLE,
                                   &sMaxY,
                                   ID_SIZEOF( sMaxY ),
                                   &sLen ) )
    {
        IDE_RAISE( ERR_BIND_COL );
    }

    printHeader();
    
    sScaleWidth  = gDisplayWidth  / (sTreeMaxX - sTreeMinX);
    sScaleHeight = gDisplayHeight / (sTreeMaxY - sTreeMinY);

    while( SQL_SUCCESS == SQLFetch( sSelectStmt ) )
    {
        printRectangle( (sMinX - sTreeMinX) * sScaleWidth,
                        (sMinY - sTreeMinY) * sScaleHeight,
                        (sMaxX - sMinX) * sScaleWidth,
                        (sMaxY - sMinY) * sScaleHeight );

        if( gDisplayText == ID_TRUE )
        {
            printText( (sMinX - sTreeMinX) * sScaleWidth,
                       (sMinY - sTreeMinY) * sScaleHeight,
                       (sMaxX - sMinX) * sScaleWidth,
                       (sMaxY - sMinY) * sScaleHeight,
                       sPageSeq,
                       sSlotSeq );
        }
    }

    printFooter();

    sSqlRC = SQLFreeStmt( sSelectStmt, SQL_DROP );
    IDE_TEST_RAISE( sSqlRC != SQL_SUCCESS, ERR_FREE_STMT );

    sSqlRC = SQLDisconnect( sCon );
    IDE_TEST_RAISE( sSqlRC != SQL_SUCCESS, ERR_DISCONNECT );
    
    sSqlRC = SQLFreeConnect( sCon );
    IDE_TEST_RAISE( sSqlRC != SQL_SUCCESS, ERR_FREE_CONNECT );
    
    sSqlRC = SQLFreeEnv( sEnv );
    IDE_TEST_RAISE( sSqlRC != SQL_SUCCESS, ERR_FREE_ENV );

    return 0;
    
    IDE_EXCEPTION( ERR_ALLOC_ENV )
    {
        idlOS::printf( "Environment Allocation Failed\n" );
    }
    IDE_EXCEPTION( ERR_ALLOC_CONNECT )
    {
        idlOS::printf( "Connection Allocation Failed\n" );
    }
    IDE_EXCEPTION( ERR_DRIVER_CONNECT )
    {
        idlOS::printf( "Connection Failed\n%s\n", sConnectString );
    }
    IDE_EXCEPTION( ERR_ALLOC_STMT )
    {
        idlOS::printf( "Statement Allocation Failed\n" );
    }
    IDE_EXCEPTION( ERR_EXEC_DIRECT )
    {
        idlOS::printf( "Execution Failed\n%s\n", sQuery );
    }
    IDE_EXCEPTION( ERR_BIND_COL )
    {
        sSqlRC = SQLError ( sEnv, sCon, sSelectStmt, sErrODBC, &sErrNo,
                            sErrMsg, 256, &sMsgLength );
        idlOS::printf(" Error : # %s, %"ID_INT32_FMT", %s\n", sErrODBC, sErrNo, sErrMsg);
    }
    IDE_EXCEPTION( ERR_FREE_STMT )
    {
    }
    IDE_EXCEPTION( ERR_DISCONNECT )
    {
    }
    IDE_EXCEPTION( ERR_FREE_CONNECT )
    {
    }
    IDE_EXCEPTION( ERR_FREE_ENV )
    {
    }
    IDE_EXCEPTION_END;
    
    return 1;
}


void printHeader()
{
    idlOS::printf( "<?xml version=\"1.0\" standalone=\"no\"?>\n"
                   "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n"
                   "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"
                   "<svg width=\"100%\" height=\"100%\" version=\"1.1\"\n"
                   "xmlns=\"http://www.w3.org/2000/svg\">\n" );
}

void printFooter()
{
    idlOS::printf( "</svg>\n" );
}    

void printRectangle( SQLDOUBLE aMinX,
                     SQLDOUBLE aMinY,
                     SQLDOUBLE aWidth,
                     SQLDOUBLE aHeight )
{
    idlOS::printf( "<rect x=\"%"ID_DOUBLE_G_FMT"\" y=\"%"ID_DOUBLE_G_FMT"\" "
                   "width=\"%"ID_DOUBLE_G_FMT"\" height=\"%"ID_DOUBLE_G_FMT"\" "
                   "style=\"fill:blue;stroke:pink;stroke-width:3;"
                   "fill-opacity:0.1;stroke-opacity:0.9\"/>\n",
                   aMinX,
                   aMinY,
                   aWidth,
                   aHeight );
}

void printText( SQLDOUBLE    aMinX,
                SQLDOUBLE    aMinY,
                SQLDOUBLE    aWidth,
                SQLDOUBLE    aHeight,
                SQLUBIGINT   aPageSeq,
                SQLSMALLINT  aSlotSeq )
{
    idlOS::printf( "<text x=\"%"ID_DOUBLE_G_FMT"\" y=\"%"ID_DOUBLE_G_FMT"\" "
                   "font-family=\"Verdana\" font-size=\"15\" fill=\"blue\" >"
                   "(%"ID_UINT64_FMT",%"ID_INT32_FMT")"
                   "</text>\n",
                   aMinX + (aWidth/2),
                   aMinY + (aHeight/2),
                   aPageSeq,
                   aSlotSeq );
}

void initGlobalVariable()
{
    gFilter      = NULL;
    gDisplayText = ID_TRUE;
    gIndexName   = NULL;
}

IDE_RC parseArgs( int &aArgc, char **&aArgv )
{
    SInt sOpr;

    sOpr = idlOS::getopt( aArgc, aArgv, "i:f:w:h:u:p:d:n:s" );
    IDE_TEST_RAISE( sOpr == EOF, ERR_INVALID_ARGUMENTS );

    gDisplayWidth  = DISPLAY_WIDTH;
    gDisplayHeight = DISPLAY_HEIGHT;
    
    do
    {
        switch( sOpr )
        {
            case 'i':
                gIndexName     = (UChar*)optarg;
                break;
            case 'f':
                gFilter        = (UChar*)optarg;
                break;
            case 'w':
                gDisplayWidth  = idlOS::atoi( optarg );
                break;
            case 'h':
                gDisplayHeight = idlOS::atoi( optarg );
                break;
            case 's':
                gDisplayText   = ID_FALSE;
                break;
            case 'u':
                gUID           = (UChar*)optarg;
                break;
            case 'd':
                gDNS           = (UChar*)optarg;
                break;
            case 'p':
                gPasswd        = (UChar*)optarg;
                break;
            case 'n':
                gPortNo        = idlOS::atoi( optarg );
                break;
            default:
                idlOS::exit( -1 );
                break;
        }
    }
    while( (sOpr = idlOS::getopt(aArgc, aArgv, "i:f:w:h:u:p:d:n:s")) != EOF );

    IDE_TEST_RAISE( gIndexName == NULL, ERR_INVALID_ARGUMENTS );

    idlOS::strUpper( (void*)gIndexName, idlOS::strlen( (SChar*)gIndexName ) );
    
    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_INVALID_ARGUMENTS );
    {
        (void)usage();
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void usage()
{
    idlOS::printf( "\n%-6s:  asv -i Index_name [-f Filter_string] "
                   "[-w display_Width] [-h display_Height] [-s] "
                   "[-u user_name] [-p password] [-d dns] [-n port_no] > dest_file.svg\n",
                   "Usage" );

    idlOS::printf( " %-4s : %s\n", "-i", "specify index name" );
    idlOS::printf( " %-4s : %s\n", "-f", "specify filter string" );
    idlOS::printf( " %-4s : %s\n", "-w", "specify display width ( default: 700 )" );
    idlOS::printf( " %-4s : %s\n", "-h", "specify display height ( default: 700 )" );
    idlOS::printf( " %-4s : %s\n", "-s", "silence text string" );
    idlOS::printf( " %-4s : %s\n", "-u", "specify UID ( default: SYS )" );
    idlOS::printf( " %-4s : %s\n", "-p", "specify password ( default: MANAGER )" );
    idlOS::printf( " %-4s : %s\n", "-d", "specify DNS ( default: 127.0.0.1 )" );
    idlOS::printf( " %-4s : %s\n", "-n", "specify port no ( default: 20300 )" );
    idlOS::printf( "\n" );
}

IDE_RC getTreeMBR( SQLHENV   aEnv,
                   SQLHDBC   aCon,
                   SDouble * aTreeMinX,
                   SDouble * aTreeMinY,
                   SDouble * aTreeMaxX,
                   SDouble * aTreeMaxY )
{
    SQLHSTMT     sSelectStmt;
    SQLCHAR      sQuery[1000];
    
    SQLRETURN    sSqlRC;
    SQLCHAR      sErrODBC[6];
    SQLINTEGER   sErrNo;
    SQLCHAR      sErrMsg[256];
    SQLSMALLINT  sMsgLength;

    SQLDOUBLE    sMinX;
    SQLDOUBLE    sMinY;
    SQLDOUBLE    sMaxX;
    SQLDOUBLE    sMaxY;
    SQLLEN       sLen = ID_SIZEOF(SQLDOUBLE);
    
    sSqlRC = SQLAllocStmt( aCon, &sSelectStmt );
    IDE_TEST_RAISE( sSqlRC != SQL_SUCCESS, ERR_ALLOC_STMT );

    idlOS::sprintf( (SChar*)sQuery,
                    "SELECT MIN_X, MIN_Y, MAX_X, MAX_Y "
                    "FROM X$DISK_RTREE_HEADER "
                    "WHERE INDEX_NAME = '%s';",
                    gIndexName );

    sSqlRC = SQLExecDirect( sSelectStmt, sQuery, SQL_NTS );
    IDE_TEST_RAISE( sSqlRC != SQL_SUCCESS, ERR_EXEC_DIRECT );

    if( SQL_SUCCESS != SQLBindCol( sSelectStmt,
                                   1,
                                   SQL_C_DOUBLE,
                                   &sMinX,
                                   ID_SIZEOF( sMinX ),
                                   &sLen ) )
    {
        IDE_RAISE( ERR_BIND_COL );
    }

    if( SQL_SUCCESS != SQLBindCol( sSelectStmt,
                                   2,
                                   SQL_C_DOUBLE,
                                   &sMinY,
                                   ID_SIZEOF( sMinY ),
                                   &sLen ) )
    {
        IDE_RAISE( ERR_BIND_COL );
    }

    if( SQL_SUCCESS != SQLBindCol( sSelectStmt,
                                   3,
                                   SQL_C_DOUBLE,
                                   &sMaxX,
                                   ID_SIZEOF( sMaxX ),
                                   &sLen ) )
    {
        IDE_RAISE( ERR_BIND_COL );
    }

    if( SQL_SUCCESS != SQLBindCol( sSelectStmt,
                                   4,
                                   SQL_C_DOUBLE,
                                   &sMaxY,
                                   ID_SIZEOF( sMaxY ),
                                   &sLen ) )
    {
        IDE_RAISE( ERR_BIND_COL );
    }
    
    while( SQL_SUCCESS == SQLFetch( sSelectStmt ) )
    {
        *aTreeMinX = sMinX;
        *aTreeMinY = sMinY;
        *aTreeMaxX = sMaxX;
        *aTreeMaxY = sMaxY;
    }

    sSqlRC = SQLFreeStmt( sSelectStmt, SQL_DROP );
    IDE_TEST_RAISE( sSqlRC != SQL_SUCCESS, ERR_FREE_STMT );
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION( ERR_ALLOC_STMT )
    {
    }
    IDE_EXCEPTION( ERR_EXEC_DIRECT )
    {
    }
    IDE_EXCEPTION( ERR_BIND_COL )
    {
        sSqlRC = SQLError ( aEnv, aCon, sSelectStmt, sErrODBC, &sErrNo,
                            sErrMsg, 256, &sMsgLength );
        idlOS::printf(" Error : # %s, %"ID_INT32_FMT", %s\n", sErrODBC, sErrNo, sErrMsg);
    }
    IDE_EXCEPTION( ERR_FREE_STMT )
    {
    }
    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
}
