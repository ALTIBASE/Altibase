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

-- SET DATA NODE
procedure SET_NODE( node_name         in varchar(40),
                    host_ip           in varchar(16),
                    port_no           in integer,
                    alternate_host_ip in varchar(16) default NULL,
                    alternate_port_no in integer default NULL )
as
    dummy integer;
    up_node_name varchar(40);
begin
    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    dummy := shard_set_node(up_node_name, host_ip, port_no, alternate_host_ip, alternate_port_no);
end SET_NODE;

-- RESET DATA NODE
procedure RESET_NODE( node_name in varchar(40),
                      host_ip   in varchar(16),
                      port_no   in integer )
as
    dummy integer;
    up_node_name varchar(40);
begin
    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    dummy := shard_reset_node(up_node_name, host_ip, port_no, 0);
end RESET_NODE;

-- RESET ALTERNATE DATA NODE
procedure RESET_ALTERNATE_NODE( node_name in varchar(40),
                                host_ip   in varchar(16),
                                port_no   in integer )
as
    dummy integer;
    up_node_name varchar(40);
begin
    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    dummy := shard_reset_node(up_node_name, host_ip, port_no, 1);
end RESET_ALTERNATE_NODE;

-- UNSET DATA NODE
procedure UNSET_NODE( node_name in varchar(40) )
as
    dummy integer;
    up_node_name varchar(40);
begin
    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    dummy := shard_unset_node( up_node_name );
end UNSET_NODE;

-- SET SHARD TABLE INFO
procedure SET_SHARD_TABLE( user_name         in varchar(128),
                           table_name        in varchar(128),
                           split_method      in varchar(1),
                           key_column_name   in varchar(128) default NULL,
                           default_node_name in varchar(40) default NULL )
as
    dummy integer;
    up_user_name varchar(128);
    up_table_name varchar(128);
    up_split_method varchar(1);
    up_key_column_name varchar(128);
    up_default_node_name varchar(40);
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

    if instr( default_node_name, chr(34) ) = 0 then
        up_default_node_name := upper( default_node_name );
    else
        up_default_node_name := replace2( default_node_name, chr(34) );
    end if;

    dummy := shard_set_shard_table( up_user_name,
                                    up_table_name,
                                    up_split_method,
                                    up_key_column_name,
                                    NULL,                  -- sub_split_method
                                    NULL,                  -- sub_key_column_name
                                    up_default_node_name );
end SET_SHARD_TABLE;

--SET COMPOSITE SHARD TABLE INFO
procedure SET_SHARD_TABLE_COMPOSITE( user_name           in varchar(128),
                                     table_name          in varchar(128),
                                     split_method        in varchar(1),
                                     key_column_name     in varchar(128),
                                     sub_split_method    in varchar(1),
                                     sub_key_column_name in varchar(128),
                                     default_node_name   in varchar(40) default NULL )
as
    dummy integer;
    up_user_name varchar(128);
    up_table_name varchar(128);
    up_split_method varchar(1);
    up_key_column_name varchar(128);
    up_sub_split_method varchar(1);
    up_sub_key_column_name varchar(128);
    up_default_node_name varchar(40);
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

    dummy := shard_set_shard_table( up_user_name,
                                    up_table_name,
                                    up_split_method,
                                    up_key_column_name,
                                    up_sub_split_method,
                                    up_sub_key_column_name,
                                    up_default_node_name );
end SET_SHARD_TABLE_COMPOSITE;

-- SET SHARD PROCEDURE INFO
procedure SET_SHARD_PROCEDURE( user_name          in varchar(128),
                               proc_name          in varchar(128),
                               split_method       in varchar(1),
                               key_parameter_name in varchar(128) default NULL,
                               default_node_name  in varchar(40) default NULL )
as
    dummy integer;
    up_user_name varchar(128);
    up_proc_name varchar(128);
    up_split_method varchar(1);
    up_key_parameter_name varchar(128);
    up_default_node_name varchar(40);
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

    dummy := shard_set_shard_procedure( up_user_name,
                                        up_proc_name,
                                        up_split_method,
                                        up_key_parameter_name,
                                        NULL,                  -- sub_split_method
                                        NULL,                  -- sub_key_parameter_name
                                        up_default_node_name );
end SET_SHARD_PROCEDURE;

-- SET COMPOSITE SHARD PROCEDURE INFO
procedure SET_SHARD_PROCEDURE_COMPOSITE( user_name              in varchar(128),
                                         proc_name              in varchar(128),
                                         split_method           in varchar(1),
                                         key_parameter_name     in varchar(128),
                                         sub_split_method       in varchar(1),
                                         sub_key_parameter_name in varchar(128),
                                         default_node_name      in varchar(40) default NULL )
as
    dummy integer;
    up_user_name varchar(128);
    up_proc_name varchar(128);
    up_split_method varchar(1);
    up_key_parameter_name varchar(128);
    up_sub_split_method varchar(1);
    up_sub_key_parameter_name varchar(128);
    up_default_node_name varchar(40);
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

    dummy := shard_set_shard_procedure( up_user_name,
                                        up_proc_name,
                                        up_split_method,
                                        up_key_parameter_name,
                                        up_sub_split_method,
                                        up_sub_key_parameter_name,
                                        up_default_node_name );
