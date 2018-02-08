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
 * $Id: iSQLHostVarMgr.cpp 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#include <ide.h>
#include <utString.h>
#include <iSQLProperty.h>
#include <iSQLSpool.h>
#include <iSQLHostVarMgr.h>
#if !defined(PDL_HAS_WINCE)
#include <errno.h> /* For strToLL() */
#endif

extern iSQLSpool *gSpool;
extern iSQLProperty gProperty;

iSQLHostVarMgr::iSQLHostVarMgr()
{
    SInt i;

    m_BindList      = NULL;
    m_Head = m_Tail = NULL;
    m_BindListCnt   = 0;

    for (i=0; i<MAX_TABLE_ELEMENTS; i++)
    {
        m_SymbolTable[i] = NULL;
    }
}

iSQLHostVarMgr::~iSQLHostVarMgr()
{
    SInt i;
    HostVarNode *t_node;
    HostVarNode *t_node_b;

    for (i=0; i<MAX_TABLE_ELEMENTS; i++)
    {
        t_node = m_SymbolTable[i];
        while (t_node != NULL)
        {
            t_node_b = t_node;
            if (t_node_b->element.c_value != NULL)
            {
                idlOS::free(t_node_b->element.c_value);
            }
            t_node   = t_node->next;
            idlOS::free(t_node_b);
        }
        m_SymbolTable[i] = NULL;
    }
    initBindList();
    m_BindList      = NULL;
    m_Head = m_Tail = NULL;
    m_BindListCnt   = 0;
}

void
iSQLHostVarMgr::initBindList()
{
    HostVarNode *sNode = NULL;
    HostVarNode *sCurNode = NULL;
    sNode = sCurNode = m_Head;
    while ( sNode != NULL )
    {
        sCurNode = sNode;
        if (sCurNode->element.c_value != NULL)
        {
            idlOS::free(sCurNode->element.c_value);
        }
        sNode = sCurNode->host_var_next;
        idlOS::free(sCurNode);
    }
    m_BindList      = NULL;
    m_Head = m_Tail = NULL;
    m_BindListCnt   = 0;
}

