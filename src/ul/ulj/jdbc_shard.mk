#  Java JDBC specific source/tests

JDBC_SHARD_SRC =      $(DRIVER_DIR)/cm/CmProtocolContextShardConnect.java                           \
                      $(DRIVER_DIR)/cm/CmProtocolContextShardStmt.java                              \
                      $(DRIVER_DIR)/cm/CmShardAnalyzeResult.java                                    \
                      $(DRIVER_DIR)/cm/CmShardHandshakeResult.java                                  \
                      $(DRIVER_DIR)/cm/CmShardTransactionResult.java                                \
                      $(DRIVER_DIR)/cm/CmShardOperation.java                                        \
                      $(DRIVER_DIR)/cm/CmShardProtocol.java                                         \
                      $(DRIVER_DIR)/ex/ShardFailOverSuccessException.java                           \
                      $(DRIVER_DIR)/ex/ShardJdbcException.java                                      \
                      $(DRIVER_DIR)/ex/ShardFailoverIsNotAvailableException.java                    \
                      $(DRIVER_DIR)/ex/ShardError.java                                              \
                      $(DRIVER_DIR)/sharding/core/AbstractShardingCallableStatement.java            \
                      $(DRIVER_DIR)/sharding/core/AbstractShardingPreparedStatement.java            \
                      $(DRIVER_DIR)/sharding/core/AltibaseShardingConnection.java                   \
                      $(DRIVER_DIR)/sharding/core/AltibaseShardingPreparedStatement.java            \
                      $(DRIVER_DIR)/sharding/core/AltibaseShardingStatement.java                    \
                      $(DRIVER_DIR)/sharding/core/AltibaseShardingCallableStatement.java            \
                      $(DRIVER_DIR)/sharding/core/AltibaseShardingDataSource.java                   \
                      $(DRIVER_DIR)/sharding/core/AltibaseShardingResultSet.java                    \
                      $(DRIVER_DIR)/sharding/core/AltibaseShardingConnectionPoolDataSource.java     \
                      $(DRIVER_DIR)/sharding/core/AltibaseShardingLogicalConnection.java            \
                      $(DRIVER_DIR)/sharding/core/AltibaseShardingPooledConnection.java             \
                      $(DRIVER_DIR)/sharding/core/AltibaseShardingFailover.java                     \
                      $(DRIVER_DIR)/sharding/core/DataNode.java                                     \
                      $(DRIVER_DIR)/sharding/core/DataNodeShardingCallableStatement.java            \
                      $(DRIVER_DIR)/sharding/core/DataNodeShardingPreparedStatement.java            \
                      $(DRIVER_DIR)/sharding/core/DataNodeShardingStatement.java                    \
                      $(DRIVER_DIR)/sharding/core/DataNodeFailoverDestination.java                  \
                      $(DRIVER_DIR)/sharding/core/JdbcMethodInvocation.java                         \
                      $(DRIVER_DIR)/sharding/core/JdbcMethodInvoker.java                            \
                      $(DRIVER_DIR)/sharding/core/ShardKeyDataType.java                             \
                      $(DRIVER_DIR)/sharding/core/ShardNodeConfig.java                              \
                      $(DRIVER_DIR)/sharding/core/ShardRange.java                                   \
                      $(DRIVER_DIR)/sharding/core/ShardRangeList.java                               \
                      $(DRIVER_DIR)/sharding/core/ShardSplitMethod.java                             \
                      $(DRIVER_DIR)/sharding/core/GlobalTransactionLevel.java                       \
                      $(DRIVER_DIR)/sharding/core/ShardValueInfo.java                               \
                      $(DRIVER_DIR)/sharding/core/ShardValueInfoPrimaryStrategy.java                \
                      $(DRIVER_DIR)/sharding/core/ShardValueInfoStrategy.java                       \
                      $(DRIVER_DIR)/sharding/core/ShardValueInfoSubKeyStrategy.java                 \
                      $(DRIVER_DIR)/sharding/core/InternalShardingStatement.java                    \
                      $(DRIVER_DIR)/sharding/core/InternalShardingPreparedStatement.java            \
                      $(DRIVER_DIR)/sharding/core/InternalShardingCallableStatement.java            \
                      $(DRIVER_DIR)/sharding/core/ServerSideShardingStatement.java                  \
                      $(DRIVER_DIR)/sharding/core/ServerSideShardingPreparedStatement.java          \
                      $(DRIVER_DIR)/sharding/core/ServerSideShardingCallableStatement.java          \
                      $(DRIVER_DIR)/sharding/core/FailoverAlignInfo.java                            \
                      $(DRIVER_DIR)/sharding/core/NodeConnectionReport.java                         \
                      $(DRIVER_DIR)/sharding/core/ShardConnType.java                                \
                      $(DRIVER_DIR)/sharding/core/ShardValueType.java                               \
                      $(DRIVER_DIR)/sharding/core/ShardVersion.java                                 \
                      $(DRIVER_DIR)/sharding/core/DistTxInfo.java                                   \
                      $(DRIVER_DIR)/sharding/core/Shard2PhaseCommitState.java                       \
                      $(DRIVER_DIR)/sharding/algorithm/CompositeShardingAlgorithm.java              \
                      $(DRIVER_DIR)/sharding/algorithm/HashGenerator.java                           \
                      $(DRIVER_DIR)/sharding/algorithm/ListShardingAlgorithm.java                   \
                      $(DRIVER_DIR)/sharding/algorithm/RangeShardingAlgorithm.java                  \
                      $(DRIVER_DIR)/sharding/algorithm/ShardingAlgorithm.java                       \
                      $(DRIVER_DIR)/sharding/algorithm/StandardCompositeShardingAlgorithm.java      \
                      $(DRIVER_DIR)/sharding/algorithm/StandardRangeShardingAlgorithm.java          \
                      $(DRIVER_DIR)/sharding/executor/BaseStatementUnit.java                        \
                      $(DRIVER_DIR)/sharding/executor/BatchPreparedStatementExecutor.java           \
                      $(DRIVER_DIR)/sharding/executor/BatchPreparedStatementUnit.java               \
                      $(DRIVER_DIR)/sharding/executor/ConnectionParallelProcessCallback.java        \
                      $(DRIVER_DIR)/sharding/executor/ExecuteCallback.java                          \
                      $(DRIVER_DIR)/sharding/executor/ExecutorEngine.java                           \
                      $(DRIVER_DIR)/sharding/executor/ExecutorExceptionHandler.java                 \
                      $(DRIVER_DIR)/sharding/executor/GenerateCallback.java                         \
                      $(DRIVER_DIR)/sharding/executor/MultiThreadExecutorEngine.java                \
                      $(DRIVER_DIR)/sharding/executor/PreparedStatementExecutor.java                \
                      $(DRIVER_DIR)/sharding/executor/StatementExecutor.java                        \
                      $(DRIVER_DIR)/sharding/executor/SingletonExecutorService.java                 \
                      $(DRIVER_DIR)/sharding/executor/ParallelProcessCallback.java                  \
                      $(DRIVER_DIR)/sharding/merger/AbstractStreamResultSetMerger.java              \
                      $(DRIVER_DIR)/sharding/merger/IteratorStreamResultSetMerger.java              \
                      $(DRIVER_DIR)/sharding/merger/ResultSetMerger.java                            \
                      $(DRIVER_DIR)/sharding/routing/AllNodesRoutingEngine.java                     \
                      $(DRIVER_DIR)/sharding/routing/CompositeShardKeyRoutingEngine.java            \
                      $(DRIVER_DIR)/sharding/routing/RandomNodeRoutingEngine.java                   \
                      $(DRIVER_DIR)/sharding/routing/RoutingEngine.java                             \
                      $(DRIVER_DIR)/sharding/routing/ShardKeyBaseRoutingEngine.java                 \
                      $(DRIVER_DIR)/sharding/routing/SimpleShardKeyRoutingEngine.java               \
                      $(DRIVER_DIR)/sharding/strategy/CompositeShardingStrategy.java                \
                      $(DRIVER_DIR)/sharding/strategy/ShardingStrategy.java                         \
                      $(DRIVER_DIR)/sharding/strategy/StandardShardingStrategy.java                 \
                      $(DRIVER_DIR)/sharding/util/Range.java                                        \
                      $(DRIVER_DIR)/sharding/util/ShardingTraceLogger.java                          \
                      $(DRIVER_DIR)/sharding/util/ShardSingleLineFormatter.java                     \
                      $(DRIVER_DIR)/sharding/util/DistTxInfoForVerify.java

