/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

create or replace package body DBMS_SHARD is


-- CREATE META
procedure CREATE_META()
as
    dummy integer;
begin
    dummy := shard_create_meta();
end CREATE_META;

-- RESET META NODE ID
procedure RESET_META_NODE_ID( meta_node_id in integer )
as
    dummy integer;
begin
    dummy := shard_reset_meta_node_id( meta_node_id );
end RESET_META_NODE_ID;

-- SET LOCAL NODE
procedure SET_LOCAL_NODE( shard_node_id                in  integer,
                          node_name                    in  varchar(12),
                          host_ip                      in  varchar(64),
                          port_no                      in  integer,
                          internal_host_ip             in  varchar(64),
                          internal_port_no             in  integer,
                          internal_replication_host_ip in  varchar(64),
                          internal_replication_port_no in  integer,
                          conn_type                    in  integer default NULL )
as
    dummy integer;
    up_node_name varchar(12);
begin
    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    dummy := shard_set_local_node(shard_node_id, up_node_name, 
                      host_ip, port_no, 
                      internal_host_ip, internal_port_no, 
                      internal_replication_host_ip, internal_replication_port_no,
                      conn_type);
end SET_LOCAL_NODE;
              
procedure SET_REPLICATION( k_safety            in  integer,
                           replication_mode in  varchar(10) default NULL,
                           parallel_count   in  integer default NULL )
as
    dummy integer;
    up_replication_mode varchar(10);
begin
    if instr( replication_mode, chr(34) ) = 0 then
        up_replication_mode := upper( replication_mode );
    else
        up_replication_mode := replace2( replication_mode, chr(34) );
    end if;

    dummy := shard_set_replication(k_safety, up_replication_mode, parallel_count);
end SET_REPLICATION;

-- SET DATA NODE
procedure SET_NODE( node_name         in varchar(12),
                    host_ip           in varchar(16),
                    port_no           in integer,
                    internal_host_ip  in varchar(16) default NULL,
                    internal_port_no  in integer default NULL,
                    conn_type         in integer default NULL,
                    node_id           in integer default NULL )
as
    dummy integer;
    up_node_name varchar(12);
begin
    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    dummy := shard_set_node(up_node_name, host_ip, port_no, internal_host_ip, internal_port_no, conn_type, node_id);
end SET_NODE;

-- RESET DATA NODE (INTERNAL)
procedure RESET_NODE_INTERNAL( node_name         in varchar(12),
                               host_ip           in varchar(16),
                               port_no           in integer,
                               alternate_host_ip in varchar(16),
                               alternate_port_no in integer,
                               conn_type         in integer)
as
    dummy integer;
    up_node_name varchar(12);
begin
    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    dummy := shard_reset_node_internal( up_node_name,
                                        host_ip,
                                        port_no,
                                        alternate_host_ip,
                                        alternate_port_no,
                                        conn_type );
end RESET_NODE_INTERNAL;

-- RESET DATA NODE (EXTERNAL)
procedure RESET_NODE_EXTERNAL( node_name         in varchar(12),
                               host_ip           in varchar(16),
                               port_no           in integer,
                               alternate_host_ip in varchar(16),
                               alternate_port_no in integer )
as
    dummy integer;
    up_node_name varchar(12);
begin
    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    dummy := shard_reset_node_external( up_node_name,
                                        host_ip,
                                        port_no,
                                        alternate_host_ip,
                                        alternate_port_no );
end RESET_NODE_EXTERNAL;

-- UNSET DATA NODE
procedure UNSET_NODE( node_name    in varchar(12) )
as
    dummy integer;
    up_node_name varchar(12);
begin
    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    dummy := shard_unset_node( up_node_name );
end UNSET_NODE;

-- BACKUP_REPLICASETS
procedure BACKUP_REPLICASETS( user_name        in varchar(128),
                              old_node_name    in varchar(12),
                              new_node_name    in varchar(12))
as
    dummy integer;
    up_user_name varchar(128);
    up_old_node_name varchar(12);
    up_new_node_name varchar(12);
begin
    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( old_node_name, chr(34) ) = 0 then
        up_old_node_name := upper( old_node_name );
    else
        up_old_node_name := replace2( old_node_name, chr(34) );
    end if;

    if instr( new_node_name, chr(34) ) = 0 then
        up_new_node_name := upper( new_node_name );
    else
        up_new_node_name := replace2( new_node_name, chr(34) );
    end if;


    dummy := shard_backup_replicasets( up_user_name,
                                       up_old_node_name,
                                       up_new_node_name );

end BACKUP_REPLICASETS;

-- RESET_REPLICASETS
procedure RESET_REPLICASETS( user_name        in varchar(128),
                             old_node_name    in varchar(12),
                             new_node_name    in varchar(12))
as
    dummy integer;
    up_user_name varchar(128);
    up_old_node_name varchar(12);
    up_new_node_name varchar(12);
begin
    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( old_node_name, chr(34) ) = 0 then
        up_old_node_name := upper( old_node_name );
    else
        up_old_node_name := replace2( old_node_name, chr(34) );
    end if;

    if instr( new_node_name, chr(34) ) = 0 then
        up_new_node_name := upper( new_node_name );
    else
        up_new_node_name := replace2( new_node_name, chr(34) );
    end if;


    dummy := shard_reset_replicasets( up_user_name,
                                      up_old_node_name,
                                      up_new_node_name );

end reset_REPLICASETS;

-- SET SHARD TABLE INFO
procedure SET_SHARD_TABLE( user_name                 in varchar(128),
                           table_name                in varchar(128),
                           split_method              in varchar(1) default NULL,
                           partition_key_column_name in varchar(128) default NULL,
                           default_node_name         in varchar(12) default NULL )
as
    dummy integer;
    up_user_name varchar(128);
    up_table_name varchar(128);
    up_split_method varchar(1);
    up_key_column_name varchar(128);
    up_default_node_name varchar(12);
    test_property_val INTEGER;
begin
    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( table_name, chr(34) ) = 0 then
        up_table_name := upper( table_name );
    else
        up_table_name := replace2( table_name, chr(34) );
    end if;

    up_split_method := upper( split_method );

    if instr( partition_key_column_name, chr(34) ) = 0 then
        up_key_column_name := upper( partition_key_column_name );
    else
        up_key_column_name := replace2( partition_key_column_name, chr(34) );
    end if;

    if instr( default_node_name, chr(34) ) = 0 then
        up_default_node_name := upper( default_node_name );
    else
        up_default_node_name := replace2( default_node_name, chr(34) );
    end if;

    SELECT memory_value1 INTO test_property_val 
    FROM x$property 
    WHERE name = '__SHARD_LOCAL_FORCE';
    
    if test_property_val > 0 then
        dummy := shard_set_shard_table( up_user_name,
                                         up_table_name,
                                         up_split_method,
                                         up_key_column_name,
                                         NULL,                  -- sub_split_method
                                         NULL,                  -- sub_key_column_name
                                         up_default_node_name );
    else
        dummy := shard_set_shard_table_global( up_user_name,
                                               up_table_name,
                                               up_split_method,
                                               up_key_column_name,
                                               NULL,                  -- sub_split_method
                                               NULL,                  -- sub_key_column_name
                                               up_default_node_name );
    end if;

end SET_SHARD_TABLE;

-- SET SHARD TABLE INFO
procedure SET_SHARD_TABLE_LOCAL( user_name                   in varchar(128),
                                 table_name                  in varchar(128),
                                 split_method                in varchar(1) default NULL,
                                 partition_key_column_name   in varchar(128) default NULL,
                                 sub_split_method            in varchar(1) default NULL,
                                 sub_key_column_name         in varchar(128) default NULL,
                                 default_node_name           in varchar(12) default NULL )
