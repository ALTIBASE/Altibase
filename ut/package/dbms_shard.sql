/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

CREATE OR REPLACE PACKAGE DBMS_SHARD IS

procedure CREATE_META();

procedure RESET_META_NODE_ID( meta_node_id in integer );

procedure SET_LOCAL_NODE( shard_node_id                in  integer,
                          node_name                    in  varchar(12),
                          host_ip                      in  varchar(64),
                          port_no                      in  integer,
                          internal_host_ip             in  varchar(64),
                          internal_port_no             in  integer,
                          internal_replication_host_ip in  varchar(64),
                          internal_replication_port_no in  integer,
                          conn_type                    in  integer default NULL ); 

procedure SET_REPLICATION( k_safety    	    in  integer,
                           replication_mode in  varchar(10) default NULL,
                           parallel_count   in  integer default NULL ); 

procedure SET_NODE( node_name         in varchar(12),
                    host_ip           in varchar(16),
                    port_no           in integer,
                    internal_host_ip  in varchar(16) default NULL,
                    internal_port_no  in integer default NULL,
                    conn_type         in integer default NULL,
                    node_id           in integer default NULL );

procedure RESET_NODE_INTERNAL( node_name         in varchar(12),
                               host_ip           in varchar(16),
                               port_no           in integer,
                               alternate_host_ip in varchar(16),
                               alternate_port_no in integer,
                               conn_type         in integer );

procedure RESET_NODE_EXTERNAL( node_name         in varchar(12),
                               host_ip           in varchar(16),
                               port_no           in integer,
                               alternate_host_ip in varchar(16),
                               alternate_port_no in integer );

procedure UNSET_NODE( node_name    in varchar(12) );

procedure BACKUP_REPLICASETS( user_name         in varchar(128),
                              old_node_name     in varchar(12),
                              new_node_name     in varchar(12));

procedure RESET_REPLICASETS( user_name         in varchar(128),
                             old_node_name     in varchar(12),
                             new_node_name     in varchar(12));

procedure SET_SHARD_TABLE( user_name                 in varchar(128),
                           table_name                in varchar(128),
                           split_method              in varchar(1) default NULL,
                           partition_key_column_name in varchar(128) default NULL,
                           default_node_name         in varchar(12) default NULL );
                           
procedure SET_SHARD_TABLE_LOCAL( user_name                   in varchar(128),
                                 table_name                  in varchar(128),
                                 split_method                in varchar(1) default NULL,
                                 partition_key_column_name   in varchar(128) default NULL,
                                 sub_split_method            in varchar(1) default NULL,
                                 sub_key_column_name         in varchar(128) default NULL,
                                 default_node_name           in varchar(12) default NULL );

procedure SET_SHARD_TABLE_COMPOSITE( user_name           in varchar(128),
                                     table_name          in varchar(128),
                                     split_method        in varchar(1),
                                     key_column_name     in varchar(128),
                                     sub_split_method    in varchar(1),
                                     sub_key_column_name in varchar(128),
                                     default_node_name   in varchar(12) default NULL );

procedure SET_SHARD_PROCEDURE( user_name          in varchar(128),
                               proc_name          in varchar(128),
                               split_method       in varchar(1),
                               key_parameter_name in varchar(128) default NULL,
                               default_node_name  in varchar(12) default NULL );

procedure SET_SHARD_PROCEDURE_LOCAL( user_name          in varchar(128),
                                     proc_name          in varchar(128),
                                     split_method       in varchar(1),
                                     key_parameter_name in varchar(128) default NULL,
                                     sub_split_method       in varchar(1),
                                     sub_key_parameter_name in varchar(128),
                                     default_node_name  in varchar(12) default NULL );

procedure SET_SHARD_PROCEDURE_COMPOSITE( user_name              in varchar(128),
                                         proc_name              in varchar(128),
                                         split_method           in varchar(1),
                                         key_parameter_name     in varchar(128),
                                         sub_split_method       in varchar(1),
                                         sub_key_parameter_name in varchar(128),
                                         default_node_name      in varchar(12) default NULL );

procedure SET_SHARD_COMPOSITE( user_name   in varchar(128),
                               object_name in varchar(128),
                               value       in varchar(100),
                               sub_value   in varchar(100),
                               node_name   in varchar(12) );

procedure SET_SHARD_COMPOSITE_LOCAL( user_name   in varchar(128),
                                     object_name in varchar(128),
                                     value       in varchar(100),
                                     sub_value   in varchar(100),
                                     node_name   in varchar(12) );
                               
procedure SET_SHARD_PARTITION_NODE( user_name      in varchar(128),
                                    object_name    in varchar(128),
                                    partition_name in varchar(128),
                                    node_name      in varchar(12) );
                                    
procedure SET_SHARD_PARTITION_NODE_LOCAL( user_name      in varchar(128),
                                    	  object_name    in varchar(128),
                                    	  partition_name in varchar(128),
                                    	  node_name      in varchar(12) );
                          
procedure SET_SHARD_HASH( user_name   in varchar(128),
                          object_name in varchar(128),
                          value       in integer,
                          node_name   in varchar(12) );

procedure SET_SHARD_HASH_LOCAL( user_name   in varchar(128),
                                object_name in varchar(128),
                                value       in integer,
                                node_name   in varchar(12) );

