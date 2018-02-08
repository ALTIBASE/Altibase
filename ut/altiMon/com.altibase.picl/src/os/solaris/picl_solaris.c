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
 
/*
 * Picl_hpux.c
 *
 *  Created on: 2010. 1. 13
 *      Author: canpc815
 */

#include "picl_os.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/proc.h>
#include <sys/mnttab.h>
#include <kstat.h>
#include <unistd.h>
#include <sys/swap.h>
#include <sys/sysinfo.h>
#include <stdio.h>
#include <fcntl.h>
#include <procfs.h>
#include <sys/time.h>

#define BURST 20

#define MemFree		0
#define MemUsed		1
#define MemTotal	2

#define CP_USER 0
#define CP_SYS  1
#define CP_WAIT 2
#define CP_IDLE 3

#include <sys/procfs.h>
#include <sys/fcntl.h>


kstat_ctl_t *kc = 0;

/*
 * Class:     com_altibase_picl_Cpu
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_Cpu_update
(JNIEnv *env, jobject cpu_obj, jobject picl_obj)
{
  jfieldID prev_fid, sys_fid, user_fid;
  jlongArray mPrevTime;
  float mUserPerc;
  float mSysPerc;
  
  jboolean isCopy = JNI_TRUE;
  int i = 0;
  int core = 0;

  /* Variable for calculate CPU Percentage */
  unsigned long long delta; 
  cpu_stat_t *knp;
  unsigned long long CURRENT_CP_USER = 0;
  unsigned long long CURRENT_CP_SYS = 0;
  unsigned long long CURRENT_CP_WAIT = 0;
  unsigned long long CURRENT_CP_IDLE = 0;
  uint_t cpu_time[CPU_STATES];

  /* kstat variable */
  kstat_t* ksp;

  /* Get a reference to obj's class */
  jclass cls = (*env)->GetObjectClass(env, cpu_obj);
  jclass picl_cls = (*env)->GetObjectClass(env, picl_obj);

  /* Look for the instance field mPrevTime in cls */
  prev_fid = (*env)->GetFieldID(env, picl_cls, "mPrevTime", "[J");
  sys_fid = (*env)->GetFieldID(env, cls, "mSysPerc", "D");
  user_fid = (*env)->GetFieldID(env, cls, "mUserPerc", "D");

  if(prev_fid == NULL || sys_fid == NULL || user_fid == NULL)
  {
    return;// failed to find the field
  }

  core = sysconf(_SC_NPROCESSORS_CONF);

  // kstat_ctl_t initialize
  if( kc == 0 )
  {
    kc = kstat_open();
  }

  cpu_stat_t newdata[core];

  for (ksp = kc->kc_chain; ksp != NULL; ksp = ksp->ks_next) 
  {   
    if(strncmp(ksp->ks_name, "cpu_stat", strlen("cpu_stat"))==0)
    {	  
      kstat_read(kc, ksp, &newdata[i]);
      
      CURRENT_CP_USER += newdata[i].cpu_sysinfo.cpu[CPU_USER];
      CURRENT_CP_SYS  += newdata[i].cpu_sysinfo.cpu[CPU_KERNEL];
      CURRENT_CP_WAIT += newdata[i].cpu_sysinfo.cpu[CPU_WAIT];
      CURRENT_CP_IDLE += newdata[i].cpu_sysinfo.cpu[CPU_IDLE];

      i++;      
    }
  }

  /*
  while(1)
  {
    if((cpu = kstat_lookup(kc, "cpu_stat", i, NULL))==NULL)
      break;
    printf("ks_instance = %d\n", cpu->ks_instance);
    //kstat_read(kc, cpu, &newdata[i]);

    printf("read number : %d\n", i);
    i++;
    //CURRENT_CP_USER += newdata[i].cpu_sysinfo.cpu[CPU_USER];
    //CURRENT_CP_SYS  += newdata[i].cpu_sysinfo.cpu[CPU_KERNEL];
    //CURRENT_CP_WAIT += newdata[i].cpu_sysinfo.cpu[CPU_WAIT];
    //CURRENT_CP_IDLE += newdata[i].cpu_sysinfo.cpu[CPU_IDLE];
  }
  */
  // JNI Object Type
  mPrevTime = (jlongArray)(*env)->GetObjectField(env, picl_obj, prev_fid);
  mSysPerc = (*env)->GetDoubleField(env, cpu_obj, sys_fid);
  mUserPerc = (*env)->GetDoubleField(env, cpu_obj, user_fid);

  // Native Type
  jlong* mPrevTimeArr = (*env)->GetLongArrayElements(env, mPrevTime, &isCopy);
  
  // Cpu time delta

  delta = CURRENT_CP_USER - mPrevTimeArr[CP_USER] +
          CURRENT_CP_SYS  - mPrevTimeArr[CP_SYS]  +
          CURRENT_CP_WAIT - mPrevTimeArr[CP_WAIT] +
          CURRENT_CP_IDLE - mPrevTimeArr[CP_IDLE];

  if(delta == 0)
  {
    mSysPerc = 0.0;
    mUserPerc = 0.0;
  }
  else
  {
    mSysPerc = (double)(CURRENT_CP_SYS - mPrevTimeArr[CP_SYS]) / delta * 100.0;
    mUserPerc = (double)(CURRENT_CP_USER - mPrevTimeArr[CP_USER]) / delta * 100.0;

    mPrevTimeArr[CP_USER] = CURRENT_CP_USER;
    mPrevTimeArr[CP_SYS] = CURRENT_CP_SYS;
    mPrevTimeArr[CP_WAIT] = CURRENT_CP_WAIT;
    mPrevTimeArr[CP_IDLE] = CURRENT_CP_IDLE;
  }

  (*env)->ReleaseLongArrayElements(env, mPrevTime, mPrevTimeArr, 0);

  /* Read the instance field mSysPerc */
  (*env)->SetObjectField(env, picl_obj, prev_fid, mPrevTime);
  (*env)->SetDoubleField(env, cpu_obj, sys_fid, mSysPerc);
  (*env)->SetDoubleField(env, cpu_obj, user_fid, mUserPerc);  
}

