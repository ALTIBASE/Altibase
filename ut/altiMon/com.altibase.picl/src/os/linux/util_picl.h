/** 
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/procfs.h>
#include <mntent.h>
#include <time.h>
#include <assert.h>
#include <ctype.h>

#define MemFree         0
#define MemUsed         1
#define MemTotal        2

#define PARAM_MEMTOTAL "MemTotal"
#define PARAM_MEMFREE "MemFree"

#define CP_USER 0
#define CP_SYS  1
#define CP_NICE 2
#define CP_WAIT 3
#define CP_IDLE 4

#define DECIMALBASE 10

#define PROC "/proc/"

#define TOTAL_STAT    PROC "stat"
#define TOTAL_MEMINFO PROC "meminfo"
#define TOTAL_SWAP    PROC "meminfo"

#define PROC_STAT "/stat"
#define MEM_STATM "/statm"

#define INT_BUFFER_SIZE \
(sizeof(int) * 3 + 1)

#define PARAM_SWAP_TOTAL "SwapTotal"
#define PARAM_SWAP_FREE  "SwapFree"

#define DIR_NAME_MAX 4096

#define DEVICE_INFO_MAX 256
#define DEVICE_MAX 10

// To make parameter
#define GET_PARAM_MEM(ch) ch ":", (sizeof(ch ":")-1)

// To initialize structure
#define INITSTRUCT(s) \
    memset(s, '\0', sizeof(*(s)))

// The strtoul() function converts the initial part of the string in nptr to an unsigned long integer value according to the given base, which must be between 2 and 36 inclusive, or be the special value 0. 
// The base means notation. If base has 10, function returns value that based on decimal notation.
#define EXTRACTDECIMALULL(index) \
    strtoull(index, &index, DECIMALBASE)

// To convert CPU tick into milli secunit.
#define CPUTICK2MILLISEC(s) \
   ((unsigned long long)(s) * ((unsigned long long)1000 / (double)sysconf(_SC_CLK_TCK)))

#define SKIP_SPACE(ptr) \
  while (isspace((unsigned char)(*ptr))) ++ptr

#define CONVERT_TO_BYTES(numOfBlocks, blockSize) ((numOfBlocks * blockSize) >> 1)

enum {
  DEVICE_DIRNAME,
  DEVICE_DEVNAME,
    DEVICE_FIELD_MAX
};

enum {
  IO_FIELD_DIRNAME,
  IO_FIELD_READ,
  IO_FIELD_WRITE,
  IO_FIELD_TIME,
  IO_FIELD_TOTAL,
  IO_FIELD_AVAIL,
  IO_FIELD_USED,
  IO_FIELD_FREE,
    IO_FIELD_MAX
};

// Structure of CPU
typedef struct {
  unsigned long long
    user,
    sys,
    nice,
    idle,
    wait,
    total;
} cpu_t;

// Structure of Process
typedef struct {
  /* BUG-43351 Coercion Alter Value */
  long long utime;
  long long stime;
} proc_cpu_t;

typedef struct {
  long disk_usage;
} dir_usage_t;

typedef struct {
  /* BUG-43351 Coercion Alter Value */
  long numOfReads;
  long numOfWrites;
  long write_bytes;
  long read_bytes;
  unsigned long total_bytes;
  unsigned long avail_bytes;
  unsigned long used_bytes;
  unsigned long free_bytes;
} disk_load_t;


typedef struct {
  char dir_name[DIR_NAME_MAX];
  char dev_name[DIR_NAME_MAX];
} device_t_;

typedef struct {
  unsigned long number;
  unsigned long size;
  device_t_ *data;
} device_list_t_;

char *clearSpace(char *p);
int convertName2Content(const char *fileName, char *buff, int buflen);
void getCpuMtx(cpu_t *cpu, char *line);
void getCpuTime(cpu_t *cpu);
char *ui2chararray(char *aTarget, unsigned int aNum, int *aLen);
char *getProcName(char *aProcNameBuffer, unsigned int aBufferLength,
                  unsigned long aPid, const char *aProcFileName, unsigned int aFileNameLength);
int getProcString(char *aProcNameBuffer, unsigned int aBufferLength,
                  unsigned long aPid, const char *aProcFileName, unsigned int aFileNameLength);
int getProcCpuMtx(proc_cpu_t *pcpu, long pid);
unsigned long long getMem(char *aMemBuff, char *aTarget, int aLength);
int getDeviceList(device_list_t_ *fslist);
int getDirUsage(const char *dirname, dir_usage_t *dirstats);
int getCmdLine(char *file, char *buf);
void getDevName(const char *mountName, char **devName);
int getDiskIo(const char *mountName, disk_load_t *diskLoad);
