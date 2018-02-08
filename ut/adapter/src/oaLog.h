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
 
/*
 * Copyright 2011, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 */

#ifndef __OA_LOG_H__
#define __OA_LOG_H__

extern ace_rc_t oaLogInitialize( oaContext    * aContext,
                                 acp_str_t    * aLogFileName,
                                 acp_sint32_t   aBackupLimit,
                                 acp_offset_t   aBackupSize,
                                 acp_str_t    * aMessageDirectory );

extern void oaLogFinalize(void);

extern void oaLogMessage(ace_msgid_t aMessageId, ...);
extern void oaLogMessageInfo(const acp_char_t *aFormat);
#endif /* __OA_LOG_H__ */
