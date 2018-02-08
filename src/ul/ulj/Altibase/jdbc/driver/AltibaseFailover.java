/*
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

package Altibase.jdbc.driver;

import java.sql.SQLException;
import java.sql.SQLWarning;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;

import Altibase.jdbc.driver.cm.CmChannel;
import Altibase.jdbc.driver.ex.Error;
import Altibase.jdbc.driver.ex.ErrorDef;
import Altibase.jdbc.driver.logging.LoggingProxy;
import Altibase.jdbc.driver.logging.TraceFlag;
import Altibase.jdbc.driver.util.AltibaseProperties;
import Altibase.jdbc.driver.util.StringUtils;

final class AltibaseFailover
{
    enum CallbackState
    {
        STOPPED,
        IN_PROGRESS,
    }

    /* BUG-31390 Failover info for v$session */
    enum FailoverType
    {
        CTF,
        STF,
    }

    private static Logger    mLogger;

    static 
    {
        if (TraceFlag.TRACE_COMPILE && TraceFlag.TRACE_ENABLED)
        {
            mLogger = Logger.getLogger(LoggingProxy.JDBC_LOGGER_FAILOVER);
        }
    }

    private AltibaseFailover()
    {
    }

    /**
     * CTF를 시도하고, 결과에 따라 경고를 남기거나 원래났던 예외를 다시 던진다.
     *
     * @param aContext Failover context
     * @param aWarning 경고를 담을 객체. null이면 새 객체를 만든다.
     * @param aException 원래 났던 예외
     * @return 경고를 담은 객체
     * @throws SQLException CTF를 할 상황이 아니거나 CTF에 실패한 경우
     */
    public static SQLWarning tryCTF(AltibaseFailoverContext aContext, SQLWarning aWarning, SQLException aException) throws SQLException
    {
        if (!AltibaseFailover.checkNeedCTF(aContext, aException))
        {
            throw aException;
        }

        boolean sCTFSuccess = AltibaseFailover.doCTF(aContext);
        if (sCTFSuccess == false)
        {
            throw aException;
        }

        /* BUG-32917 Connection Time Failover(CTF) information should be logged */
        aWarning = Error.createWarning(aWarning, ErrorDef.FAILOVER_SUCCESS, aException.getSQLState(), aException.getMessage(), aException);
        if (TraceFlag.TRACE_COMPILE && TraceFlag.TRACE_ENABLED)
        {
            mLogger.log(Level.WARNING, "Failover completed");
        }

        return aWarning;
    }

    /**
     * STF를 시도하고, 결과에 따라 예외를 던진다.
     * 성공했을때도 예외를 던져 STF 성공을 알린다.
     * <p>
     * 만약 failover context가 초기화되지 않았다면 원래 났던 예외를 던지거나 사용자가 넘긴 경고 객체를 그대로 반환한다.
     *
     * @param aContext Failover context
     * @param aException 원래 났던 예외
     * @throws SQLException STF에 실패했다면 원래 났던 예외, 아니면 STF 성공을 알리는 예외
     */
    public static void trySTF(AltibaseFailoverContext aContext, SQLException aException) throws SQLException
    {
        if (!AltibaseFailover.checkNeedSTF(aContext, aException))
        {
            throw aException;
        }

        boolean sSTFSuccess = AltibaseFailover.doSTF(aContext);
        if (sSTFSuccess)
        {
            Error.throwSQLExceptionForFailover(aException);
        }
        else 
        {
            throw aException;
        }           
    }

    /**
     * CTF를 위해 alternateservers에서 사용 가능한 서버를 선택해 접속한다.
     *
     * @param aContext Failover context
     * @return CTF 성공 여부
     */
    private static boolean doCTF(AltibaseFailoverContext aContext)
    {
        return doCTF(aContext, FailoverType.CTF);
    }

    /**
     * CTF 또는 STF를 위해 alternateservers에서 사용 가능한 서버를 선택해 접속한다.
     *
     * @param aContext Failover context
     * @param aFailoverType Failover 유형
     * @return CTF 성공 여부
     */
    private static boolean doCTF(AltibaseFailoverContext aContext, FailoverType aFailoverType)
    {
        final AltibaseProperties sProp = aContext.connectionProperties();
        final AltibaseFailoverServerInfo sOldServerInfo = aContext.getCurrentServer();
        final int sMaxConnectionRetry = sProp.getConnectionRetryCount();
        final int sConnectionRetryDelay = sProp.getConnectionRetryDelay() * 1000; // millisecond

        ArrayList<AltibaseFailoverServerInfo> sTryList = new ArrayList<AltibaseFailoverServerInfo>(aContext.getFailoverServerList());
        if (sTryList.get(0) != sOldServerInfo)
        {
            sTryList.remove(sOldServerInfo);
            sTryList.add(0, sOldServerInfo);
        }
        for (AltibaseFailoverServerInfo sNewServerInfo : sTryList)
        {
            for (int sRemainRetryCnt = Math.max(1, sMaxConnectionRetry); sRemainRetryCnt > 0; sRemainRetryCnt--)
            {
                /* BUG-43219 매 접속시마다 채널을 재생성해야 한다. 그렇지 않으면 첫번째 접속 정보가 뒤에오는 접속에 영향을 끼친다. */
                CmChannel sNewChannel = new CmChannel();
                try
                {
                    sNewChannel.open(sNewServerInfo.getServer(),
                                     sProp.getSockBindAddr(),
                                     sNewServerInfo.getPort(),                                         
                                     sProp.getLoginTimeout(),
                                     sProp.getResponseTimeout());
                    setFailoverSource(aContext, aFailoverType, sOldServerInfo);
                    aContext.setCurrentServer(sNewServerInfo);
                    AltibaseFailover.changeChannelAndConnect(aContext, sNewChannel, sNewServerInfo);
                    if (TraceFlag.TRACE_COMPILE && TraceFlag.TRACE_ENABLED)
                    {
                        mLogger.log(Level.INFO, "Success to connect to (" + sNewServerInfo + ")");
                    }
                    return true;
                }
                catch (Exception sEx)
                {
                    if (TraceFlag.TRACE_COMPILE && TraceFlag.TRACE_ENABLED)
                    {
                        mLogger.log(Level.INFO, "Failure to connect to (" + sNewServerInfo + ")", sEx);
                        mLogger.log(Level.INFO, "Sleep "+ sConnectionRetryDelay);
                    }
                    try
                    {
                        Thread.sleep(sConnectionRetryDelay);
                    }
                    catch (InterruptedException sExSleep)
                    {
                        // ignore
                    }
                }
            }
        }

        return false;
    }

    /**
     * <tt>CmChannel</tt>을 바꾸고, 그를 이용해 <tt>connect</tt>를 시도한다.
     *
     * @param aContext Failover contest
     * @param aChannel 바꿀 <tt>CmChannel</tt>
     * @param aServerInfo AltibaseFailoverServerInfo 정보
     * @throws SQLException 새 <tt>CmChannel</tt>을 이용하 <tt>connect</tt>에 실패한 경우
     */
    private static void changeChannelAndConnect(AltibaseFailoverContext aContext, CmChannel aChannel, AltibaseFailoverServerInfo aServerInfo) throws SQLException
    {
        AltibaseConnection sConn = aContext.getConnection();

        sConn.setChannel(aChannel);
        sConn.handshake();
        /* BUG-43219 set dbname property */
        AltibaseProperties sProps = aContext.connectionProperties();
        sProps.setDatabase(aServerInfo.getDatabase());
        sConn.connect(sProps);
    }

    /**
     * CTF와 Failover callback을 수행한다.
     * callback 결과에 따라 STF를 계속 진행할지 그만둘지를 결정한다.
     *
     * @param aContext Failover context
     * @return STF 성공 여부
     * @throws SQLException STF를 위해 <tt>Statement</tt>를 정리하다가 실패한 경우
     */
    private static boolean doSTF(AltibaseFailoverContext aContext) throws SQLException
    {

        int sFailoverIntention;
        boolean sFOSuccess;
        int sFailoverEvent;
        AltibaseFailoverCallback sFailoverCallback = aContext.getCallback();

        aContext.getConnection().clearStatements4STF();

        aContext.setCallbackState(CallbackState.IN_PROGRESS);
        sFailoverIntention = sFailoverCallback.failoverCallback(aContext.getConnection(), aContext.getAppContext(), AltibaseFailoverCallback.Event.BEGIN);
        aContext.setCallbackState(CallbackState.STOPPED);

        if (sFailoverIntention == AltibaseFailoverCallback.Result.QUIT)
        {
            sFOSuccess = false;
        }
        else
        {
            sFOSuccess = doCTF(aContext, FailoverType.STF);

            if (sFOSuccess == true)
            {
                aContext.setCallbackState(CallbackState.IN_PROGRESS);
                try
                {
                    AltibaseXAResource sXAResource = aContext.getRelatedXAResource();
                    if ((sXAResource != null) && (sXAResource.isOpen() == true))
                    {
                        sXAResource.xaOpen();
                    }
                    sFailoverEvent = AltibaseFailoverCallback.Event.COMPLETED;
                }
                catch (SQLException e)
                {
                    sFailoverEvent = AltibaseFailoverCallback.Event.ABORT;
                }
                sFailoverIntention = sFailoverCallback.failoverCallback(aContext.getConnection(), aContext.getAppContext(), sFailoverEvent);
                aContext.setCallbackState(CallbackState.STOPPED);

                if ((sFailoverEvent == AltibaseFailoverCallback.Event.ABORT) ||
                    (sFailoverIntention == AltibaseFailoverCallback.Result.QUIT))
                {
                    sFOSuccess = false;
                    aContext.getConnection().quiteClose();
                }
            }
            else
            {
                aContext.setCallbackState(CallbackState.IN_PROGRESS);
                sFailoverIntention = sFailoverCallback.failoverCallback(aContext.getConnection(), aContext.getAppContext(), AltibaseFailoverCallback.Event.ABORT);
                aContext.setCallbackState(CallbackState.STOPPED);
            }
        }
        return sFOSuccess;

    }

    /* BUG-31390 Failover info for v$session */
    /**
     * V$SESSION.FAILOVER_SOURCE로 보여줄 값을 생성해 Connection property로 설정한다.
     *
     * @param aContext Failover Context
     * @param aFailoverType Failover 유형
     * @param aServerInfo 설정할 서버 정보
     */
    private static void setFailoverSource(AltibaseFailoverContext aContext, FailoverType aFailoverType, AltibaseFailoverServerInfo aServerInfo)
    {
        StringBuilder sStrBdr = new StringBuilder();

        switch (aFailoverType)
        {
            case CTF:
                sStrBdr.append("CTF ");
                break;

            case STF:
                sStrBdr.append("STF ");
                break;

            default:
                // 내부에서만 쓰므로, 이런일은 절대 일어나선 안된다.
                Error.throwInternalError(ErrorDef.INTERNAL_ASSERTION);
                break;
        }

        sStrBdr.append(aServerInfo.getServer());
        sStrBdr.append(':');
        sStrBdr.append(aServerInfo.getPort());
        if (!StringUtils.isEmpty(aServerInfo.getDatabase()))
        {
            sStrBdr.append('/');
            sStrBdr.append(aServerInfo.getDatabase());
        }
        aContext.setFailoverSource(sStrBdr.toString());
    }

    /**
     * CTF가 필요한지 확인한다.
     *
     * @param aContext Failover에 대한 정보를 담은 객체
     * @param aException CTF가 필요한지 볼 예외
     * @return CTF가 필요하면 true, 아니면 false
     */
    private static boolean checkNeedCTF(AltibaseFailoverContext aContext, SQLException aException) throws SQLException
    {
        if ((aContext == null) ||
            (aContext.getFailoverServerList().size() <= 0) ||
            (isNeedToFailover(aException) == false))
        {
            return false;
        }
        return true;
    }

    /**
     * STF가 필요한지 확인한다.
     *
     * @param aContext Failover에 대한 정보를 담은 객체
     * @param aException STF가 필요한지 볼 예외
     * @return STF가 필요하면 true, 아니면 false
     */
    private static boolean checkNeedSTF(AltibaseFailoverContext aContext, SQLException aException) throws SQLException
    {
        if ((aContext == null) ||
            (aContext.useSessionFailover() == false) ||
            (aContext.getFailoverServerList().size() <= 0) ||
            (isNeedToFailover(aException) == false) ||
            (aContext.getCallbackState() != CallbackState.STOPPED))
        {
            return false;
        }
        return true;
    }

    /**
     * Failover가 필요한 예외인지 확인한다.
     *
     * @param aException Failover가 필요한지 볼 예외
     * @return Failover가 필요하면 true, 아니면 false
     */
    private static boolean isNeedToFailover(SQLException aException)
    {
        switch (aException.getErrorCode())
        {
            case ErrorDef.COMMUNICATION_ERROR:
            case ErrorDef.UNKNOWN_HOST:
            case ErrorDef.CLIENT_UNABLE_ESTABLISH_CONNECTION:
            case ErrorDef.CLOSED_CONNECTION:
            case ErrorDef.RESPONSE_TIMEOUT:
            case ErrorDef.ADMIN_MODE_ERROR:
                return true;
            default:
                return false;
        }
    }
}