char *convertUItoCA(char *buf, unsigned int source, int *slength)
{
  char *target = buf + (sizeof(int) * 3);

  *target = 0;

  do {
    *--target = '0' + (source % 10);
    ++*slength;
    source /= 10;
  } while (source);

  return target;
}

void getProcFileName(char *buf, pid_t aPid, const char *fileName, int fileNameLength)
{
  int slength = 0;
  char *loc = buf;
  unsigned int pid = (unsigned int)aPid;
  char pid_buf[sizeof(int) * 3 + 1];
  char *pid_str = convertUItoCA(pid_buf, pid, &slength);

  memcpy(loc, "/proc/", sizeof("/proc/")-1);
  loc += (sizeof("/proc/")-1);

  memcpy(loc, pid_str, slength);
  loc += slength;

  memcpy(loc, fileName, fileNameLength);
  loc += fileNameLength;
  *loc = '\0';
}

int getProcUsage(prusage_t *pus, pid_t aPid)
{
  int fd, retval = 1;
  char buffer[BUFSIZ];

  getProcFileName(buffer, aPid, "/usage", sizeof("/usage")-1);

  if ((fd = open(buffer, O_RDONLY)) < 0)
    {
      return ESRCH;
    }

  if (read(fd, pus, sizeof(prusage_t))==-1)
    {
      retval = errno;
    }

  close(fd);

  return retval;
}

#define CALCULATE_MILLISEC(t) \
  ((t.tv_sec * MILLISEC) + (t.tv_nsec / (1000 * 1000)))

