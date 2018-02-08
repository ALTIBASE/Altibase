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
 

#include <qmsPreservedTable.h>

IDE_RC qmsPreservedTable::initialize( qcStatement  * aStatement,
                                      qmsSFWGH     * aSFWGH )
{
/***********************************************************************
 *
 * Description : PROJ-2204 Join Update, Delete
 *               BUG-39399 remove search key preserved table
 *
 * Implementation :
 *
 ***********************************************************************/

    qmsPreservedInfo  * sPreservedTable;
    UInt                i;

    IDU_FIT_POINT_FATAL( "qmsPreservedTable::initialize::__FT__" );

    IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(qmsPreservedInfo),
                                             (void**) & sPreservedTable )
              != IDE_SUCCESS );

    // init
    sPreservedTable->tableCount = 0;

    for ( i = 0; i < QC_MAX_REF_TABLE_CNT; i++ )
    {
        sPreservedTable->tableRef[i] = NULL;
        sPreservedTable->isKeyPreserved[i] = ID_FALSE;
    }
    
    aSFWGH->preservedInfo = sPreservedTable;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC qmsPreservedTable::addTable( qmsSFWGH     * aSFWGH,
                                    qmsTableRef  * aTableRef )
{
/***********************************************************************
 *
 * Description : PROJ-2204 Join Update, Delete
 *               BUG-39399 remove search key preserved table
 *
 * Implementation :
 *
 ***********************************************************************/

    qmsPreservedInfo  * sPreservedTable;

    IDU_FIT_POINT_FATAL( "qmsPreservedTable::addTable::__FT__" );

    sPreservedTable = aSFWGH->preservedInfo;

    if ( sPreservedTable != NULL )
    {
        IDE_DASSERT( sPreservedTable->tableCount + 1 < QC_MAX_REF_TABLE_CNT );

        // add tableRef
        sPreservedTable->tableRef[sPreservedTable->tableCount] = aTableRef;

        sPreservedTable->tableCount++;
    }
    else
    {
        // Nothing to do.
    }
    
    return IDE_SUCCESS;
}

