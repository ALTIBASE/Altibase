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
#include <uttString.h>

SChar* uttString::utt_strdup(const SChar* s)

{

    SChar* temp;

    

    if (s == NULL)

    {

        return NULL;

    }

    if ((temp = (SChar*)idlOS::malloc(idlOS::strlen(s)+1)) == NULL)

    {

        return NULL;

    }

    idlOS::strcpy(temp, s);

    return temp;

}



SChar* uttString::utt_strcpy(SChar* s1, const SChar* s2, int n)

{

    if (s1 == NULL || s2 == NULL)

    {

        return NULL;

    }

    

    if (n > 0 && n < (SInt)idlOS::strlen(s2)+1 )

    {

        return NULL;

    }

    return idlOS::strcpy(s1,s2);

}

SInt   uttString::streq(SChar* s1, const SChar* s2)

{

    if (s1 == NULL || s2 == NULL)

    {

        return -1;

    }

    return idlOS::strcmp(s1,s2);

}

SInt   uttString::strneq(SChar* s1, const SChar* s2, int n)

{

    if (s1 == NULL || s2 == NULL)

    {

        return -1;

    }

    return idlOS::strncmp(s1,s2,n);

}

SInt   uttString::strcaseeq(SChar* s1, const SChar* s2)

{

    if (s1 == NULL || s2 == NULL)

    {

        return -1;

    }

    return idlOS::strcasecmp(s1,s2);

}

SInt   uttString::strncaseeq(SChar* s1, const SChar* s2, int n)

{

    if (s1 == NULL || s2 == NULL)

    {

        return -1;

    }

    return idlOS::strncasecmp(s1,s2,n);

}



void   uttString::strfree(SChar* s)

{

    if (s != NULL)

    {

        idlOS::free(s);

        s = NULL;

    }

}

