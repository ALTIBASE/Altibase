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

package Altibase.jdbc.driver.cm;

import java.io.IOException;
import java.io.InputStream;
import java.io.Reader;
import java.lang.reflect.Method;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.transaction.xa.Xid;

import Altibase.jdbc.driver.AltibaseXAResource;
import Altibase.jdbc.driver.BlobInputStream;
import Altibase.jdbc.driver.LobConst;
import Altibase.jdbc.driver.datatype.Column;
import Altibase.jdbc.driver.datatype.ColumnTypes;
import Altibase.jdbc.driver.datatype.ListBufferHandle;
import Altibase.jdbc.driver.datatype.RowHandle;
import Altibase.jdbc.driver.ex.Error;
import Altibase.jdbc.driver.ex.ErrorDef;
import Altibase.jdbc.driver.logging.LoggingProxy;
import Altibase.jdbc.driver.logging.TraceFlag;
import Altibase.jdbc.driver.util.AltiSqlProcessor;
import Altibase.jdbc.driver.util.AltibaseProperties;

public class CmProtocol
{
    private static long totaltime;
    private static Logger mLogger = null;
    
    static 
    {
        if (TraceFlag.TRACE_COMPILE && TraceFlag.TRACE_ENABLED)
        {
            mLogger = Logger.getLogger(LoggingProxy.JDBC_LOGGER_DEFAULT);
        }        
    }
    
