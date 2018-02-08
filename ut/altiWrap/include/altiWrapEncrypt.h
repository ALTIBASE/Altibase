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
 * $Id: altiWrapEncrypt.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/ 

#ifndef _ALTIWRAPENCRYPT_H_
#define _ALTIWRAPENCRYPT_H_ 1

#include <altiWrap.h>
#include <ideErrorMgr.h>
#include <idsAltiWrap.h>
#include <uttMemory.h>



class altiWrapEncrypt 
{
public:
    static IDE_RC doEncryption( altiWrap * aAltiWrap );

    static void setTextPositionInfo( altiWrap             * aAltiWrap,
                                     altiWrapNamePosition   aHeader,
                                     altiWrapNamePosition   aBody );

    static IDE_RC setEncryptedText( altiWrap * aAltiWrap,
                                    SChar    * aText,
                                    SInt       aTextLen,
                                    idBool     aIsNewLine );

private:
    static void adjustPlainTextLen( altiWrap             * aAltiWrap,
                                    altiWrapNamePosition   aBody );

    static IDE_RC combineEncryptedText( altiWrap             * aAltiWrap,
                                        SChar                * aBodyEncryptedText,
                                        SInt                   aBodyEncryptedTextLen,
                                        altiWrapNamePosition   aHeaderPos );
};    

#endif /* _ALTIWRAPENCRYPT_H_ */