IDE_RC
iSQLHostVarMgr::add( SChar       * a_name,
                     iSQLVarType   a_type,
                     SShort        a_InOutType,
                     SInt          a_precision,
                     SChar       * a_scale )
{
    HostVarNode *t_node = NULL;
    HostVarNode *s_node = NULL;
    UInt         i_hash   = 0;
    SInt         sNewNode = 0;
    idBool       sCheckPrecision = ID_FALSE;

    IDE_TEST_RAISE(a_type == iSQL_BAD, invalid_datatype);

    IDE_TEST_RAISE(idlOS::strlen(a_name) > QP_MAX_NAME_LEN, name_len_error);

    IDE_TEST(utString::toUpper(a_name) != IDE_SUCCESS);

    if ( (t_node = getVar(a_name)) == NULL )
    {
        // memory alloc error 가 발생한 경우
        t_node = (HostVarNode*) idlOS::malloc(ID_SIZEOF(HostVarNode));
        IDE_TEST_RAISE( t_node == NULL, mem_alloc_error );
        idlOS::memset(t_node, 0x00, ID_SIZEOF(HostVarNode));
        sNewNode = 1;
    }
    else
    {
        if (t_node->element.c_value != NULL)
        {
            idlOS::free(t_node->element.c_value);
            t_node->element.c_value = NULL;
        }
    }

    switch (a_type)
    {
    case iSQL_REAL   :
    case iSQL_DOUBLE :
    case iSQL_BLOB_LOCATOR :
    case iSQL_CLOB_LOCATOR :
        t_node->element.size = 0;
        t_node->element.c_value = NULL;
        break;
    case iSQL_CHAR       :
    case iSQL_VARCHAR    :
    case iSQL_NIBBLE :
        sCheckPrecision = ID_TRUE;
        t_node->element.size = a_precision + 1;
        break;
    case iSQL_NCHAR       :
    case iSQL_NVARCHAR    :
        sCheckPrecision = ID_TRUE;
        t_node->element.size = a_precision * 3 + 1;
        // bug-33948: codesonar: Integer Overflow of Allocation Size
        IDE_TEST_RAISE(t_node->element.size < 0 , invalidAllocSize);
        break;
    case iSQL_SMALLINT :
    case iSQL_INTEGER :
    case iSQL_BIGINT :
    case iSQL_FLOAT   :
    case iSQL_DECIMAL :
    case iSQL_NUMERIC :
    case iSQL_NUMBER  :
        t_node->element.size = 21 + 1;
        break;
    case iSQL_DATE :
        t_node->element.size = DATE_SIZE;
        break;
    case iSQL_BYTE :
    case iSQL_VARBYTE:
        sCheckPrecision = ID_TRUE;
        t_node->element.size = a_precision*2 + 1;
        IDE_TEST_RAISE(t_node->element.size < 0 , invalidAllocSize);
        break;
    default :
        // BUG-25315 [CodeSonar] 메모리 릭
        idlOS::free(t_node);
        t_node = NULL;
        IDE_RAISE( invalid_datatype );
        break;
    }

    if (sCheckPrecision == ID_TRUE)
    {
        IDE_TEST_RAISE( a_precision == 0, invalid_datatype );
    }

    if (t_node->element.size > 0)
    {
        IDE_TEST_RAISE( (t_node->element.c_value =
                            (SChar*)idlOS::malloc(t_node->element.size))
                         == NULL, mem_alloc_error );
        idlOS::memset(t_node->element.c_value, 0x00,
                      t_node->element.size);
    }

    idlOS::strcpy(t_node->element.name, a_name);
    idlOS::strcpy(t_node->element.scale, a_scale);
    t_node->element.type      = a_type;
    t_node->element.inOutType = a_InOutType; /* PROJ-1584 DML Return Clause */
    t_node->element.precision = a_precision;
    t_node->element.d_value   = 0;
    t_node->element.f_value   = 0;
    t_node->element.mLobLoc   = ID_ULONG(0);
    t_node->element.mInd      = SQL_NULL_DATA;
    t_node->host_var_next     = NULL;
    
    if (sNewNode == 1)
    {
        i_hash = hashing(a_name);

        if ( (s_node = m_SymbolTable[i_hash]) != NULL )
        {
            t_node->next = s_node;
        }
        else
        {
            t_node->next = NULL;
        }
        m_SymbolTable[i_hash] = t_node;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(invalid_datatype);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Invalid_dataType_Error);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        gSpool->Print();
    }

    IDE_EXCEPTION(name_len_error);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_name_length_overflow_Error,
                        (UInt)QP_MAX_NAME_LEN);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);

        gSpool->Print();
    }

    IDE_EXCEPTION(mem_alloc_error);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_memory_error, __FILE__, (UInt)__LINE__);
        utePrintfErrorCode(stderr, &gErrorMgr);
        Exit(0);
    }
    IDE_EXCEPTION( invalidAllocSize );
    {
        idlOS::printf("Invalid alloc size: %"ID_INT32_FMT"\n", t_node->element.size);
    }
    IDE_EXCEPTION_END;

    if (t_node != NULL)
    {
        idlOS::free(t_node);
    }

    return IDE_FAILURE;
}

