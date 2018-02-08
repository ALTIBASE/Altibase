/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

create or replace package dbms_stats authid current_user 
as

procedure gather_system_stats();

procedure gather_database_stats( estimate_percent    in float   default 0,
                                 degree              in integer default 0,
                                 gather_system_stats in boolean default false,
                                 no_invalidate       in boolean default false );

procedure gather_table_stats( ownname          in varchar(128),
                              tabname          in varchar(128),
                              partname         in varchar(128) default null,
                              estimate_percent in float   default 0,
                              degree           in integer default 0,
                              no_invalidate    in boolean default false );

procedure gather_index_stats( ownname          in varchar(128),
                              idxname          in varchar(128),
                              estimate_percent in float   default 0,
                              degree           in integer default 0,
                              no_invalidate    in boolean default false );

procedure set_system_stats( statname  in varchar(100),
                            statvalue in double );

procedure set_table_stats( ownname        in varchar(128),
                           tabname        in varchar(128),
                           partname       in varchar(128) default null,
                           numrow         in bigint  default null,
                           numpage        in bigint  default null,
                           avgrlen        in bigint  default null,
                           onerowreadtime in double  default null,
                           no_invalidate  in boolean default false );

procedure set_column_stats( ownname       in varchar(128),
                            tabname       in varchar(128),
                            colname       in varchar(128),
                            partname      in varchar(128) default null,
                            numdist       in bigint  default null,
                            numnull       in bigint  default null,
                            avgclen       in bigint  default null,
                            minvalue      in varchar(48) default null,
                            maxvalue      in varchar(48) default null,
                            no_invalidate in boolean default false );

procedure set_index_stats( ownname          in varchar(128),
                           idxname          in varchar(128),
                           keycount         in bigint  default null,
                           numpage          in bigint  default null,
                           numdist          in bigint  default null,
                           clusteringfactor in bigint  default null,
                           indexheight      in bigint  default null,
                           avgslotcnt       in bigint  default null,
                           no_invalidate    in boolean default false );

procedure delete_system_stats();

procedure delete_table_stats( ownname         in varchar(128),
                              tabname         in varchar(128),
                              partname        in varchar(128) default null,
                              cascade_part    in boolean default true,
                              cascade_columns in boolean default true,
                              cascade_indexes in boolean default true,
                              no_invalidate   in boolean default false );

procedure delete_column_stats( ownname       in varchar(128),
                               tabname       in varchar(128),
                               colname       in varchar(128),
                               partname      in varchar(128) default null,
                               cascade_part  in boolean default true,
                               no_invalidate in boolean default false );

procedure delete_index_stats( ownname       in varchar(128),
                              idxname       in varchar(128),
                              no_invalidate in boolean default false );

procedure delete_database_stats( no_invalidate in boolean default false );

procedure get_system_stats( statname  in varchar(100),
                            statvalue out double );

procedure get_table_stats( ownname        in varchar(128),
                           tabname        in varchar(128),
                           partname       in varchar(128) default null,
                           numrow         out bigint,
                           numpage        out bigint,
                           avgrlen        out bigint,
                           cachedpage     out bigint,
                           onerowreadtime out double );

procedure get_column_stats( ownname  in varchar(128),
                            tabname  in varchar(128),
                            colname  in varchar(128),
                            partname in varchar(128) default null,
                            numdist  out bigint,
                            numnull  out bigint,
                            avgclen  out bigint,
                            minvalue out varchar(48),
                            maxvalue out varchar(48) );

procedure get_index_stats( ownname          in varchar(128),
                           idxname          in varchar(128),
                           partname         in varchar(128) default null,
                           keycount         out bigint,
                           numpage          out bigint,
                           numdist          out bigint,
                           clusteringfactor out bigint,
                           indexheight      out bigint,
                           cachedpage       out bigint,
                           avgslotcnt       out bigint );

procedure copy_table_stats( ownname     in varchar(128),
                            tabname     in varchar(128),
                            srcpartname in varchar(128),
                            dstpartname in varchar(128) );

procedure set_primary_key_stats( ownname          in varchar(128),
                                 tabname          in varchar(128),
                                 keycount         in bigint  default null,
                                 numpage          in bigint  default null,
                                 numdist          in bigint  default null,
                                 clusteringfactor in bigint  default null,
                                 indexheight      in bigint  default null,
                                 avgslotcnt       in bigint  default null,
                                 no_invalidate    in boolean default false );

procedure set_unique_key_stats( ownname          in varchar(128),
                                tabname          in varchar(128),
                                colnamelist      in varchar(32000),
                                keycount         in bigint  default null,
                                numpage          in bigint  default null,
                                numdist          in bigint  default null,
                                clusteringfactor in bigint  default null,
                                indexheight      in bigint  default null,
                                avgslotcnt       in bigint  default null,
                                no_invalidate    in boolean default false );
end dbms_stats;
/

create public synonym dbms_stats for sys.dbms_stats;
grant execute on sys.dbms_stats to public;