as
    dummy integer;
    up_user_name varchar(128);
    up_table_name varchar(128);
    up_split_method varchar(1);
    up_key_column_name varchar(128);
    up_sub_split_method varchar(1);
    up_sub_key_column_name varchar(128);
    up_default_node_name varchar(12);
begin
    if instr( user_name, chr(34) ) = 0 then
        up_user_name := user_name;
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( table_name, chr(34) ) = 0 then
        up_table_name := table_name;
    else
        up_table_name := replace2( table_name, chr(34) );
    end if;

    up_split_method := upper( split_method );

    if instr( partition_key_column_name, chr(34) ) = 0 then
        up_key_column_name := partition_key_column_name;
    else
        up_key_column_name := replace2( partition_key_column_name, chr(34) );
    end if;

    up_sub_split_method := upper( sub_split_method );

    if instr( sub_key_column_name, chr(34) ) = 0 then
        up_sub_key_column_name := sub_key_column_name;
    else
        up_sub_key_column_name := replace2( sub_key_column_name, chr(34) );
    end if;

    if instr( default_node_name, chr(34) ) = 0 then
        up_default_node_name := default_node_name;
    else
        up_default_node_name := replace2( default_node_name, chr(34) );
    end if;

    dummy := shard_set_shard_table( up_user_name,
                                    up_table_name,
                                    up_split_method,
                                    up_key_column_name,
                                    up_sub_split_method,
                                    up_sub_key_column_name,
                                    up_default_node_name );
end SET_SHARD_TABLE_LOCAL;

--SET COMPOSITE SHARD TABLE INFO
procedure SET_SHARD_TABLE_COMPOSITE( user_name           in varchar(128),
                                     table_name          in varchar(128),
                                     split_method        in varchar(1),
                                     key_column_name     in varchar(128),
                                     sub_split_method    in varchar(1),
                                     sub_key_column_name in varchar(128),
                                     default_node_name   in varchar(12) default NULL )
as
    dummy integer;
    up_user_name varchar(128);
    up_table_name varchar(128);
    up_split_method varchar(1);
    up_key_column_name varchar(128);
    up_sub_split_method varchar(1);
    up_sub_key_column_name varchar(128);
    up_default_node_name varchar(12);
    test_property_val INTEGER;
begin
    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( table_name, chr(34) ) = 0 then
        up_table_name := upper( table_name );
    else
        up_table_name := replace2( table_name, chr(34) );
    end if;

    up_split_method := upper( split_method );

    if instr( key_column_name, chr(34) ) = 0 then
        up_key_column_name := upper( key_column_name );
    else
        up_key_column_name := replace2( key_column_name, chr(34) );
    end if;

    up_sub_split_method := upper( sub_split_method );

    if instr( sub_key_column_name, chr(34) ) = 0 then
        up_sub_key_column_name := upper( sub_key_column_name );
    else
        up_sub_key_column_name := replace2( sub_key_column_name, chr(34) );
    end if;

    if instr( default_node_name, chr(34) ) = 0 then
        up_default_node_name := upper( default_node_name );
    else
        up_default_node_name := replace2( default_node_name, chr(34) );
    end if;
    SELECT memory_value1 INTO test_property_val 
    FROM x$property 
    WHERE name = '__SHARD_LOCAL_FORCE';
    
    if test_property_val > 0 then
        dummy := shard_set_shard_table( up_user_name,
                                               up_table_name,
                                               up_split_method,
                                               up_key_column_name,
                                               up_sub_split_method,
                                               up_sub_key_column_name,
                                               up_default_node_name );
    else
        dummy := shard_set_shard_table_global( up_user_name,
                                               up_table_name,
                                               up_split_method,
                                               up_key_column_name,
                                               up_sub_split_method,
                                               up_sub_key_column_name,
                                               up_default_node_name );
    end if;

end SET_SHARD_TABLE_COMPOSITE;

-- SET SHARD PROCEDURE INFO
procedure SET_SHARD_PROCEDURE( user_name          in varchar(128),
                               proc_name          in varchar(128),
                               split_method       in varchar(1),
                               key_parameter_name in varchar(128) default NULL,
                               default_node_name  in varchar(12) default NULL )
as
    dummy integer;
    up_user_name varchar(128);
    up_proc_name varchar(128);
    up_split_method varchar(1);
    up_key_parameter_name varchar(128);
    up_default_node_name varchar(12);
    test_property_val INTEGER;
begin
    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( proc_name, chr(34) ) = 0 then
        up_proc_name := upper( proc_name );
    else
        up_proc_name := replace2( proc_name, chr(34) );
    end if;

    up_split_method := upper( split_method );

    if instr( key_parameter_name, chr(34) ) = 0 then
        up_key_parameter_name := upper( key_parameter_name );
    else
        up_key_parameter_name := replace2( key_parameter_name, chr(34) );
    end if;

    if instr( default_node_name, chr(34) ) = 0 then
        up_default_node_name := upper( default_node_name );
    else
        up_default_node_name := replace2( default_node_name, chr(34) );
    end if;

    SELECT memory_value1 INTO test_property_val 
    FROM x$property 
    WHERE name = '__SHARD_LOCAL_FORCE';
    
    if test_property_val > 0 then
        dummy := shard_set_shard_procedure( up_user_name,
                                                   up_proc_name,
                                                   up_split_method,
                                                   up_key_parameter_name,
                                                   NULL,                  -- sub_split_method
                                                   NULL,                  -- sub_key_parameter_name
                                                   up_default_node_name );
    else
        dummy := shard_set_shard_procedure_global( up_user_name,
                                           up_proc_name,
                                           up_split_method,
                                           up_key_parameter_name,
                                           NULL,                  -- sub_split_method
                                           NULL,                  -- sub_key_parameter_name
                                           up_default_node_name );
    end if;
    

end SET_SHARD_PROCEDURE;

-- SET SHARD PROCEDURE INFO
procedure SET_SHARD_PROCEDURE_LOCAL( user_name          in varchar(128),
                                     proc_name          in varchar(128),
                                     split_method       in varchar(1),
                                     key_parameter_name in varchar(128) default NULL,
                                     sub_split_method       in varchar(1) default NULL,
                                     sub_key_parameter_name in varchar(128) default NULL,
                                     default_node_name  in varchar(12) default NULL )
as
    dummy integer;
    up_user_name varchar(128);
    up_proc_name varchar(128);
    up_split_method varchar(1);
    up_key_parameter_name varchar(128);
    up_sub_split_method varchar(1);
    up_sub_key_parameter_name varchar(128);
    up_default_node_name varchar(12);
begin
    if instr( user_name, chr(34) ) = 0 then
        up_user_name := user_name;
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( proc_name, chr(34) ) = 0 then
        up_proc_name := proc_name;
    else
        up_proc_name := replace2( proc_name, chr(34) );
    end if;

    up_split_method := upper( split_method );

    if instr( key_parameter_name, chr(34) ) = 0 then
        up_key_parameter_name := key_parameter_name;
    else
        up_key_parameter_name := replace2( key_parameter_name, chr(34) );
    end if;

    up_sub_split_method := upper( sub_split_method );

    if instr( sub_key_parameter_name, chr(34) ) = 0 then
        up_sub_key_parameter_name := sub_key_parameter_name;
    else
        up_sub_key_parameter_name := replace2( sub_key_parameter_name, chr(34) );
    end if;

    if instr( default_node_name, chr(34) ) = 0 then
        up_default_node_name := default_node_name;
    else
        up_default_node_name := replace2( default_node_name, chr(34) );
    end if;

    dummy := shard_set_shard_procedure( up_user_name,
                                        up_proc_name,
                                        up_split_method,
                                        up_key_parameter_name,
                                        up_sub_split_method,
                                        up_sub_key_parameter_name,
                                        up_default_node_name );
end SET_SHARD_PROCEDURE_LOCAL;


