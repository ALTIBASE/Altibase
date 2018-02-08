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
 * $Id: utmExtern.h $
 **********************************************************************/

#ifndef _O_UTM_EXTERN_H_
#define _O_UTM_EXTERN_H_ 1

extern UserInfo       *m_UserInfo;
extern SInt            m_user_cnt;
extern utmProgOption   gProgOption;
extern ObjectModeInfo *gObjectModeInfo;  
extern FileStream      gFileStream;

extern SQLHENV m_henv;
extern SQLHDBC m_hdbc;
extern SChar   m_collectDbStatsStr[1024*1024]; // BUG-40174
extern SChar   m_revokeStr[1024*1024];  //BUG-22769
extern SChar   m_alterStr[1024*1024];   /* PROJ-2359 Table/Partition Access Option */

#endif /* _O_UTM_EXTERN_H_ */
