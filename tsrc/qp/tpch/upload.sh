#!/bin/bash
echo "TPCH:TABLE:REGION"
iloader in -S 127.0.0.1 -U SYS -P MANAGER -d region.tbl -f region.fmt   -t '|' -log region.log -array 1000 &

echo "TPCH:TABLE:NATION"
iloader in -S 127.0.0.1 -U SYS -P MANAGER -d nation.tbl -f nation.fmt   -t '|' -log nation.log -array 1000 &

echo "TPCH:TABLE:SUPPLIER"
iloader in -S 127.0.0.1 -U SYS -P MANAGER -d supplier.tbl -f supplier.fmt -t '|' -log supplier.log -array 1000 &

echo "TPCH:TABLE:CUSTOMER"
iloader in -S 127.0.0.1 -U SYS -P MANAGER -d customer.tbl -f customer.fmt -t '|' -log customer.log -array 1000 &

echo "TPCH:TABLE:PART"
iloader in -S 127.0.0.1 -U SYS -P MANAGER -d part.tbl -f part.fmt     -t '|' -log part.log -array 1000 &

echo "TPCH:TABLE:PARTSUPP"
iloader in -S 127.0.0.1 -U SYS -P MANAGER -d partsupp.tbl -f partsupp.fmt -t '|' -log partsupp.log -array 1000 &

echo "TPCH:TABLE:ORDERS"
iloader in -S 127.0.0.1 -U SYS -P MANAGER -d orders.tbl -f orders.fmt   -t '|' -log orders.log -array 1000 &

echo "TPCH:TABLE:LINEITEM"
iloader in -S 127.0.0.1 -U SYS -P MANAGER -d lineitem.tbl -f lineitem.fmt -t '|' -log lineitem.log -array 1000 &

wait
