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
 * $Id: qsxExtProc.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#ifndef _O_QSX_EXTPROC_H_
#define  _O_QSX_EXTPROC_H_  1

#include <idx.h>
#include <mtc.h>
#include <qsParseTree.h>

class qsxExtProc
{
private:
    static void   convertDate2Timestamp( mtdDateType    * aDate,
                                         idxTimestamp   * aTimestamp );

    static IDE_RC convertTimestamp2Date( idxTimestamp   * aTimestamp,
                                         mtdDateType    * aDate );

    static idxParamType getParamType( UInt aTypeId );

    static IDE_RC fillParamAndPropValue( iduMemory    * aQxeMem,
                                         mtcColumn    * aColumn,
                                         SChar        * aRow,
                                         qcTemplate   * aTmplate,
                                         idxParamInfo * aParamInfo );

    static IDE_RC fillTimestampParamValue( UInt             aDataTypeId,
                                           SChar          * aValue,
                                           idxTimestamp   * aTimestamp );

    static IDE_RC returnParamValue( iduMemory        * aQxeMem,
                                    idxParamInfo     * aParamInfo,
                                    qcTemplate       * aTmplate );

    static void   returnParamProperty( idxParamInfo     * aParamInfo,
                                       qcTemplate       * aTmplate );

    static void   returnCharLength( void       * aValue,
                                    SInt         aMaxLength,
                                    UShort     * aOutLength );

    static void   returnNcharLength( mtcColumn * aColumn,
                                     void      * aValue,
                                     SInt        aMaxLength,
                                     UShort    * aOutLength );

public:
    static void   initializeMsg( idxExtProcMsg * aMsg );

    static void   initializeParamInfo( idxParamInfo * aParamInfo );

    static IDE_RC fillParamInfo( iduMemory        * aQxeMem,
                                 qsCallSpecParam  * aParam,
                                 qcTemplate       * aTmplate,
                                 idxParamInfo     * aParamInfo,
                                 UInt               aOrder );

    static IDE_RC fillReturnInfo( iduMemory        * aQxeMem,
                                  qsVariableItems    aRetItem,
                                  qcTemplate       * aTmplate,
                                  idxParamInfo     * aParamInfo );

    static IDE_RC returnAllParams( iduMemory     * aQxeMem,
                                   idxExtProcMsg * aMsg,
                                   qcTemplate    * aTmplate );
};

#endif /* _O_QSX_EXTPROC_H_ */
