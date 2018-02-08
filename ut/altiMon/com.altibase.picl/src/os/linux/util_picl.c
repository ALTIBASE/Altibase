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
 
#include "util_picl.h"

// The function 'isspace' distincts whether arguments are blank characters('\n', '\f', '\t', '\v', '\r') or general characters 
// ASCII 0 means null == if(*p)
char *clearSpace(char *index)
{
  while (isspace(((unsigned char)(*index))))
  {
    index++;
  }
  
  while (*index && !isspace(((unsigned char)(*index))))
  {
    index++;
  }
  
  return index;
}

// To get string from file in the proc file system
int convertName2Content(const char *aProcFileName, char *aTarget, int aBufferLength)
{
  int mLength, retval;
  int fd = open(aProcFileName, O_RDONLY);

  if (fd < 0) {
    return ENOENT;
  }

  /* BUG-43351 Buffer Overrun */
  if ((mLength = read(fd, aTarget, aBufferLength - 1)) < 0) {
    retval = errno;
  }
  else {
    retval = 1;
    aTarget[mLength] = '\0';
  }
  close(fd);

  return retval;
}

// To get cpu matrix from convertName2Content function (/proc/stat or /proc/pid/stat)
void getCpuMtx(cpu_t *cpu, char *line)
{
  char *index = clearSpace(line);

  cpu->user += CPUTICK2MILLISEC(EXTRACTDECIMALULL(index));
  cpu->nice += CPUTICK2MILLISEC(EXTRACTDECIMALULL(index));
  cpu->sys += CPUTICK2MILLISEC(EXTRACTDECIMALULL(index));
  cpu->idle += CPUTICK2MILLISEC(EXTRACTDECIMALULL(index));
  cpu->wait += CPUTICK2MILLISEC(EXTRACTDECIMALULL(index));
  cpu->total = cpu->user + cpu->nice + cpu->sys + cpu->idle + cpu->wait;
}

// To store cpu data into cpu structure
void getCpuTime(cpu_t *cpu)
{
  char buff[BUFSIZ];

  buff[0] = '\0'; /* BUG-43351 Uninitialized Variable */
  convertName2Content(TOTAL_STAT, buff, sizeof(buff));

  INITSTRUCT(cpu);
  getCpuMtx(cpu, buff);
}

// To convert unsigned int to char array
char *ui2chararray(char *aTarget, unsigned int aNum, int *aLen)
{
  char *retval = aTarget + INT_BUFFER_SIZE - 1;

  *retval = 0;

  do {
    retval--;
    *retval  = '0' + (aNum % 10);
    ++*aLen;
    aNum = aNum / 10;
  } while (aNum != 0);

  return retval;
}

// To create string /proc/pid/stat
char *getProcName(char *aProcNameBuffer,
                  __attribute__((unused)) unsigned int aBufferLength,
                  unsigned long aPid,
                  const char *aProcFileName,
                  unsigned int aFileNameLength)
{
  int mLength = 0;
  char *index = aProcNameBuffer;
  char mProcNameBuffer[INT_BUFFER_SIZE];
  char *mProcID = ui2chararray(mProcNameBuffer, aPid, &mLength);

  memcpy(index, PROC, sizeof(PROC)-1);
  index += sizeof(PROC)-1;

  memcpy(index, mProcID, mLength);
  index += mLength;

  memcpy(index, aProcFileName, aFileNameLength);
  index += aFileNameLength;
  *index = '\0';

  return aProcNameBuffer;
}

// To extract string from /proc/pid/stat
int getProcString(char *aProcNameBuffer, unsigned int aBufferLength,
		  unsigned long aPid, const char *aProcFileName, unsigned int aFileNameLength)
{
  int retval;

  aProcNameBuffer = getProcName(aProcNameBuffer, aBufferLength, aPid, aProcFileName, aFileNameLength);

  retval = convertName2Content(aProcNameBuffer, aProcNameBuffer, aBufferLength);

  if (retval != 1) {
    retval = -1;
  }

  return retval;
}