/*
 * Class:     com_altibase_picl_ProcCpu
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;J)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_ProcCpu_update
(JNIEnv * env, jobject proccpu_obj, jobject picl_obj, jlong pid)
{
  jfieldID sys_fid, user_fid, prev_ctime_fid, prev_utime_fid, prev_stime_fid, prev_cutime_fid, prev_cstime_fid;
  
  jlongArray mPrevTime;
  jlong mPrevTimeArr[5];
  double mUserPerc;
  double mSysPerc;

  int i, core;
  int j=0;

  unsigned long long sdelta;
  unsigned long long udelta;
  unsigned long long cdelta;
  unsigned long long cudelta;
  unsigned long long csdelta;

  unsigned long long prev_utime;
  unsigned long long prev_stime;
  unsigned long long prev_ctime;

  unsigned long long prev_cutime;
  unsigned long long prev_cstime;

  cpu_stat_t *knp;
  unsigned long long CURRENT_CP_USER = 0;
  unsigned long long CURRENT_CP_SYS = 0;
  unsigned long long CURRENT_CP_WAIT = 0;
  unsigned long long CURRENT_CP_IDLE = 0;
  uint_t cpu_time[CPU_STATES];

  /* kstat variable */
  kstat_t* ksp;

  /* Get a reference to obj's class */
  jclass proccpu_cls = (*env)->GetObjectClass(env, proccpu_obj);
  jclass picl_cls = (*env)->GetObjectClass(env, picl_obj);  

  /* Look for the instance field mPrevTime in cls */
  sys_fid = (*env)->GetFieldID(env, proccpu_cls, "mSysPerc", "D");
  user_fid = (*env)->GetFieldID(env, proccpu_cls, "mUserPerc", "D");
  prev_ctime_fid = (*env)->GetFieldID(env, proccpu_cls, "mPrevCtime", "J");
  prev_utime_fid = (*env)->GetFieldID(env, proccpu_cls, "mPrevUtime", "J");
  prev_stime_fid = (*env)->GetFieldID(env, proccpu_cls, "mPrevStime", "J");
  
  prev_cutime_fid = (*env)->GetFieldID(env, proccpu_cls, "mPrevCUtime", "J");
  prev_cstime_fid = (*env)->GetFieldID(env, proccpu_cls, "mPrevCStime", "J");

  if(sys_fid == NULL || 
     user_fid == NULL || 
     prev_ctime_fid == NULL || 
     prev_utime_fid == NULL || 
     prev_stime_fid == NULL ||
     prev_cstime_fid == NULL ||
     prev_cutime_fid == NULL)
  {
    return;// failed to find the field
  }

  /* Get the ProcCpu object member variable */
  mSysPerc = (*env)->GetDoubleField(env, proccpu_obj, sys_fid);
  mUserPerc = (*env)->GetDoubleField(env, proccpu_obj, user_fid);
  prev_utime = (*env)->GetLongField(env, proccpu_obj, prev_utime_fid);
  prev_stime = (*env)->GetLongField(env, proccpu_obj, prev_stime_fid);
  prev_ctime = (*env)->GetLongField(env, proccpu_obj, prev_ctime_fid);
  prev_cutime = (*env)->GetLongField(env, proccpu_obj, prev_cutime_fid);
  prev_cstime = (*env)->GetLongField(env, proccpu_obj, prev_cstime_fid);

  if(pid==-1)
  {
    mSysPerc = -1.0;
    mUserPerc = -1.0;
  }
  else
  {
    prusage_t pus;

    core = sysconf(_SC_NPROCESSORS_CONF);

    getProcUsage(&pus, (pid_t)pid);
    
    // kstat_ctl_t initialize
    if( kc == 0 )
    {
      kc = kstat_open();
    }

    cpu_stat_t newdata[core];

    for (ksp = kc->kc_chain; ksp != NULL; ksp = ksp->ks_next)
      {
	if(strncmp(ksp->ks_name, "cpu_stat", strlen("cpu_stat"))==0)
	  {
	    kstat_read(kc, ksp, &newdata[j]);

	    CURRENT_CP_USER += newdata[j].cpu_sysinfo.cpu[CPU_USER];
	    CURRENT_CP_SYS  += newdata[j].cpu_sysinfo.cpu[CPU_KERNEL];
	    CURRENT_CP_WAIT += newdata[j].cpu_sysinfo.cpu[CPU_WAIT];
	    CURRENT_CP_IDLE += newdata[j].cpu_sysinfo.cpu[CPU_IDLE];

	    j++;
	  }
      }


    /*    for(i=0; i<core; i++)
    {
      cpu = kstat_lookup(kc, "cpu_stat", i, NULL);
    
      kstat_read(kc, cpu, &newdata[i]);
    
      CURRENT_CP_USER += newdata[i].cpu_sysinfo.cpu[CPU_USER];
      CURRENT_CP_SYS  += newdata[i].cpu_sysinfo.cpu[CPU_KERNEL];
      CURRENT_CP_WAIT += newdata[i].cpu_sysinfo.cpu[CPU_WAIT];
      CURRENT_CP_IDLE += newdata[i].cpu_sysinfo.cpu[CPU_IDLE];
      }*/

    int ticks = sysconf(_SC_CLK_TCK);
    int tick2msec = 1000 / (double) ticks;

    CURRENT_CP_USER = CURRENT_CP_USER * tick2msec;
    CURRENT_CP_SYS = CURRENT_CP_SYS * tick2msec;
    CURRENT_CP_WAIT = CURRENT_CP_WAIT * tick2msec;
    CURRENT_CP_IDLE = CURRENT_CP_IDLE * tick2msec;

    udelta = CALCULATE_MILLISEC(pus.pr_utime) - prev_utime; // 10^-3 s
    sdelta = CALCULATE_MILLISEC(pus.pr_stime) - prev_stime;
    
    cdelta = CURRENT_CP_USER + CURRENT_CP_SYS + CURRENT_CP_WAIT + CURRENT_CP_IDLE - prev_ctime; // 10^-2 s
    cudelta = CURRENT_CP_USER - prev_cutime; // 10^-2 s
    csdelta = CURRENT_CP_SYS - prev_cstime;

    if(udelta > cudelta)
    {
      udelta = cudelta;
    }

    mSysPerc = (double)100.0 * sdelta / cdelta;
    mUserPerc = (double)100.0 * udelta / cdelta;

    prev_ctime = CURRENT_CP_USER + CURRENT_CP_SYS + CURRENT_CP_WAIT + CURRENT_CP_IDLE;
    prev_cutime = CURRENT_CP_USER;
    prev_cstime = CURRENT_CP_SYS;
    prev_utime = CALCULATE_MILLISEC(pus.pr_utime);
    prev_stime = CALCULATE_MILLISEC(pus.pr_stime); 
  }
  
  (*env)->SetDoubleField(env, proccpu_obj, sys_fid, mSysPerc);
  (*env)->SetDoubleField(env, proccpu_obj, user_fid, mUserPerc);
  (*env)->SetLongField(env, proccpu_obj, prev_utime_fid, prev_utime);
  (*env)->SetLongField(env, proccpu_obj, prev_stime_fid, prev_stime);
  (*env)->SetLongField(env, proccpu_obj, prev_ctime_fid, prev_ctime);
  (*env)->SetLongField(env, proccpu_obj, prev_cstime_fid, prev_cstime);
  (*env)->SetLongField(env, proccpu_obj, prev_cutime_fid, prev_cutime);
}

