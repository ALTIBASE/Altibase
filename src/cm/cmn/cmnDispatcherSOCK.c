/**
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <cmAllClient.h>

#if !defined(CM_DISABLE_TCP) || !defined(CM_DISABLE_UNIX)

ACI_RC cmnDispatcherWaitLinkSOCK(cmnLink *aLink, cmnDirection aDirection, acp_time_t aTimeout)
{
    acp_sock_t *sSock;
    acp_rc_t    sResult;

    /*
     * LinkÀÇ socket È¹µæ
     */
    ACI_TEST(aLink->mOp->mGetSocket(aLink, (void**)&sSock) != ACI_SUCCESS);
    
    /* BUG-37360 [cm] The EINTR signal should be handled when the acpSockPoll function is used. */
    ACI_EXCEPTION_CONT(Restart);

    /*
     * select ¼öÇà
     */
    switch (aDirection)
    {
        case CMN_DIRECTION_RD:
            sResult = acpSockPoll(sSock, ACP_POLL_IN, aTimeout);
            break;
        case CMN_DIRECTION_WR:
            sResult = acpSockPoll(sSock, ACP_POLL_OUT, aTimeout);
            break;
        case CMN_DIRECTION_RDWR:
            sResult = acpSockPoll(sSock, ACP_POLL_IN | ACP_POLL_OUT, aTimeout);
            break;
        default:
            sResult = acpSockPoll(sSock, 0, aTimeout);
            break;
    }

    ACI_TEST_RAISE(ACP_RC_IS_ETIMEDOUT(sResult), TimedOut);

    /* BUG-37360 [cm] The EINTR signal should be handled when the acpSockPoll function is used. */
    ACI_TEST_RAISE(ACP_RC_IS_EINTR(sResult), Restart);
    ACI_TEST_RAISE(ACP_RC_NOT_SUCCESS(sResult), SelectError);

    return ACI_SUCCESS;

    ACI_EXCEPTION(SelectError);
    {
        ACI_SET(aciSetErrorCode(cmERR_ABORT_SELECT_ERROR));
    }
    ACI_EXCEPTION(TimedOut);
    {
        ACI_SET(aciSetErrorCode(cmERR_ABORT_TIMED_OUT));
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}


#endif