IDE_RC
iSQLHostVarMgr::setValue( SChar * a_name )
{
    HostVarNode *t_node;

    // 선언되지 않은 호스트 변수
    IDE_TEST_RAISE((t_node = getVar(a_name)) == NULL, not_defined);
    t_node->element.mInd = SQL_NULL_DATA;

    idlOS::sprintf(gSpool->m_Buf, "Execute success.\n");
    gSpool->Print();

    return IDE_SUCCESS;

    IDE_EXCEPTION(not_defined);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_host_variable_not_defined_error, a_name);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        gSpool->Print();
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLHostVarMgr::setValue( SChar * a_name,
                          SChar * a_value )
{
    SInt         sValLen = 0;
    HostVarNode *t_node = NULL;
    SChar       *begin_pos = NULL;
    SChar       *end_pos = NULL;

    // 선언되지 않은 호스트 변수
    IDE_TEST_RAISE((t_node = getVar(a_name)) == NULL, not_defined);

    if ( idlOS::strcasecmp(a_value, "NULL") == 0 )
    {
        return setValue(a_name);
    }

    switch (t_node->element.type)
    {
    case iSQL_CHAR       :
    case iSQL_NCHAR      :
    case iSQL_NVARCHAR   :
    case iSQL_DATE       :
    case iSQL_BYTE       :
    case iSQL_VARBYTE    :
    case iSQL_NIBBLE     :
    case iSQL_VARCHAR    :  // value가 const string('...') 이어야 한다.
        begin_pos = idlOS::strchr(a_value, '\'');
        IDE_TEST_RAISE(begin_pos == NULL, type_mismatch); // value가 const string이 아닌경우

        end_pos = idlOS::strrchr(begin_pos+1, '\'');
        IDE_TEST_RAISE(end_pos == NULL, type_mismatch);   // value가 '로 끝나지 않은 경우, 절대 들어올 수 없는 경우
        // value가 null string 인 경우
        IDE_TEST_RAISE(end_pos-begin_pos == 1, null_value);

        idlOS::memset(t_node->element.c_value, 0x00,
                      t_node->element.size);
        sValLen = end_pos - begin_pos - 1;
        IDE_TEST_RAISE( sValLen > t_node->element.size - 1, too_large_error );
        idlOS::strncpy(t_node->element.c_value, begin_pos+1, sValLen);
        t_node->element.c_value[sValLen] = '\0';
        break;
    case iSQL_FLOAT    :
    case iSQL_DECIMAL  :
    case iSQL_NUMBER   :
    case iSQL_NUMERIC  :
    case iSQL_BIGINT   :
    case iSQL_INTEGER  :
    case iSQL_SMALLINT :
        begin_pos = idlOS::strchr(a_value, '\'');
        if (begin_pos != NULL)    // value가 const string이 아닌경우
        {
            end_pos = idlOS::strrchr(begin_pos+1, '\'');
            IDE_TEST_RAISE(end_pos == NULL, type_mismatch); // value가 '로 끝나지 않은 경우, 절대 들어올 수 없는 경우
            // value가 null string 인 경우
            IDE_TEST_RAISE(end_pos-begin_pos == 1, null_value);

            // value가 const string인 경우
            IDE_RAISE(type_mismatch);
        }

        idlOS::memset(t_node->element.c_value, 0x00, t_node->element.size);
        IDE_TEST_RAISE( idlOS::strlen(a_value) > (UInt)t_node->element.size-1,
                        too_large_error );
        idlOS::strcpy(t_node->element.c_value, a_value);
        if ( t_node->element.type == iSQL_SMALLINT )
        {
            errno = 0;
            SInt sTmpInt = idlOS::atoi( (const char *) a_value );
            IDE_TEST_RAISE( sTmpInt > (SShort)0x7FFF ||
                            sTmpInt < (SShort)0x8001 ||
                            errno == ERANGE, outof_range_error );
        }
        else if ( t_node->element.type == iSQL_INTEGER )
        {
            errno = 0;
            SLong sTmpLong = strToLL( (char *) a_value, (char**) NULL, 10 );
            IDE_TEST_RAISE( sTmpLong > (SInt)0x7FFFFFFF ||
                            sTmpLong < (SInt)0x80000001 ||
                            errno == ERANGE, outof_range_error );
        }
        break;
    case iSQL_DOUBLE :
        begin_pos = idlOS::strchr(a_value, '\'');
        if (begin_pos != NULL)    // value가 const string이 아닌경우
        {
            end_pos = idlOS::strrchr(begin_pos+1, '\'');
            IDE_TEST_RAISE(end_pos == NULL, type_mismatch); // value가 '로 끝나지 않은 경우, 절대 들어올 수 없는 경우
            // value가 null string 인 경우
            IDE_TEST_RAISE(end_pos-begin_pos == 1, null_value);

            // value가 const string인 경우
            IDE_RAISE(type_mismatch);
        }

        t_node->element.d_value = (SDouble)atof(a_value);
        break;
    case iSQL_REAL :
        begin_pos = idlOS::strchr(a_value, '\'');
        if (begin_pos != NULL)    // value가 const string이 아닌경우
        {
            end_pos = idlOS::strrchr(begin_pos+1, '\'');
            IDE_TEST_RAISE(end_pos == NULL, type_mismatch); // value가 '로 끝나지 않은 경우, 절대 들어올 수 없는 경우

            // value가 null string 인 경우
            IDE_TEST_RAISE(end_pos-begin_pos == 1, null_value);

            // value가 const string인 경우
            IDE_RAISE(type_mismatch);
        }

        t_node->element.f_value = (SFloat)atof(a_value);
        break;
     default:
        IDE_RAISE(type_mismatch);
        break;
    }

    t_node->element.mInd = SQL_NTS;

    idlOS::sprintf(gSpool->m_Buf, "Execute success.\n");
    gSpool->Print();

    return IDE_SUCCESS;

    IDE_EXCEPTION(null_value);
    {
        t_node->element.mInd = SQL_NULL_DATA;
        idlOS::sprintf(gSpool->m_Buf, "Execute success.\n");
        gSpool->Print();
        return IDE_SUCCESS;
    }
    IDE_EXCEPTION(not_defined);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_host_variable_not_defined_error, a_name);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        gSpool->Print();
    }
    IDE_EXCEPTION(type_mismatch);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_type_mismatch_error);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        gSpool->Print();
    }
    IDE_EXCEPTION(too_large_error);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_too_large_error);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        gSpool->Print();
    }
    IDE_EXCEPTION(outof_range_error);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_value_OutOfRange_error);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        gSpool->Print();
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLHostVarMgr::lookup( SChar * a_name )
{
    return ( getVar(a_name) != NULL ) ? IDE_SUCCESS : IDE_FAILURE;
}