procedure SET_SHARD_RANGE( user_name   in varchar(128),
                           object_name in varchar(128),
                           value       in varchar(100),
                           node_name   in varchar(12) );

procedure SET_SHARD_RANGE_LOCAL( user_name   in varchar(128),
                                 object_name in varchar(128),
                                 value       in varchar(100),
                                 node_name   in varchar(12) );

procedure SET_SHARD_LIST( user_name   in varchar(128),
                          object_name in varchar(128),
                          value       in varchar(100),
                          node_name   in varchar(12) );

procedure SET_SHARD_LIST_LOCAL( user_name   in varchar(128),
                                object_name in varchar(128),
                                value       in varchar(100),
                                node_name   in varchar(12) );

procedure SET_SHARD_CLONE( user_name   in varchar(128),
                           object_name in varchar(128),
                           node_name   in varchar(12) );

procedure SET_SHARD_CLONE_LOCAL( user_name     in varchar(128),
                                 object_name   in varchar(128),
                                 node_name     in varchar(12),
                                 replicaset_id in integer default NULL,
                                 called_by     in integer default 0 );

procedure SET_SHARD_SOLO( user_name   in varchar(128),
                          object_name in varchar(128),
                          node_name   in varchar(12) );

procedure SET_SHARD_SOLO_LOCAL( user_name   in varchar(128),
                                object_name in varchar(128),
                                node_name   in varchar(12) );

procedure RESET_SHARD_RESIDENT_NODE( user_name     in varchar(128),
                                     object_name   in varchar(128),
                                     old_node_name in varchar(12),
                                     new_node_name in varchar(12),
                                     value         in varchar(100) default NULL,
                                     sub_value     in varchar(100) default NULL,
                                     is_default    in integer default 0 );

procedure RESET_SHARD_PARTITION_NODE( user_name      in varchar(128),
                                      object_name    in varchar(128),
                                      old_node_name  in varchar(12),
                                      new_node_name  in varchar(12),
                                      partition_name in varchar(128) default NULL,
                                      called_by      in integer default 0 );
                                     
procedure UNSET_SHARD_TABLE( user_name    in varchar(128),
                             table_name   in varchar(128),
                             drop_bak_tbl in char(1) default 'Y' );

procedure UNSET_SHARD_TABLE_LOCAL( user_name  in varchar(128),
                                   table_name in varchar(128) );

procedure UNSET_SHARD_PROCEDURE( user_name    in varchar(128),
                                 proc_name    in varchar(128) );

procedure UNSET_SHARD_TABLE_BY_ID( shard_id in integer );

procedure UNSET_SHARD_TABLE_BY_ID_LOCAL( shard_id in integer );

procedure UNSET_SHARD_PROCEDURE_BY_ID( shard_id in integer );

procedure EXECUTE_IMMEDIATE( query     in varchar(65534),
                             node_name in varchar(12) default NULL );

procedure CHECK_DATA( user_name            in varchar(128),
                      table_name           in varchar(128),
                      additional_node_list in varchar(1000) default NULL );

procedure REBUILD_DATA_NODE( user_name   in varchar(128),
                             table_name  in varchar(128),
                             node_name   in varchar(12),
                             batch_count in bigint default 0 );

procedure REBUILD_DATA( user_name            in varchar(128),
                        table_name           in varchar(128),
                        batch_count          in bigint default 0,
                        additional_node_list in varchar(1000) default NULL );

procedure PURGE_OLD_SHARD_META_INFO( smn in bigint default NULL );

procedure SET_SHARD_TABLE_SHARDKEY( user_name                   in varchar(128),
                                    table_name                  in varchar(128),
                                    partition_node_list         in varchar(32000),
                                    method_for_irregular        in char(1) default 'E',
                                    replication_parallel_count  in integer default 1 );

procedure SET_SHARD_TABLE_SOLO( user_name                   in varchar(128),
                                table_name                  in varchar(128),
                                node_name                   in varchar(12),
                                method_for_irregular        in char(1) default 'E',
                                replication_parallel_count  in integer default 1 );

procedure SET_SHARD_TABLE_CLONE( user_name                   in varchar(128),
                                 table_name                  in varchar(128),
                                 reference_node_name         in varchar(12) default NULL,
                                 replication_parallel_count  in integer default 1 );

procedure SET_SHARD_PROCEDURE_SHARDKEY( user_name          in varchar(128),
                                        proc_name          in varchar(128),
                                        split_method       in varchar(1),
                                        key_parameter_name in varchar(128),
                                        value_node_list    in varchar(32000),
                                        default_node_name  in varchar(12) default NULL,
                                        proc_replace       in char(1) default 'N' );

procedure SET_SHARD_PROCEDURE_SOLO( user_name    in varchar(128),
                                    proc_name    in varchar(128),
                                    node_name    in varchar(12),
                                    proc_replace  in char(1) default 'N' );

procedure SET_SHARD_PROCEDURE_CLONE( user_name    in varchar(128),
                                     proc_name    in varchar(128),
                                     proc_replace in char(1) default 'N' );

procedure SET_REPLICASCN();

procedure TOUCH_META();

end dbms_shard;
/

CREATE OR REPLACE PUBLIC SYNONYM dbms_shard FOR sys.dbms_shard;
GRANT EXECUTE ON dbms_shard TO PUBLIC;

