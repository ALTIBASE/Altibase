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
 * $Id: utmProperty.cpp 80542 2017-07-19 08:01:20Z daramix $
 **********************************************************************/

/***********************************************************************
 *
 * NAME
 *   utmProperty.cpp
 *
 * DESCRIPTION
 *
 *
 * NOTES
 *
 * MODIFIED    (MM/DD/YYYY)
 *    jdlee     10/23/2000 - Created
 *
 **********************************************************************/

#include <idl.h>
#include <ideLog.h>
#include <ide.h>

/* ---------------------------------------
 * % utmProperty loading을 위한 매크로
 * % 동일한 작업을 줄이기 위해 매크로 사용
 * ---------------------------------------*/

#define  IDU_PROP_SET_VALUE(macro, value)                \
{                                                        \
    SChar *sEnvValue;                                    \
    SChar *sPropValue;                                   \
                                                         \
    sEnvValue = getEnv((SChar *)#macro);                 \
                                                         \
    if (sEnvValue != NULL)                               \
    {                                                    \
        if (idlOS::strlen(sEnvValue) == 0)               \
        {                                                \
            sEnvValue = NULL;                            \
        }                                                \
    }                                                    \
    if (sEnvValue == NULL)                               \
    {                                                    \
        sPropValue = iduPM_->getValue((SChar *)#macro);  \
                                                         \
        if (sPropValue == NULL)                          \
        {                                                \
            value = IDU_DEFAULT_##macro;                 \
        }                                                \
        else                                             \
        {                                                \
            value = idlVA::fstrToLong(sPropValue);       \
        }                                                \
    }                                                    \
    else                                                 \
    {                                                    \
        value = idlVA::fstrToLong(sEnvValue);            \
    }                                                    \
}

#define IDU_PROP_NOT_NULL(a)  if ((a) == NULL) { return IDE_FAILURE; }

#define IDU_PROP_SET_STRING(macro,value) \
        if ( (value = getEnv((SChar *)#macro)) == NULL) \
        {\
            value = iduPM_->getValue((SChar*)#macro);\
        }


/* -------------------------------
 * utmProperty static 멤버 정의
 * ------------------------------*/

utmPropertyMgr * utmProperty::iduPM_ = NULL;

SChar          * utmProperty::home_dir_;

SChar   *
utmProperty::getEnv (SChar *str)
{
#define IDE_FN "utmProperty::getEnv (SChar *str)"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY("str=%s", IDE_STR(str)));

    SChar   name[1024];
    idlOS::sprintf(name, "%s%s", IDU_PROPERTY_PREFIX, str);
    return idlOS::getenv ((const SChar *)name );


#undef IDE_FN
}

