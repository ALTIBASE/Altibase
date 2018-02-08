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
#include <sys/pstat.h>
#include <sys/pstat/pstat_ops.h>
#include <sys/dk.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mntent.h>
#include <sys/time.h>
#include <unistd.h>

#define BURST 20

#define MemFree         0
#define MemUsed         1
#define MemTotal        2

#define MAX_LENGTH 1024

#define DIR_NAME_MAX 4096

#define DEVICE_INFO_MAX 256
#define DEVICE_MAX 10

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

typedef struct {
  unsigned long numOfReads;
  unsigned long numOfWrites;
  unsigned long write_bytes;
  unsigned long read_bytes;
  unsigned long total_bytes;
  unsigned long avail_bytes;
  unsigned long used_bytes;
  unsigned long free_bytes;
} disk_load_t;

typedef struct {
  int disk_usage;
} dir_usage_t;

typedef struct {
  char dir_name[DIR_NAME_MAX];
  char dev_name[DIR_NAME_MAX];
} device_t_;

typedef struct {
  unsigned long number;
  unsigned long size;
  device_t_ *data;
} device_list_t_;

int getDirUsage(const char *dirname, dir_usage_t *dirusage);
