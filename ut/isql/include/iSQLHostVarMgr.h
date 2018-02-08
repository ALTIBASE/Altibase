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
 * $Id: iSQLHostVarMgr.h 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#ifndef _O_ISQLHOSTVARMGR_H_
#define _O_ISQLHOSTVARMGR_H_ 1

#include <utISPApi.h>
#include <iSQL.h>

class iSQLHostVarMgr; // host variable manger

enum iSQLInOutType
{
    iSQL_IN=1, iSQL_OUT=2, iSQL_IN_OUT=3
};

union hv_value
{
    SChar   * c_value;
    SFloat    f_value;
    SDouble   d_value;
};

typedef struct HostVarElement
{
    SChar           name[QP_MAX_NAME_LEN+1];
    iSQLVarType     type;
    SShort          inOutType;       /* PROJ-1584 DML Return Clause */
    SInt            precision;       // for show/print var
    SChar           scale[WORD_LEN]; // for show/print var
    SInt            size;
    SChar         * c_value;
    SFloat          f_value;
    SDouble         d_value;
    ULong           mLobLoc;
    SQLLEN          mInd;            // SQL_NTS or SQL_NULL_DATA
    SShort          mSqlType;        // SQL Data Type
    SShort          mCType;          // C Data Type
} HostVarElement;

typedef struct HostVarNode
{
    HostVarElement   element;
    HostVarNode    * next;
    HostVarNode    * host_var_next;  // procedure 수행 시 호스트 변수들의 순서
} HostVarNode;

class iSQLHostVarMgr
{
public:
    iSQLHostVarMgr();
    ~iSQLHostVarMgr();

    IDE_RC add( SChar       * a_name,
                iSQLVarType   a_type,
                SShort        a_InOutType,
                SInt          a_precision,
                SChar       * a_ccale );
    IDE_RC setValue( SChar * a_name );
    IDE_RC setValue( SChar * a_name, SChar * a_value );
    IDE_RC lookup( SChar * a_name );
    IDE_RC typeConvert( HostVarElement * a_host_var, SChar * r_type );
    IDE_RC isCorrectType( iSQLVarType a_type );
    IDE_RC showVar( SChar * a_name );
    void   print();

    HostVarElement * getHostVar( SChar * a_name );

    void             initBindList();
    IDE_RC           putBindList( SChar * a_name );
    HostVarNode    * getBindList()              { return m_Head; }
    SInt             getBindListCnt()           { return m_BindListCnt; }
    void             setHostVar( idBool aIsFunc, HostVarNode * a_host_var_list );

protected:
    UInt          hashing( SChar * a_name );
    HostVarNode * getVar( SChar * a_name );
    HostVarNode * getVar( UInt key, SChar * a_name );

    SLong         strToLL( SChar  * aNPtr,
                           SChar ** aEndPtr,
                           int      aBase );

private:
    /* ============================================
     * m_BindList    : for manage host variable with execute procedure/function
     *                 append to last node, get from first node
     * m_Head        : first node of m_BindList
     * m_Tail        : last node of m_BindList
     * m_BindListCnt : count of node of m_BindList
     * ============================================ */
    HostVarNode * m_SymbolTable[MAX_TABLE_ELEMENTS];
    HostVarNode * m_BindList;

    HostVarNode * m_Head;
    HostVarNode * m_Tail;
    SInt          m_BindListCnt;
};

#endif // _O_ISQLHOSTVARMGR_H_