JDBC_SHARD_SRC4WAS =  $(DRIVER_DIR4WAS)/cm/CmProtocolContextShardConnect.java                           \
                      $(DRIVER_DIR4WAS)/cm/CmProtocolContextShardStmt.java                              \
                      $(DRIVER_DIR4WAS)/cm/CmShardAnalyzeResult.java                                    \
                      $(DRIVER_DIR4WAS)/cm/CmShardHandshakeResult.java                                  \
                      $(DRIVER_DIR4WAS)/cm/CmShardTransactionResult.java                                \
                      $(DRIVER_DIR4WAS)/cm/CmShardOperation.java                                        \
                      $(DRIVER_DIR4WAS)/cm/CmShardProtocol.java                                         \
                      $(DRIVER_DIR4WAS)/ex/ShardFailOverSuccessException.java                           \
                      $(DRIVER_DIR4WAS)/ex/ShardJdbcException.java                                      \
                      $(DRIVER_DIR4WAS)/ex/ShardFailoverIsNotAvailableException.java                    \
                      $(DRIVER_DIR4WAS)/ex/ShardError.java                                              \
                      $(DRIVER_DIR4WAS)/sharding/core/AbstractShardingCallableStatement.java            \
                      $(DRIVER_DIR4WAS)/sharding/core/AbstractShardingPreparedStatement.java            \
                      $(DRIVER_DIR4WAS)/sharding/core/AltibaseShardingConnection.java                   \
                      $(DRIVER_DIR4WAS)/sharding/core/AltibaseShardingPreparedStatement.java            \
                      $(DRIVER_DIR4WAS)/sharding/core/AltibaseShardingStatement.java                    \
                      $(DRIVER_DIR4WAS)/sharding/core/AltibaseShardingCallableStatement.java            \
                      $(DRIVER_DIR4WAS)/sharding/core/AltibaseShardingDataSource.java                   \
                      $(DRIVER_DIR4WAS)/sharding/core/AltibaseShardingResultSet.java                    \
                      $(DRIVER_DIR4WAS)/sharding/core/AltibaseShardingConnectionPoolDataSource.java     \
                      $(DRIVER_DIR4WAS)/sharding/core/AltibaseShardingLogicalConnection.java            \
                      $(DRIVER_DIR4WAS)/sharding/core/AltibaseShardingPooledConnection.java             \
                      $(DRIVER_DIR4WAS)/sharding/core/AltibaseShardingFailover.java                     \
                      $(DRIVER_DIR4WAS)/sharding/core/DataNode.java                                     \
                      $(DRIVER_DIR4WAS)/sharding/core/DataNodeShardingCallableStatement.java            \
                      $(DRIVER_DIR4WAS)/sharding/core/DataNodeShardingPreparedStatement.java            \
                      $(DRIVER_DIR4WAS)/sharding/core/DataNodeShardingStatement.java                    \
                      $(DRIVER_DIR4WAS)/sharding/core/DataNodeFailoverDestination.java                  \
                      $(DRIVER_DIR4WAS)/sharding/core/JdbcMethodInvocation.java                         \
                      $(DRIVER_DIR4WAS)/sharding/core/JdbcMethodInvoker.java                            \
                      $(DRIVER_DIR4WAS)/sharding/core/ShardKeyDataType.java                             \
                      $(DRIVER_DIR4WAS)/sharding/core/ShardNodeConfig.java                              \
                      $(DRIVER_DIR4WAS)/sharding/core/ShardRange.java                                   \
                      $(DRIVER_DIR4WAS)/sharding/core/ShardRangeList.java                               \
                      $(DRIVER_DIR4WAS)/sharding/core/ShardSplitMethod.java                             \
                      $(DRIVER_DIR4WAS)/sharding/core/GlobalTransactionLevel.java                       \
                      $(DRIVER_DIR4WAS)/sharding/core/ShardValueInfo.java                               \
                      $(DRIVER_DIR4WAS)/sharding/core/ShardValueInfoPrimaryStrategy.java                \
                      $(DRIVER_DIR4WAS)/sharding/core/ShardValueInfoStrategy.java                       \
                      $(DRIVER_DIR4WAS)/sharding/core/ShardValueInfoSubKeyStrategy.java                 \
                      $(DRIVER_DIR4WAS)/sharding/core/InternalShardingStatement.java                    \
                      $(DRIVER_DIR4WAS)/sharding/core/InternalShardingPreparedStatement.java            \
                      $(DRIVER_DIR4WAS)/sharding/core/InternalShardingCallableStatement.java            \
                      $(DRIVER_DIR4WAS)/sharding/core/ServerSideShardingStatement.java                  \
                      $(DRIVER_DIR4WAS)/sharding/core/ServerSideShardingPreparedStatement.java          \
                      $(DRIVER_DIR4WAS)/sharding/core/ServerSideShardingCallableStatement.java          \
                      $(DRIVER_DIR4WAS)/sharding/core/FailoverAlignInfo.java                            \
                      $(DRIVER_DIR4WAS)/sharding/core/NodeConnectionReport.java                         \
                      $(DRIVER_DIR4WAS)/sharding/core/ShardConnType.java                                \
                      $(DRIVER_DIR4WAS)/sharding/core/ShardValueType.java                               \
                      $(DRIVER_DIR4WAS)/sharding/core/ShardVersion.java                                 \
                      $(DRIVER_DIR4WAS)/sharding/core/DistTxInfo.java                                   \
                      $(DRIVER_DIR4WAS)/sharding/core/Shard2PhaseCommitState.java                       \
                      $(DRIVER_DIR4WAS)/sharding/algorithm/CompositeShardingAlgorithm.java              \
                      $(DRIVER_DIR4WAS)/sharding/algorithm/HashGenerator.java                           \
                      $(DRIVER_DIR4WAS)/sharding/algorithm/ListShardingAlgorithm.java                   \
                      $(DRIVER_DIR4WAS)/sharding/algorithm/RangeShardingAlgorithm.java                  \
                      $(DRIVER_DIR4WAS)/sharding/algorithm/ShardingAlgorithm.java                       \
                      $(DRIVER_DIR4WAS)/sharding/algorithm/StandardCompositeShardingAlgorithm.java      \
                      $(DRIVER_DIR4WAS)/sharding/algorithm/StandardRangeShardingAlgorithm.java          \
                      $(DRIVER_DIR4WAS)/sharding/executor/BaseStatementUnit.java                        \
                      $(DRIVER_DIR4WAS)/sharding/executor/BatchPreparedStatementExecutor.java           \
                      $(DRIVER_DIR4WAS)/sharding/executor/BatchPreparedStatementUnit.java               \
                      $(DRIVER_DIR4WAS)/sharding/executor/ConnectionParallelProcessCallback.java        \
                      $(DRIVER_DIR4WAS)/sharding/executor/ExecuteCallback.java                          \
                      $(DRIVER_DIR4WAS)/sharding/executor/ExecutorEngine.java                           \
                      $(DRIVER_DIR4WAS)/sharding/executor/ExecutorExceptionHandler.java                 \
                      $(DRIVER_DIR4WAS)/sharding/executor/GenerateCallback.java                         \
                      $(DRIVER_DIR4WAS)/sharding/executor/MultiThreadExecutorEngine.java                \
                      $(DRIVER_DIR4WAS)/sharding/executor/PreparedStatementExecutor.java                \
                      $(DRIVER_DIR4WAS)/sharding/executor/StatementExecutor.java                        \
                      $(DRIVER_DIR4WAS)/sharding/executor/SingletonExecutorService.java                 \
                      $(DRIVER_DIR4WAS)/sharding/executor/ParallelProcessCallback.java                  \
                      $(DRIVER_DIR4WAS)/sharding/merger/AbstractStreamResultSetMerger.java              \
                      $(DRIVER_DIR4WAS)/sharding/merger/IteratorStreamResultSetMerger.java              \
                      $(DRIVER_DIR4WAS)/sharding/merger/ResultSetMerger.java                            \
                      $(DRIVER_DIR4WAS)/sharding/routing/AllNodesRoutingEngine.java                     \
                      $(DRIVER_DIR4WAS)/sharding/routing/CompositeShardKeyRoutingEngine.java            \
                      $(DRIVER_DIR4WAS)/sharding/routing/RandomNodeRoutingEngine.java                   \
                      $(DRIVER_DIR4WAS)/sharding/routing/RoutingEngine.java                             \
                      $(DRIVER_DIR4WAS)/sharding/routing/ShardKeyBaseRoutingEngine.java                 \
                      $(DRIVER_DIR4WAS)/sharding/routing/SimpleShardKeyRoutingEngine.java               \
                      $(DRIVER_DIR4WAS)/sharding/strategy/CompositeShardingStrategy.java                \
                      $(DRIVER_DIR4WAS)/sharding/strategy/ShardingStrategy.java                         \
                      $(DRIVER_DIR4WAS)/sharding/strategy/StandardShardingStrategy.java                 \
                      $(DRIVER_DIR4WAS)/sharding/util/Range.java                                        \
                      $(DRIVER_DIR4WAS)/sharding/util/ShardingTraceLogger.java                          \
                      $(DRIVER_DIR4WAS)/sharding/util/ShardSingleLineFormatter.java                     \
                      $(DRIVER_DIR4WAS)/sharding/util/DistTxInfoForVerify.java
