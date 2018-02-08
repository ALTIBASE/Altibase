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
 * $Id: rpDef.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#ifndef _O_RP_DEF_H_
#define _O_RP_DEF_H_ 1

#define RP_STATUS_NONE       (0)
#define RP_STATUS_TX_BEGIN   (1 << 0)
#define RP_STATUS_STMT_BEGIN (1 << 1)
#define RP_STATUS_CUR_OPEN   (1 << 2)

#define RP_SET_STATUS_INIT(st)       {  (st) = RP_STATUS_NONE; }
#define RP_SET_STATUS_TX_BEGIN(st)   {  (st) |= RP_STATUS_TX_BEGIN; }
#define RP_SET_STATUS_STMT_BEGIN(st) {  (st) |= RP_STATUS_STMT_BEGIN; }
#define RP_SET_STATUS_CUR_OPEN(st)   {  (st) |= RP_STATUS_CUR_OPEN; }

#define RP_SET_STATUS_TX_END(st)     {  (st) &= ~RP_STATUS_TX_BEGIN; }
#define RP_SET_STATUS_STMT_END(st)   {  (st) &= ~RP_STATUS_STMT_BEGIN; }
#define RP_SET_STATUS_CUR_CLOSE(st)  {  (st) &= ~RP_STATUS_CUR_OPEN; }

#define RP_IS_STATUS_TX_BEGIN(st)    ( (st) & RP_STATUS_TX_BEGIN )
#define RP_IS_STATUS_STMT_BEGIN(st)  ( (st) & RP_STATUS_STMT_BEGIN )
#define RP_IS_STATUS_CUR_OPEN(st)    ( (st) & RP_STATUS_CUR_OPEN )

#define RP_STATUS_VALID                                 0
#define RP_STATUS_INVALID_CURSOR_OPEN_WITHOUT_STMT_OPEN 1
#define RP_STATUS_INVALID_CURSOR_OPEN_WITHOUT_TX_BEGIN  2
#define RP_STATUS_INVALUD_STMT_BEGIN_WITHOUT_TX_BEGIN   3

#define RP_GET_INVALID_STATUS_CODE(st)                            \
(                                                                  \
    ( (st) & RP_STATUS_CUR_OPEN) ?                                \
         ( (st) & RP_STATUS_STMT_OPEN ) ?                         \
             ( (st) & RP_STATUS_TX_BEGIN ) ?                      \
                 (RP_STATUS_VALID)                                \
             :                                                     \
                 (RP_STATUS_INVALID_CURSOR_OPEN_WITHOUT_TX_BEGIN) \
         :                                                         \
             (RP_STATUS_INVALID_CURSOR_OPEN_WITHOUT_STMT_OPEN)    \
    :                                                              \
         ( (st) & RP_STATUS_STMT_BEGIN ) ?                        \
             ( (st) & RP_STATUS_TX_BEGIN ) ?                      \
                 (RP_STATUS_VALID)                                \
             :                                                     \
             (RP_STATUS_INVALUD_STMT_BEGIN_WITHOUT_TX_BEGIN)      \
         :                                                         \
         (RP_STATUS_VALID)                                        \
)        

#define RP_TEST_STATUS_VALID(st, err_label)                       \
{                                                                  \
    if ( RP_GET_INVALID_STATUS_CODE(st) != RP_STATUS_VALID )     \
    {                                                              \
        IDE_RAISE(err_label);                                      \
    }                                                              \
}

#define RP_GET_STATUS_MESSAGE(st)                                      \
(                                                                       \
    ( RP_GET_INVALID_STATUS_CODE((st)) ==                              \
        RP_STATUS_INVALID_CURSOR_OPEN_WITHOUT_STMT_OPEN ) ?            \
        ("Cursor is open with statement closed")                        \
    :                                                                   \
        ( RP_GET_INVALID_STATUS_CODE((st)) ==                          \
            RP_STATUS_INVALID_CURSOR_OPEN_WITHOUT_TX_BEGIN ) ?         \
            ("Cursor is open with transaction inactive")                \
        :                                                               \
            ( RP_GET_INVALID_STATUS_CODE((st)) ==                      \
                RP_STATUS_INVALUD_STMT_BEGIN_WITHOUT_TX_BEGIN ) ?      \
                ("Statement is open with transaction inactive")         \
            :                                                           \
                ( RP_GET_INVALID_STATUS_CODE((st)) ==                  \
                     RP_STATUS_VALID ) ?                               \
                     ("Everything is valid (Internal server error).")   \
                :                                                       \
                     ("Internal server error - Invalid status." )       \
)

typedef enum
{
    RP_START_RECV_OK = 0,
    RP_START_RECV_NETWORK_ERROR,
    RP_START_RECV_INVALID_VERSION,
    RP_START_RECV_HBT_OCCURRED,
    
    RP_START_RECV_MAX_MAX
} rpRecvStartStatus;

#define RP_SENDER_APPLY_CHECK_INTERVAL 10 //10 sec
#define RP_UNUSED_RECEIVER_INDEX (SMX_NOT_REPL_TX_ID - 1)


#endif /* _O_RP_DEF_H_ */
