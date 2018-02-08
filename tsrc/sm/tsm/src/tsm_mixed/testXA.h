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
 * $Id: testXA.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#ifndef _O_TEST_XA_H_
#define _O_TEST_XA_H_ 1

#define TEST_MAX_COUNT    200
#define TEST_THREAD_COUNT 3 //Max is 4

typedef IDE_RC (*tsmRecFuncType)(smiTrans*, void*);

typedef struct tsmRecFunc
{
    SChar           m_parm[512];
    smiTrans       *m_pTrans;
    tsmRecFuncType  m_pFunc;
} tsmRecFunc;

typedef struct tsmThreadInfo
{
    UInt            m_threadNo;
    smiTrans        m_trans;
    tsmRecFunc      m_arrTestFunc[TEST_MAX_COUNT];
    idBool          m_bRun;
} tsmThreadInfo;

#define TEST_XA_TABLE_ROW_COUNT 10000
#define TEST_XA_TABLE_COUNT 10

#define TEST_XA_TABLE0 ((SChar*)"testXA_Table0")
#define TEST_XA_TABLE1 ((SChar*)"testXA_Table1")
#define TEST_XA_TABLE2 ((SChar*)"testXA_Table2")
#define TEST_XA_TABLE3 ((SChar*)"testXA_Table3")
#define TEST_XA_TABLE4 ((SChar*)"testXA_Table4")
#define TEST_XA_TABLE5 ((SChar*)"testXA_Table5")
#define TEST_XA_TABLE6 ((SChar*)"testXA_Table6")
#define TEST_XA_TABLE7 ((SChar*)"testXA_Table7")
#define TEST_XA_TABLE8 ((SChar*)"testXA_Table8")
#define TEST_XA_TABLE9 ((SChar*)"testXA_Table9")

typedef struct
{
    UInt    m_userID;
    SChar   m_tableName[256];
    UInt    m_schemaType;
} tsmRecTableInfo;

typedef struct
{
    UInt    m_userID;
    SChar   m_tableName[256];
    UInt    m_schemaType;
    
    UInt    m_nStart;
    UInt    m_nEnd;
} tsmRecInsertInfo;

typedef struct
{
    UInt    m_userID;
    SChar   m_tableName[256];
    UInt    m_updateValue;
    UInt    m_nStart;
    UInt    m_nEnd;
} tsmRecUpdateInfo;

typedef struct
{
    UInt    m_userID;
    SChar   m_tableName[256];
    UInt    m_nStart;
    UInt    m_nEnd;
} tsmRecDeleteInfo;

typedef struct
{
    SChar    m_xidChar;
} tsmPrepareInfo;

IDE_RC testXA_beginTrans(smiTrans *a_pTrans,
                               void     * /*pParm*/);

IDE_RC testXA_prepareTrans(smiTrans *a_pTrans,
                           SChar     a_xidChar,
                                void     * /*pParm*/);

IDE_RC testXA_abortTrans(smiTrans *a_pTrans,
                               void     * /*pParm*/);

IDE_RC testXA_createTable(smiTrans * /*a_pTrans*/,
                                void     *a_pTableInfo);
IDE_RC testXA_dropTable(smiTrans    *a_pTrans,
                              void        *a_pTableInfo);
IDE_RC testXA_insertIntoTable(smiTrans *a_pTrans,
                                    void     *a_pInsertInfo);
IDE_RC testXA_updateAtTable(smiTrans   *a_pTrans,
                                  void       *a_pUpdateInfo);
IDE_RC testXA_deleteAtTable(smiTrans *a_pTrans,
                                  void     *a_pDeleteInfo);

IDE_RC testXA();
IDE_RC testXA2();

void   testXA_init_threads();
void*  testXA_thread(void *a_pParm);
IDE_RC testXA_multi();

#endif