HostVarNode *
iSQLHostVarMgr::getVar( SChar * a_name )
{
    UInt key = hashing(a_name);

    return getVar(key, a_name);
}

HostVarNode *
iSQLHostVarMgr::getVar( UInt    a_key,
                        SChar * a_name )
{
    HostVarNode *t_node = m_SymbolTable[a_key];

    utString::toUpper(a_name);

    while(t_node != NULL)
    {
        if ( idlOS::strcmp(t_node->element.name, a_name) == 0)
        {
            return t_node;
        }
        t_node = t_node->next;
    }
    return NULL;
}

UInt
iSQLHostVarMgr::hashing( SChar * a_name )
{
    SChar *t_ptr = a_name;
    ULong  key = 0;

    while (*t_ptr != 0)
    {
        key = key*31 + *t_ptr++;
    }

    return (UInt)(key%MAX_TABLE_ELEMENTS);
}

IDE_RC
iSQLHostVarMgr::typeConvert( HostVarElement * a_host_var,
                             SChar          * r_type )
{
    SChar tmp[WORD_LEN];

    switch (a_host_var->type)
    {
    case iSQL_BIGINT       : idlOS::strcpy(tmp, "BIGINT");      break;
    case iSQL_BLOB_LOCATOR : idlOS::strcpy(tmp, "BLOB");        break;
    case iSQL_CHAR         : idlOS::strcpy(tmp, "CHAR");        break;
    case iSQL_CLOB_LOCATOR : idlOS::strcpy(tmp, "CLOB");        break;
    case iSQL_DATE         : idlOS::strcpy(tmp, "DATE");        break;
    case iSQL_DECIMAL      : idlOS::strcpy(tmp, "DECIMAL");     break;
    case iSQL_DOUBLE       : idlOS::strcpy(tmp, "DOUBLE");      break;
    case iSQL_FLOAT        : idlOS::strcpy(tmp, "FLOAT");       break;
    case iSQL_BYTE         : idlOS::strcpy(tmp, "BYTE");        break;
    case iSQL_VARBYTE      : idlOS::strcpy(tmp, "VARBYTE");     break;
    case iSQL_NCHAR        : idlOS::strcpy(tmp, "NCHAR");       break;
    case iSQL_NVARCHAR     : idlOS::strcpy(tmp, "NVARCHAR");    break;
    case iSQL_NIBBLE       : idlOS::strcpy(tmp, "NIBBLE");      break;
    case iSQL_INTEGER      : idlOS::strcpy(tmp, "INTEGER");     break;
    case iSQL_NUMBER       : idlOS::strcpy(tmp, "NUMBER");      break;
    case iSQL_NUMERIC      : idlOS::strcpy(tmp, "NUMERIC");     break;
    case iSQL_REAL         : idlOS::strcpy(tmp, "REAL");        break;
    case iSQL_SMALLINT     : idlOS::strcpy(tmp, "SMALLINT");    break;
    case iSQL_VARCHAR      : idlOS::strcpy(tmp, "VARCHAR");     break;
    case iSQL_GEOMETRY     : idlOS::strcpy(tmp, "GEOMETRY");    break;
    default                : return IDE_FAILURE;
    }

    if (a_host_var->precision != -1 && a_host_var->precision != 0)
    {
        if ( idlOS::strlen(a_host_var->scale) != 0 )
        {
            idlOS::sprintf(r_type, "%s(%"ID_INT32_FMT", %s)",
                           tmp, a_host_var->precision, a_host_var->scale);
        }
        else
        {
            idlOS::sprintf(r_type, "%s(%"ID_INT32_FMT")",
                           tmp, a_host_var->precision);
        }
    }
    else
    {
        idlOS::sprintf(r_type, "%s", tmp);
    }

    return IDE_SUCCESS;
}

IDE_RC
iSQLHostVarMgr::isCorrectType( iSQLVarType a_type )
{
    switch (a_type)
    {
    case iSQL_BIGINT       :
    case iSQL_BLOB_LOCATOR :
    case iSQL_CHAR         :
    case iSQL_CLOB_LOCATOR :
    case iSQL_DATE         :
    case iSQL_DECIMAL      :
    case iSQL_DOUBLE       :
    case iSQL_FLOAT        :
    case iSQL_BYTE         :
    case iSQL_VARBYTE      :
    case iSQL_NCHAR        :
    case iSQL_NVARCHAR     :
    case iSQL_NIBBLE       :
    case iSQL_INTEGER      :
    case iSQL_NUMBER       :
    case iSQL_NUMERIC      :
    case iSQL_REAL         :
    case iSQL_SMALLINT     :
    case iSQL_VARCHAR      :
    case iSQL_GEOMETRY     : return IDE_SUCCESS;
    default                : return IDE_FAILURE;
    }
}

