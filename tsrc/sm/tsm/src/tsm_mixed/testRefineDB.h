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
 * $Id: testRefineDB.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#ifndef _O_TESTREFINEDB_H_
# define _O_TESTREFINEDB_H_ 1

IDE_RC testInsertCommitRefineDB1();
IDE_RC testInsertRollbackRefineDB1();
IDE_RC testUpdateCommitRefineDB1();
IDE_RC testUpdateRollbackRefineDB1();
IDE_RC testDeleteCommitRefineDB1();
IDE_RC testDeleteRollbackRefineDB1();
IDE_RC testRefineDB2();


#endif /* _O_TESTREFINEDB_H_ */
