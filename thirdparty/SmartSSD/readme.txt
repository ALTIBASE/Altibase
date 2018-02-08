####################################################################
######      readme.txt for unit test program for libsdm.so    ######
####################################################################

### Scope ###
   This readme.txt file is written to explain how-to-use libsdm.so. Please make sure you get the right software package with "libsdm.tar.gz" file name.


### libsdm.tar.gz packages ####
   Please check you have the following files when you uncompress libsdm.tar.gz

   libsdm.so : shared library. 64-bits Linux version
   sdm_errno/mgnt/types_public.h : contains public header files. Your application shall include the header files.
   utest.c/Makefile : contains unit test application binary and source code


### compile & build unit test application ###
   Here is the step to build the enclosed unit test application.

   1. unzip "libsdm.tar.gz" to any folder in your Linux Machine. You will see the top folder name "sdm_api".
   2. change directory to "sdm_api/sdm_mgnt/test".
   3. type "make".
   4. make sure "utest", binary executible, is created.
   5. type "export LD_LIBRARY_PATH=./". This is to set library path for libsdm.so.
   6. check the device node name of your SDD that you run test against with. Ex.) /dev/sdb.
   7. To see the usage of the unit test, type "./utest <device node> h". Ex.) ./utest /dev/sdb h


   Please note that libsdm.so does not have any special 3rd party library dependency other than the standard C library. If you fail at step3, please check GNU g++ and make tool are installed in your Linux Machine.
   
