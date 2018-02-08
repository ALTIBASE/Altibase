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
 * $Id: altiWrapi.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/ 

#ifndef _ALTIWRAPI_H_
#define _ALTIWRAPI_H_ 1

#include <altiWrap.h>



class altiWrapi
{
public:

    static IDE_RC parsingCommand( altiWrap     * aAltiWrap,
                                  acp_sint32_t   aArgc,
                                  acp_char_t   * aArgv[],
                                  idBool       * aHasHelpOpt );

    static IDE_RC allocAltiWrap( altiWrap ** aAltiWrap );

    static void finalizeAltiWrap( altiWrap * aAltiWrap );

private:
    static void printHelp();
};

#endif /* _ALTIWRAPI_H_ */