/*
 * Class:     com_altibase_picl_Memory
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_Memory_update
(JNIEnv * env, jobject mem_obj, jobject picl_obj)
{
  jfieldID memUsage_fid;
  jlongArray memUsage;
  jlong* memUsageArr;

  jboolean isCopy = JNI_TRUE;
  
  /* Variable for calculate Memory Usage */

  /* Get a reference to obj's class */
  jclass mem_cls = (*env)->GetObjectClass(env, mem_obj);
  jclass picl_cls = (*env)->GetObjectClass(env, picl_obj);

  /* Look for the instance field mMemoryUsage in cls */
  memUsage_fid = (*env)->GetFieldID(env, mem_cls, "mMemoryUsage", "[J");

  if(memUsage_fid == NULL)
  {
    return;// failed to find the field
  }

  memUsage = (jlongArray)(*env)->GetObjectField(env, mem_obj, memUsage_fid);

  // Native Type
  memUsageArr = (*env)->GetLongArrayElements(env, memUsage, &isCopy);

  // Calculate Memory Usage
  // Virtual Memory = Real Memory + Paging Space
  //printf("Total Real Memory Pages : %ld\n", ps_dyn.psd_rm);
  //printf("Active Real Memory Pages : %ld\n", ps_dyn.psd_arm);
  //printf("Total Virtual Memory Pages : %ld\n", ps_dyn.psd_vm);
  //printf("Active Virtual Memory Pages : %ld\n", ps_dyn.psd_avm);
  //printf("Free Memory Pages : %ld\n", ps_dyn.psd_free);
  //memUsageArr[MemTotal] = pstatic.physical_memory * 4;
  //memUsageArr[MemFree] = ps_dyn.psd_free * 4;
  //memUsageArr[MemUsed] = memUsageArr[MemTotal] - memUsageArr[MemFree];

  memUsageArr[MemTotal] = (sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE)) >> 10; 
  memUsageArr[MemFree] = (sysconf(_SC_AVPHYS_PAGES) * sysconf(_SC_PAGE_SIZE)) >> 10;
  memUsageArr[MemUsed] = memUsageArr[MemTotal] - memUsageArr[MemFree];

  // Save into Java Object
  (*env)->ReleaseLongArrayElements(env, memUsage, memUsageArr, 0);
  (*env)->SetObjectField(env, mem_obj, memUsage_fid, memUsage);  
}

int getMemUsage(psinfo_t *ps, pid_t aPid)
{
  int fd, retval = 1;
  char buffer[BUFSIZ];

  getProcFileName(buffer, aPid, "/psinfo", sizeof("/psinfo")-1);

  if ((fd = open(buffer, O_RDONLY)) < 0)
  {
    return ESRCH;
  }

  if (read(fd, ps, sizeof(psinfo_t))==-1)
  {
    retval = errno;
  }

  close(fd);

  return retval;
}


