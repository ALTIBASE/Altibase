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
 * $Id: iloDownLoad.h 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

#ifndef _O_ILO_DOWNLOAD_H
#define _O_ILO_DOWNLOAD_H

#include <iloApi.h>

class iloDownLoad
{
public:
    iloDownLoad( ALTIBASE_ILOADER_HANDLE aHandle );

    void SetProgOption(iloProgOption *pProgOption)
    { m_pProgOption = pProgOption; }

    void SetSQLApi(iloSQLApi *pISPApi) { m_pISPApi = pISPApi; }

    SInt GetTableTree(ALTIBASE_ILOADER_HANDLE aHandle );

    SInt GetTableInfo( ALTIBASE_ILOADER_HANDLE aHandle );

    SInt GetQueryString( ALTIBASE_ILOADER_HANDLE aHandle );

    SInt DownLoad( ALTIBASE_ILOADER_HANDLE aHandle );

    SInt ExecuteQuery( ALTIBASE_ILOADER_HANDLE aHandle );

    SInt CompareAttrType();
    

    //PROJ-1714
    static void*    DownloadToFile_ThreadRun( void *arg );   

    void            DownloadToFile( ALTIBASE_ILOADER_HANDLE aHandle );

    iloTableInfo       m_TableInfo;
    SInt               mTotalCount;    
private:
    SInt               m_LoadCount;      //Progress를 확인하기 위함..
    iloProgOption     *m_pProgOption;
    iloFormCompiler    m_FormCompiler;
    iloTableTree       m_TableTree;
    iloSQLApi         *m_pISPApi;
    iloDataFile        m_DataFile;
    iloLogFile         m_LogFile;
    SInt               m_DataFileCnt;
    SInt               m_CBFrequencyCnt; /* BUG-30413 */

    iloBool IsLOBColExist();
    
    /* BUG-32114 aexport must support the import/export of partition tables. */
    void   PrintProgress( ALTIBASE_ILOADER_HANDLE aHandle,
                          SInt aDownLoadCount );

    /* BUG-42609 Code Refactoring */
    IDE_RC InitFiles(iloaderHandle *sHandle,
                     SChar         *sTableName);
    IDE_RC RunThread(iloaderHandle *sHandle);
    IDE_RC PrintMessages(iloaderHandle *sHandle,
                         SChar         *sTableName);
};

#endif /* _O_ILO_DOWNLOAD_H */