-- SET COMPOSITE SHARD PROCEDURE INFO
procedure SET_SHARD_PROCEDURE_COMPOSITE( user_name              in varchar(128),
                                         proc_name              in varchar(128),
                                         split_method           in varchar(1),
                                         key_parameter_name     in varchar(128),
                                         sub_split_method       in varchar(1),
                                         sub_key_parameter_name in varchar(128),
                                         default_node_name      in varchar(12) default NULL )
as
    dummy integer;
    up_user_name varchar(128);
    up_proc_name varchar(128);
    up_split_method varchar(1);
    up_key_parameter_name varchar(128);
    up_sub_split_method varchar(1);
    up_sub_key_parameter_name varchar(128);
    up_default_node_name varchar(12);
    test_property_val INTEGER;
begin
    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( proc_name, chr(34) ) = 0 then
        up_proc_name := upper( proc_name );
    else
        up_proc_name := replace2( proc_name, chr(34) );
    end if;

    up_split_method := upper( split_method );

    if instr( key_parameter_name, chr(34) ) = 0 then
        up_key_parameter_name := upper( key_parameter_name );
    else
        up_key_parameter_name := replace2( key_parameter_name, chr(34) );
    end if;

    up_sub_split_method := upper( sub_split_method );

    if instr( sub_key_parameter_name, chr(34) ) = 0 then
        up_sub_key_parameter_name := upper( sub_key_parameter_name );
    else
        up_sub_key_parameter_name := replace2( sub_key_parameter_name, chr(34) );
    end if;

    if instr( default_node_name, chr(34) ) = 0 then
        up_default_node_name := upper( default_node_name );
    else
        up_default_node_name := replace2( default_node_name, chr(34) );
    end if;

    SELECT memory_value1 INTO test_property_val 
    FROM x$property 
    WHERE name = '__SHARD_LOCAL_FORCE';
    
    if test_property_val > 0 then
        dummy := shard_set_shard_procedure( up_user_name,
                                               up_proc_name,
                                               up_split_method,
                                               up_key_parameter_name,
                                               up_sub_split_method,
                                               up_sub_key_parameter_name,
                                               up_default_node_name );
    else
        dummy := shard_set_shard_procedure_global( up_user_name,
                                               up_proc_name,
                                               up_split_method,
                                               up_key_parameter_name,
                                               up_sub_split_method,
                                               up_sub_key_parameter_name,
                                               up_default_node_name );
    end if;
end SET_SHARD_PROCEDURE_COMPOSITE;

-- SET SHARD PARTITION NODE
procedure SET_SHARD_PARTITION_NODE( user_name      in varchar(128),
                                    object_name    in varchar(128),
                                    partition_name in varchar(128),
                                    node_name      in varchar(12) )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_partition_name varchar(128);
    up_node_name varchar(12);
    test_property_val INTEGER;
begin

    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( object_name, chr(34) ) = 0 then
        up_object_name := upper( object_name );
    else
        up_object_name := replace2( object_name, chr(34) );
    end if;

    if instr( partition_name, chr(34) ) = 0 then
        up_partition_name := upper( partition_name );
    else
        up_partition_name := replace2( partition_name, chr(34) );
    end if;
    
    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    SELECT memory_value1 INTO test_property_val 
    FROM x$property 
    WHERE name = '__SHARD_LOCAL_FORCE';
    
    if test_property_val > 0 then
        dummy := shard_set_shard_partition_node( up_user_name,
                                                    up_object_name,
                                                    up_partition_name,
                                                    up_node_name );
    else
        dummy := shard_set_shard_partition_node_global( up_user_name,
                                                    up_object_name,
                                                    up_partition_name,
                                                    up_node_name );
    end if;
    
end SET_SHARD_PARTITION_NODE;

-- SET SHARD PARTITION NODE
procedure SET_SHARD_PARTITION_NODE_LOCAL( user_name      in varchar(128),
                                          object_name    in varchar(128),
                                          partition_name in varchar(128),
                                          node_name      in varchar(12) )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_partition_name varchar(128);
    up_node_name varchar(12);

begin

    if instr( user_name, chr(34) ) = 0 then
        up_user_name := user_name;
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( object_name, chr(34) ) = 0 then
        up_object_name := object_name;
    else
        up_object_name := replace2( object_name, chr(34) );
    end if;

    if instr( partition_name, chr(34) ) = 0 then
        up_partition_name := partition_name;
    else
        up_partition_name := replace2( partition_name, chr(34) );
    end if;
    
    if instr( node_name, chr(34) ) = 0 then
        up_node_name := node_name;
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    dummy := shard_set_shard_partition_node( up_user_name,
                                             up_object_name,
                                             up_partition_name,
                                             up_node_name );
end SET_SHARD_PARTITION_NODE_LOCAL;

                                    
-- SET SHARD HASH
procedure SET_SHARD_HASH( user_name   in varchar(128),
                          object_name in varchar(128),
                          value       in integer,
                          node_name   in varchar(12) )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_node_name varchar(12);
    test_property_val INTEGER;
begin

    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( object_name, chr(34) ) = 0 then
        up_object_name := upper( object_name );
    else
        up_object_name := replace2( object_name, chr(34) );
    end if;

    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    SELECT memory_value1 INTO test_property_val 
    FROM x$property 
    WHERE name = '__SHARD_LOCAL_FORCE';
    
    if test_property_val > 0 then
        dummy := shard_set_shard_range( up_user_name,
                                       up_object_name,
                                       value,
                                       NULL,          -- sub_value
                                       up_node_name,
                                       'H' );
    else
        dummy := shard_set_shard_range_global( up_user_name,
                                       up_object_name,
                                       value,
                                       NULL,          -- sub_value
                                       up_node_name,
                                       'H' );
    end if;

end SET_SHARD_HASH;

-- SET SHARD HASH
procedure SET_SHARD_HASH_LOCAL( user_name   in varchar(128),
                                object_name in varchar(128),
                                value       in integer,
                                node_name   in varchar(12) )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_node_name varchar(12);

begin

    if instr( user_name, chr(34) ) = 0 then
        up_user_name := user_name;
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( object_name, chr(34) ) = 0 then
        up_object_name := object_name;
    else
        up_object_name := replace2( object_name, chr(34) );
    end if;

    if instr( node_name, chr(34) ) = 0 then
        up_node_name := node_name;
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    dummy := shard_set_shard_range( up_user_name,
                                    up_object_name,
                                    value,
                                    NULL,          -- sub_value
                                    up_node_name,
                                    'H' );
end SET_SHARD_HASH_LOCAL;


-- SET SHARD RANGE
procedure SET_SHARD_RANGE( user_name   in varchar(128),
                           object_name in varchar(128),
                           value       in varchar(100),
                           node_name   in varchar(12) )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_node_name varchar(12);
    test_property_val INTEGER;
begin

    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( object_name, chr(34) ) = 0 then
        up_object_name := upper( object_name );
    else
        up_object_name := replace2( object_name, chr(34) );
    end if;

    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    SELECT memory_value1 INTO test_property_val 
    FROM x$property 
    WHERE name = '__SHARD_LOCAL_FORCE';
    
    if test_property_val > 0 then
        dummy := shard_set_shard_range( up_user_name,
                                               up_object_name,
                                               value,
                                               NULL,          -- sub_value
                                               up_node_name,
                                               'R' );
    else
        dummy := shard_set_shard_range_global( up_user_name,
                                               up_object_name,
                                               value,
                                               NULL,          -- sub_value
                                               up_node_name,
                                               'R' );
    end if;

end SET_SHARD_RANGE;

-- SET SHARD RANGE
procedure SET_SHARD_RANGE_LOCAL( user_name   in varchar(128),
                                 object_name in varchar(128),
                                 value       in varchar(100),
                                 node_name   in varchar(12) )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_node_name varchar(12);