/*
 * Class:     com_altibase_picl_ProcMemory
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;J)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_ProcMemory_update
(JNIEnv * env, jobject procmem_obj, jobject picl_obj, jlong pid)
{
  jfieldID memUsed_fid;
  long memUsed;

  /* Get a reference to obj's class */
  jclass procmem_cls = (*env)->GetObjectClass(env, procmem_obj);
  jclass picl_cls = (*env)->GetObjectClass(env, picl_obj);

  /* Look for the instance field mMemoryUsage in cls */
  memUsed_fid = (*env)->GetFieldID(env, procmem_cls, "mMemUsed", "J");

  if(memUsed_fid == NULL)
  {
    return;// failed to find the field
  }

  /* Get the ProcCpu object member variable */
  memUsed = (*env)->GetLongField(env, procmem_obj, memUsed_fid);
  
  if(pid==-1)
  {
    memUsed = -1;
  }
  else
  {
    psinfo_t ps;

    getMemUsage(&ps, (pid_t)pid);
  
    memUsed = ps.pr_rssize;
  }

  // Save into Java Object
  (*env)->SetLongField(env, procmem_obj, memUsed_fid, memUsed);
}

/*
 * Class:     com_altibase_picl_Swap
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_Swap_update
(JNIEnv * env, jobject swap_obj, jobject picl_obj)
{
  jfieldID swapTotal_fid, swapFree_fid;
  unsigned long long swapTotal, swapFree;
  int i=0;

  /* Get a reference to obj's class */
  jclass swap_cls = (*env)->GetObjectClass(env, swap_obj);
  jclass picl_cls = (*env)->GetObjectClass(env, picl_obj);

  /* Look for the instance field in swap_cls */
  swapTotal_fid = (*env)->GetFieldID(env, swap_cls, "mSwapTotal", "J");
  swapFree_fid = (*env)->GetFieldID(env, swap_cls, "mSwapFree", "J");

  if(swapTotal_fid == NULL || swapFree_fid == NULL)
  {
    return;// failed to find the field
  }

  /* Get the ProcCpu object member variable */
  swapTotal = (*env)->GetLongField(env, swap_obj, swapTotal_fid);
  swapFree = (*env)->GetLongField(env, swap_obj, swapFree_fid);

  swaptbl_t *st;
  int swap_count;
  long pgsize_in_kbytes = sysconf(_SC_PAGE_SIZE) / 1024L;

  if ((swap_count=swapctl(SC_GETNSWP, NULL)) == -1){}


  st = (swaptbl_t*)malloc(sizeof(int) + swap_count * sizeof(struct swapent));

  st->swt_n = swap_count;
  for (i=0; i < swap_count; i++) {
    if ((st->swt_ent[i].ste_path = (char*)malloc(MAXPATHLEN)) == NULL){
    }
  }

  if ((swap_count=swapctl(SC_LIST, (void*)st)) == -1){
    perror("swapctl(SC_LIST)");
  }
  for (i=0; i < swap_count; i++) {
    swapTotal += st->swt_ent[i].ste_pages * pgsize_in_kbytes;
    swapFree += st->swt_ent[i].ste_free * pgsize_in_kbytes;
    free(st->swt_ent[i].ste_path);
  }

  free(st);
  
  // Save into Java Object
  (*env)->SetLongField(env, swap_obj, swapTotal_fid, swapTotal);
  (*env)->SetLongField(env, swap_obj, swapFree_fid, swapFree);
}

#define DIR_NAME_MAX 4096

typedef struct {
  long disk_usage;
} dir_usage_t;

int getDirUsage(const char *dirname, dir_usage_t *dirstats)
{
  int status;
  char name[DIR_NAME_MAX+1];
  int len = strlen(dirname);
  int max = sizeof(name)-len-1;
  char *ptr = name;

  struct dirent *ent;
  struct stat info;
  struct dirent dbuf;

  DIR *dirp = opendir(dirname);

  if(!dirp) {
    return;
  }

  strncpy(name, dirname, sizeof(name));
  ptr += len;
  if (name[len] != '/') {
    *ptr++ = '/';
    len++;
    max--;
  }

  while ((ent = readdir(dirp)))
  {
    if (ent == NULL)
    {
      break;
    }
    
    if (((ent->d_name[0] == '.') && (!ent->d_name[1] || ((ent->d_name[1] == '.') && !ent->d_name[2])))) {
      continue;
    }
    
    strncpy(ptr, ent->d_name, max);
    ptr[max] = '\0';
    
    if (lstat(name, &info) != 0) {
      continue;
    }
    
    dirstats->disk_usage += info.st_size;
    
    if((info.st_mode & S_IFMT) == S_IFDIR)
    {
      getDirUsage(name, dirstats);
    }
  }
  
  closedir(dirp);
  
  return 1;
}


