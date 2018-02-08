rm run.out
is -f schema.sql
make -f MakeInsert.mk clean;  make -f MakeInsert.mk
make -f MakeUpdate.mk clean;  make -f MakeUpdate.mk
make -f MakeSelect.mk clean;  make -f MakeSelect.mk
make -f MakeDelete.mk clean;  make -f MakeDelete.mk

insertWKT | tee -a run.out
updateWKT | tee -a run.out
selectWKT | tee -a run.out
deleteWKT | tee -a run.out

is -f drop_schema.sql
diff run.lst run.out > run.diff