begin

    if instr( user_name, chr(34) ) = 0 then
        up_user_name := user_name;
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( object_name, chr(34) ) = 0 then
        up_object_name := object_name;
    else
        up_object_name := replace2( object_name, chr(34) );
    end if;

    if instr( node_name, chr(34) ) = 0 then
        up_node_name := node_name;
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    dummy := shard_set_shard_range( up_user_name,
                                    up_object_name,
                                    value,
                                    NULL,          -- sub_value
                                    up_node_name,
                                    'R' );
end SET_SHARD_RANGE_LOCAL;


-- SET SHARD LIST
procedure SET_SHARD_LIST( user_name   in varchar(128),
                          object_name in varchar(128),
                          value       in varchar(100),
                          node_name   in varchar(12) )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_node_name varchar(12);
    test_property_val INTEGER;
begin

    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( object_name, chr(34) ) = 0 then
        up_object_name := upper( object_name );
    else
        up_object_name := replace2( object_name, chr(34) );
    end if;

    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;
    
    SELECT memory_value1 INTO test_property_val 
    FROM x$property 
    WHERE name = '__SHARD_LOCAL_FORCE';
    
    if test_property_val > 0 then
        dummy := shard_set_shard_range( up_user_name,
                                           up_object_name,
                                           value,
                                           NULL,          -- sub_value
                                           up_node_name,
                                           'L' );
    else
        dummy := shard_set_shard_range_global( up_user_name,
                                           up_object_name,
                                           value,
                                           NULL,          -- sub_value
                                           up_node_name,
                                           'L' );
    end if;
    
end SET_SHARD_LIST;

-- SET SHARD LIST
procedure SET_SHARD_LIST_LOCAL( user_name   in varchar(128),
                                object_name in varchar(128),
                                value       in varchar(100),
                                node_name   in varchar(12) )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_node_name varchar(12);

begin

    if instr( user_name, chr(34) ) = 0 then
        up_user_name := user_name;
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( object_name, chr(34) ) = 0 then
        up_object_name := object_name;
    else
        up_object_name := replace2( object_name, chr(34) );
    end if;

    if instr( node_name, chr(34) ) = 0 then
        up_node_name := node_name;
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    dummy := shard_set_shard_range( up_user_name,
                                    up_object_name,
                                    value,
                                    NULL,          -- sub_value
                                    up_node_name,
                                    'L' );
end SET_SHARD_LIST_LOCAL;

-- SET SHARD CLONE
procedure SET_SHARD_CLONE( user_name   in varchar(128),
                           object_name in varchar(128),
                           node_name   in varchar(12) )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_node_name varchar(12);
    test_property_val INTEGER;
begin

    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( object_name, chr(34) ) = 0 then
        up_object_name := upper( object_name );
    else
        up_object_name := replace2( object_name, chr(34) );
    end if;

    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;
    SELECT memory_value1 INTO test_property_val 
    FROM x$property 
    WHERE name = '__SHARD_LOCAL_FORCE';
    
    if test_property_val > 0 then
        dummy := shard_set_shard_clone( up_user_name,
                                        up_object_name,
                                        up_node_name,
                                        NULL,
                                        0 );
    else
        dummy := shard_set_shard_clone_global( up_user_name,
                                               up_object_name,
                                               up_node_name );
    end if;

end SET_SHARD_CLONE;

-- SET SHARD CLONE LOCAL
procedure SET_SHARD_CLONE_LOCAL( user_name     in varchar(128),
                                 object_name   in varchar(128),
                                 node_name     in varchar(12),
                                 replicaset_id in integer default NULL,
                                 called_by     in integer default 0 )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_node_name varchar(12);

begin

    if instr( user_name, chr(34) ) = 0 then
        up_user_name := user_name;
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( object_name, chr(34) ) = 0 then
        up_object_name := object_name;
    else
        up_object_name := replace2( object_name, chr(34) );
    end if;

    if instr( node_name, chr(34) ) = 0 then
        up_node_name := node_name;
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    dummy := shard_set_shard_clone( up_user_name,
                                    up_object_name,
                                    up_node_name,
                                    replicaset_id,
                                    called_by );
end SET_SHARD_CLONE_LOCAL;

-- SET SHARD SOLO 
procedure SET_SHARD_SOLO( user_name   in varchar(128),
                          object_name in varchar(128),
                          node_name   in varchar(12) )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_node_name varchar(12);
    test_property_val INTEGER;
begin

    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( object_name, chr(34) ) = 0 then
        up_object_name := upper( object_name );
    else
        up_object_name := replace2( object_name, chr(34) );
    end if;

    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;
    
    SELECT memory_value1 INTO test_property_val 
    FROM x$property 
    WHERE name = '__SHARD_LOCAL_FORCE';
    
    if test_property_val > 0 then
        dummy := shard_set_shard_solo( up_user_name,
                                              up_object_name,
                                              up_node_name );
    else
        dummy := shard_set_shard_solo_global( up_user_name,
                                              up_object_name,
                                              up_node_name );
    end if;
end SET_SHARD_SOLO;

-- SET SHARD SOLO LOCAL 
procedure SET_SHARD_SOLO_LOCAL( user_name   in varchar(128),
                                object_name in varchar(128),
                                node_name   in varchar(12) )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_node_name varchar(12);

begin

    if instr( user_name, chr(34) ) = 0 then
        up_user_name := user_name;
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( object_name, chr(34) ) = 0 then
        up_object_name := object_name;
    else
        up_object_name := replace2( object_name, chr(34) );
    end if;

    if instr( node_name, chr(34) ) = 0 then
        up_node_name := node_name;
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    dummy := shard_set_shard_solo( up_user_name,
                                   up_object_name,
                                   up_node_name );
end SET_SHARD_SOLO_LOCAL;


-- SET SHARD COMPOSITE
procedure SET_SHARD_COMPOSITE( user_name   in varchar(128),
                               object_name in varchar(128),
                               value       in varchar(100),
                               sub_value   in varchar(100),
                               node_name   in varchar(12) )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_node_name varchar(12);
    test_property_val INTEGER;
begin

    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( object_name, chr(34) ) = 0 then
        up_object_name := upper( object_name );
    else
        up_object_name := replace2( object_name, chr(34) );
    end if;

    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    SELECT memory_value1 INTO test_property_val 
    FROM x$property 
    WHERE name = '__SHARD_LOCAL_FORCE';
    
    if test_property_val > 0 then
        dummy := shard_set_shard_range( up_user_name,
                                           up_object_name,
                                           value,
                                           sub_value,
                                           up_node_name,
                                           'P' );
    else
        dummy := shard_set_shard_range_global( up_user_name,
                                           up_object_name,
                                           value,
                                           sub_value,
                                           up_node_name,
                                           'P' );
    end if;
    
end SET_SHARD_COMPOSITE;

-- SET SHARD COMPOSITE LOCAL
procedure SET_SHARD_COMPOSITE_LOCAL( user_name   in varchar(128),
                                     object_name in varchar(128),
                                     value       in varchar(100),
                                     sub_value   in varchar(100),
                                     node_name   in varchar(12) )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_node_name varchar(12);

begin

    if instr( user_name, chr(34) ) = 0 then
        up_user_name := user_name;
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( object_name, chr(34) ) = 0 then
        up_object_name := object_name;
    else
        up_object_name := replace2( object_name, chr(34) );
    end if;

    if instr( node_name, chr(34) ) = 0 then
        up_node_name := node_name;
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    dummy := shard_set_shard_range( up_user_name,
                                    up_object_name,
                                    value,
                                    sub_value,
                                    up_node_name,
                                    'P' );
end SET_SHARD_COMPOSITE_LOCAL;


-- RESET_SHARD_RESIDENT_NODE
procedure RESET_SHARD_RESIDENT_NODE( user_name     in varchar(128),
                                     object_name   in varchar(128),
                                     old_node_name in varchar(12),
                                     new_node_name in varchar(12),
                                     value         in varchar(100) default NULL,
                                     sub_value     in varchar(100) default NULL,
                                     is_default    in integer default 0 )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_old_node_name varchar(12);
    up_new_node_name varchar(12);

