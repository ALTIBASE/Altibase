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
 * $Id: iloFormCompiler.h 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

#ifndef _O_ILO_FORMCOMPILER_H
#define _O_ILO_FORMCOMPILER_H

extern FILE *Formin;

class iloFormCompiler
{
private:
    // BUG-25393 Æû ÆÄ½Ì¿¡·¯½Ã Á×½À´Ï´Ù.
    FILE *mFormFile;
public:
    iloFormCompiler()  { mFormFile = NULL; };

    SInt FormFileParse( ALTIBASE_ILOADER_HANDLE aHandle, SChar *szFileName);

    FILE *GetInputFp()              { return mFormFile; }
};

#endif /* _O_ILO_FORMCOMPILER_H */
