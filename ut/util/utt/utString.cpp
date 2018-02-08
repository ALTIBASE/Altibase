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
 * $Id: utString.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <ide.h>
#include <idl.h>
#include <mtcl.h>
#include <utString.h>
// bug-26661: nls_use not applied to nls module for ut
#include <uln.h>                 // gNlsModuleForUT
#include <ulnCatalogFunctions.h> // ulnMakeNameInSQL

/* BUG-34502: handling quoted identifiers */
SChar *gQpKeywords[] =
{
#include "qpKeywords.c"
    NULL
};

void utString::eraseWhiteSpace(SChar *a_Str)
{
    SInt i, j;
    SInt len;
    
    // 1. 앞에서 부터 검사 시작..

    len = idlOS::strlen(a_Str);
    if( len <= 0 )
    {
        return;
    }

    for (i=0; i<len && a_Str[i]; i++)
    {
        if (a_Str[i]==' ') // 스페이스 임
        {
            for (j=i; a_Str[j]; j++)
            {
                a_Str[j] = a_Str[j+1];
            }
            i--;
        }
        else
        {
            break;
        }
    }
    
    // 2. 끝에서 부터 검사 시작.. : 스페이스 없애기

    len = idlOS::strlen(a_Str);
    if( len <= 0 )
    {
        return;
    }

    for (i = len-1; i >= 0; i--)
    {
        // fix BUG-17441
        // FIX BUG-18408
        if ( (a_Str[i] == 0) ||
             ((a_Str[i] != ' ') && (a_Str[i] != '\r'))
            )
        {
            break;
        }
        else // if (a_Str[i]==' ' || a_Str[i] == '\r') 스페이스 없애기
        {
            a_Str[i] = 0;
        }
    }
}

void utString::removeLastSpace(SChar *a_Str)
{
    SInt i, len;

    len = idlOS::strlen(a_Str);
    if( len <= 0 )
    {
        return;
    }

    for (i = len-1; i >= 0; i--)
    {
        // fix BUG-17441
        // FIX BUG-18408        
        if ( (a_Str[i] == 0) ||
             ((a_Str[i] != ' ') && (a_Str[i] != '\r'))
            )
        {
            break;
        }
        else // if (a_Str[i]==' ' || a_Str[i] == '\r') 스페이스 없애기
        {
            a_Str[i] = 0;
        }
    }
}

IDE_RC utString::toUpper(SChar *a_Str)
{
    UChar unit;
    SChar *s_Str;
    SChar * sFence;
    const mtlModule * sNlsModule = NULL;

    sFence = a_Str + idlOS::strlen(a_Str);

    // bug-26661: nls_use not applied to nls module for ut
    // 변경전: mtl::defaultModule만 사용
    // 변경후: gNlsModuleForUT를 사용
    // ulnInitialze 호출 전에는 null일 수 있음.
    // ex) isql에서 인자로 user, passwd 처리시 null
    if (gNlsModuleForUT == NULL)
    {
        sNlsModule = mtlDefaultModule();
    }
    else
    {
        sNlsModule = gNlsModuleForUT;
    }

    for (s_Str=a_Str; *s_Str!='\0';)
    {
        unit = *((UChar*)s_Str);
        if ( unit >= 97 && unit <=122 )
            *s_Str = unit - 32;

        sNlsModule->nextCharPtr((UChar**)&s_Str, (UChar*)sFence);
    }

    return IDE_SUCCESS;
}

void utString::removeLastCR(SChar *a_Str)
{
    SInt i, len;

    len = idlOS::strlen(a_Str);
    if( len <= 0 )
    {
        return;
    }

    for (i=len-1; a_Str[i] && i>=0; i--)
    {
        if (a_Str[i]=='\n' || a_Str[i] == '\t') 
        {
            a_Str[i] = 0;
        }
        else
        {
            break;
        }
    }
}