begin

    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( object_name, chr(34) ) = 0 then
        up_object_name := upper( object_name );
    else
        up_object_name := replace2( object_name, chr(34) );
    end if;

    if instr( old_node_name, chr(34) ) = 0 then
        up_old_node_name := upper( old_node_name );
    else
        up_old_node_name := replace2( old_node_name, chr(34) );
    end if;

    if instr( new_node_name, chr(34) ) = 0 then
        up_new_node_name := upper( new_node_name );
    else
        up_new_node_name := replace2( new_node_name, chr(34) );
    end if;

    dummy := shard_reset_shard_resident_node( up_user_name,
                                              up_object_name,
                                              up_old_node_name,
                                              up_new_node_name,
                                              value,
                                              sub_value,
                                              is_default );
end RESET_SHARD_RESIDENT_NODE;

-- RESET_SHARD_PARTITION_NODE
procedure RESET_SHARD_PARTITION_NODE( user_name      in varchar(128),
                                      object_name    in varchar(128),
                                      old_node_name  in varchar(12),
                                      new_node_name  in varchar(12),
                                      partition_name in varchar(128) default NULL,
                                      called_by      in integer default 0 )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_old_node_name varchar(12);
    up_new_node_name varchar(12);

begin

    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( object_name, chr(34) ) = 0 then
        up_object_name := upper( object_name );
    else
        up_object_name := replace2( object_name, chr(34) );
    end if;

    if instr( old_node_name, chr(34) ) = 0 then
        up_old_node_name := upper( old_node_name );
    else
        up_old_node_name := replace2( old_node_name, chr(34) );
    end if;

    if instr( new_node_name, chr(34) ) = 0 then
        up_new_node_name := upper( new_node_name );
    else
        up_new_node_name := replace2( new_node_name, chr(34) );
    end if;

    dummy := shard_reset_shard_partition_node( up_user_name,
                                               up_object_name,
                                               up_old_node_name,
                                               up_new_node_name,
                                               partition_name,
                                               called_by );
end RESET_SHARD_PARTITION_NODE;

-- UNSET SHARD TABLE
procedure UNSET_SHARD_TABLE( user_name    in varchar(128),
                             table_name   in varchar(128),
                             drop_bak_tbl in char(1) default 'Y' )
as
    dummy integer;
    up_user_name varchar(128);
    up_table_name varchar(128);
    test_property_val INTEGER;
    up_drop_bak_tbl char(1);
begin
    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( table_name, chr(34) ) = 0 then
        up_table_name := upper( table_name );
    else
        up_table_name := replace2( table_name, chr(34) );
    end if;

    if instr( drop_bak_tbl, chr(34) ) = 0 then
        up_drop_bak_tbl := upper( drop_bak_tbl );
    else
        up_drop_bak_tbl := replace2( drop_bak_tbl, chr(34) );
    end if;

    SELECT memory_value1 INTO test_property_val 
    FROM x$property 
    WHERE name = '__SHARD_LOCAL_FORCE';
    
    if test_property_val > 0 then
        dummy := shard_unset_shard_table( up_user_name, up_table_name );
    else
        dummy := shard_unset_shard_table_global( up_user_name, up_table_name, up_drop_bak_tbl );
    end if;
end UNSET_SHARD_TABLE;

-- UNSET SHARD TABLE LOCAL
procedure UNSET_SHARD_TABLE_LOCAL( user_name  in varchar(128),
                                   table_name in varchar(128) )
as
    dummy integer;
    up_user_name varchar(128);
    up_table_name varchar(128);
begin
    if instr( user_name, chr(34) ) = 0 then
        up_user_name := user_name;
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( table_name, chr(34) ) = 0 then
        up_table_name :=  table_name;
    else
        up_table_name := replace2( table_name, chr(34) );
    end if;

    dummy := shard_unset_shard_table( up_user_name, up_table_name );
end UNSET_SHARD_TABLE_LOCAL;


-- UNSET SHARD PROCEDURE
procedure UNSET_SHARD_PROCEDURE( user_name    in varchar(128),
                                 proc_name    in varchar(128) )
as
    dummy integer;
    up_user_name varchar(128);
    up_proc_name varchar(128);
    test_property_val INTEGER;
begin
    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( proc_name, chr(34) ) = 0 then
        up_proc_name := upper( proc_name );
    else
        up_proc_name := replace2( proc_name, chr(34) );
    end if;
    
    SELECT memory_value1 INTO test_property_val 
    FROM x$property 
    WHERE name = '__SHARD_LOCAL_FORCE';
   
    if test_property_val > 0 then
        dummy := shard_unset_shard_procedure( up_user_name, up_proc_name );
    else
        dummy := shard_unset_shard_procedure_global( up_user_name, up_proc_name, 'Y' );
    end if;
    
end UNSET_SHARD_PROCEDURE;

-- UNSET SHARD TABLE BY ID
procedure UNSET_SHARD_TABLE_BY_ID( shard_id in integer )
as
    dummy integer;
    test_property_val INTEGER;
begin
    SELECT memory_value1 INTO test_property_val 
    FROM x$property 
    WHERE name = '__SHARD_LOCAL_FORCE';
    
    if test_property_val > 0 then
        dummy := shard_unset_shard_table_by_id( shard_id );
    else
        dummy := shard_unset_shard_table_by_id_global( shard_id );
    end if;

end UNSET_SHARD_TABLE_BY_ID;

-- UNSET SHARD TABLE BY ID
procedure UNSET_SHARD_TABLE_BY_ID_LOCAL( shard_id in integer )
as
    dummy integer;
begin
    dummy := shard_unset_shard_table_by_id( shard_id );
end UNSET_SHARD_TABLE_BY_ID_LOCAL;


-- UNSET SHARD PROCEDURE BY ID
procedure UNSET_SHARD_PROCEDURE_BY_ID( shard_id in integer )
as
    dummy integer;
begin
    dummy := shard_unset_shard_procedure_by_id( shard_id );
end UNSET_SHARD_PROCEDURE_BY_ID;

-- EXECUTE ADHOC QUERY
procedure EXECUTE_IMMEDIATE( query     in varchar(65534),
                             node_name in varchar(12) default NULL )
as
    dummy integer;
    up_node_name varchar(12);
begin
    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    dummy := shard_execute_immediate( query, up_node_name );
end EXECUTE_IMMEDIATE;

-- CHECK SHARD DATA
procedure CHECK_DATA( user_name            in varchar(128),
                      table_name           in varchar(128),
                      additional_node_list in varchar(1000) default NULL )
as
  up_node_name varchar(12);
  up_user_name varchar(128);
  up_table_name varchar(128);
  key_column_name varchar(128);
  shard_info varchar(1000);
  node_name varchar(12);
  stmt varchar(4000);
  type my_cur is ref cursor;
  cur my_cur;
  record_count bigint;
  correct_count bigint;
  total_record_count bigint;
  total_incorrect_count bigint;

  type additional_node_arr_t is table of varchar(10) index by integer;
  additional_node_arr additional_node_arr_t;
  additional_node_arr_idx integer;
  additional_node_arr_length integer;
  additional_node_list_length integer;
  double_q_section integer;
  node_list varchar(1000);