IDE_RC
iSQLHostVarMgr::showVar( SChar * a_name )
{
    SChar tmp[WORD_LEN], tmp_name[UT_MAX_NAME_BUFFER_SIZE];
    HostVarNode *t_node;

    IDE_TEST_RAISE((t_node = getVar(a_name)) == NULL, not_defined);
    IDE_TEST_RAISE(t_node->element.type == iSQL_BLOB_LOCATOR ||
                   t_node->element.type == iSQL_CLOB_LOCATOR,
                   UndisplayableType);

    idlOS::sprintf(gSpool->m_Buf, "NAME                 TYPE                 VALUE\n");
    gSpool->Print();
    idlOS::sprintf(gSpool->m_Buf, "-------------------------------------------------------\n");
    gSpool->Print();

    idlOS::snprintf(tmp_name, 21, "%s", t_node->element.name);

    typeConvert(&(t_node->element), tmp);

    if ( t_node->element.mInd != SQL_NULL_DATA )
    {
        switch (t_node->element.type)
        {
        case iSQL_DOUBLE :
            idlOS::sprintf(gSpool->m_Buf, "%-20s %-20s %f\n",
                           tmp_name, tmp, t_node->element.d_value);
            break;
        case iSQL_REAL :
            idlOS::sprintf(gSpool->m_Buf, "%-20s %-20s %f\n",
                           tmp_name, tmp, t_node->element.f_value);
            break;
        default :
            idlOS::sprintf(gSpool->m_Buf, "%-20s %-20s %s\n",
                           tmp_name, tmp, t_node->element.c_value);
            break;
        }
    }
    else
    {
        idlOS::sprintf(gSpool->m_Buf, "%-20s %-20s\n", tmp_name, tmp);
    }
    gSpool->Print();

    return IDE_SUCCESS;

    IDE_EXCEPTION(not_defined);
    {
        idlOS::sprintf(gSpool->m_Buf, "%s not defined.\n", a_name);
        gSpool->Print();
    }

    IDE_EXCEPTION(UndisplayableType);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_UNDISPLAYABLE_DATATYPE_Error);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        gSpool->Print();
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void
iSQLHostVarMgr::print()
{
    SInt         ix;
    SChar        tmp[WORD_LEN], tmp_name[UT_MAX_NAME_BUFFER_SIZE];
    HostVarNode* t_node;

    idlOS::sprintf(gSpool->m_Buf, "[ HOST VARIABLE ]\n");
    gSpool->Print();
    idlOS::sprintf(gSpool->m_Buf, "-------------------------------------------------------\n");
    gSpool->Print();
    idlOS::sprintf(gSpool->m_Buf, "NAME                 TYPE                 VALUE\n");
    gSpool->Print();
    idlOS::sprintf(gSpool->m_Buf, "-------------------------------------------------------\n");
    gSpool->Print();

    for (ix=0; ix<MAX_TABLE_ELEMENTS; ix++)
    {
        t_node = m_SymbolTable[ix];
        while (t_node != NULL)
        {
            idlOS::snprintf(tmp_name, 21, "%s", t_node->element.name);

            typeConvert(&(t_node->element), tmp);

            if ( t_node->element.mInd != SQL_NULL_DATA )
            {
                switch (t_node->element.type)
                {
                case iSQL_BLOB_LOCATOR :
                case iSQL_CLOB_LOCATOR :
                    idlOS::sprintf(gSpool->m_Buf, "%-20s %-20s\n",
                                   tmp_name, tmp);
                    break;
                case iSQL_DOUBLE :
                    idlOS::sprintf(gSpool->m_Buf, "%-20s %-20s %f\n",
                                   tmp_name, tmp, t_node->element.d_value);
                    break;
                case iSQL_REAL :
                    idlOS::sprintf(gSpool->m_Buf, "%-20s %-20s %f\n",
                                   tmp_name, tmp, t_node->element.f_value);
                    break;
                default :
                    idlOS::sprintf(gSpool->m_Buf, "%-20s %-20s %s\n",
                                   tmp_name, tmp, t_node->element.c_value);
                    break;
                }
            }
            else
            {
                idlOS::sprintf(gSpool->m_Buf, "%-20s %-20s\n", tmp_name, tmp);
            }
            gSpool->Print();

            if (t_node->element.type == iSQL_BLOB_LOCATOR ||
                t_node->element.type == iSQL_CLOB_LOCATOR)
            {
                uteSetErrorCode(&gErrorMgr, utERR_ABORT_UNDISPLAYABLE_DATATYPE_Error);
                uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
                gSpool->Print();
            }

            t_node = t_node->next;
        }
    }

    idlOS::sprintf(gSpool->m_Buf, "\n");
    gSpool->Print();
}
IDE_RC
iSQLHostVarMgr::putBindList( SChar * a_name )
{
    HostVarNode *t_node = NULL;
    HostVarNode *s_node = NULL;

    // 선언되지 않은 호스트 변수
    IDE_TEST_RAISE((t_node = getVar(a_name)) == NULL, not_defined);

    // memory alloc error 가 발생한 경우
    IDE_TEST_RAISE( (s_node = (HostVarNode*)
                              idlOS::malloc(ID_SIZEOF(HostVarNode)))
                    == NULL, mem_alloc_error);
    idlOS::memset(s_node, 0x00, ID_SIZEOF(HostVarNode) );

    idlOS::strcpy(s_node->element.name, t_node->element.name);
    idlOS::strcpy(s_node->element.scale, t_node->element.scale);
    s_node->element.type       = t_node->element.type;
    /* PROJ-1584 DML Return Clause */
    s_node->element.inOutType  = t_node->element.inOutType;
    s_node->element.size       = t_node->element.size;
    s_node->element.precision  = t_node->element.precision;
    s_node->element.mInd       = t_node->element.mInd;

    /* BUG-36480, 42320:
     *   mCType, mSqlType and size ... are decided depending on the values
     *   which of the host variables are specified by a user. */
    switch (t_node->element.type)
    {
    case iSQL_REAL   :
        s_node->element.mCType = SQL_C_FLOAT;
        s_node->element.mSqlType = SQL_REAL;
        s_node->element.size = ID_SIZEOF(t_node->element.f_value);
        s_node->element.c_value = NULL;
        s_node->element.precision = 0;
        break;
    case iSQL_DOUBLE :
        s_node->element.mCType = SQL_C_DOUBLE;
        s_node->element.mSqlType = SQL_DOUBLE;
        s_node->element.size = ID_SIZEOF(t_node->element.d_value);
        s_node->element.c_value = NULL;
        s_node->element.precision = 0;
        break;
    case iSQL_BLOB_LOCATOR :
        s_node->element.mCType = SQL_C_BLOB_LOCATOR;
        s_node->element.mSqlType = SQL_BLOB;
        s_node->element.size = ID_SIZEOF(t_node->element.mLobLoc);
        s_node->element.c_value = NULL;
        s_node->element.precision = 0;
        break;
    case iSQL_CLOB_LOCATOR :
        s_node->element.mCType = SQL_C_CLOB_LOCATOR;
        s_node->element.mSqlType = SQL_CLOB;
        s_node->element.size = ID_SIZEOF(t_node->element.mLobLoc);
        s_node->element.c_value = NULL;
        s_node->element.precision = 0;
        break;
    case iSQL_CHAR       :
        s_node->element.mCType = SQL_C_CHAR;
        s_node->element.mSqlType = SQL_CHAR;
        s_node->element.precision = t_node->element.precision;
        break;
    case iSQL_VARCHAR    :
        s_node->element.mCType = SQL_C_CHAR;
        s_node->element.mSqlType = SQL_VARCHAR;
        s_node->element.precision = t_node->element.precision;
        break;
    case iSQL_NIBBLE :
        s_node->element.mCType = SQL_C_CHAR;
        s_node->element.mSqlType = SQL_NIBBLE;
        s_node->element.precision = t_node->element.precision;
        break;
    case iSQL_NCHAR       :
        s_node->element.mCType = SQL_C_CHAR;
        s_node->element.mSqlType = SQL_WCHAR;
        s_node->element.precision = t_node->element.precision;
        break;
    case iSQL_NVARCHAR    :
        s_node->element.mCType = SQL_C_CHAR;
        s_node->element.mSqlType = SQL_WVARCHAR;
        s_node->element.precision = t_node->element.precision;
        break;
    case iSQL_SMALLINT :
        s_node->element.mCType = SQL_C_CHAR;
        s_node->element.mSqlType = SQL_SMALLINT;
        s_node->element.precision = 0;
        break;
    case iSQL_INTEGER :
        s_node->element.mCType = SQL_C_CHAR;
        s_node->element.mSqlType = SQL_INTEGER;
        s_node->element.precision = 0;
        break;
    case iSQL_BIGINT :
        s_node->element.mCType = SQL_C_CHAR;
        s_node->element.mSqlType = SQL_BIGINT;
        s_node->element.precision = 0;
        break;
    case iSQL_FLOAT   :
        s_node->element.mCType = SQL_C_CHAR;
        s_node->element.mSqlType = SQL_FLOAT;
        break;
    case iSQL_DECIMAL :
        s_node->element.mCType = SQL_C_CHAR;
        s_node->element.mSqlType = SQL_DECIMAL;
        break;
    case iSQL_NUMERIC :
        s_node->element.mCType = SQL_C_CHAR;
        s_node->element.mSqlType = SQL_NUMERIC;
        break;
    case iSQL_NUMBER  :
        s_node->element.mCType = SQL_C_CHAR;
        s_node->element.mSqlType = SQL_FLOAT;
        break;
    case iSQL_DATE :
        s_node->element.mCType = SQL_C_CHAR;
        s_node->element.mSqlType = SQL_DATE;
        s_node->element.precision = DATE_SIZE;
        break;
    case iSQL_BYTE :
        s_node->element.mCType = SQL_C_CHAR;
        s_node->element.mSqlType = SQL_BYTE;
        s_node->element.precision = t_node->element.precision;
        break;
    case iSQL_VARBYTE:
        s_node->element.mCType = SQL_C_CHAR;
        s_node->element.mSqlType = SQL_VARBYTE;
        s_node->element.precision = t_node->element.precision;
        break;
    default:
        IDE_RAISE( invalid_datatype );
        break;
    }

    if (s_node->element.type == iSQL_BLOB_LOCATOR ||
        s_node->element.type == iSQL_CLOB_LOCATOR)
    {
        s_node->element.mLobLoc = t_node->element.mLobLoc;
    }
    else if (s_node->element.type == iSQL_DOUBLE)
    {
        s_node->element.d_value = t_node->element.d_value;
    }
    else if (s_node->element.type == iSQL_REAL)
    {
        s_node->element.f_value  = t_node->element.f_value;
    }
    else
    {
        IDE_TEST_RAISE((s_node->element.c_value = (SChar*)
                            idlOS::malloc(t_node->element.size))
                       == NULL, mem_alloc_error);
        idlOS::memset(s_node->element.c_value, 0x00, t_node->element.size);
        if ( s_node->element.mInd != SQL_NULL_DATA )
        {
            idlOS::strcpy(s_node->element.c_value, t_node->element.c_value);
        }
    }

    if (m_Head == NULL)
    {
        m_Head = m_Tail = s_node;
        s_node->host_var_next = NULL;
    }
    else
    {
        m_Tail->host_var_next = s_node;
        m_Tail = s_node;
    }

    m_BindListCnt++;

    return IDE_SUCCESS;

    IDE_EXCEPTION(not_defined);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_host_variable_not_defined_error, a_name);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        gSpool->Print();
    }
    IDE_EXCEPTION(mem_alloc_error);
    {
        idlOS::fprintf(stderr, "Memory allocation error!!! --- "
                       "(%"ID_INT32_FMT", %s)\n", __LINE__, __FILE__);
        Exit(0);
    }
    IDE_EXCEPTION(invalid_datatype);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Invalid_dataType_Error);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        gSpool->Print();
    }
    IDE_EXCEPTION_END;

    if (s_node != NULL)
    {
        idlOS::free(s_node);
    }

    return IDE_FAILURE;
}

