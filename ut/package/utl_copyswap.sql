/***********************************************************************
 * Copyright 1999-2018, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

CREATE OR REPLACE PACKAGE utl_copyswap AUTHID CURRENT_USER IS

PROCEDURE check_precondition( source_user_name  IN VARCHAR(128),
                              source_table_name IN VARCHAR(128) );
PRAGMA RESTRICT_REFERENCES( check_precondition, WNDS );

PROCEDURE copy_table_schema( target_user_name   IN VARCHAR(128),
                             target_table_name  IN VARCHAR(128),
                             source_user_name   IN VARCHAR(128),
                             source_table_name  IN VARCHAR(128) );

PROCEDURE replicate_table( replication_name         IN VARCHAR(35),
                           target_user_name         IN VARCHAR(128),
                           target_table_name        IN VARCHAR(128),
                           source_user_name         IN VARCHAR(128),
                           source_table_name        IN VARCHAR(128),
                           sync_parallel_factor     IN INTEGER DEFAULT 8,
                           receiver_applier_count   IN INTEGER DEFAULT 8 );

PROCEDURE swap_table( replication_name                  IN VARCHAR(35),
                      target_user_name                  IN VARCHAR(128),
                      target_table_name                 IN VARCHAR(128),
                      source_user_name                  IN VARCHAR(128),
                      source_table_name                 IN VARCHAR(128),
                      force_to_rename_encrypt_column    IN BOOLEAN DEFAULT FALSE,
                      ignore_foreign_key_child          IN BOOLEAN DEFAULT FALSE );

PROCEDURE finish( replication_name     IN VARCHAR(35),
                  target_user_name     IN VARCHAR(128),
                  target_table_name    IN VARCHAR(128),
                  print_all_errors     IN BOOLEAN DEFAULT FALSE );

END utl_copyswap;
/

CREATE OR REPLACE PUBLIC SYNONYM utl_copyswap FOR sys.utl_copyswap;