int getProcCpuMtx(proc_cpu_t *pcpu, long pid)
{
  char mProcStat[BUFSIZ], *index=mProcStat;
  int i;

  // To check buffer capacity
  if((unsigned int)sizeof(mProcStat) < (sizeof(PROC)-1 + INT_BUFFER_SIZE + sizeof(PROC_STAT)))
  {
    return 0;
  }

  // To get value list from /proc/pid/stat
  if(getProcString(mProcStat, sizeof(mProcStat), pid, PROC_STAT, sizeof(PROC_STAT)-1) != 1)
  {
    return 0;
  }

  for(i=0; i<13; i++)
  {
    // To skip 1~13 value
    index = clearSpace(index);
  }

  pcpu->utime = CPUTICK2MILLISEC(EXTRACTDECIMALULL(index));  // 14 - utime %lu = The number of jiffies that this process has been scheduled in user mode.
  pcpu->stime = CPUTICK2MILLISEC(EXTRACTDECIMALULL(index));  // 15 - stime %lu = The number of jiffies that this process has been scheduled in kernel mode.

  return 1;
}

unsigned long long getMem(char *aMemBuff, char *aTarget, int aLength)
{
  unsigned long long retval = 0;
  char *index, *partial;

  // To find MemTotal: xxx and get pointer for this point
  // To find MemFree: xxx and get pointer for this point
  if ((index = strstr(aMemBuff, aTarget))) {
    // To skip aTarget token and get pointer following this token
    index = index + aLength;
    
    // To extract value about MemTotal and MemFree
    retval = strtoull(index, &partial, 0);

    // To skip null
    while (*partial == ' ') {
      ++partial;
    }

    // To convert value into byte metric
    if (*partial == 'k') {
      retval *= 1024;
    }
    else if (*partial == 'M') {
      retval *= (1024 * 1024);
    }
  }

  return retval;
}

int getDirUsage(const char *dirname, dir_usage_t *dirstats)
{
  char name[DIR_NAME_MAX+1];
  int len = strlen(dirname);
  int max = sizeof(name)-len-1;
  char *ptr = name;

  struct dirent *ent;
  struct stat info;

  /*
  struct dirent
  {
    long d_ino;                 // inode number
    off_t d_off;                // offset to this dirent
    unsigned short d_reclen;    // length of this d_name
    char d_name [NAME_MAX+1];   // filename (null-terminated)
  }
  */

  // To get a pointer to the directory stream
  DIR *dirp = opendir(dirname);

  if(!dirp) {
    return 0; /* BUG-43351 Missing Return Value */
  }

  // To copy dirname into name buffer
  /* BUG-43351 No Space For Null Terminator */
  strncpy(name, dirname, sizeof(name) - 1);

  // To shift pointer at the end of the dirname
  ptr += len;

  if (name[len] != '/') {
    // To add '/' at the end of the dirname
    // It makes name buffer into subdirectory path
    *ptr++ = '/';
    len++;
    max--;
  }

  // To return a pointer to a dirent structure representing the next directory entry in the directory stream pointed to by dir
  while ((ent = readdir(dirp)))
  {
    if (ent == NULL)
    {
      break;
    }

    // To skip '.' or '..'
    if (((ent->d_name[0] == '.') && (!ent->d_name[1] || ((ent->d_name[1] == '.') && !ent->d_name[2])))) 
    {
      continue;
    }

    // To add subdirectory at the end of dirname buffer
    strncpy(ptr, ent->d_name, max);
    ptr[max] = '\0';
    
    // The lstat() function shall be equivalent to stat(), except when path refers to a symbolic link
    // If lsatt doesn't completed, skip this subdirectory.
    if (lstat(name, &info) != 0) {
      continue;
    }

    // To add dir_size for this subdirectory
    dirstats->disk_usage += info.st_size;

    // If the subdirectory type is directory, we call the function getDirUsage recursively.
    if((info.st_mode & S_IFMT) == S_IFDIR)
    {
      getDirUsage(name, dirstats);
    }
  }

  closedir(dirp);

  return 1;
}

