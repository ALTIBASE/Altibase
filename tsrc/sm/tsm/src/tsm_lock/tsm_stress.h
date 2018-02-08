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
 * $Id: tsm_stress.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#ifndef _TSM_STRESS_H_
#define _TSM_STRESS_H_ 1

#define TSM_STRESS_CLIENT_COUNT 10
//  #define TSM_STRESS_TRANS_COUNT  2
//  #define TSM_STRESS_LOCK_COUNT   500
#define TSM_STRESS_TRANS_COUNT  8
#define TSM_STRESS_LOCK_COUNT   1024


IDE_RC tsm_stress();
IDE_RC startThread(SInt s_cClient);
void*  transThread(void* pParam);

#endif
