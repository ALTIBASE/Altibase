#/usr/java/jdk1.5.0_22/bin/java -Xdebug -Xss200k -Xms3m -Xmx5m -XX:NewSize=1m -XX:SurvivorRatio=8 -jar amond.jar
/usr/java/jdk1.5.0_22/bin/java -XX:+PrintTenuringDistribution -Xss200k -Xms3m -Xmx5m -XX:NewSize=1m -jar altimon.jar
#-verbosegc -XX:+PrintGCDetails -XX:+PrintGCTimeStamps -Xloggc:gc.log 