/*
 * Class:     com_altibase_picl_Disk
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_Disk_update
(JNIEnv *env, jobject disk_obj, jobject picl_obj , jstring aPath)
{
  jfieldID dirusage_fid;

  const char *dirname;
  dir_usage_t dirusage;

  /* Get a reference to obj's class */
  jclass disk_cls = (*env)->GetObjectClass(env, disk_obj);

  /* Look for the instance field mMemoryUsage in cls */
  dirusage_fid = (*env)->GetFieldID(env, disk_cls, "mDirUsage", "J");

  dirusage.disk_usage = -1;

  dirname = (*env)->GetStringUTFChars(env, aPath, NULL);

  getDirUsage(dirname, &dirusage);

  // Save into Java Object
  (*env)->SetLongField(env, disk_obj, dirusage_fid, dirusage.disk_usage);
}

#define DEVICE_INFO_MAX 256
#define DEVICE_MAX 10

typedef struct {
  char dir_name[DIR_NAME_MAX];
  char dev_name[DIR_NAME_MAX];
} device_t_;

typedef struct {
  unsigned long number;
  unsigned long size;
  device_t_ *data;
} device_list_t_;

int getDeviceList(device_list_t_ *fslist)
{

  struct mnttab ent;
  device_t_ *fsp;
  FILE *fp = fopen(MNTTAB, "r");

  if (!fp) {
    return errno;
  }

  fslist->number = 0;
  fslist->size = DEVICE_MAX;
  fslist->data = malloc(sizeof(*(fslist->data))*fslist->size);

  while (getmntent(fp, &ent) == 0)
  {
    if (strstr(ent.mnt_mntopts, "ignore")) {
      continue;
    }

    if (fslist->number >= fslist->size) { 
      fslist->data = realloc(fslist->data, sizeof(*(fslist->data))*(fslist->size + DEVICE_MAX));
      fslist->size += DEVICE_MAX;
    }
    
    fsp = &fslist->data[fslist->number++];
      
    strncpy(fsp->dir_name, ent.mnt_mountp, sizeof(fsp->dir_name));
    fsp->dir_name[sizeof(fsp->dir_name)-1] = '\0';
    strncpy(fsp->dev_name, ent.mnt_special, sizeof(fsp->dev_name));
    fsp->dev_name[sizeof(fsp->dev_name)-1] = '\0';
  }
  
  fclose(fp);
  
  return 1;

}

enum {
  DEVICE_DIRNAME,
  DEVICE_DEVNAME,
    DEVICE_FIELD_MAX
};


/*
 * Class:     com_altibase_picl_DiskLoad
 * Method:    getDeviceListNative
 * Signature: ()[Lcom/altibase/picl/Device;
 */
JNIEXPORT jobjectArray JNICALL Java_com_altibase_picl_DiskLoad_getDeviceListNative
  (JNIEnv *env, jobject device_obj)
{
  int status;
  int i;
  device_list_t_ device_list;
  jobjectArray device_arr;
  jfieldID fids[DEVICE_FIELD_MAX];
  jclass nfs_cls=NULL;
  jclass device_cls = (*env)->FindClass(env, "com/altibase/picl/Device");

  if ((status = getDeviceList(&device_list)) != 1) {
    return NULL;
  }

  fids[DEVICE_DIRNAME] = (*env)->GetFieldID(env, device_cls, "mDirName", "Ljava/lang/String;");
  fids[DEVICE_DEVNAME] = (*env)->GetFieldID(env, device_cls, "mDevName", "Ljava/lang/String;");

  device_arr = (*env)->NewObjectArray(env, device_list.number, device_cls, 0);

  for (i=0; i<device_list.number; i++) {
    device_t_ *device = &(device_list.data)[i];
    jobject temp_obj;

    temp_obj = (*env)->AllocObject(env, device_cls);
    
    (*env)->SetObjectField(env, temp_obj, fids[DEVICE_DIRNAME], (*env)->NewStringUTF(env, device->dir_name));
    (*env)->SetObjectField(env, temp_obj, fids[DEVICE_DEVNAME], (*env)->NewStringUTF(env, device->dev_name));

    (*env)->SetObjectArrayElement(env, device_arr, i, temp_obj);
  }

  if (device_list.size > 0 ) {
    free(device_list.data);
    device_list.number = 0;
    device_list.size = 0;
  }
  
  return device_arr;
}

void getDevName(const char *mountName, char **devName)
{
  device_list_t_ device_list;
  int i;

  if(getDeviceList(&device_list) == 1)
  {
    for (i=0; i<device_list.number; i++) {
      device_t_ *device = &(device_list.data)[i];
      if(strncmp(device->dir_name, mountName, strlen(mountName))==0)
      {
	*devName = device->dev_name;
	break;
      }
    }
  }

  if (device_list.size > 0 ) {
    free(device_list.data);
    device_list.number = 0;
    device_list.size = 0;
  }
}

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