void
utString::makeNameInSQL( SChar * aDstName,
                         SInt    aDstLen,
                         SChar * aSrcName,
                         SInt    aSrcLen )
{
/***********************************************************************
 *
 * Description :
 *
 *    입력된 name을 SQL 문장에서 사용할 이름으로 변경함.
 *    향후 Error Handling 할 수 있도록 해야 함.
 *
 * Implementation :
 *
 **********************************************************************/

    SInt sSize;

    sSize = (aDstLen > aSrcLen ) ? aSrcLen : aDstLen - 1;

    // bug-26661: nls_use not applied to nls module for ut
    // 변경전: mtl::defaultModule을 사용한 makeNameInSQL 호출
    // 변경후: gNlsModuleForUT를 사용
    // ulnInitialze 호출 전에는 null일 수 있음.
    mtlMakeNameInSQL( (gNlsModuleForUT == NULL) ? mtlDefaultModule() : gNlsModuleForUT,
                       aDstName, aSrcName, sSize);
}

void
utString::makeNameInCLI( SChar * aDstName,
                         SInt    aDstLen,
                         SChar * aSrcName,
                         SInt    aSrcLen )
{
/***********************************************************************
 *
 * Description :
 *
 *    입력된 name을 CLI 함수에서 사용할 이름으로 변경함.
 *    향후 Error Handling 할 수 있도록 해야 함.
 *
 * Implementation :
 *
 **********************************************************************/

    SInt sSize;

    sSize = (aDstLen > aSrcLen ) ? aSrcLen : aDstLen - 1;

    // bug-26661: nls_use not applied to nls module for ut
    // 변경전: mtl::defaultModule을 사용한 makeNameInFunc 호출
    // 변경후: gNlsModuleForUT를 사용
    // ulnInitialze 호출 전에는 null일 수 있음.
    // ex) isql에서 인자로 user, passwd 처리시 null
    mtlMakeNameInFunc( (gNlsModuleForUT == NULL) ? mtlDefaultModule() : gNlsModuleForUT,
                       aDstName, aSrcName, sSize);
}

void
utString::makeQuotedName( SChar * aDstName,
                          SChar * aSrcName,
                          SInt    aSrcLen )
{
/***********************************************************************
 *
 * Description :
 *
 *    입력된 name에 대하여 Quoted Name을 생성한다.
 *    서버로부터 얻어온 이름을 재사용할 때 사용한다.
 *
 * Implementation :
 *
 *    앞뒤로 Double Quotation을 붙인다.
 *
 **********************************************************************/

    mtlMakeQuotedName(aDstName, aSrcName, aSrcLen);
}

/* BUG-34502: handling quoted identifiers */
idBool
utString::needQuotationMarksForFile( SChar * aSrcText )
{
/***********************************************************************
 *
 * Description :
 *
 *    입력된 name에 알파벳, 숫자, _ 외의 문자가 포함되어 있는지 검사한다.
 *    포함되어 있으면 ID_TRUE를(즉 따옴표가 필요함),
 *    그렇지 않으면 ID_FALSE를 반환한다.
 *
 * Implementation :
 *
 *    idlOS::regcomp, regexec 등이 없어서 그냥 캐릭터 비교함..
 *
 **********************************************************************/

    idBool ret = ID_FALSE;
    UChar  unit = 0;
    SChar *sStr = NULL;
    SChar *sFence = NULL;
    const  mtlModule * sNlsModule = NULL;

    sFence = aSrcText + idlOS::strlen(aSrcText);

    if (gNlsModuleForUT == NULL)
    {
        sNlsModule = mtlDefaultModule();
    }
    else
    {
        sNlsModule = gNlsModuleForUT;
    }

    ret = ID_FALSE;
    for (sStr=aSrcText; *sStr!='\0';)
    {
        unit = *((UChar*)sStr);
        if ( ( unit >= 65 && unit <=90 )  ||  /* A-Z */
             ( unit == 95 )               ||  /* _ */
             ( unit >= 48 && unit <=57 )  ||  /* 0-9 */
             ( unit >= 97 && unit <=122 ) )   /* a-z */
        {
            // do nothing...
        }
        else
        {
            ret = ID_TRUE;
            break;
        }
        sNlsModule->nextCharPtr((UChar**)&sStr, (UChar*)sFence);
    }

    return ret;
}

