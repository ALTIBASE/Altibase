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
 * $Id: uttString.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

/***********************************************************************
 *
 * NAME
 *   uttString.h
 * 
 * DESCRIPTION
 *   Dynamic memory allocator.
 * 
 * PUBLIC FUNCTION(S)
 * NOTES
 * 
 * MODIFIED    (MM/DD/YYYY)
 *    hjmin     01/17/2001 - Created
 * 
 **********************************************************************/

#ifndef _O_UTTSTRING_H_
# define _O_UTTSTRING_H_  1

#include <idl.h>

class PDL_Proper_Export_Flag uttString
{
    /*
    SChar* str;
    int    length;
    */
    
public:
    /*
    uttString();
    uttString(int size);
    uttString(uttString s);
    ~uttString();
    */
    
    /* strdup */
    static SChar* utt_strdup(const SChar* s);
    static SChar* utt_strcpy(SChar *s1, const SChar* s2, int n = -1);
    
    /* compare if two strings are the same */
    /* return 0 : same, o/w : not same     */
    static SInt   streq(SChar* s1, const SChar* s2);
    static SInt   strneq(SChar* s1, const SChar* s2, int n);
    static SInt   strcaseeq(SChar* s1, const SChar* s2);
    static SInt   strncaseeq(SChar* s1, const SChar* s2, int n);
    
    static void   strfree(SChar* s);
    
    /*
    static SInt   strcmp(SChar* s1, SChar* s2);
    static SInt   strcasecmp(SChar* s1, SChar* s2);
    static SInt   strncmp(SChar* s1, SChar* s2, int n);
    */
    
    //SInt strdup(uttString s);
    
    /* strcpy */
    /*
    SInt strcpy(SChar* s);
    SInt strdup(uttString s);
    
    SInt   getLength() { return length; };
    SChar* getString() { return str; };
    */
};

#endif //_O_UTTSTRING_H_