end SET_SHARD_PROCEDURE_COMPOSITE;

-- SET SHARD HASH
procedure SET_SHARD_HASH( user_name   in varchar(128),
                          object_name in varchar(128),
                          hash_max    in integer,
                          node_name   in varchar(40) )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_node_name varchar(40);

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

    dummy := shard_set_shard_range( up_user_name,
                                    up_object_name,
                                    hash_max,
                                    NULL,          -- sub_value
                                    up_node_name,
                                    'H' );
end SET_SHARD_HASH;

-- SET SHARD RANGE
procedure SET_SHARD_RANGE( user_name   in varchar(128),
                           object_name in varchar(128),
                           value_max   in varchar(100),
                           node_name   in varchar(40) )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_node_name varchar(40);

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

    dummy := shard_set_shard_range( up_user_name,
                                    up_object_name,
                                    value_max,
                                    NULL,          -- sub_value
                                    up_node_name,
                                    'R' );
end SET_SHARD_RANGE;

-- SET SHARD LIST
procedure SET_SHARD_LIST( user_name   in varchar(128),
                          object_name in varchar(128),
                          value       in varchar(100),
                          node_name   in varchar(40) )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_node_name varchar(40);

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

    dummy := shard_set_shard_range( up_user_name,
                                    up_object_name,
                                    value,
                                    NULL,          -- sub_value
                                    up_node_name,
                                    'L' );
end SET_SHARD_LIST;

-- SET SHARD CLONE
procedure SET_SHARD_CLONE( user_name   in varchar(128),
                           object_name in varchar(128),
                           node_name   in varchar(40) )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_node_name varchar(40);

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

    dummy := shard_set_shard_clone( up_user_name,
                                    up_object_name,
                                    up_node_name );
end SET_SHARD_CLONE;

-- SET SHARD SOLO 
procedure SET_SHARD_SOLO( user_name   in varchar(128),
                          object_name in varchar(128),
                          node_name   in varchar(40) )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_node_name varchar(40);

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

    dummy := shard_set_shard_solo( up_user_name,
                                   up_object_name,
                                   up_node_name );
end SET_SHARD_SOLO;

-- SET SHARD COMPOSITE
procedure SET_SHARD_COMPOSITE( user_name   in varchar(128),
                               object_name in varchar(128),
                               value       in varchar(100),
                               sub_value   in varchar(100),
                               node_name   in varchar(40) )
as
    dummy integer;
    up_user_name varchar(128);
    up_object_name varchar(128);
    up_node_name varchar(40);

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

    dummy := shard_set_shard_range( up_user_name,
                                    up_object_name,
                                    value,
                                    sub_value,
                                    up_node_name,
                                    'P' );
end SET_SHARD_COMPOSITE;

-- UNSET SHARD TABLE
procedure UNSET_SHARD_TABLE( user_name  in varchar(128),
                             table_name in varchar(128) )
as
    dummy integer;
    up_user_name varchar(128);
    up_table_name varchar(128);

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

    dummy := shard_unset_shard_table( up_user_name, up_table_name );
end UNSET_SHARD_TABLE;

-- UNSET SHARD PROCEDURE
procedure UNSET_SHARD_PROCEDURE( user_name in varchar(128),
                                 proc_name in varchar(128) )
as
    dummy integer;
    up_user_name varchar(128);
    up_proc_name varchar(128);

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

    dummy := shard_unset_shard_procedure( up_user_name, up_proc_name );
end UNSET_SHARD_PROCEDURE;

-- UNSET SHARD TABLE BY ID
procedure UNSET_SHARD_TABLE_BY_ID( shard_id in integer )
as
    dummy integer;
begin
    dummy := shard_unset_shard_table_by_id( shard_id );
end UNSET_SHARD_TABLE_BY_ID;

-- UNSET SHARD PROCEDURE BY ID
procedure UNSET_SHARD_PROCEDURE_BY_ID( shard_id in integer )
as
    dummy integer;
begin
    dummy := shard_unset_shard_procedure_by_id( shard_id );
end UNSET_SHARD_PROCEDURE_BY_ID;

-- EXECUTE ADHOC QUERY
procedure EXECUTE_IMMEDIATE( query     in varchar(65534),
                             node_name in varchar(40) default NULL )
as
    dummy integer;
    up_node_name varchar(40);
begin
    if instr( node_name, chr(34) ) = 0 then
        up_node_name := upper( node_name );
    else
        up_node_name := replace2( node_name, chr(34) );
    end if;

    dummy := shard_execute_immediate( query, up_node_name );
end EXECUTE_IMMEDIATE;

-- CHECK SHARD DATA
procedure CHECK_DATA( user_name  in varchar(128),
                      table_name in varchar(128) )
