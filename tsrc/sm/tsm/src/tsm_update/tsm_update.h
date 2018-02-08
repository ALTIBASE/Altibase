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
 * $Id: tsm_update.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#ifndef _TSM_MULTI_H_
#define _TSM_MULTI_H_ 1

void * tsm_dml_upt1( void *data );
void * tsm_dml_upt2( void *data );
void * tsm_dml_sel(void *data);
void * tsm_dml_del(void *data);
void * tsm_dml_ins(void *data);

#define TSM_MULTI_TABLE ((SChar*)"TABLE1")
#define TSM_THREAD_COUNT 10 

#endif
