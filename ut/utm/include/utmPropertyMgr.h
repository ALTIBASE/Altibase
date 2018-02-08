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
 * $Id: utmPropertyMgr.h 80542 2017-07-19 08:01:20Z daramix $
 **********************************************************************/

/***********************************************************************
 *
 * NAME
 *
 *
 * DESCRIPTION
 *
 *
 * PUBLIC FUNCTION(S)
 *
 *
 * PRIVATE FUNCTION(S)
 *
 *
 * NOTES
 *
 * MODIFIED    (MM/DD/YYYY)
 *
 *
 **********************************************************************/

#ifndef _O_UTMPROPERTYMGR_H_
#define _O_UTMPROPERTYMGR_H_

#include <idl.h>

#define MAX_PROPERTIES     1024

class utmPropertyPair
{
    friend class utmPropertyMgr;
    SChar *name_;
    SChar *value_;
};

class utmPropertyMgr
{
    SInt   cnt_of_properties_;
    SChar  home_dir_[256];
    SChar  conf_file_[256];
    utmPropertyPair ppair_[MAX_PROPERTIES];

    SInt parseBuffer(SChar *buffer, SChar **name, SChar **value);

public:
    void initialize(SChar *HomeDir, SChar *ConfFile);
    SInt readProperties();
    SChar* getValue(SChar *name);
    SChar* getHomeDir();
    SChar* getConfFile();
};

utmPropertyMgr* utmGetPropertyMgr(SChar *EnvString = NULL, /*환경변수 스트링*/
                                    SChar *HomeDir   = NULL,  /* 홈디렉토리 */
                                    SChar *ConfFile  = NULL); /* Conf 화일명 */

utmPropertyMgr* utmReadPropertyMgr(SChar *HomeDir,
                                     SChar *ConfFile);

#endif /* _O_UTMPROPERTYMGR_H_ */
