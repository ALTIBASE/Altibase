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
 
/*******************************************************************************
 * $Id: utdb.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <utdb.h>

// BUG-19365
uteErrorMgr gErrorMgr;

IDL_EXTERN_C_BEGIN
#if defined(HP_HPUX) || defined(IA64_HP_HPUX)
    SInt _init(void) {}
    SInt _find(void) {}
#endif


#define TO_LOWER(c)  ( (c>='A' && c<='Z')?(c|0x20):c )

    SChar * str_case_str(const SChar *s, const SChar *w)
    {

        SChar         b, c;
        const SChar *n, *r;

        IDE_TEST(w == NULL);
        IDE_TEST( *w == 0 );

        IDE_TEST(s == NULL);
        IDE_TEST( *s == 0 );

      M1: /* get first match */
        for( n = w, b = TO_LOWER( (*w) ), c = TO_LOWER( (*s) )
                 ;b != c;
             ++s, c = TO_LOWER((*s)) )
        {
            IDE_TEST( c == '\0' );
        }

        /* match string */
        r = s;
        do
        {

            IDE_TEST(*s == '\0' );

            ++n; b = TO_LOWER( (*n) );
            ++s; c = TO_LOWER( (*s) );

            if( *n == '\0' ) goto M0;

        } while( c == b );

        goto M1;


      M0: return (SChar*)r;

        IDE_EXCEPTION_END;

        return NULL;
    }

#undef TO_LOWER
IDL_EXTERN_C_END