IDE_RC qmsPreservedTable::find( qcStatement  * aStatement,
                                qmsSFWGH     * aSFWGH )
{
/***********************************************************************
 *
 * Description : PROJ-2204 Join Update, Delete
 *               BUG-39399 remove search key preserved table
 * Implementation :
 *      1. key preserved table HINT check.
 *      3. key preserved column set flag.
 *
 ***********************************************************************/

    mtcTemplate          * sMtcTemplate;
    qmsPreservedInfo     * sPreservedTable;
    qmsTableRef          * sTableRef;
    UInt                   i;

    IDU_FIT_POINT_FATAL( "qmsPreservedTable::find::__FT__" );

    sPreservedTable = aSFWGH->preservedInfo;

    // simple view에 order by만 허용한다.
    if ( ( sPreservedTable != NULL ) &&
         ( aSFWGH->hierarchy == NULL ) &&
         ( aSFWGH->group == NULL ) &&
         ( aSFWGH->having == NULL ) &&
         ( aSFWGH->aggsDepth1 == NULL ) &&
         ( aSFWGH->aggsDepth2 == NULL ) &&
         ( aSFWGH->rownum == NULL ) )
    {            
        sMtcTemplate = & QC_SHARED_TMPLATE(aStatement)->tmplate;

        IDE_TEST ( checkKeyPreservedTableHints( aSFWGH )
                   != IDE_SUCCESS );
        
        // tuple에 반영한다.
        for ( i = 0; i < sPreservedTable->tableCount; i++ )
        {
            if ( sPreservedTable->isKeyPreserved[i] == ID_TRUE )
            {
                sTableRef = sPreservedTable->tableRef[i];
            
                sMtcTemplate->rows[sTableRef->table].lflag &=
                    ~MTC_TUPLE_KEY_PRESERVED_MASK;
                sMtcTemplate->rows[sTableRef->table].lflag |=
                    MTC_TUPLE_KEY_PRESERVED_TRUE;
            }
            else
            {
                // Nothing to do.
            }
        }
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC qmsPreservedTable::getFirstKeyPrevTable( qmsSFWGH      * aSFWGH,
                                                qmsTableRef  ** aTableRef )
{
/***********************************************************************
 *
 * Description : PROJ-2204 Join Update, Delete
 *               BUG-39399 remove search key preserved table
 * Implementation :
 *   ex>
 * CASE 1. delete from (select t1.i1 a, t2.i1 b from t1, t2 where t1.i1 = t2.i1 ) ....  
 * CASE 2. delete from (select t1.i1 a, t2.i1 b from t2, t1 where t1.i1 = t2.i1 ) ....
 * t1,t2 key preserved table이다. delete 시 첫번째 key preserved table만 delete된다.
 * CASE 1는 T1, CASE 2는 T2.
 *
 ***********************************************************************/

    qmsPreservedInfo  * sPreservedTable;
    UInt                i;

    IDU_FIT_POINT_FATAL( "qmsPreservedTable::getFirstKeyPrevTable::__FT__" );

    sPreservedTable = aSFWGH->preservedInfo;

    *aTableRef = NULL;
    
    if ( sPreservedTable != NULL )
    {
        for ( i = 0; i < sPreservedTable->tableCount; i++ )
        {
            if ( sPreservedTable->isKeyPreserved[i] == ID_TRUE )
            {
                *aTableRef = sPreservedTable->tableRef[i];
                break;
            }
            else
            {
                // Nothing to do.
            }
        }        
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;
}


IDE_RC qmsPreservedTable::checkKeyPreservedTableHints( qmsSFWGH      * aSFWGH )
{
/***********************************************************************
 *
 * Description : PROJ-2204 Join Update, Delete
 *               BUG-39399 remove search key preserved table
 * Implementation :
 *       key preserved table hint check.
 * hint : key_preserved_table(table1,table2,....n)
 *        *+ key_preserved_table(t1,t2,t3)
 *        *+ key_preserved_table(t1,t3) key_preserved_table(t2) 
 ***********************************************************************/
    
    qmsPreservedInfo     * sPreservedTable;
    qmsTableRef          * sTableRef;
    qmsKeyPreservedHints * sKeyPreservedHint;
    qmsHintTables        * sHintTable;
    UInt                   i;

    IDU_FIT_POINT_FATAL( "qmsPreservedTable::checkKeyPreservedTableHints::__FT__" );

    sPreservedTable = aSFWGH->preservedInfo;

    if ( sPreservedTable != NULL )
    {
        for ( sKeyPreservedHint = aSFWGH->hints->keyPreservedHint;
              sKeyPreservedHint != NULL;
              sKeyPreservedHint = sKeyPreservedHint->next )
        {                            
            for ( sHintTable = sKeyPreservedHint->table;
                  sHintTable != NULL;
                  sHintTable = sHintTable->next )
            {
                for ( i = 0; i < sPreservedTable->tableCount; i++ )
                {
                    sTableRef = sPreservedTable->tableRef[i];
                        
                    if ( idlOS::strMatch( sHintTable->tableName.stmtText +
                                          sHintTable->tableName.offset,
                                          sHintTable->tableName.size,
                                          sTableRef->aliasName.stmtText +
                                          sTableRef->aliasName.offset,
                                          sTableRef->aliasName.size ) == 0 )
                    {
                        // HINT예 명시한 테이블이 존재하는 경우
                        sPreservedTable->isKeyPreserved[i] = ID_TRUE;
                        break;
                    }
                    else
                    {
                        // HINT예 명시한 테이블이 없는 경우
                        if ( i == sPreservedTable->tableCount - 1 )
                        {
                            // Hint 무시
                            aSFWGH->hints->keyPreservedHint = NULL;
                        
                            IDE_CONT( NORMAL_EXIT );
                        }
                        else
                        {
                            /* Nothing To Do */
                        }
                    }
                }
            }
        }
        
        IDE_EXCEPTION_CONT( NORMAL_EXIT );
        
        if ( aSFWGH->hints->keyPreservedHint == NULL )
        {
            for ( i = 0; i < sPreservedTable->tableCount; i++ )
            {       
                sPreservedTable->isKeyPreserved[i] = ID_TRUE;
            }
        }
        else
        {
            /* Nothing To Do */
        }
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;
}