    public static void handshake(CmProtocolContextConnect aContext) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeHandshake(aContext.channel());
            aContext.channel().sendAndReceive();
            CmOperation.readHandshake(aContext.channel(), aContext.getHandshakeResult());
        }
    }

    // Logical Connection
    public static void connect(CmProtocolContextConnect aContext, String aDBName, String aUser, String aPassword, short aConnectMode) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeConnectEx(aContext.channel(), aDBName, aUser, aPassword, aConnectMode);
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
            if (aContext.getError() == null || !Error.isException(aContext.getError()))
            {
                for (int i = 0; i < aContext.getPropertyCount(); i++)
                {
                    CmOperation.writeSetPropertyV2(aContext.channel(), aContext.getPropertyKey(i), aContext.getPropertyValue(i));
                }
                CmOperation.writeGetProperty(aContext.channel(), AltibaseProperties.PROP_CODE_NLS_CHARACTERSET);
                CmOperation.writeGetProperty(aContext.channel(), AltibaseProperties.PROP_CODE_NLS_NCHAR_CHARACTERSET);
                CmOperation.writeGetProperty(aContext.channel(), AltibaseProperties.PROP_CODE_EXPLAIN_PLAN);
                CmOperation.writeGetProperty(aContext.channel(), AltibaseProperties.PROP_CODE_AUTOCOMMIT);
                /* BUG-39817 */
                if (!aContext.isSetProperty(AltibaseProperties.PROP_CODE_ISOLATION_LEVEL))
                {
                    CmOperation.writeGetProperty(aContext.channel(), AltibaseProperties.PROP_CODE_ISOLATION_LEVEL);
                }
                aContext.channel().sendAndReceive();
                readProtocolResult(aContext);
            }
        }
    }

    public static void getProperty(CmProtocolContextConnect aContext, byte aPropCode) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeGetProperty(aContext.channel(), aPropCode);
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }
    
    public static void sendProperties(CmProtocolContextConnect aContext) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            for (int i=0; i<aContext.getPropertyCount(); i++)
            {
                CmOperation.writeSetPropertyV2(aContext.channel(), aContext.getPropertyKey(i), aContext.getPropertyValue(i));
            }
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }
    
    public static void disconnect(CmProtocolContextConnect aContext) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeDisconnect(aContext.channel());
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }
    
    public static void commit(CmProtocolContext aContext) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeCommit(aContext.channel());
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }
    
    public static void rollback(CmProtocolContext aContext) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeRollback(aContext.channel());
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }
    
    public static void directExecute(CmProtocolContextDirExec aContext, int aCID, String aSql, boolean aHoldable, 
                                     boolean aForKeySetDriven, boolean aNliteralReplace, boolean aClientSideAutoCommit, 
                                     boolean aShouldCloseCursor) throws SQLException
    {
        aContext.clearError();
        
        byte sHoldability = aHoldable ? CmOperation.PREPARE_MODE_HOLD_ON : CmOperation.PREPARE_MODE_HOLD_OFF;
        byte sForKeySetDriven = aForKeySetDriven ? CmOperation.PREPARE_MODE_KEYSET_ON : CmOperation.PREPARE_MODE_KEYSET_OFF;
        synchronized (aContext.channel())
        {
            aContext.getFetchResult().init();
            // PROJ-2427 성능을 위해 clearAllResults에서 closeCursor를 하지 않고 이곳에서 같이 보낸다.
            if (aShouldCloseCursor)
            {
                CmOperation.writeCloseCursor(aContext.channel(), aContext.getStatementId(), CmOperation.FREE_ALL_RESULTSET);                
            }
            CmOperation.writePrepare(aContext.channel(), aCID, aSql, CmOperation.PREPARE_MODE_EXEC_DIRECT, sHoldability, sForKeySetDriven, aNliteralReplace);
            CmOperation.writeGetColumn(aContext.channel(), aContext.getStatementId(), aContext.getResultSetId());
            // The zero value (0) can be used for statement id during direct Execute
            CmOperation.writeExecuteV2(aContext.channel(), aContext.getStatementId(), CmOperation.EXECUTION_ARRAY_INDEX_NONE, CmOperation.EXECUTION_MODE_NORMAL);
            if (!AltiSqlProcessor.isSelectQuery(aSql)) // BUG-38462 select 문장이 아닐때만 commit프로토콜을 바로 write한다.
            {
                CmOperation.writeClientCommit(aContext.channel(), aClientSideAutoCommit); // PROJ-2190 커밋프로토콜을 버퍼에 write한다.
            }
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }

    /**
     * ClientSideAutoCommit이 on인 경우에만 서버로 commit 메세지를 전송한다.
     * @param aContext CmProtocolContext 객체
     * @throws SQLException commit시 에러가 발생한 경우
     */
    public static void clientCommit(CmProtocolContext aContext, boolean aClientSideAutoCommit) throws SQLException
    {
        if (aClientSideAutoCommit)
        {
            commit(aContext);
        }
    }

    /*
     * BUG-39463 Add new fetch protocol that can request over 65535 rows.
     *
     * 1. writeFetch -> writeFetchV2로 변경
     * 2. aFetchCount의 type을 short -> int로 변경
     */
    public static void fetch(CmProtocolContextDirExec aContext, int aFetchCount, int aMaxRows, int aMaxFieldSize) throws SQLException
    {
        aContext.clearError();

        CmFetchResult sFetchResult = aContext.getFetchResult();
        sFetchResult.initFetchRequest(); // PROJ-2625
        sFetchResult.setMaxFieldSize(aMaxFieldSize);
        sFetchResult.setMaxRowCount(aMaxRows);

        synchronized (aContext.channel())
        {
            // To distinguish CallableStatement and PreparedStatement
            if (aContext.getGetColumnInfoResult().getColumns()==null && aContext.getPrepareResult().getResultSetCount() > 0)
            {
                CmOperation.writeGetColumn(aContext.channel(), aContext.getStatementId(), aContext.getResultSetId());                
                aContext.channel().sendAndReceive();
                readProtocolResult(aContext);
                // 이전 ResultSet의 ColInfo 를 가져옴
            }
            if (aContext.getError() == null)
            {
                CmOperation.writeFetchV2(aContext.channel(), aContext.getStatementId(), aContext.getResultSetId(), aFetchCount);
                aContext.channel().sendAndReceive();
                readProtocolResult(aContext);
            }
        }
    }

    public static void fetchNext(CmProtocolContextDirExec aContext, int aFetchCount) throws SQLException
    {
        aContext.clearError();

        CmFetchResult sFetchResult = aContext.getFetchResult();
        sFetchResult.initFetchRequest(); // PROJ-2625

        synchronized (aContext.channel())
        {
            CmOperation.writeFetchV2(aContext.channel(), aContext.getStatementId(), aContext.getResultSetId(), aFetchCount);
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }

    /**
     * 비동기적으로 fetch 프로토콜을 송신한다.
     * 
     * @param aContext Protocol context
     * @param aFetchCount fetch 를 요청할 row 수
     * @throws SQLException 송수신 과정에서 에러가 발생하였을 경우
     */
    public static void sendFetchNextAsync(CmProtocolContextDirExec aContext, int aFetchCount) throws SQLException
    {
        synchronized (aContext.channel())
        {
            CmOperation.writeFetchV2(aContext.channel(), aContext.getStatementId(), aContext.getResultSetId(), aFetchCount);
            aContext.channel().send();

            aContext.channel().setAsyncContext(aContext);
        }
    }

    /**
     * 비동기적으로 송신한 fetch 프로토콜을 수신한다.
     * 
     * @param aContext Protocol context
     * @throws SQLException 송수신 과정에서 에러가 발생하였을 경우
     */
    public static void receivefetchNextAsync(CmProtocolContextDirExec aContext) throws SQLException
    {
        aContext.clearError();

        CmFetchResult sFetchResult = aContext.getFetchResult();
        sFetchResult.initFetchRequest(); // PROJ-2625

        synchronized (aContext.channel())
        {
            aContext.channel().receive();
            readProtocolResultAsync(aContext.channel());
        }
    }

    public static void directExecuteBatch(CmProtocolContextDirExec aContext, int aCID, String[] aSql, boolean aNliteralReplace, boolean aClientSideAutoCommit)  throws SQLException
    {
        aContext.clearError();
        aContext.getExecutionResult().clearBatchUpdateCount();
        ((CmExecutionResult)aContext.getCmResult(CmExecutionResult.MY_OP)).setBatchMode(true);

        synchronized (aContext.channel())
        {
            aContext.getFetchResult().init();
            for (int i = 0; i < aSql.length; i++)
            {
                // batch execution에서는 select 구문이 없다고 가정하므로 PREPARE_MODE_HOLD_ON으로 줄 필요가 없다.
                CmOperation.writePrepare(aContext.channel(), aCID, aSql[i], CmOperation.PREPARE_MODE_EXEC_DIRECT, CmOperation.PREPARE_MODE_HOLD_OFF, CmOperation.PREPARE_MODE_KEYSET_OFF, aNliteralReplace);
                CmOperation.writeExecuteV2(aContext.channel(), aContext.getStatementId(), i + 1, CmOperation.EXECUTION_MODE_NORMAL);

                // select 구문이 있어도 다음 update 구문을 제대로 수행하기 위해 cursor를 닫는다.
                // 이렇게 하는 이유는, '프로토콜을 한번에 보내는 방식'으로는 SELECT가 있는걸 알았을 때 바로 멈출 수 없기 때문이다.
                // SELECT를 발견했을 때 에러를 내고 바로 멈추려면 하나씩 수행하거나(old-JDBC 방식), executeBatch를 위한 새로운 프로토콜을 추가해야한다.
                CmOperation.writeCloseCursor(aContext.channel(), aContext.getStatementId(), CmOperation.FREE_ALL_RESULTSET);
            }
            CmOperation.writeClientCommit(aContext.channel(), aClientSideAutoCommit); // PROJ-2190 clientAutoCommit이 활성화 되어 있는 경우 버퍼에 write한다.
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }

        ((CmExecutionResult)aContext.getCmResult(CmExecutionResult.MY_OP)).setBatchMode(false);
    }
    
    public static void directExecuteAndFetch(CmProtocolContextDirExec aContext, int aCID, String aSql,
                                             int aFetchCount, int aMaxRows, int aMaxFieldSize, boolean aHoldable,
                                             boolean aForKeySetDriven, boolean aNliteralReplace,
                                             boolean aShouldCloseCursor) throws SQLException
    {
        aContext.clearError();

        CmFetchResult sFetchResult = aContext.getFetchResult();
        sFetchResult.initFetchRequest(); // PROJ-2625
        sFetchResult.setMaxFieldSize(aMaxFieldSize);
        sFetchResult.setMaxRowCount(aMaxRows);

        byte sHoldability = aHoldable ? CmOperation.PREPARE_MODE_HOLD_ON : CmOperation.PREPARE_MODE_HOLD_OFF;
        byte sForKeySetDriven = aForKeySetDriven ? CmOperation.PREPARE_MODE_KEYSET_ON : CmOperation.PREPARE_MODE_KEYSET_OFF;

        synchronized (aContext.channel())
        {
            aContext.getFetchResult().init();
            // PROJ-2427 성능을 위해 clearAllResults에서 closeCursor를 하지 않고 이곳에서 같이 보낸다.
            if (aShouldCloseCursor)
            {
                CmOperation.writeCloseCursor(aContext.channel(), aContext.getStatementId(), CmOperation.FREE_ALL_RESULTSET);                
            }
            CmOperation.writePrepare(aContext.channel(), aCID, aSql, CmOperation.PREPARE_MODE_EXEC_DIRECT, sHoldability, sForKeySetDriven, aNliteralReplace);
            CmOperation.writeGetColumn(aContext.channel(), aContext.getStatementId(), aContext.getResultSetId());
            CmOperation.writeExecuteV2(aContext.channel(), aContext.getStatementId(), CmOperation.EXECUTION_ARRAY_INDEX_NONE, CmOperation.EXECUTION_MODE_NORMAL);
            CmOperation.writeFetchV2(aContext.channel(), aContext.getStatementId(), aContext.getResultSetId(), aFetchCount);
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }
    
    public static void prepare(CmProtocolContextDirExec aContext, int aCID, String aSql, boolean aHoldable, boolean aForKeySetDriven, 
                               boolean aNliteralReplace, boolean aIsDeferred) throws SQLException
    {
        aContext.clearError();
        
        byte sHoldability = aHoldable ? CmOperation.PREPARE_MODE_HOLD_ON : CmOperation.PREPARE_MODE_HOLD_OFF;
        byte sForKeySetDriven = aForKeySetDriven ? CmOperation.PREPARE_MODE_KEYSET_ON : CmOperation.PREPARE_MODE_KEYSET_OFF;
        long beforetime = System.currentTimeMillis();
        synchronized (aContext.channel())
        {
            CmOperation.writePrepare(aContext.channel(), aCID, aSql, CmOperation.PREPARE_MODE_EXEC_PREPARE, sHoldability, sForKeySetDriven, aNliteralReplace);
            CmOperation.writeGetColumn(aContext.channel(), aContext.getStatementId(), aContext.getResultSetId());
            CmOperation.writeGetBindParamInfo(aContext.channel(), aContext.getStatementId());
            // BUG-42424 deferred일 경우에는 prepare요청을 보내지 않고 로그만 남긴 후 메소드를 그냥 빠져나간다.
            if (aIsDeferred) 
            {
                if (TraceFlag.TRACE_COMPILE && TraceFlag.TRACE_ENABLED)
                {
                    mLogger.log(Level.INFO, "Defer prepare request for sql :{0}", aSql);
                }
            }
            else
            {
                aContext.channel().sendAndReceive();
                readProtocolResult(aContext);
            }
        }
        long aftertime = System.currentTimeMillis();
        
        totaltime = totaltime + (aftertime - beforetime);
    }

    public static void preparedExecute(CmProtocolContextPrepExec aContext, List<Column> aParams, boolean aClientSideAutoCommit,
                                       boolean aShouldCloseCursor, List aDeferredRequests) throws SQLException
    {
        aContext.clearError();
        aContext.getBindParamDataOutResult().setBindParams(aParams);
        
        synchronized (aContext.channel())
        {
            if (aDeferredRequests.size() > 0)
            {
                invokeDeferredRequests(aDeferredRequests);
            }
            aContext.getFetchResult().init();
            // PROJ-2427 성능을 위해 clearAllResults에서 closeCursor를 하지 않고 이곳에서 같이 보낸다.
            if (aShouldCloseCursor)
            {
                CmOperation.writeCloseCursor(aContext.channel(), aContext.getStatementId(), CmOperation.FREE_ALL_RESULTSET);                
            }
            CmOperation.writeSetBindParamInfoList(aContext.channel(), aContext.getStatementId(), aParams);
            CmOperation.writeBindParamDataIn(aContext.channel(), aContext.getStatementId(), aParams);
            CmOperation.writeExecuteV2(aContext.channel(), aContext.getStatementId(),
                                     CmOperation.EXECUTION_ARRAY_INDEX_NONE, CmOperation.EXECUTION_MODE_NORMAL);
            
            // To distinguish CallableStatement and PreparedStatement
            if (aContext.getGetColumnInfoResult().getColumns()==null && hasResultSet(aContext))
            {
                CmOperation.writeGetColumn(aContext.channel(), aContext.getStatementId(), aContext.getResultSetId());
            }
            // PROJ-2190, BUG-38462 lob파라메터와 resultset이 없을 때만 commit프로토콜을 write한다.  
            if (aClientSideAutoCommit)
            {
                if (!lobColumnExists(aParams) && !hasResultSet(aContext)) 
                {
                    CmOperation.writeCommit(aContext.channel());
                }
            }
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }

    private static boolean hasResultSet(CmProtocolContextPrepExec aContext)
    {
        return aContext.getPrepareResult().getResultSetCount() > 0;
    }
    
    /**
     * lob 컬럼이 존재하는지 여부를 반환한다.
     * @param aColumns 컬럼 리스트
     * @return lob컬럼이 존재하면 true 아니면 false
     */
    private static boolean lobColumnExists(List<Column> aColumns)
    {
        if (aColumns == null || aColumns.size() == 0)
        {
            return false;
        }
        for (Column sColumn : aColumns)
        {
            if (sColumn.getDBColumnType() == ColumnTypes.BLOB_LOCATOR ||
                sColumn.getDBColumnType() == ColumnTypes.CLOB_LOCATOR)
            {
                return true;
            }
        }
        
        return false;
    }

    public static void preparedExecuteBatch(CmProtocolContextPrepExec aContext, List<Column> aParams,
                                            RowHandle aRowHandle, int aRowCount, List aDeferredRequests) throws SQLException
    {
        aContext.clearError();
        aContext.getBindParamDataOutResult().setBindParams(aParams);
        aContext.getBindParamDataOutResult().setRowHandle(aRowHandle);
        aContext.getExecutionResult().clearBatchUpdateCount();
        ((CmExecutionResult)aContext.getCmResult(CmExecutionResult.MY_OP)).setBatchMode(true);
        
        synchronized (aContext.channel())
        {
            if (aDeferredRequests.size() > 0)
            {
                invokeDeferredRequests(aDeferredRequests);
            }
            aContext.getFetchResult().init();
            CmOperation.writeSetBindParamInfoList(aContext.channel(), aContext.getStatementId(), aParams);
            CmOperation.writeExecuteV2(aContext.channel(), aContext.getStatementId(),
                                     CmOperation.EXECUTION_ARRAY_INDEX_NONE,
                                     CmOperation.EXECUTION_MODE_BEGIN_ARRAY);
            for (int i = 0; i < aRowCount; i++)
            {
                aRowHandle.next();
                CmOperation.writeBindParamDataIn(aContext.channel(), aContext.getStatementId(), aParams);
                CmOperation.writeExecuteV2(aContext.channel(), aContext.getStatementId(), i + 1,
                                         CmOperation.EXECUTION_MODE_ARRAY);
            }
            CmOperation.writeExecuteV2(aContext.channel(), aContext.getStatementId(),
                                     CmOperation.EXECUTION_ARRAY_INDEX_NONE, CmOperation.EXECUTION_MODE_END_ARRAY);
            aContext.channel().sendAndReceive();
            aRowHandle.initToStore();
            aRowHandle.beforeFirst();
            readProtocolResult(aContext);
        }
        
        ((CmExecutionResult)aContext.getCmResult(CmExecutionResult.MY_OP)).setBatchMode(false);
    }
    
    // List Protocol 로 Server 에 Data 를 전송한다.
    public static void preparedExecuteBatchUsingList(CmProtocolContextPrepExec aContext, List<Column> aParams,
                                                     ListBufferHandle aBufferHandle, int aRowCount,
                                                     boolean aIsAtomic,
                                                     List aDeferredRequests) throws SQLException
    {
        aContext.clearError();
        aContext.getExecutionResult().clearBatchUpdateCount();
        ((CmExecutionResult)aContext.getCmResult(CmExecutionResult.MY_OP)).setBatchMode(true);
        
        synchronized (aContext.channel())
        {
            if (aDeferredRequests.size() > 0)
            {
                invokeDeferredRequests(aDeferredRequests);
            }
            CmOperation.writeExecuteV2(aContext.channel(),
                                     aContext.getStatementId(),
                                     CmOperation.EXECUTION_ARRAY_INDEX_NONE,
                                     (aIsAtomic)? CmOperation.EXECUTION_MODE_BEGIN_ATOMIC : CmOperation.EXECUTION_MODE_BEGIN_ARRAY);

            CmOperation.writeSetBindParamInfoList(aContext.channel(), aContext.getStatementId(), aParams);
            CmOperation.writeBindParamDataInListV2(aContext.channel(), aContext.getStatementId(), aBufferHandle, aIsAtomic);            
            CmOperation.writeExecuteV2(aContext.channel(),
                                     aContext.getStatementId(),
                                     CmOperation.EXECUTION_ARRAY_INDEX_NONE,
                                     (aIsAtomic)? CmOperation.EXECUTION_MODE_END_ATOMIC : CmOperation.EXECUTION_MODE_END_ARRAY);

            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }

        ((CmExecutionResult)aContext.getCmResult(CmExecutionResult.MY_OP)).setBatchMode(false);
    }
    
    public static void preparedExecuteAndFetch(CmProtocolContextPrepExec aContext, List<Column> aParams, int aFetchCount,
                                               int aMaxRows, int aMaxFieldSize, boolean aShouldCloseCursor,
                                               List aDeferredRequests) throws SQLException
    {
        aContext.clearError();

        aContext.getBindParamDataOutResult().setBindParams(aParams);
        aContext.getFetchResult().initFetchRequest(); // PROJ-2625
        aContext.getFetchResult().setMaxFieldSize(aMaxFieldSize);
        aContext.getFetchResult().setMaxRowCount(aMaxRows);

        synchronized (aContext.channel())
        {
            if (aDeferredRequests.size() > 0)
            {
                invokeDeferredRequests(aDeferredRequests);
            }
            aContext.getFetchResult().init();
            // PROJ-2427 성능을 위해 clearAllResults에서 closeCursor를 하지 않고 이곳에서 같이 보낸다.
            if (aShouldCloseCursor)
            {
                CmOperation.writeCloseCursor(aContext.channel(), aContext.getStatementId(), CmOperation.FREE_ALL_RESULTSET);                
            }
            CmOperation.writeSetBindParamInfoList(aContext.channel(), aContext.getStatementId(), aParams);
            CmOperation.writeBindParamDataIn(aContext.channel(), aContext.getStatementId(), aParams);            
            CmOperation.writeExecuteV2(aContext.channel(), aContext.getStatementId(), CmOperation.EXECUTION_ARRAY_INDEX_NONE, CmOperation.EXECUTION_MODE_NORMAL);
            if (aContext.getGetColumnInfoResult().getColumns()==null && aContext.getPrepareResult().getResultSetCount() > 0)
            {
                CmOperation.writeGetColumn(aContext.channel(), aContext.getStatementId(), aContext.getResultSetId());
            }
            CmOperation.writeFetchV2(aContext.channel(), aContext.getStatementId(), aContext.getResultSetId(), aFetchCount);
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }

    public static void closeCursor(CmProtocolContextDirExec aContext, int aStmtID, short aResultSetID, boolean aClientSideAutoCommit) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeCloseCursor(aContext.channel(), aStmtID, aResultSetID);
            CmOperation.writeClientCommit(aContext.channel(), aClientSideAutoCommit); // PROJ-2190 cursor를 close한 후 commit 프로토콜을 write한다. 
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }

    public static void freeStatement(CmProtocolContextDirExec aContext, int aStmtID) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeDropStatement(aContext.channel(), aStmtID);
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }

    public static void cancelStatement(CmProtocolContextDirExec aContext, int aCID) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeCancelStatement(aContext.channel(), aCID);
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }
    
    public static void getLobByteLength(CmProtocolContextLob aContext) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeLobGetSize(aContext.channel(), aContext.locatorId());
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }
    
    public static void getBlob(CmProtocolContextLob aContext, long aServerOffset, long aLength) throws SQLException
    {
        int sLength = (int)Math.min(aLength, LobConst.LOB_BUFFER_SIZE);
        aContext.clearError();
        synchronized (aContext.channel())
        {
            while(aLength > 0)
            {
                CmOperation.writeLobGet(aContext.channel(), aContext.locatorId(), aServerOffset, sLength);
                aContext.channel().sendAndReceive();
                readProtocolResult(aContext);
                aContext.setDstOffset(aContext.getDstOffset() + sLength);
                aServerOffset += sLength;
                aLength -= sLength;
                sLength = (int)Math.min(aLength, LobConst.LOB_BUFFER_SIZE);
            }
        }
    }
    
    public static void putBlob(CmProtocolContextLob aContext, long aServerOffset, byte[] aSource, int aSourceOffset, int aSourceLength) throws SQLException, IOException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            try 
            {
                CmOperation.writeLobPutBegin(aContext.channel(), aContext.locatorId(), aServerOffset);
                aContext.channel().sendAndReceive();
                readProtocolResult(aContext);
                CmOperation.writeLobPut(aContext.channel(), aContext.locatorId(), aSource, aSourceOffset, aSourceLength);
                CmOperation.writeLobPutEnd(aContext.channel(), aContext.locatorId());
                aContext.channel().sendAndReceive();
                readProtocolResult(aContext);
            }
            catch(ArrayIndexOutOfBoundsException e)
            {
                aContext.channel().sendAndReceive();
                readProtocolResult(aContext);
                throw e; 
            }
        }
    }
    
    public static void putBlob(CmProtocolContextLob aContext, long aServerOffset, InputStream aSource, long aSourceLength) throws SQLException, IOException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            try 
            {
                CmOperation.writeLobPutBegin(aContext.channel(), aContext.locatorId(), aServerOffset);
                aContext.channel().sendAndReceive();
                readProtocolResult(aContext);
                
                byte[] sTmpBuf = aContext.channel().getLobBuffer();
                while (aSourceLength > 0)
                {
                    int sLengthPerOp = (int)Math.min(LobConst.LOB_BUFFER_SIZE, aSourceLength);
                    sLengthPerOp = aSource.read(sTmpBuf, 0, sLengthPerOp);
                    if (sLengthPerOp == -1)
                    {
                        // BUGBUG (2013-08-30) 지정한 길이보다 Stream이 짧다면 에러인가?
                        break;
                    }
                    CmOperation.writeLobPut(aContext.channel(), aContext.locatorId(), sTmpBuf, 0, sLengthPerOp);
                    aSourceLength -= sLengthPerOp;

                    if (aContext.isCopyMode())
                    {
                        ((BlobInputStream)aSource).readyToCopy();
                    }
                }
                CmOperation.writeLobPutEnd(aContext.channel(), aContext.locatorId());
                aContext.channel().sendAndReceive();
                readProtocolResult(aContext);
            }
            catch(ArrayIndexOutOfBoundsException e)
            {
                aContext.channel().sendAndReceive();
                readProtocolResult(aContext);
                throw e; 
            }
        }
    }

    public static void truncate(CmProtocolContextLob aContext, int aLength) throws SQLException
    {
        aContext.clearError();
        getLobByteLength(aContext);
        
        synchronized (aContext.channel())
        {
            CmOperation.writeLobTruncate(aContext.channel(), aContext.locatorId(), aLength);
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }
    
    public static void free(CmProtocolContextLob aContext) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeLobFree(aContext.channel(), aContext.locatorId());
        }
    }
    
    public static void getCharLength(CmProtocolContextLob aContext) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeLobCharLength(aContext.channel(), aContext.locatorId());
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }
    
    public static void getBytePos(CmProtocolContextLob aContext, int aCharLength) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeLobBytePos(aContext.channel(), aContext.locatorId(), aCharLength);
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }
    
    public static void getClobBytePos(CmProtocolContextLob aContext, long aByteOffset, long aCharLength) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmClobGetResult sResult = (CmClobGetResult)aContext.getCmResult(CmClobGetResult.MY_OP);
            sResult.init(aContext.locatorId(), aByteOffset, aCharLength);
            CmOperation.writeLobGetBytePosCharLen(aContext.channel(), aContext.locatorId(), aByteOffset, aCharLength);
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
            if (sResult.getCharLengthToGet() < sResult.getCharLength())
            {
                Error.throwSQLException(ErrorDef.INTERNAL_ASSERTION);
            }
        }
    }
    
    public static void getClobCharPos(CmProtocolContextLob aContext, long aCharOffset, long aCharLength) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeLobGetCharPosCharLen(aContext.channel(), aContext.locatorId(), aCharOffset, aCharLength);
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }

    /**
     * CLob 데이타를 서버로 보낸다.
     * 
     * @param aContext 프로토콜 컨텍스트
     * @param aServerOffset 데이타를 반영할 서버 데이타의 기준 위치(byte 단위)
     * @param aSrc 보낼 데이타
     * @return 보낸 데이타 사이즈(byte 단위)
     * @throws SQLException 프로토콜을 쓰는데 실패했을 경우
     * @throws IOException 데이타를 읽는데 실패했을 경우
     */
    public static long putClob(CmProtocolContextLob aContext, long aServerOffset, char[] aSrc) throws SQLException, IOException
    {
        return putClob(aContext, aServerOffset, aSrc, 0, aSrc.length);
    }

    /**
     * CLob 데이타를 서버로 보낸다.
     * 
     * @param aContext 프로토콜 컨텍스트
     * @param aServerOffset 데이타를 반영할 서버 데이타의 기준 위치(byte 단위)
     * @param aSrc 보낼 데이타
     * @param aSrcOffset 보내기 시작할 위치
     * @param aSrcLength 보낼 길이
     * @return 보낸 데이타 사이즈(byte 단위)
     * @throws SQLException 프로토콜을 쓰는데 실패했을 경우
     * @throws IOException 데이타를 읽는데 실패했을 경우
     */
    public static long putClob(CmProtocolContextLob aContext, long aServerOffset, char[] aSrc, int aSrcOffset, int aSrcLength) throws SQLException, IOException
    {
        long sWrited;
        aContext.clearError();
        synchronized (aContext.channel())
        {
            try 
            {
                CmOperation.writeLobPutBegin(aContext.channel(), aContext.locatorId(), aServerOffset);
                aContext.channel().sendAndReceive();
                readProtocolResult(aContext);

                ReadableCharChannel sChannelFromSource = aContext.channel().getReadChannel4Clob(aSrc, aSrcOffset, aSrcLength);
                sWrited = CmOperation.writeLobPut(aContext, sChannelFromSource);
                sChannelFromSource.close();

                CmOperation.writeLobPutEnd(aContext.channel(), aContext.locatorId());
                aContext.channel().sendAndReceive();
                readProtocolResult(aContext);
            }
            catch(ArrayIndexOutOfBoundsException e)
            {
                aContext.channel().sendAndReceive();
                readProtocolResult(aContext);
                throw e; 
            }
        }
        return sWrited;
    }

    /**
     * CLob 데이타를 서버로 보낸다.
     * 
     * @param aContext 프로토콜 컨텍스트
     * @param aServerOffset 데이타를 반영할 서버 데이타의 기준 위치(byte 단위)
     * @param aSource 보낼 데이타
     * @param aSourceLength 보낼 데이타 길이
     * @return 보낸 데이타 사이즈(byte 단위)
     * @throws SQLException 프로토콜을 쓰는데 실패했을 경우
     * @throws IOException 데이타를 읽는데 실패했을 경우
     */
    public static long putClob(CmProtocolContextLob aContext, long aServerOffset, Reader aSource, int aSourceLength) throws SQLException, IOException
    {
        long sWrited;
        ReadableCharChannel sChannelFromSource;
        aContext.clearError();
        synchronized (aContext.channel())
        {
            try 
            {
                CmOperation.writeLobPutBegin(aContext.channel(), aContext.locatorId(), aServerOffset);
                aContext.channel().sendAndReceive();
                readProtocolResult(aContext);

                if (aSourceLength > 0)
                {
                    sChannelFromSource = aContext.channel().getReadChannel4Clob(aSource, aSourceLength);
                }
                else
                {
                    sChannelFromSource = aContext.channel().getReadChannel4Clob(aSource);
                }
                sWrited = CmOperation.writeLobPut(aContext, sChannelFromSource);
                sChannelFromSource.close();
                
                CmOperation.writeLobPutEnd(aContext.channel(), aContext.locatorId());
                aContext.channel().sendAndReceive();
                readProtocolResult(aContext);
            }
            catch(ArrayIndexOutOfBoundsException e)
            {
                aContext.channel().sendAndReceive();
                readProtocolResult(aContext);
                throw e; 
            }
        }
        return sWrited;
    }
    
    public static void xaOpen(CmProtocolContextXA aContext) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeXaOperation(aContext.channel(), CmOperation.XA_OP_OPEN, aContext.getResourceManagerId(), AltibaseXAResource.TMNOFLAGS, 0L);
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }
    
    public static void xaClose(CmProtocolContextXA aContext) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeXaOperation(aContext.channel(), CmOperation.XA_OP_CLOSE, aContext.getResourceManagerId(), AltibaseXAResource.TMMULTIPLE, 0L);
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }
    
    public static void xaPrepare(CmProtocolContextXA aContext, Xid aXid) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeXaTransaction(aContext.channel(), CmOperation.XA_OP_PREPARE, aContext.getResourceManagerId(), AltibaseXAResource.TMNOFLAGS, 0L, aXid.getFormatId(), aXid.getGlobalTransactionId(), aXid.getBranchQualifier());
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }

    public static void xaCommit(CmProtocolContextXA aContext, Xid aXid, long aFlag) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeXaTransaction(aContext.channel(), CmOperation.XA_OP_COMMIT, aContext.getResourceManagerId(), aFlag, 0L, aXid.getFormatId(), aXid.getGlobalTransactionId(), aXid.getBranchQualifier());
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }
    
    public static void xaRollback(CmProtocolContextXA aContext, Xid aXid) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeXaTransaction(aContext.channel(), CmOperation.XA_OP_ROLLBACK, aContext.getResourceManagerId(), AltibaseXAResource.TMNOFLAGS, 0L, aXid.getFormatId(), aXid.getGlobalTransactionId(), aXid.getBranchQualifier());
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }

    public static void xaStart(CmProtocolContextXA aContext, Xid aXid, long aFlag) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeXaTransaction(aContext.channel(), CmOperation.XA_OP_START, aContext.getResourceManagerId(), aFlag, 0L, aXid.getFormatId(), aXid.getGlobalTransactionId(), aXid.getBranchQualifier());
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }

    public static void xaEnd(CmProtocolContextXA aContext, Xid aXid, long aFlag) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeXaTransaction(aContext.channel(), CmOperation.XA_OP_END, aContext.getResourceManagerId(), aFlag, 0L, aXid.getFormatId(), aXid.getGlobalTransactionId(), aXid.getBranchQualifier());
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }

    public static void xaForget(CmProtocolContextXA aContext, Xid aXid) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            CmOperation.writeXaTransaction(aContext.channel(), CmOperation.XA_OP_FORGET, aContext.getResourceManagerId(), AltibaseXAResource.TMNOFLAGS, 0L, aXid.getFormatId(), aXid.getGlobalTransactionId(), aXid.getBranchQualifier());
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }

    public static void xaRecover(CmProtocolContextXA aContext, int aFlag) throws SQLException
    {
        aContext.clearError();
        aContext.getXidResult().clearXids();
        
        synchronized (aContext.channel())
        {
            CmOperation.writeXaOperation(aContext.channel(), CmOperation.XA_OP_RECOVER, aContext.getResourceManagerId(), aFlag, 1024L);
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }

    /**
     * Statement ID에 해당하는 Plan text를 얻는다.
     *
     * @param aContext Protocol context
     * @param aStmtID Plan text를 얻을 Statement의 ID
     * @param aDeferredRequests deferred된 prepare요청
     * @throws SQLException 요청을 보내지 못했거나, 또는 Plan text를 얻을 수 없는 경우
     */
    public static void getPlan(CmProtocolContext aContext, int aStmtID, List aDeferredRequests) throws SQLException
    {
        aContext.clearError();
        synchronized (aContext.channel())
        {
            if (aDeferredRequests.size() > 0)
            {
                invokeDeferredRequests(aDeferredRequests);
            }
            CmOperation.writeGetPlan(aContext.channel(), aStmtID);
            aContext.channel().sendAndReceive();
            readProtocolResult(aContext);
        }
    }

    /**
     * 프로토콜을 해석한다. 만약, 비동기적으로 요청한 프로토콜이 있다면 먼저 해석한다.
     * 
     * @param aContext Protocol context
     * @throws SQLException 수신 도중 에러가 발생하였을 경우
     */
    private static void readProtocolResult(CmProtocolContext aContext) throws SQLException
    {
        if (aContext.channel().isAsyncSent())
        {
            readProtocolResultAsync(aContext.channel());
            aContext.channel().receive();
        }

        CmOperation.readProtocolResult(aContext);
    }

    /**
     * 비동기적으로 송신한 프로토콜을 수신한다.
     * 
     * @param aChannel 수신하고자 하는 communication channel
     * @throws SQLException 수신 도중 에러가 발생하였을 경우
     */
    private static void readProtocolResultAsync(CmChannel aChannel) throws SQLException
    {
        CmProtocolContext sAsyncContext = aChannel.getAsyncContext();
        if (sAsyncContext != null)
        {
            CmOperation.readProtocolResult(sAsyncContext);
            aChannel.setAsyncContext(null);
        }
    }

    // BUG-42712 deferred 된 동작들을 CmBuffer에 write한다.
    public static void invokeDeferredRequests(List aDeferredRequests)
    {
        for (Iterator sItr = aDeferredRequests.iterator(); sItr.hasNext();)
        {
            HashMap sMethodInfo = (HashMap)sItr.next();
            String sMethodName = (String)sMethodInfo.get("methodname");
            Object[] sArgs = (Object[])sMethodInfo.get("args");
            try
            {
                Method sMethod = getMethod(Class.forName("Altibase.jdbc.driver.cm.CmProtocol"), sMethodName);
                if (sMethod != null)
                {
                    sMethod.invoke(null, sArgs); // BUG-42712 CmProtocol이 static 클래스이기때문에 첫번째 매개변수를 null로 준다.
                }
            }
            catch (Exception sEx)
            {
                Error.throwInternalError(ErrorDef.INTERNAL_ASSERTION, sEx);
            }
        }
        aDeferredRequests.clear();
    }

    // BUG-42712 reflection을 이용해 해당하는 name의 Method객체를 반환한다.
    private static Method getMethod(Class aClass, String sMethodName)
    {
        Method[] sMethods = aClass.getDeclaredMethods();
        Method sMethod = null;
        for (int i = 0; i < sMethods.length; i++)
        {
            if (sMethods[i].getName().equals(sMethodName))
            {
                sMethod = sMethods[i];
                break;
            }
        }
        return sMethod;
    }
}
