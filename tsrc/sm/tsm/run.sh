#! /bin/sh
TSM_TABLE_TYPE="DISK"; export TSM_TABLE_TYPE
sh init.sh
ddd tsm_mixed/tsm_mixed 
#tsm_mixed/tsm_mixed -s tsm_LOB_tablecursor1
#tsm_mixed/tsm_mixed -s tsm_LOB_function
#tsm_mixed/tsm_mixed -s tsm_LOB_recovery2
#tsm_mixed/tsm_mixed -s tsm_LOB_stress1
#tsm_mixed/tsm_mixed -s tsm_LOB_stress2
