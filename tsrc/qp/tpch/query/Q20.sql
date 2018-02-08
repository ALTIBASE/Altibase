ALTER SESSION SET EXPLAIN PLAN = ON;
ALTER SYSTEM SET TRCLOG_EXPLAIN_GRAPH = 1;

SET TIMING ON;
SET TIMESCALE MILSEC;

SELECT S_NAME,
       S_ADDRESS
FROM SUPPLIER,
     NATION
WHERE S_SUPPKEY IN ( SELECT PS_SUPPKEY
                     FROM PARTSUPP
                     WHERE PS_PARTKEY IN ( SELECT P_PARTKEY
                                           FROM	PART
                                           --WHERE P_NAME LIKE 'forest%' )
                                           WHERE SUBSTRING(P_NAME, 1, 6) = 'forest' )
                     AND PS_AVAILQTY > 
                         ( SELECT	0.5 * SUM(L_QUANTITY)
                           FROM LINEITEM
                           WHERE L_PARTKEY = PS_PARTKEY
                           AND L_SUPPKEY = PS_SUPPKEY
                           AND L_SHIPDATE >= DATE'01-Jan-1994' 
                           AND L_SHIPDATE < 
                               ADD_MONTHS(DATE'01-Jan-1994',12)
                         )
                   )
  AND S_NATIONKEY = N_NATIONKEY
  AND N_NAME = 'CANADA'
ORDER BY S_NAME;

ALTER SYSTEM SET TRCLOG_EXPLAIN_GRAPH = 0;