begin

  if instr( user_name, chr(34) ) = 0 then
    up_user_name := upper( user_name );
  else
    up_user_name := replace2( user_name, chr(34) );
  end if;

  if instr( table_name, chr(34) ) = 0 then
    up_table_name := upper( table_name );
  else
    up_table_name := replace2( table_name, chr(34) );
  end if;

  if additional_node_list is not null then
    additional_node_list_length := length(additional_node_list);
    double_q_section := 0;
    additional_node_arr_idx := 0;
    additional_node_arr[additional_node_arr_idx] := '';

    for i in 1 .. additional_node_list_length loop
      if instr( substr( additional_node_list, i, 1 ), chr(34) ) = 1 then -- chr(34) = double quotation mark 
        double_q_section := abs(double_q_section-1);
      elseif instr( substr( additional_node_list, i, 1 ), chr(44) ) = 1 then -- chr(44) = comma 
        if double_q_section = 0 then
          additional_node_arr_idx := additional_node_arr_idx + 1;
          additional_node_arr[additional_node_arr_idx] := '';
          continue;
        end if;      
      end if;
      additional_node_arr[additional_node_arr_idx] := additional_Node_arr[additional_node_arr_idx]||substr( additional_node_list, i, 1 );
    end loop;

     for i in 0 .. additional_node_arr_idx loop
      additional_node_arr_length := length(additional_node_arr[i]);
      if ( instr( substr( additional_node_arr[i], 1, 1 ), chr(34) ) = 1 ) and 
         ( instr( substr( additional_node_arr[i], additional_node_arr_length, 1 ), chr(34) ) = 1 ) then
        additional_node_arr[i] := replace2( additional_node_arr[i], chr(34) );
      else
        additional_node_arr[i] := upper( additional_node_arr[i] );
      end if;
    end loop;
  end if;

  stmt :=
  'select /*+group bucket count(1000)*/ nvl2(sub_key_column_name,key_column_name||'', ''||sub_key_column_name,key_column_name), ''{"SplitMethod":''||nvl2(sub_split_method,''["''||split_method||''","''||sub_split_method||''"]'',''"''||split_method||''"'')||nvl2(n.node_name,'',"DefaultNode":"''||n.node_name||''"'',null)||'',"RangeInfo":[''||group_concat(''{"Value":''||decode(sub_value,''$$N/A'',''"''||value||''"'',''["''||value||''","''||sub_value||''"]'')||'',"Node":"''||v.node_name||''"}'','','')||'']}'' 
from (
  select o.key_column_name, o.sub_key_column_name, o.split_method, o.sub_split_method, o.default_node_id, r.value, r.sub_value, n.node_name, g.shard_meta_number smn
  from sys_shard.objects_ o, sys_shard.ranges_ r, sys_shard.nodes_ n, sys_shard.global_meta_info_ g
  where o.object_type=''T'' and o.user_name='''||up_user_name||''' and o.object_name='''||up_table_name||''' and o.split_method in (''H'',''R'',''L'') 
    and o.shard_id=r.shard_id and r.node_id=n.node_id
    and o.smn = g.shard_meta_number
    and r.smn = g.shard_meta_number
    and n.smn = g.shard_meta_number
) v left outer join ( select node_id, node_name
                        from sys_shard.nodes_ n, 
                             sys_shard.global_meta_info_ g
                       where n.smn = g.shard_meta_number ) n
    on default_node_id=n.node_id
