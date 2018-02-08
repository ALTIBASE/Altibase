#!/bin/bash

rm ./*.pem

# [for server]
# ca-cert.pem: CA certificate
# server-key.pem: private key
# server-cert.pem: public key
#!/bin/bash

# delete certificates from truststore
rm ./truststore

# [for server]
# ca-cert.pem: CA 
# server-key.pem: private key
# server-cert.pem: certificate
openssl genrsa -des3 -out server-key.pem 2048
openssl req -newkey rsa:2048 -days 3600 -nodes -keyout server-key.pem -config ./gencert.cnf -out server-req.pem
openssl x509 -req -in server-key.pem -out server-key.pem
openssl x509 -req -days 3600 -in server-req.pem -signkey server-key.pem -out server-cert.pem

cat server-cert.pem > ca-cert.pem

# [for client]
# client-key.pem: private key
# client-cert.pem: certificate
openssl genrsa -des3 -out client-key.pem 2048
openssl req -newkey rsa:2048 -days 3600 -nodes -keyout client-key.pem -config ./gencert_cli.cnf -out client-req.pem
openssl x509 -req -in client-key.pem -out client-key.pem
openssl x509 -req -days 3600 -in client-req.pem -signkey client-key.pem -out client-cert.pem

cat client-cert.pem >> ca-cert.pem

# verify the certificates
openssl verify -CAfile ca-cert.pem server-cert.pem client-cert.pem

# for JDBC
keytool -import -alias altibaseServerCert -file server-cert.pem -keystore truststore -storepass altibase
#keytool -import -alias altibaseJDBCCert -file client-cert.pem -keystore truststore -storepass altibase

#openssl pkcs12 -export -in server-cert.pem -inkey server-key.pem > server.p12
openssl pkcs12 -export -in client-cert.pem -inkey client-key.pem > client.p12

#keytool -importkeystore -srckeystore server.p12 -destkeystore keystore.jks -srcstoretype pkcs12
keytool -importkeystore -srckeystore client.p12 -destkeystore keystore.jks -srcstoretype pkcs12

rm ./*-req.pem

ls -al *.pem