void
iSQLHostVarMgr::setHostVar( idBool aIsFunc, HostVarNode * /*a_host_var_list*/ )
{
    // after execute procedure/function, set value host var in m_SymbolTable
    HostVarNode *t_node;
    HostVarNode *s_node;
    HostVarNode *sHeadNode;

    /* BUG-37224: 함수의 경우 리턴값을 받는 호스트 변수에 최우선으로 값이
     * 할당되어야 함. 따라서 m_Head가 m_Tail이 되도록 리스트를 변경함 */
    if (aIsFunc == ID_TRUE)
    {
        m_Tail->host_var_next = m_Head;
        m_Tail = m_Head;
        m_Head = m_Head->host_var_next;
        m_Tail->host_var_next = NULL;
    }
    else
    {
        /* do nothing */
    }

    sHeadNode = m_Head;
    t_node = m_Head;
    m_Head = t_node->host_var_next;

    while (1)
    {
        s_node = getVar(t_node->element.name);
        IDE_ASSERT(s_node != NULL);

        if (t_node->element.mInd != SQL_NULL_DATA)
        {
            s_node->element.mInd = SQL_NTS;
        }
        else
        {
            s_node->element.mInd = SQL_NULL_DATA;
        }

        if (t_node->element.mInd != SQL_NULL_DATA)
        {
            if (t_node->element.type == iSQL_BLOB_LOCATOR ||
                t_node->element.type == iSQL_CLOB_LOCATOR)
            {
                s_node->element.mLobLoc = t_node->element.mLobLoc;
            }
            else if (t_node->element.type == iSQL_DOUBLE)
            {
                if (s_node->element.d_value != t_node->element.d_value)
                {
                    s_node->element.d_value = t_node->element.d_value;
                }
            }
            else if (t_node->element.type == iSQL_REAL)
            {
                if (s_node->element.f_value != t_node->element.f_value)
                {
                    s_node->element.f_value = t_node->element.f_value;
                }
            }
            else
            {
                // BUG-37224 copy value only if param type is OUTPUT or INPUT_OUTPUT
                // BUG-42320 Redesigned host variables
                if ((t_node->element.inOutType == SQL_PARAM_OUTPUT) ||
                    (t_node->element.inOutType == SQL_PARAM_INPUT_OUTPUT))
                //if (idlOS::strcmp(s_node->element.c_value, t_node->element.c_value) != 0)
                {
                    idlOS::strcpy(s_node->element.c_value,
                                  t_node->element.c_value);
                    utString::removeLastSpace(s_node->element.c_value);
                }
            }
        }

        t_node = m_Head;
        if (t_node != NULL)
        {
            m_Head = t_node->host_var_next;
        }
        else
        {
            break;
        }
    }
    m_Head = sHeadNode;
}

