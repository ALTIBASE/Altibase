### Altibase: Enterprise grade High performance Relational DBMS
- Enterprise grade: 17 years of experience in servicing over 600 global enterprise clients including Samsung, HP, E-Trade, China Mobile and more. (Ref: http://altibase.com/product/enterprise-grade-database/ )
- High performance: Accelerate the performance of your mission critical applications by over 10X with in-memory and on-disk hybrid architecture. (Ref: http://altibase.com/product/high-performance-database/ )
- Relational DBMS: Function and feature rich with all the tools and relational capabilities expected and required by enterprise grade applications. Enjoy the benefits without radically changing your applications. (Ref: http://altibase.com/product/relational-database/ )
- Low cost: Open source costs with enterprise grade quality. Lower your TCO with flexible subscription fees. (Ref: http://altibase.com/product/low-cost-database/ )

### Help
- http://altibase.com/resources/
- http://altibase.com/services/support/
- http://altibase.com/case-studies/
- http://altibase.com/faqs/product/

### Q&A and Bug Reports
- Q&A and bug reports regarding Altibase should be submitted at http://support.altibase.com
- The code for Altibase can be found at: https://github.com/Altibase

### License
- Altibase is an open source with GNU Affero General Public License version 3(GNU AGPLv3) for the Altibase server code and GNU Lesser General Public License version 3(GNU LGPLv3) for the Altibase client code. 
- While Altibase is free, we provide a paid support subscription for those who wish to have professional support.
- License information can be found in the COPYING files.
- Altibase includes GPC sources, so, if you want to use those sources for commertial use then you need to buy "General Polygon Clipper (GPC) License".

### Build environment setting steps
<pre><code>
- OS: Red Hat Enterprise Linux 6.x
- CPU: x86_64
- selinux disabling
  vi /etc/sysconfig/selinux 
  SELINUX=disabled
- ntsysv 
  IPTables = iptables, ip6tables *uncheck*
  vsftpd = *check* 
  /etc/init.d/iptable stop
  /etc/init.d/vsftpd start
- /etc/rc.local
  echo 16147483648 > /proc/sys/kernel/shmmax
  echo 1024 32000 1024 1024 > /proc/sys/kernel/sem
- /etc/sysctl.conf
  # Controls the default maxmimum size of a mesage queue
  kernel.msgmnb = 65536
  # Controls the maximum size of a message, in bytes
  kernel.msgmax = 65536
  # Controls the maximum shared segment size, in bytes
  kernel.shmmax = 68719476736000
  # Controls the maximum number of shared memory segments, in pages
  kernel.shmall = 4294967296
  fs.suid_dumpable = 1
  fs.aio-max-nr = 1048576
  fs.file-max = 6815744
  # semaphores: semmsl, semmns, semopm, semmni
  kernel.sem = 1024 32000 1024 1024
  net.ipv4.ip_local_port_range = 32768 61000
  net.core.rmem_default = 4194304
  net.core.rmem_max = 4194304
  net.core.wmem_default = 262144
  net.core.wmem_max = 1048586
  # core filename pattern (core.execution_file_name.time)
  kernel.core_uses_pid = 0
  kernel.core_pattern = core.%e.%t
- glibc 2.12 ~ 2.20
- gcc 4.6.3
  Install following libraries
  https://gmplib.org/ 
  http://www.mpfr.org/
  http://www.multiprecision.org/
  http://www.mr511.de/software/english.html
  Gmp => ./configure --enable-shared --enable-static --prefix=/usr/gmp
  Mpfr => ./configure --enable-shared --enable-static --prefix=/usr/mpfr --with-gmp=/usr/gmp
  Mpc => ./configure --enable-shared --enable-static --prefix=/usr/mpc --with-gmp=/usr/gmp --with-mpfr=/usr/mpfr
  Libelf => ./configure --enable-shared --enable-static --prefix=/usr/elf --with-gmp=/usr/gmp --with-mpfr=/usr/mpfr
  LD_LIBRARY_PATH=/usr/gmp/lib:/usr/mpfr/lib:/usr/mpc/lib:/usr/elf/lib:$LD_LIBRARY_PATH
  Install glibc-devel package(e.g. glibc-devel-2.*.el6.i686.rpm)
  https://gcc.gnu.org/
  ./configure --prefix=/usr/local/gcc-4.6.3 \
  --enable-shared \
  --enable-threads=posix \
  --enable-languages=c,c++ \
  --with-gmp=/usr/gmp \
  --with-mpfr=/usr/mpfr \
  --with-mpc=/usr/mpc \
  --with-libelf=/usr/elf \
  make; make install
- Install both of Oracle Java JDK 1.5 and 1.7
- Install development tools
  autoconf
  g++
  gawk
  flex
  bison
  libncurses5-dev
  binutils-dev
  ddd
  tkdiff
  manpages-dev 
  libldap2-dev
- Modify /usr/include/sys/select.h
  $ diff select.h_old select.h_new
  62a63,67
  > /* Maximum number of file descriptors in `fd_set'. */
  > #ifndef FD_SETSIZE
  > #define FD_SETSIZE __FD_SETSIZE
  > #endif
  > 
  69c74
  <     __fd_mask fds_bits[__FD_SETSIZE / __NFDBITS];
  ---
  >     __fd_mask fds_bits[FD_SETSIZE / __NFDBITS];
  72c77
  <     __fd_mask __fds_bits[__FD_SETSIZE / __NFDBITS];
  ---
  >     __fd_mask __fds_bits[FD_SETSIZE / __NFDBITS];
- re2c 
  /home/user/local/pkg$ tar xvf ./re2c-0.13.5.tar.gz
  /home/user/local/pkg$ cd ./re2c-0.13.5
  /home/user/local/pkg/re2c-0.13.5$ ./configure --prefix=/user/local
  /home/user/local/pkg/re2c-0.13.5$ make
  /home/user/local/pkg/re2c-0.13.5$ make install
- Locale setting
  locale-gen ko_KR.EUC-KR
  export LANG=ko_KR.EUC-KR
- Other environment variable setting
  export ALTIDEV_HOME=*source code directory*
  export ALTIBASE_DEV=${ALTIDEV_HOME}
  export ALTIBASE_HOME=${ALTIDEV_HOME}/altibase_home
  export ALTIBASE_NLS_USE=US7ASCII
  export ALTIBASE_PORT_NO=17730
  export ADAPTER_JAVA_HOME=/usr/java/jdk1.7.0_71
  export JAVA_HOME=/usr/java/jdk1.5.0_22
  export PATH=.:${ALTIBASE_HOME}/bin:${JAVA_HOME}/bin:${PATH}
  export CLASSPATH=.:${JAVA_HOME}/lib:${JAVA_HOME}/jre/lib:${ALTIBASE_HOME}/lib/Altibase.jar:${CLASSPATH}
  export LD_LIBRARY_PATH=$ADAPTER_JAVA_HOME/jre/lib/amd64/server:${ALTIBASE_HOME}/lib:${LD_LIBRARY_PATH}
- Compile Altibase
  ./configure --with-build_mode=release
  make clean
  make build
</code></pre>
