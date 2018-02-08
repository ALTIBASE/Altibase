echo "=======================================";
echo "JDBC TEST START : SslSimpleSQL";
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


javac SslSimpleSQL.java
java SslSimpleSQL $ALTIBASE_SSL_PORT_NO | tee run.out
is -f drop_schema.sql
diff run.out run.lst > run.diff

if [ -s run.diff ] ; then
    echo "[JDBC TEST FAIL   ] : SslSimpleSQL";
else
    echo "[JDBC TEST SUCCESS] : SslSimpleSQL";
fi

server kill > /dev/null

export ALTIBASE_SSL_ENABLE=
export ALTIBASE_SSL_PORT_NO=
export ALTIBASE_SSL_CA=
export ALTIBASE_SSL_CERT=
export ALTIBASE_SSL_KEY=

rm -rf ${ALTIBASE_HOME}/sample/CERT

server start > /dev/null