SLong
iSQLHostVarMgr::strToLL( SChar  * aNPtr,
                         SChar ** aEndPtr,
                         int      aBase )
{
    SChar * sS;
    SInt    sAny;
    SInt    sC;
    SInt    sCutLim;
    SInt    sNeg;
    SLong   sLLVal;
    ULong   sAcc;
    ULong   sCutoff;

    sS = aNPtr;
    sNeg = 0;

    /*
     * Skip white space and pick up leading +/- sign if any.
     * If aBase is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if aBase is already 16, allow 0x.
     */
    do
    {
        sC = *sS++;
    }
    while (sC == ' ' || sC == '\t');

    if (sC == '-')
    {
        sNeg = 1;
        sC = *sS++;
    }
    else if (sC == '+')
    {
        sC = *sS++;
    }

    if ((aBase == 0 || aBase == 16) &&
         sC == '0' &&
         (*sS == 'x' || *sS == 'X'))
    {
        sC = sS[1];
        sS += 2;
        aBase = 16;
    }

    if (aBase == 0)
    {
        aBase = (sC == '0') ? 8 : 10;
    }

    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for long longs is
     * [-2147483648..2147483647] and the input base is 10,
     * sCutoff will be set to 214748364 and sCutLim to either
     * 7 (sNeg==0) or 8 (sNeg==1), meaning that if we have accumulated
     * a value > 214748364, or equal but the next digit is > 7 (or 8),
     * the number is too big, and we will return a range error.
     *
     * Set sAny if any `digits' consumed; make it negative to indicate
     * overflow.
     */
    sCutoff = (sNeg) ? ID_ULONG(0x8000000000000000)
                     : ID_ULONG(0x7FFFFFFFFFFFFFFF);
    sCutLim = (SInt)(sCutoff % (ULong)aBase);
    sCutoff /= (ULong)aBase;

    for (sAcc = 0, sAny = 0; ; sC = *sS++)
    {
        if ('0' <= sC && sC <= '9')
        {
            sC -= '0';
        }
        else
        {
            break;
        }

        if (sC >= aBase)
        {
            break;
        }

        if (sAny < 0 ||
            sAcc > sCutoff ||
            (sAcc == sCutoff && sC > sCutLim))
        {
            sAny = -1;
        }
        else
        {
            sAny = 1;
            sAcc *= aBase;
            sAcc += sC;
        }
    }

    if (sAny < 0)
    {
        sLLVal = (sNeg) ? (SLong)ID_ULONG(0x8000000000000000)
                        : ID_LONG(0x7FFFFFFFFFFFFFFF);
        errno = ERANGE;
    }
    else if (sNeg)
    {
        if (sAcc < ID_ULONG(0x8000000000000000))
        {
            sLLVal = -(SLong)sAcc;
        }
        else /* sAcc == ID_ULONG(0x8000000000000000) */
        {
            sLLVal = (SLong)sAcc;
        }
    }
    else
    {
        sLLVal = (SLong)sAcc;
    }

    if (aEndPtr != 0)
    {
        *aEndPtr = (SChar *)((sAny) ? (sS - 1) : aNPtr);
    }

    return sLLVal;
}
