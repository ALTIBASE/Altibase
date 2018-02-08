echo "=======================================";
echo "APRE SSL TEST START";
echo "=======================================";

server kill > /dev/null

mkdir ${ALTIBASE_HOME}/sample/CERT
cp -r ${ALTIDEV_HOME}/ut/sample/CERT/* ${ALTIBASE_HOME}/sample/CERT

export ALTIBASE_SSL_ENABLE=1
export ALTIBASE_SSL_PORT_NO=20443
export ALTIBASE_SSL_CA="${ALTIBASE_HOME}/sample/CERT/ca-cert.pem"
export ALTIBASE_SSL_CERT="${ALTIBASE_HOME}/sample/CERT/server-cert.pem"
export ALTIBASE_SSL_KEY="${ALTIBASE_HOME}/sample/CERT/server-key.pem"

server start > /dev/null

is -f schema.sql -o schema.out -silent

make clean
apre sslSample.sc
make  
sslSample > run.out
is -f drop_schema.sql -silent
diff run.out run.lst > run.diff

if [ -s run.diff ] ; then
    echo "[APRE TEST FAIL   ] : SslSample";
else
    echo "[APRE TEST SUCCESS] : SslSample";
    cat run.out
fi

make clean
rm *.out

server kill > /dev/null

export ALTIBASE_SSL_ENABLE=
export ALTIBASE_SSL_PORT_NO=
export ALTIBASE_SSL_CA=
export ALTIBASE_SSL_CERT=
export ALTIBASE_SSL_KEY=

rm -rf ${ALTIBASE_HOME}/sample/CERT

server start > /dev/null