as
  up_node_name varchar(40);
  up_user_name varchar(128);
  up_table_name varchar(128);
  key_column_name varchar(128);
  shard_info varchar(1000);
  node_name varchar(40);
  stmt varchar(2000);
  type my_cur is ref cursor;
  cur my_cur;
  record_count bigint;
  correct_count bigint;
  total_record_count bigint;
  total_incorrect_count bigint;
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

  stmt :=
  'select /*+group bucket count(1000)*/ nvl2(sub_key_column_name,key_column_name||'', ''||sub_key_column_name,key_column_name), ''{"SplitMethod":''||nvl2(sub_split_method,''["''||split_method||''","''||sub_split_method||''"]'',''"''||split_method||''"'')||nvl2(n.node_name,'',"DefaultNode":"''||n.node_name||''"'',null)||'',"RangeInfo":[''||group_concat(''{"Value":''||decode(sub_value,''$$N/A'',''"''||value||''"'',''["''||value||''","''||sub_value||''"]'')||'',"Node":"''||v.node_name||''"}'','','')||'']}'' 
from (
  select o.key_column_name, o.sub_key_column_name, o.split_method, o.sub_split_method, o.default_node_id, r.value, r.sub_value, n.node_name
  from sys_shard.objects_ o, sys_shard.ranges_ r, sys_shard.nodes_ n
  where o.object_type=''T'' and o.user_name='''||up_user_name||''' and o.object_name='''||up_table_name||''' and o.split_method in (''H'',''R'',''L'') 
    and o.shard_id=r.shard_id and r.node_id=n.node_id
) v left outer join sys_shard.nodes_ n on default_node_id=n.node_id
group by key_column_name, sub_key_column_name, split_method, sub_split_method, n.node_name';
  --println(stmt);
  execute immediate stmt into key_column_name, shard_info;

  println('shard_key_column:'||key_column_name);
  println('shard_information:'||shard_info);
  println('');

  stmt := 'shard 
  select shard_node_name() node_name, count(*) total_cnt, decode_count(1, shard_node_name(), home_name) correct_cnt 
    from (select shard_condition('||key_column_name||', '''||shard_info||''') home_name from '||up_user_name||'.'||up_table_name||')';
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
                             node_name   in varchar(40),
                             batch_count in bigint default 0 )
as
  up_user_name varchar(128);
  up_table_name varchar(128);
  up_node_name varchar(40);
  key_column_name varchar(128);
  shard_info varchar(1000);
  stmt1 varchar(2000);
  stmt2 varchar(2000);
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
  select o.key_column_name, o.sub_key_column_name, o.split_method, o.sub_split_method, o.default_node_id, r.value, r.sub_value, n.node_name
  from sys_shard.objects_ o, sys_shard.ranges_ r, sys_shard.nodes_ n
  where o.object_type=''T'' and o.user_name='''||up_user_name||''' and o.object_name='''||up_table_name||''' and o.split_method in (''H'',''R'',''L'') 
    and o.shard_id=r.shard_id and r.node_id=n.node_id
) v left outer join sys_shard.nodes_ n on default_node_id=n.node_id
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
procedure REBUILD_DATA( user_name   in varchar(128),
                        table_name  in varchar(128),
                        batch_count in bigint default 0 )
as
  up_user_name varchar(128);
  up_table_name varchar(128);
  type name_arr_t is table of varchar(40) index by integer;
  name_arr name_arr_t;
  stmt1 varchar(2000);
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

  stmt1 := '
  SELECT n.node_name node_name 
    FROM sys_shard.objects_ o, sys_shard.ranges_ r, sys_shard.nodes_ n 
   WHERE o.split_method in (''H'',''R'',''L'') and r.node_id = n.node_id and o.shard_id = r.shard_id 
     AND o.user_name = '''|| up_user_name ||''' and o.object_name = '''|| up_table_name ||''' 
  UNION
  SELECT n.node_name
    FROM sys_shard.objects_ o, sys_shard.nodes_ n 
   WHERE o.split_method in (''H'',''R'',''L'') and o.default_node_id = n.node_id
     AND o.user_name = '''|| up_user_name ||''' and o.object_name = '''|| up_table_name ||'''
   ORDER BY node_name';

  --println(stmt1);
  execute immediate stmt1 bulk collect into name_arr;

  if name_arr.count() = 0 then
    raise_application_error(990000, 'Non-shard objects or unsupported split method.');
  end if;

  --println('['||to_char(sysdate, 'HH24:MI:SS')||'] node count: '||name_arr.count());
 
  for i in name_arr.first() .. name_arr.last() loop
    println('['||to_char(sysdate, 'HH24:MI:SS')||'] target node('||i||'/'||name_arr.count()||'): "'||name_arr[i]||'"');
    rebuild_data_node(user_name, table_name, name_arr[i], batch_count);
    commit;
  end loop;
  println('['||to_char(sysdate, 'HH24:MI:SS')||'] done.');

exception
when others then
  raise;
end REBUILD_DATA;

end DBMS_SHARD;
/
