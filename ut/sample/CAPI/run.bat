@echo off

setlocal

rm -rf run.out run.diff
make clean
make

echo ========>> run.out
echo DEMO EX1>> run.out
echo ========>> run.out
call is -silent -f demo_ex1.sql
demo_ex1 >> run.out

echo ========>> run.out
echo DEMO EX2>> run.out
echo ========>> run.out
call is -silent -f demo_ex2.sql
demo_ex2 >> run.out

echo ========>> run.out
echo DEMO EX3>> run.out
echo ========>> run.out
call is -silent -f demo_ex3.sql
demo_ex3 >> run.out

echo ========>> run.out
echo DEMO EX4>> run.out
echo ========>> run.out
call is -silent -f demo_ex4.sql
demo_ex4 >> run.out

echo ========>> run.out
echo DEMO EX5>> run.out
echo ========>> run.out
call is -silent -f demo_ex5.sql
demo_ex5 >> run.out

echo ========>> run.out
echo DEMO EX6>> run.out
echo ========>> run.out
call is -silent -f demo_ex6.sql
demo_ex6 >> run.out

echo ========>> run.out
echo DEMO EX7>> run.out
echo ========>> run.out
call is -silent -f demo_ex7.sql
demo_ex7 >> run.out

echo ==========>> run.out
echo DEMO META1>> run.out
echo ==========>> run.out
demo_meta1 >> run.out

echo ==========>> run.out
echo DEMO META2>> run.out
echo ==========>> run.out
call is -silent -f demo_meta2.sql
demo_meta2 >> run.out

echo ==========>> run.out
echo DEMO META8>> run.out
echo ==========>> run.out
call is -silent -f demo_meta8.sql
demo_meta8 >> run.out

echo ==========>> run.out
echo DEMO TRAN1>> run.out
echo ==========>> run.out
call is -silent -f demo_tran1.sql
demo_tran1 >> run.out

echo ==========>> run.out
echo DEMO TRAN2>> run.out
echo ==========>> run.out
call is -silent -f demo_tran2.sql
demo_tran2 >> run.out
call is -silent -f demo_tran2_sel.sql >> run.out

echo ==========>> run.out
echo DEMO MT   >> run.out
echo ==========>> run.out
demo_mt >> run.out

echo ==========>> run.out
echo DEMO RTL  >> run.out
echo ==========>> run.out
call is -silent -f demo_ex2.sql
demo_rtl >> run.out

echo ==========>> run.out
echo DEMO CLOB >> run.out
echo ==========>> run.out
call is -silent -f demo_clob.sql
demo_clob >> run.out

echo ==========>> run.out
echo DEMO BLOB >> run.out
echo ==========>> run.out
call is -silent -f demo_blob.sql
demo_blob >> run.out

make clean
call is -silent -f drop_schema.sql

diff run.lst run.out > run.diff

set diffLineCnt=
for /f "tokens=1 delims= " %%p ('wc -l run.diff') do (
    set diffLineCnt=%%p
)
if "%diffLineCnt%" == "0" (
    echo PASS
) else (
    echo FAIL: check run.diff
)

endlocal