/* BUG-34502: handling quoted identifiers */
idBool
utString::needQuotationMarksForObject( SChar * aSrcText,
                                       idBool  aCheckKeywords )
{
/***********************************************************************
 *
 * Description :
 *
 *    1. 입력된 name에 알파벳 대문자, 숫자, _ 외의 문자가 포함되어 있는지 검사한다.
 *    포함되어 있으면 ID_TRUE를(즉 따옴표가 필요함),
 *    그렇지 않으면 ID_FALSE를 반환한다.
 *
 *    2. 입력 문자열이 database keyword 이라면 따옴표가 필요하다.
 *    qcpll.l 파일로부터 생성한 qpKeywords.c 의 keywords와 비교
 *
 * Implementation :
 *
 *    그냥 캐릭터 비교함..
 *
 **********************************************************************/

    UInt   i = 0;
    idBool sRet = ID_FALSE;
    UChar  unit = 0;
    SChar *sStr = NULL;
    SChar *sFence = NULL;
    const  mtlModule * sNlsModule = NULL;

    sFence = aSrcText + idlOS::strlen(aSrcText);

    if (gNlsModuleForUT == NULL)
    {
        sNlsModule = mtlDefaultModule();
    }
    else
    {
        sNlsModule = gNlsModuleForUT;
    }

    for (sStr=aSrcText; *sStr!='\0';)
    {
        unit = *((UChar*)sStr);
        if ( ( unit >= 65 && unit <=90 )  ||  /* A-Z */
             ( unit >= 48 && unit <=57 )  ||  /* 0-9 */
             ( unit == 95 ) )                 /* _ */
        {
            // do nothing...
        }
        else
        {
            sRet = ID_TRUE;
            IDE_RAISE( STOP_COMPARING ); //BUGBUG: IDE_CONT로 바꿀 것
        }
        sNlsModule->nextCharPtr((UChar**)&sStr, (UChar*)sFence);
    }

    IDE_TEST_RAISE( aCheckKeywords == ID_FALSE, STOP_COMPARING );

    /*
     * 입력 문자열이 database keyword인지 검사
     */
    for (i = 0; gQpKeywords[i] != NULL; i++)
    {
        if (idlOS::strcmp(aSrcText, gQpKeywords[i]) == 0)
        {
            sRet = ID_TRUE;
            IDE_RAISE( STOP_COMPARING ); //BUGBUG: IDE_CONT로 바꿀 것
        }
    }

    IDE_EXCEPTION_CONT( STOP_COMPARING );

    return sRet;
}

/* BUG-39893 Validate connection string according to the new logic, BUG-39767 */
IDE_RC utString::AppendConnStrAttr( uteErrorMgr *aErrorMgr,
                                    SChar       *aConnStr,
                                    UInt         aConnStrSize,
                                    SChar       *aAttrKey,
                                    SChar       *aAttrVal )
{
    UInt sAttrValLen;

    IDE_TEST_RAISE( aAttrVal == NULL || aAttrVal[0] == '\0', NO_NEED_WORK );

    idlVA::appendString( aConnStr, aConnStrSize, aAttrKey, idlOS::strlen(aAttrKey) );
    idlVA::appendString( aConnStr, aConnStrSize, (SChar *)"=", 1 );
    sAttrValLen = idlOS::strlen(aAttrVal);
    if ( (aAttrVal[0] == '"' && aAttrVal[sAttrValLen - 1] == '"') ||
         (idlOS::strchr(aAttrVal, ';') == NULL &&
          acpCharIsSpace(aAttrVal[0]) == ACP_FALSE &&
          acpCharIsSpace(aAttrVal[sAttrValLen - 1]) == ACP_FALSE) )
    {
        idlVA::appendString( aConnStr, aConnStrSize, aAttrVal, sAttrValLen );
    }
    else if ( idlOS::strchr(aAttrVal, '"') == NULL )
    {
        idlVA::appendFormat( aConnStr, aConnStrSize, "\"%s\"", aAttrVal );
    }
    else
    {
        IDE_RAISE( InvalidConnAttr );
    }
    idlVA::appendString( aConnStr, aConnStrSize, (SChar *)";", 1 );

    IDE_EXCEPTION_CONT( NO_NEED_WORK );

    return IDE_SUCCESS;

    IDE_EXCEPTION( InvalidConnAttr )
    {
        uteSetErrorCode(aErrorMgr, utERR_ABORT_INVALID_CONN_ATTR, aAttrKey, aAttrVal);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