#define CONVERT_TO_BYTES(numOfBlocks, blockSize) ((numOfBlocks * blockSize) >> 1)

#define empty(ptr) \
  while (isspace((unsigned char)(*ptr))) ++ptr

int getDiskIo(const char *mountName, disk_load_t *diskLoad)
{
  struct stat buf;
  struct statvfs vbuf;
  char *devName;
  unsigned long numOfBlocks, blockSize;
  kstat_t     *ksp;

  getDevName(mountName, &devName);

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
    diskLoad->numOfReads = -1;
    diskLoad->numOfWrites = -1;
    diskLoad->read_bytes = -1;
    diskLoad->write_bytes = -1;
  }
  else
  {
    return 0;
  }

  if (stat(devName, &buf) == 0) {

    FILE *fp = fopen("/etc/path_to_inst", "r");

    char buffer[BUFSIZ], *ptr;
    char *dev, *inst, *drv;

    if(!fp) {
      return;
    }

    while ((ptr = fgets(buffer, sizeof(buffer), fp))) {
      char *q;

      empty(ptr);
      if (*ptr == '#') {
	continue;
      }
      if (*ptr == '"') {
	ptr++;
      }
      dev = ptr;
      if (!(q = strchr(ptr, '"'))) {
	continue;
      }
      ptr = q+1;
      *q = '\0';
      empty(ptr);
      inst = ptr;
      while (isdigit((unsigned char)*ptr)) {
	ptr++;
      }
      *ptr = '\0';
      ptr++;
      empty(ptr);
      if (*ptr == '"') {
	ptr++;
      }
      drv = ptr;
      if (!(q = strchr(ptr, '"'))) {
	continue;
      }
      *q = '\0';

      if (!(strcmp(drv, "sd")==0 ||
	    strcmp(drv, "ssd")==0 ||
	    strcmp(drv, "st")==0 ||
	    strcmp(drv, "dad")==0 ||
	    strcmp(drv, "cmdk")==0))
        {
	  continue;
        }

      if (!kstat_lookup(kc, drv, atoi(inst), NULL)) {
	continue;
      }

      char device[4096+1], *pos=device;
      int len = readlink(devName, device, sizeof(device)-1);

      if(len < 0)
      {
	continue;
      }

      char *s;
      char partition;

      device[len] = '\0';

      while (strncmp(pos, "../", 3)==0) {
	pos += 3;
      }
      
      if (strncmp(pos, "devices", 7)==0) {
	pos += 7;
      }
      
      if ((s = strchr(pos, ':'))) {
	partition = *(s+1);
      }
      else
      {
	continue;
      }

      if (strncmp(dev, pos, strlen(dev))==0) {
	//**************** mapping to paths->name[0] loop make
	char buff[256];
	//printf("drv[name] : %s\n", drv);
	//printf("instance : %d\n", atoi(inst));
	//printf("partition : %c\n", partition);
	//printf("%s%d,%c\n", drv, atoi(inst), partition);

	snprintf(buff, sizeof(buff), "%s%d,%c",
		 drv, atoi(inst), partition);
	
	if( kc == 0 )
	{
	  kc = kstat_open();
	}

	kstat_chain_update(kc);

	for (ksp = kc->kc_chain;ksp!=NULL;ksp = ksp->ks_next)
	{
	  if (ksp->ks_type == KSTAT_TYPE_IO) {
	    kstat_io_t kio;
	    kstat_read(kc, ksp, &kio);
	    if(strcmp(ksp->ks_name, buff)==0)
	    {
	      //(void)printf("%12s      %10llu       %10llu    %10u     %10u\n",
	      //	   ksp->ks_name, kio.nread, kio.nwritten,
	      //	   kio.reads, kio.writes);
	      
	      diskLoad->numOfReads = kio.nread;
	      diskLoad->numOfWrites = kio.nwritten;
	      diskLoad->read_bytes = kio.reads;
	      diskLoad->read_bytes = kio.writes;
	      
	      break;
	    }
	  }
	}
      }
    }

    fclose(fp);

    /*
    struct pst_lvinfo lv;

    int retval = pstat_getlv(&lv, sizeof(lv), 0, (int)buf.st_rdev);

    if (retval == 1) {
      diskLoad->numOfReads  = lv.psl_rxfer;
      diskLoad->numOfWrites = lv.psl_wxfer;
      diskLoad->read_bytes  = lv.psl_rcount;
      diskLoad->write_bytes = lv.psl_wcount;
    }
    */
  }
  else {
    return 0;
  }
  
  return 1;
}

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


/*
 * Class:     com_altibase_picl_DiskLoad
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;)V
 */