IDE_RC metaColumns::initialize(SChar * aTableName)
{
    IDE_TEST( aTableName == NULL );
    mTableName = aTableName;
    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

bool metaColumns::isPrimaryKey(SChar * key)
{
    nameList_t  *i;
    for(i = primaryKey; i != NULL ; i = i->next )
    {
        if( idlOS::strcmp(key,i->name) == 0)
        { return true; }
    }
    return false;
}

IDE_RC metaColumns::delCL(SChar * key, bool aIsLobType)
{
    nameList_t  *i,*p, *sColList;
    
    if(aIsLobType == false){
        sColList = columnList;
    }
    else
    {
        sColList = lobColumnList;        
    }
    for(p = NULL,i = sColList;i; i = i->next)
    {
        if( idlOS::strcmp(key,i->name) == 0)
        {
            if( p )
            {
                p->next = i->next;
            }
            else
            {
                sColList = i->next;
            }
            idlOS::free( i->name );
            idlOS::free( i );

            if(aIsLobType == false)
            {
                --clCount;
            }
            else
            {
                --lobClCount;
            }

            if(aIsLobType == false){
                columnList = sColList;
            }
            else
            {
                lobColumnList = sColList;        
            }

            return IDE_SUCCESS;
        }
        p = i;
    }



    return IDE_FAILURE;
}


IDE_RC metaColumns::addPK(SChar * key)
{
    nameList_t  *i        = NULL;
    nameList_t  *idx      = NULL;
    SChar       *name     = NULL;
    UShort       size;

    IDE_TEST( key == NULL );
    size = idlOS::strlen( key );
    IDE_TEST( size < 1 );

    for(i = primaryKey; i != NULL ; i = i->next )
    {
        IDE_TEST_RAISE( idlOS::strcmp(key,i->name) == 0, err_column_already_exist);
        idx = i;
    }

    i = (nameList_t*)idlOS::calloc(1,sizeof( nameList_t ) );
    IDE_TEST(    i == NULL );

    name = (SChar*)  idlOS::calloc(1,size + 1 );
    IDE_TEST( name == NULL );

    idlOS::strncpy(name,key,size);
    i->name = name;

    if( primaryKey )
    {
        idx->next = i;
    }
    else
    {
        primaryKey = i;
    }
    ++pkCount;
    return IDE_SUCCESS;

    // BUG-25229 [CodeSonar] Audit의 메모리 Leak
    IDE_EXCEPTION(err_column_already_exist);
    {
        // 현재 i 는 calloc 를 받지 않았다.
        i = NULL;
    }
    IDE_EXCEPTION_END;

    if (i != NULL)
    {
        idlOS::free(i);
    }
    if (name != NULL)
    {
        idlOS::free(name);
    }

    return IDE_FAILURE;
}

SChar* metaColumns::getPK(UShort idx)
{
    nameList_t *i;

    if( ( idx == 0 ) ||
        ( idx > pkCount ) )
    {
        return NULL;
    }


    for(i = primaryKey ;--idx ; )
    {
        i = i->next;
    }
    return i->name;
}


IDE_RC metaColumns::addCL(SChar * key, bool aIsLobType)
{
    nameList_t  *i        = NULL;
    nameList_t  *idx      = NULL;
    nameList_t  *sColList = NULL;
    SChar       *name     = NULL;
    UShort       size;
    
    IDE_TEST( key == NULL );
    size = idlOS::strlen( key );
    IDE_TEST( size < 1 );

    if(aIsLobType == false)
    {
        sColList = columnList;
    }
    else
    {
        sColList = lobColumnList;
    }
    
    for(i = sColList; i != NULL ; i = i->next )
    {
        IDE_TEST_RAISE( idlOS::strcmp(key,i->name) == 0, err_column_already_exist);
        idx = i;
    }

    i = (nameList_t*)idlOS::calloc(1, sizeof(nameList_t));
    IDE_TEST(i == NULL);

    name = (SChar*)  idlOS::calloc(1,size + 1 );
    IDE_TEST( name == NULL );

    idlOS::strncpy(name,key,size);
    i->name = name;

    if( sColList )
    {
        idx->next = i;
    }
    else
    {
        sColList = i;
    }

    if(aIsLobType == false)
    {
        ++clCount;
    }
    else
    {
        ++lobClCount;
    }

    if(aIsLobType == false)
    {
        columnList = sColList;
    }
    else
    {
        lobColumnList = sColList;
    }
    
    return IDE_SUCCESS;

    // BUG-25229 [CodeSonar] Audit의 메모리 Leak
    IDE_EXCEPTION(err_column_already_exist);
    {
        // 현재 i 는 calloc 를 받지 않았다.
        i = NULL;
    }
    IDE_EXCEPTION_END;

    if (i != NULL)
    {
        idlOS::free(i);
    }
    if (name != NULL)
    {
        idlOS::free(name);
    }

    return IDE_FAILURE;
}

SChar* metaColumns::getCL(UShort idx, bool aIsLobType)
{
    nameList_t *i, *sColList;
    UInt sClCount = 0;

    if(aIsLobType == false)
    {
        sColList = columnList;
        sClCount = clCount;
    }
    else
    {
        sColList = lobColumnList;
        sClCount = lobClCount;
    }

    if((idx == 0) || (idx > sClCount))
    {
        return NULL;
    }

    for(i = sColList; --idx;)
    {
        i = i->next;
    }

    if(aIsLobType == false)
    {
        columnList = sColList;
    }
    else
    {
        lobColumnList = sColList; 
    }

    return i->name;
}



metaColumns::metaColumns():Object()
{
    pkCount = 0;
    clCount = 0;
    lobClCount = 0;
    primaryKey =
    columnList = NULL;
    lobColumnList = NULL;
    mASC       = true;
}
metaColumns::~metaColumns(){ finalize();}


IDE_RC metaColumns::finalize()
{
    nameList_t   *i;
    pkCount =
    clCount = 0;
    lobClCount = 0;
    mTableName = NULL;

    for(; primaryKey;  primaryKey = i )
    {
        i = primaryKey->next;
        idlOS::free( primaryKey->name );
        idlOS::free( primaryKey );
    }

    for(; columnList; columnList = i )
    {
        i = columnList->next;
        idlOS::free( columnList->name );
        idlOS::free( columnList );
    }

    for (; lobColumnList; lobColumnList = i )
    {
        i = lobColumnList->next;
        idlOS::free( lobColumnList->name );
        idlOS::free( lobColumnList );
    }

    return IDE_SUCCESS;
}


void metaColumns::dump()
{
    UShort i;
    SChar *c;
    bool sIsLobType = false; // common column type:0, lob column type:1

    idlOS::printf("TABLE %s primary key(",getTableName());
    for( i = 1, c = getPK(1); c ; c = getPK(++i) )
    {
        idlOS::printf(" %s,",c);
    }
    idlOS::printf(") no key columns(",getTableName());
    for( i = 1, c = getCL(1, sIsLobType); c ; c = getCL(++i, sIsLobType) )
    {
        idlOS::printf(" %s,",c);
    }
    idlOS::printf(")\n");
}

IDE_RC printSelect( Query *  sQuery)
{
    Row   *row;
    Field *f  ;
    IDE_TEST( sQuery == NULL );

    idlOS::printf("\n");
    for ( row = sQuery->fetch(); row ; row = sQuery->fetch())
    {
        for( f = row->getField();f; f = (Field*)f->next() )
        {
            idlOS::printf("%s\t",f->getValue());
        }
        idlOS::printf("\n");
    }
    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}
