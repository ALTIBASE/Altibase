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
 * $Id: qmoCheckViewColumnRef.h 82075 2018-01-17 06:39:52Z jina.kim $
 *
 * Description :
 *     PROJ-2469 Optimize View Materialization
 *
 **********************************************************************/

#include <ide.h>
#include <qc.h>
#include <qmsParseTree.h>

#ifndef _O_QMO_CHECK_VIEW_COLUMN_REF_H_
#define _O_QMO_CHECK_VIEW_COLUMN_REF_H_ 1

class qmoCheckViewColumnRef
{    
public :

    static IDE_RC checkViewColumnRef( qcStatement      * aStatement,
                                      qmsColumnRefList * aParentColumnRef,
                                      idBool             aAllColumnUsed );

private :    
    
    static IDE_RC checkQuerySet( qmsQuerySet      * sQuerySet,
                                 qmsColumnRefList * aParentColumnRef,
                                 qmsSortColumns   * aOrderBy,
                                 idBool             aAllColumnUsed );
    
    static IDE_RC checkFromTree( qmsFrom          * aFrom,
                                 qmsColumnRefList * aParentColumnRef,
                                 qmsSortColumns   * aOrderBy,
                                 idBool             aAllColumnUsed );

    static IDE_RC checkUselessViewTarget( qmsTarget        * aTarget,
                                          qmsColumnRefList * aParentColumnRef,
                                          qmsSortColumns   * aOrderBy );

    static IDE_RC checkUselessViewColumnRef( qmsTableRef      * aTableRef,
                                             qmsColumnRefList * aParentColumnRef,
                                             qmsSortColumns   * aOrderBy );
};
    
#endif /* _O_QMO_CHECK_VIEW_COLUMN_REF_H_ */