JNIEXPORT jobject JNICALL Java_com_altibase_picl_DiskLoad_update
(JNIEnv *env, jobject diskload_obj, jobject picl_obj, jstring dirname)
{
  disk_load_t disk_io;
  int status;
  jclass cls = (*env)->GetObjectClass(env, diskload_obj);
  jclass iops_cls = (*env)->FindClass(env, "com/altibase/picl/Iops");
  jfieldID fids[IO_FIELD_MAX];
  jobject iops_obj;
  const char* name;
  struct timeval currentTime;

  clock_t currentTick;

  name = (*env)->GetStringUTFChars(env, dirname, 0);

  status = getDiskIo(name, &disk_io);//Get current disk io value
  gettimeofday(&currentTime, NULL);
  fids[IO_FIELD_DIRNAME] = (*env)->GetFieldID(env, iops_cls, "mDirName", "Ljava/lang/String;");
  fids[IO_FIELD_READ] = (*env)->GetFieldID(env, iops_cls, "mRead", "J");
  fids[IO_FIELD_WRITE] = (*env)->GetFieldID(env, iops_cls, "mWrite", "J");
  fids[IO_FIELD_TIME] = (*env)->GetFieldID(env, iops_cls, "mTime4Sec", "J"); 
  fids[IO_FIELD_TOTAL] = (*env)->GetFieldID(env, iops_cls, "mTotalSize", "J");
  fids[IO_FIELD_AVAIL] = (*env)->GetFieldID(env, iops_cls, "mAvailSize", "J");
  fids[IO_FIELD_USED] = (*env)->GetFieldID(env, iops_cls, "mUsedSize", "J");
  fids[IO_FIELD_FREE] = (*env)->GetFieldID(env, iops_cls, "mFreeSize", "J");

  iops_obj = (*env)->AllocObject(env, iops_cls);

  (*env)->SetObjectField(env, iops_obj, fids[IO_FIELD_DIRNAME], (*env)->NewStringUTF(env, name));
  (*env)->SetLongField(env, iops_obj, fids[IO_FIELD_READ], disk_io.numOfReads);
  (*env)->SetLongField(env, iops_obj, fids[IO_FIELD_WRITE], disk_io.numOfWrites);
  (*env)->SetLongField(env, iops_obj, fids[IO_FIELD_TIME], currentTime.tv_sec);
  (*env)->SetLongField(env, iops_obj, fids[IO_FIELD_TOTAL], disk_io.total_bytes);
  (*env)->SetLongField(env, iops_obj, fids[IO_FIELD_AVAIL], disk_io.avail_bytes);
  (*env)->SetLongField(env, iops_obj, fids[IO_FIELD_USED], disk_io.used_bytes);
  (*env)->SetLongField(env, iops_obj, fids[IO_FIELD_FREE], disk_io.free_bytes);

  return iops_obj;
}




/*
 * Class:     com_altibase_picl_ProcessFinder
 * Method:    getPid
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_altibase_picl_ProcessFinder_getPid
(JNIEnv *env, jclass cls, jstring aProcPath)
{
  DIR *procDir;                 // A variable to point directory /proc/pid
  struct dirent *entry;         // A structure to select specific file to use I-node
  struct stat fileStat;         // A structure to store file information

  int pid;
  char cmdLine[256];
  char tempPath[256];
  const char *str;

  int pathLength;

  str = (*env)->GetStringUTFChars(env, aProcPath, NULL);

  if(str == NULL) {
    return -1;
  }

  procDir = opendir("/proc");

  while((entry = readdir(procDir)) != NULL ) {
    
    if(!isdigit((unsigned char)*entry->d_name)) {
      continue;
    }

    pid = atoi(entry->d_name);

    if(pid <= 0)
    {
	continue;
    }

    sprintf(tempPath, "/proc/%d/psinfo", pid);
    
    getCmdLine(tempPath, cmdLine);

    pathLength = strlen(str);

    if(strncmp(str, cmdLine, pathLength)==0)
    {
      break;
    }
  }

  if(strncmp(str, cmdLine, pathLength)!=0)
  {
    pid = -1;
  }

  closedir(procDir);

  (*env)->ReleaseStringUTFChars(env, aProcPath, str);

  return (long)pid;

}

int getCmdLine(char *file, char *buf)
{
  psinfo_t pinfo;
  int procfd; 
  int i;

  if((procfd = open(file, O_RDONLY)) == -1)
  {
    return -1;
  }

  if(read(procfd, (void *)&pinfo, sizeof(pinfo)) < 0)
  {
    return -1;
  }

  strncpy(buf, pinfo.pr_psargs, strlen(pinfo.pr_psargs));

  close(procfd);

  return 0;
}
