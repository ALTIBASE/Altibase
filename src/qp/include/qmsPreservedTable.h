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
 * $Id: qmsPreservedTable.h 56549 2012-11-23 04:38:50Z sungminee $
 **********************************************************************/

#ifndef _O_QMS_PRESERVED_TABLE_H_
#define _O_QMS_PRESERVED_TABLE_H_ 1

#include <qc.h>
#include <qmsParseTree.h>
#include <qtc.h>

class qmsPreservedTable
{
public:
    
    static IDE_RC initialize( qcStatement  * aStatement,
                              qmsSFWGH     * aSFWGH );

    static IDE_RC addTable( qmsSFWGH     * aSFWGH,
                            qmsTableRef  * aTableRef );
    
    static IDE_RC find( qcStatement  * aStatement,
                        qmsSFWGH     * aSFWGH );

    static IDE_RC getFirstKeyPrevTable( qmsSFWGH      * aSFWGH,
                                        qmsTableRef  ** aTableRef );

    static IDE_RC checkKeyPreservedTableHints( qmsSFWGH     * aSFWGH );
    
private:

};

#endif /* _O_QMS_PRESERVED_TABLE_H_ */