int getDeviceList(device_list_t_ *fslist)
{
  /*
  struct mntent {  
    char *mnt_fsname;	// file system name  
    char *mnt_dir;	// file system path prefix
    char *mnt_type;	// hfs, nfs, swap, or xx
    char *mnt_opts;	// ro, suid, etc.
    int mnt_freq;	// dump frequency, in days
    int mnt_passno;	// pass number on parallel fsck
    long mnt_time;	// When file system was mounted;
    			// see mnttab(4).
    			// (0 for NFS)
  }; 
  */

  struct mntent ent;
  char buf[1025];	//The length of the user-supplied buffer. A buffer length of 1025 is recommended.
  device_t_ *fsp;
  FILE *fp;

  // Opens a file system description file and returns a file pointer which can then be used with getmntent(), addmntent(), delmntent(), or endmntent().
  if (!(fp = setmntent(MOUNTED, "r"))) {
    return errno;
  }

  fslist->number = 0;
  fslist->size = DEVICE_MAX;
  fslist->data = malloc(sizeof(*(fslist->data))*fslist->size);

  /* BUG-43351 Null Pointer Dereference */
  if ( fslist->data == NULL )
  {
      endmntent(fp);
      return 0;
  }

  // Returns a -1 on error or EOF, or if the supplied buffer is of insufficient length. If the operation is successful, 0 is returned.
  while (getmntent_r(fp, &ent, buf, sizeof(buf)))
  {
    // If fslists are over the specified size, we have to increase device_t_
    if (fslist->number >= fslist->size) {

      /* BUG-43351 Integer Overflow of Allocation Size
       *  If ( 8192 * fslist->size + 81920 > integer_max(2^31-1) ),
       *  allocation size (sizeof(*(fslist->data))*(fslist->size + DEVICE_MAX) overflows.
       *  ( ==> if (fslist->size > 262133) )
       */
      if (fslist->size > 262133)
      {
          endmntent(fp);
          return 0;
      }
      fslist->data = realloc(fslist->data, sizeof(*(fslist->data))*(fslist->size + DEVICE_MAX));

      /* BUG-43351 Null Pointer Dereference */
      if ( fslist->data == NULL )
      {
          endmntent(fp);
          return 0;
      }
      fslist->size += DEVICE_MAX;
    }
    
    fsp = &fslist->data[fslist->number++];
    
    // To copy the dirname and devname.
    strncpy(fsp->dir_name, ent.mnt_dir, sizeof(fsp->dir_name));
    fsp->dir_name[sizeof(fsp->dir_name)-1] = '\0';
    strncpy(fsp->dev_name, ent.mnt_fsname, sizeof(fsp->dev_name));
    fsp->dev_name[sizeof(fsp->dev_name)-1] = '\0';
  }

  endmntent(fp);

  return 1;
}

void getDevName(const char *mountName, char **devName)
{
  device_list_t_ device_list;
  int i;

  /* BUG-43351 Uninitialized Variable */
  device_list.number = 0;
  device_list.size = 0;
  device_list.data = NULL;

  // To get device list
  if(getDeviceList(&device_list) == 1)
    {
      for (i=0; i<(long)device_list.number; i++) {
        device_t_ *device = &(device_list.data)[i];
        if(strncmp(device->dir_name, mountName, strlen(mountName))==0)
          {
            *devName = device->dev_name;
            break;
          }
      }
    }

  if (device_list.size > 0 ) {
    if (device_list.data != NULL) { /* BUG-43351 Free Null Pointer */
      free(device_list.data);
    }
    device_list.number = 0;
    device_list.size = 0;
  }
}

int getDiskIo(const char *mountName, disk_load_t *diskLoad)
{
  struct statvfs vbuf;
  char *devName;
  unsigned long numOfBlocks, blockSize;

  // To extract devName using mountName
  getDevName(mountName, &devName);

  // The statvfs() function gets status information about the file system that contains the file named by the path argument. The information will be placed in the area of memory pointed to by the buf argument.
  if(statvfs(mountName, &vbuf)==0)
  {
    blockSize = vbuf.f_frsize / 512;
    
    numOfBlocks = vbuf.f_blocks;
    diskLoad->total_bytes = CONVERT_TO_BYTES(numOfBlocks, blockSize);
    
    numOfBlocks = vbuf.f_bavail;
    diskLoad->avail_bytes = CONVERT_TO_BYTES(numOfBlocks, blockSize);
    
    numOfBlocks = vbuf.f_bfree;
    diskLoad->free_bytes = CONVERT_TO_BYTES(numOfBlocks, blockSize);
    
    diskLoad->used_bytes = diskLoad->total_bytes - diskLoad->free_bytes;
    // Preparation Update
    diskLoad->numOfReads = -1;
    diskLoad->numOfWrites = -1;
    diskLoad->read_bytes = -1;
    diskLoad->write_bytes = -1;
  }
  else
  {
    return 0;
  }

  return 1;
}

int getCmdLine(char *file, char *buf)
{
  FILE *srcFp;
  srcFp = fopen(file, "r");

  if(srcFp == NULL)
  {
    return 0;
  }

  fgets(buf, 256, srcFp);
  fclose(srcFp);

  return 1;
}