group by key_column_name, sub_key_column_name, split_method, sub_split_method, n.node_name';
  --println(stmt);
  execute immediate stmt into key_column_name, shard_info;

  println('shard_key_column:'||key_column_name);
  println('shard_information:'||shard_info);
  println('');


  if additional_node_list is NULL then
    stmt := 'shard
      select shard_node_name() node_name, count(*) total_cnt, decode_count(1, shard_node_name(), home_name) correct_cnt
        from (select shard_condition('||key_column_name||', '''||shard_info||''') home_name from '||up_user_name||'.'||up_table_name||')';
  else
    stmt := 'select '||'''node[data(''''''||group_concat(distinct node_name,'''''','''''')||'''''')]''  node_list
               from ( select n.node_name 
                        from sys_shard.ranges_ r, 
                             sys_shard.nodes_ n, 
                             sys_shard.objects_ o,
                             sys_shard.global_meta_info_ g 
                       where r.node_id = n.node_id 
                         and r.shard_id = o.shard_id 
                         and r.smn = g.shard_meta_number
                         and n.smn = g.shard_meta_number
                         and o.smn = g.shard_meta_number
                         and o.user_name='''||up_user_name||'''
                         and o.object_name='''||up_table_name||'''';
     for i in 0 .. additional_node_arr_idx loop
       stmt := stmt||'union all select '''||additional_node_arr[i]||''' from dual ';
     end loop;

     stmt := stmt||')';

    execute immediate stmt into node_list;

    stmt := node_list||'select shard_node_name() node_name, count(*) total_cnt, decode_count(1, shard_node_name(), home_name) correct_cnt 
        from (select shard_condition('||key_column_name||', '''||shard_info||''') home_name from '||up_user_name||'.'||up_table_name||')';
  end if;

  --println(stmt);
  total_record_count := 0;
  total_incorrect_count := 0;
  open cur for stmt;
  loop
    fetch cur into node_name, record_count, correct_count;
    exit when cur%notfound;

    if instr( node_name, chr(34) ) = 0 then
      up_node_name := upper( node_name );
    else
      up_node_name := replace2( node_name, chr(34) );
    end if;

    total_record_count := total_record_count + record_count;
    total_incorrect_count := total_incorrect_count + (record_count-correct_count);
    println('node_name:'||up_node_name||', record_count:'||record_count||', correct_count:'||correct_count||', incorrect_count:'||(record_count-correct_count));
  end loop;
  close cur;
  println('');
  
  println('total_record_count   :'||total_record_count);
  println('total_incorrect_count:'||total_incorrect_count);

exception
when no_data_found then
  println('Non-shard objects or unsupported split method.');
--  println('SQLCODE: ' || SQLCODE); -- 에러코드 출력
--  println('SQLERRM: ' || SQLERRM); -- 에러메시지 출력
end CHECK_DATA;

-- REBUILD SHARD DATA FOR NODE
procedure REBUILD_DATA_NODE( user_name   in varchar(128),
                             table_name  in varchar(128),
                             node_name   in varchar(12),
                             batch_count in bigint default 0 )
as
  up_user_name varchar(128);
  up_table_name varchar(128);
  up_node_name varchar(12);
  key_column_name varchar(128);
  shard_info varchar(1000);
  stmt1 varchar(4000);
  stmt2 varchar(4000);
  batch_stmt varchar(100);
  total_count bigint;
  cur_count bigint;
  flag integer;
  cursor c1 is select autocommit_flag from v$session where id=session_id();
begin

  if instr( user_name, chr(34) ) = 0 then
    up_user_name := upper( user_name );
  else
    up_user_name := replace2( user_name, chr(34) );
  end if;

  if instr( table_name, chr(34) ) = 0 then
    up_table_name := upper( table_name );
  else
    up_table_name := replace2( table_name, chr(34) );
  end if;

  if instr( node_name, chr(34) ) = 0 then
    up_node_name := upper( node_name );
  else
    up_node_name := replace2( node_name, chr(34) );
  end if;

  if batch_count > 0 then
    open c1;
    fetch c1 into flag;
    close c1;

    if flag = 1 then
      raise_application_error(990000, 'Retry this statement in non-autocommit mode.');
    end if;
  end if;

  begin
    stmt1 :=
  'select /*+group bucket count(1000)*/ nvl2(sub_key_column_name,key_column_name||'', ''||sub_key_column_name,key_column_name), ''{"SplitMethod":''||nvl2(sub_split_method,''["''||split_method||''","''||sub_split_method||''"]'',''"''||split_method||''"'')||nvl2(n.node_name,'',"DefaultNode":"''||n.node_name||''"'',null)||'',"RangeInfo":[''||group_concat(''{"Value":''||decode(sub_value,''$$N/A'',''"''||value||''"'',''["''||value||''","''||sub_value||''"]'')||'',"Node":"''||v.node_name||''"}'','','')||'']}'' 
from (
  select o.key_column_name, o.sub_key_column_name, o.split_method, o.sub_split_method, o.default_node_id, r.value, r.sub_value, n.node_name, g.shard_meta_number smn
  from sys_shard.objects_ o, sys_shard.ranges_ r, sys_shard.nodes_ n, sys_shard.global_meta_info_ g
  where o.object_type=''T'' and o.user_name='''||up_user_name||''' and o.object_name='''||up_table_name||''' and o.split_method in (''H'',''R'',''L'') 
    and o.shard_id=r.shard_id and r.node_id=n.node_id
    and o.smn = g.shard_meta_number
    and r.smn = g.shard_meta_number
    and n.smn = g.shard_meta_number
) v left outer join ( select node_id, node_name
                        from sys_shard.nodes_ n, 
                             sys_shard.global_meta_info_ g
                       where n.smn = g.shard_meta_number ) n
    on default_node_id=n.node_id
group by key_column_name, sub_key_column_name, split_method, sub_split_method, n.node_name';
    --println(stmt1);
    execute immediate stmt1 into key_column_name, shard_info;
    exception
    when no_data_found then
      raise_application_error(990000, 'Non-shard objects or unsupported split method.');
    when others then
      raise;
  end;

  --println(key_column_name);
  --println(shard_info);

  if batch_count > 0 then
    batch_stmt := 'limit '||batch_count||' ';
  else
    batch_stmt := '';
  end if;

  stmt1 := '
  INSERT /*+DISABLE_INSERT_TRIGGER*/ INTO '||up_user_name||'.'||up_table_name||'
  NODE[DATA('''||up_node_name||''')] SELECT * FROM '||up_user_name||'.'||up_table_name||'
   WHERE shard_condition('||key_column_name||', '''||shard_info||''')!='''||up_node_name||''' '||batch_stmt||'
     FOR MOVE AND DELETE';
  --println(stmt1);

  stmt2 := '
   NODE[DATA('''||up_node_name||''')] SELECT COUNT(*) FROM '||up_user_name||'.'||up_table_name||'
   WHERE shard_condition('||key_column_name||', '''||shard_info||''')!='''||up_node_name||'''';
  --println(stmt2);

  total_count := 0;
  loop
    loop
      execute immediate stmt1;
      commit;
      cur_count := SQL%ROWCOUNT;
      exit when cur_count = 0;
      total_count := total_count + cur_count;
      if batch_count > 0 then
        println('['||to_char(sysdate, 'HH24:MI:SS')||'] '||total_count||' moved');
      else
        println(total_count||' moved');
      end if;
    end loop;
    execute immediate stmt2 into flag;
    exit when flag = 0;
  end loop;
  commit;

exception
when others then
  rollback;
  raise;
end REBUILD_DATA_NODE;

-- REBUILD SHARD DATA
procedure REBUILD_DATA( user_name            in varchar(128),
                        table_name           in varchar(128),
                        batch_count          in bigint default 0,
                        additional_node_list in varchar(1000) default NULL )
as
  up_user_name varchar(128);
  up_table_name varchar(128);
  type name_arr_t is table of varchar(10) index by integer;
  name_arr name_arr_t;
  node_name varchar(12);
  stmt1 varchar(4000);

  type additional_node_arr_t is table of varchar(10) index by integer;
  additional_node_arr additional_node_arr_t;
  additional_node_arr_idx integer;
  additional_node_arr_length integer;
  additional_node_list_length integer;
  double_q_section integer;
  stage integer := 0;
begin
  if instr( user_name, chr(34) ) = 0 then
    up_user_name := upper( user_name );
  else
    up_user_name := replace2( user_name, chr(34) );
  end if;

  if instr( table_name, chr(34) ) = 0 then
    up_table_name := upper( table_name );
  else
    up_table_name := replace2( table_name, chr(34) );
  end if;

  stmt1 := 'SELECT distinct node_name from (';

  if additional_node_list is not null then
    additional_node_list_length := length(additional_node_list);
    double_q_section := 0;
    additional_node_arr_idx := 0;
    additional_node_arr[additional_node_arr_idx] := '';

    for i in 1 .. additional_node_list_length loop
      if instr( substr( additional_node_list, i, 1 ), chr(34) ) = 1 then
        double_q_section := abs(double_q_section-1);
      elseif instr( substr( additional_node_list, i, 1 ), chr(44) ) = 1 then
        if double_q_section = 0 then
          additional_node_arr_idx := additional_node_arr_idx+1;
          additional_node_arr[additional_node_arr_idx] := '';
          continue;
        end if;      
      end if;
      additional_node_arr[additional_node_arr_idx] := additional_Node_arr[additional_node_arr_idx]||substr( additional_node_list, i, 1 );
    end loop;

    for i in 0 .. additional_node_arr_idx loop
      additional_node_arr_length := length(additional_node_arr[i]);
      if ( instr( substr( additional_node_arr[i], 1, 1 ), chr(34) ) = 1 ) and 
         ( instr( substr( additional_node_arr[i], additional_node_arr_length, 1 ), chr(34) ) = 1 ) then
        additional_node_arr[i] := replace2( additional_node_arr[i], chr(34) );
      else
        additional_node_arr[i] := upper( additional_node_arr[i] );
      end if;
        stmt1 := stmt1||' SELECT '''||additional_node_arr[i]||''' node_name FROM dual UNION ALL';
    end loop;
  end if;

  stmt1 := stmt1||'
  SELECT n.node_name node_name 
    FROM sys_shard.objects_ o, sys_shard.ranges_ r, sys_shard.nodes_ n, sys_shard.global_meta_info_ g
   WHERE o.split_method in (''H'',''R'',''L'') and r.node_id = n.node_id and o.shard_id = r.shard_id 
     AND o.user_name = '''|| up_user_name ||''' and o.object_name = '''|| up_table_name ||''' 
     AND o.smn = g.shard_meta_number
     AND r.smn = g.shard_meta_number
     AND n.smn = g.shard_meta_number
  UNION ALL
  SELECT n.node_name
    FROM sys_shard.objects_ o, sys_shard.nodes_ n, sys_shard.global_meta_info_ g 
   WHERE o.split_method in (''H'',''R'',''L'') and o.default_node_id = n.node_id
     AND o.user_name = '''|| up_user_name ||''' and o.object_name = '''|| up_table_name ||'''
     AND o.smn = g.shard_meta_number
     AND n.smn = g.shard_meta_number';
  stmt1 := stmt1||') ORDER BY node_name';
  --println(stmt1);
  execute immediate stmt1 bulk collect into name_arr;

  if name_arr.count() = 0 then
    raise_application_error(990000, 'Non-shard objects or unsupported split method.');
  end if;

  --println('['||to_char(sysdate, 'HH24:MI:SS')||'] node count: '||name_arr.count());
  stage := 1;
  for i in name_arr.first() .. name_arr.last() loop
    node_name := name_arr[i];
    stmt1 := 'select node_name 
                from sys_shard.nodes_ n, 
                     sys_shard.global_meta_info_ g 
               where n.smn = g.shard_meta_number 
                 and node_name = '''||node_name||'''';
    execute immediate stmt1 into node_name;
  end loop;
  stage := 0;
 
  for i in name_arr.first() .. name_arr.last() loop
    println('['||to_char(sysdate, 'HH24:MI:SS')||'] target node('||i||'/'||name_arr.count()||'): "'||name_arr[i]||'"');
    rebuild_data_node(user_name, table_name, name_arr[i], batch_count);
    commit;
  end loop;
  println('['||to_char(sysdate, 'HH24:MI:SS')||'] done.');

exception
when no_data_found then
  if stage = 1 then
    println('Invalid node name: '||node_name);
  else
    raise;
  end if;
when others then
  raise;
end REBUILD_DATA;

-- PURGE OLD SHARD META INFO
procedure PURGE_OLD_SHARD_META_INFO( smn in bigint default NULL )
as
    dummy integer;
begin
    dummy := shard_unset_old_smn( smn );
end PURGE_OLD_SHARD_META_INFO;


-- SET SHARD TABLE SHARDKEY
procedure SET_SHARD_TABLE_SHARDKEY( user_name                   in varchar(128),
                                    table_name                  in varchar(128),
                                    partition_node_list         in varchar(32000),
                                    method_for_irregular        in char(1) default 'E',
                                    replication_parallel_count  in integer default 1 )
as
    dummy                   integer;
    up_user_name            varchar(128);
    up_table_name           varchar(128);
    up_partition_node_list  varchar(32000);
    up_method_for_irregular char(1);
begin
    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( table_name, chr(34) ) = 0 then
        up_table_name := upper( table_name );
    else
        up_table_name := replace2( table_name, chr(34) );
    end if;

    if instr( partition_node_list, chr(34) ) = 0 then
        up_partition_node_list := upper( partition_node_list );
    else
        up_partition_node_list := replace2( partition_node_list, chr(34) );
    end if;

    if instr( method_for_irregular, chr(34) ) = 0 then
        up_method_for_irregular := upper( method_for_irregular );
    else
        up_method_for_irregular := replace2( method_for_irregular, chr(34) );
    end if;

    dummy := shard_set_shard_table_shardkey( up_user_name,
                                             up_table_name,
                                             up_partition_node_list,
                                             up_method_for_irregular,
                                             replication_parallel_count );
end SET_SHARD_TABLE_SHARDKEY;

-- SET SHARD TABLE SOLO
procedure SET_SHARD_TABLE_SOLO( user_name                   in varchar(128),
                                table_name                  in varchar(128),
                                node_name                   in varchar(12),
                                method_for_irregular        in char(1) default 'E',
                                replication_parallel_count  in integer default 1 )
as
    dummy                   integer;
    up_user_name            varchar(128);
    up_table_name           varchar(128);
    up_node_name            varchar(12);
    up_method_for_irregular char(1);
begin
    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( table_name, chr(34) ) = 0 then
        up_table_name := upper( table_name );
    else
        up_table_name := replace2( table_name, chr(34) );
    end if;

    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    if instr( method_for_irregular, chr(34) ) = 0 then
        up_method_for_irregular := upper( method_for_irregular );
    else
        up_method_for_irregular := replace2( method_for_irregular, chr(34) );
    end if;

    dummy := shard_set_shard_table_solo( up_user_name,
                                         up_table_name,
                                         up_node_name,
                                         up_method_for_irregular,
                                         replication_parallel_count );
end SET_SHARD_TABLE_SOLO;

-- SET SHARD TABLE CLONE
procedure SET_SHARD_TABLE_CLONE( user_name                   in varchar(128),
                                 table_name                  in varchar(128),
                                 reference_node_name         in varchar(12) default NULL,
                                 replication_parallel_count  in integer default 1 )
as
    dummy                   integer;
    up_user_name            varchar(128);
    up_table_name           varchar(128);
    up_reference_node_name  varchar(12);
begin
    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( table_name, chr(34) ) = 0 then
        up_table_name := upper( table_name );
    else
        up_table_name := replace2( table_name, chr(34) );
    end if;

    if instr( reference_node_name, chr(34) ) = 0 then
        up_reference_node_name := upper( reference_node_name );
    else
        up_reference_node_name := replace2( reference_node_name, chr(34) );
    end if;

    dummy := shard_set_shard_table_clone( up_user_name,
                                          up_table_name,
                                          up_reference_node_name,
                                          replication_parallel_count );
end SET_SHARD_TABLE_CLONE;

-- SET_SHARD_PROCEDURE_SHARDKEY
procedure SET_SHARD_PROCEDURE_SHARDKEY( user_name          in varchar(128),
                                        proc_name          in varchar(128),
                                        split_method       in varchar(1),
                                        key_parameter_name in varchar(128),
                                        value_node_list    in varchar(32000),
                                        default_node_name  in varchar(12) default NULL,
                                        proc_replace       in char(1) default 'N' )
as
    dummy integer;
    up_user_name varchar(128);
    up_proc_name varchar(128);
    up_split_method varchar(1);
    up_key_parameter_name varchar(128);
    up_value_node_list  varchar(32000);
    up_default_node_name varchar(12);
    up_proc_replace varchar(1);
begin
    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( proc_name, chr(34) ) = 0 then
        up_proc_name := upper( proc_name );
    else
        up_proc_name := replace2( proc_name, chr(34) );
    end if;

    up_split_method := upper( split_method );

    if instr( key_parameter_name, chr(34) ) = 0 then
        up_key_parameter_name := upper( key_parameter_name );
    else
        up_key_parameter_name := replace2( key_parameter_name, chr(34) );
    end if;

    if instr( value_node_list, chr(34) ) = 0 then
        up_value_node_list := upper( value_node_list );
    else
        up_value_node_list := replace2( value_node_list, chr(34) );
    end if;

    if instr( default_node_name, chr(34) ) = 0 then
        up_default_node_name := upper( default_node_name );
    else
        up_default_node_name := replace2( default_node_name, chr(34) );
    end if;

    if instr( proc_replace, chr(34) ) = 0 then
        up_proc_replace := upper( proc_replace );
    else
        up_proc_replace := replace2( proc_replace, chr(34) );
    end if;

    dummy := shard_set_shard_procedure_shardkey( up_user_name,
                                                 up_proc_name,
                                                 up_split_method,
                                                 up_key_parameter_name,
                                                 up_value_node_list,
                                                 up_default_node_name,
                                                 up_proc_replace );

end SET_SHARD_PROCEDURE_SHARDKEY;


-- SET_SHARD_PROCEDURE_SOLO
procedure SET_SHARD_PROCEDURE_SOLO( user_name     in varchar(128),
                                    proc_name     in varchar(128),
                                    node_name     in varchar(12),
                                    proc_replace  in char(1) default 'N' )
as
    dummy integer;
    up_user_name varchar(128);
    up_proc_name varchar(128);
    up_node_name varchar(12);
    up_proc_replace varchar(1);
begin
    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( proc_name, chr(34) ) = 0 then
        up_proc_name := upper( proc_name );
    else
        up_proc_name := replace2( proc_name, chr(34) );
    end if;

    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    if instr( proc_replace, chr(34) ) = 0 then
        up_proc_replace := upper( proc_replace );
    else
        up_proc_replace := replace2( proc_replace, chr(34) );
    end if;

    dummy := shard_set_shard_procedure_solo( up_user_name,
                                             up_proc_name,
                                             up_node_name,
                                             up_proc_replace );

end SET_SHARD_PROCEDURE_SOLO;

-- SET_SHARD_PROCEDURE_CLONE
procedure SET_SHARD_PROCEDURE_CLONE( user_name     in varchar(128),
                                     proc_name     in varchar(128),                                 
                                     proc_replace  in char(1) default 'N' )
as
    dummy integer;
    up_user_name varchar(128);
    up_proc_name varchar(128);
    up_proc_replace varchar(1);
begin
    if instr( user_name, chr(34) ) = 0 then
        up_user_name := upper( user_name );
    else
        up_user_name := replace2( user_name, chr(34) );
    end if;

    if instr( proc_name, chr(34) ) = 0 then
        up_proc_name := upper( proc_name );
    else
        up_proc_name := replace2( proc_name, chr(34) );
    end if;

    if instr( proc_replace, chr(34) ) = 0 then
        up_proc_replace := upper( proc_replace );
    else
        up_proc_replace := replace2( proc_replace, chr(34) );
    end if;

    dummy := shard_set_shard_procedure_clone( up_user_name,
                                              up_proc_name,
                                              up_proc_replace );

end SET_SHARD_PROCEDURE_CLONE;

-- SET REPLICASCN
procedure SET_REPLICASCN()
as
    dummy integer;
begin
    dummy := shard_set_replicascn();
end SET_REPLICASCN;

-- TOUCH META
procedure TOUCH_META()
as
    dummy integer;
begin
    dummy := shard_touch_meta();
end TOUCH_META;

end DBMS_SHARD;
/
