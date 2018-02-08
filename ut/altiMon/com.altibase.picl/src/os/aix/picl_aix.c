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
 * Picl_aix.c
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
#include <unistd.h>

#include <fcntl.h>
#include <sys/time.h>
#include <sys/procfs.h>
#include <libperfstat.h>

#include <procinfo.h>
#include <sys/cfgodm.h>
#include <odmi.h>
#include <cf.h>

#define BURST 20

#define MemFree		0
#define MemUsed		1
#define MemTotal	2

#define CP_USER 0
#define CP_SYS  1
#define CP_NICE 2
#define CP_WAIT 3
#define CP_IDLE 4

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

  /* Variable for calculate CPU Percentage */
  float delta; 

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

  // JNI Object Type
  mPrevTime = (jlongArray)(*env)->GetObjectField(env, picl_obj, prev_fid);
  mSysPerc = (*env)->GetDoubleField(env, cpu_obj, sys_fid);
  mUserPerc = (*env)->GetDoubleField(env, cpu_obj, user_fid);

  // Native Type
  jlong* mPrevTimeArr = (*env)->GetLongArrayElements(env, mPrevTime, &isCopy);
  
  perfstat_cpu_total_t cpu_total_buffer;

  /* get initial set of data */
  perfstat_cpu_total(NULL, &cpu_total_buffer, sizeof(perfstat_cpu_total_t), 1);

  /* save values for delta calculations */
  delta = cpu_total_buffer.user - mPrevTimeArr[CP_USER] +
      cpu_total_buffer.sys - mPrevTimeArr[CP_SYS] +
      cpu_total_buffer.idle - mPrevTimeArr[CP_IDLE] +
      cpu_total_buffer.wait - mPrevTimeArr[CP_WAIT];

  if(delta==0)
  {
      mSysPerc = 0.0;
      mUserPerc = 0.0;

      mPrevTimeArr[CP_USER] = cpu_total_buffer.user;
      mPrevTimeArr[CP_SYS] = cpu_total_buffer.sys;
      mPrevTimeArr[CP_WAIT] = cpu_total_buffer.wait;
      mPrevTimeArr[CP_IDLE] = cpu_total_buffer.idle;
  }
  else
  {
      mSysPerc = (double)(cpu_total_buffer.sys - mPrevTimeArr[CP_SYS]) / delta * 100.00;
      mUserPerc = (double)(cpu_total_buffer.user - mPrevTimeArr[CP_USER]) / delta * 100.00;
      
      mPrevTimeArr[CP_USER] = cpu_total_buffer.user;
      mPrevTimeArr[CP_SYS] = cpu_total_buffer.sys;
      mPrevTimeArr[CP_WAIT] = cpu_total_buffer.wait;
      mPrevTimeArr[CP_IDLE] = cpu_total_buffer.idle;
  }
  
  (*env)->ReleaseLongArrayElements(env, mPrevTime, mPrevTimeArr, 0);

  /* Read the instance field mSysPerc */
  (*env)->SetObjectField(env, picl_obj, prev_fid, mPrevTime);
  (*env)->SetDoubleField(env, cpu_obj, sys_fid, mSysPerc);
  (*env)->SetDoubleField(env, cpu_obj, user_fid, mUserPerc);  
}

#include <procinfo.h>

#define CALCULATE_MSEC 1000L

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

    unsigned long long current_utime;
    unsigned long long current_stime;
    
    unsigned long long CURRENT_CP_USER = 0;
    unsigned long long CURRENT_CP_SYS = 0;
    unsigned long long CURRENT_CP_WAIT = 0;
    unsigned long long CURRENT_CP_IDLE = 0;


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

        return;
        // failed to find the field
    }

    /* Get the ProcCpu object member variable */
    mSysPerc = (*env)->GetDoubleField(env, proccpu_obj, sys_fid);

    mUserPerc = (*env)->GetDoubleField(env, proccpu_obj, user_fid);

    prev_utime = (*env)->GetLongField(env, proccpu_obj, prev_utime_fid);

    prev_stime = (*env)->GetLongField(env, proccpu_obj, prev_stime_fid);

    prev_ctime = (*env)->GetLongField(env, proccpu_obj, prev_ctime_fid);

    prev_cutime = (*env)->GetLongField(env, proccpu_obj, prev_cutime_fid);

    prev_cstime = (*env)->GetLongField(env, proccpu_obj, prev_cstime_fid);

    perfstat_cpu_total_t cpu_total_buffer;

    /* get initial set of data */
    perfstat_cpu_total(NULL, &cpu_total_buffer, sizeof(perfstat_cpu_total_t), 1);

    if(pid!=-1)
    {
        int ticks = sysconf(_SC_CLK_TCK);
        int tick2msec = CALCULATE_MSEC / (double)ticks;

        //Calculate
        struct procsinfo64 pinfo;
        
        //pinfo = malloc(sizeof(*pinfo));
        pid_t retval = (pid_t)pid;
        
        getprocs(&pinfo, sizeof(struct procsinfo64), 0, 0, &retval, 1);

        // Actually, tv_usec include nano-second value
        
        current_utime = (unsigned long long)(pinfo.pi_ru.ru_utime.tv_sec * 1000) + (unsigned long long)(pinfo.pi_ru.ru_utime.tv_usec / (1000 * 1000));
        current_stime = (unsigned long long)(pinfo.pi_ru.ru_stime.tv_sec * 1000) + (unsigned long long)(pinfo.pi_ru.ru_stime.tv_usec / (1000 * 1000));
        
        /* save values for delta calculations */
        CURRENT_CP_USER = cpu_total_buffer.user * tick2msec;
        CURRENT_CP_SYS = cpu_total_buffer.sys * tick2msec;
        CURRENT_CP_IDLE = cpu_total_buffer.idle * tick2msec;
        CURRENT_CP_WAIT = cpu_total_buffer.wait * tick2msec;

        udelta = current_utime - prev_utime;
        sdelta = current_stime - prev_stime;

        cdelta = CURRENT_CP_USER + CURRENT_CP_SYS + CURRENT_CP_WAIT + CURRENT_CP_IDLE - prev_ctime;
        
        cudelta = CURRENT_CP_USER - prev_cutime;
        csdelta = CURRENT_CP_SYS - prev_cstime;

        if(cudelta < udelta)
        {
            udelta = cudelta;
        }

        if(csdelta < sdelta)
        {
            sdelta = csdelta;
        }

        if(cdelta!=0){
            mSysPerc = 100.0 * ((double)sdelta) / cdelta;
            mUserPerc = 100.0 * ((double)udelta) / cdelta;
        }
        else
        {
            mSysPerc = 0.0;
            mUserPerc = 0.0;
        }
        
        prev_ctime = CURRENT_CP_USER + CURRENT_CP_SYS + CURRENT_CP_WAIT + CURRENT_CP_IDLE;
        prev_cutime = CURRENT_CP_USER;
        prev_cstime = CURRENT_CP_SYS;
        prev_utime = current_utime;
        prev_stime = current_stime;
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

  memUsageArr = (*env)->GetLongArrayElements(env, memUsage, &isCopy);
  
  //Calculate
  perfstat_memory_total_t minfo;

  perfstat_memory_total(NULL, &minfo, sizeof(perfstat_memory_total_t), 1);

  int pagesize = getpagesize();

  memUsageArr[MemTotal] = minfo.real_total * pagesize / 1024;
  memUsageArr[MemFree] = minfo.real_free * pagesize / 1024;
  memUsageArr[MemUsed] = memUsageArr[MemTotal] - memUsageArr[MemFree];
    
  // Save into Java Object
  (*env)->ReleaseLongArrayElements(env, memUsage, memUsageArr, 0);
  (*env)->SetObjectField(env, mem_obj, memUsage_fid, memUsage);  
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

  /* pstat information */

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

  //Calculate
  struct procsinfo64 pinfo;


  //pinfo = malloc(sizeof(*pinfo));
  pid_t retval = (pid_t)pid;


  getprocs(&pinfo, sizeof(struct procsinfo64), 0, 0, &retval, 1);

  memUsed = (pinfo.pi_drss + pinfo.pi_trss) * getpagesize() / 1024;
  
  
  // Save into Java Object
  (*env)->SetLongField(env, procmem_obj, memUsed_fid, memUsed);
}

#include <sys/vminfo.h>

/*
 * Class:     com_altibase_picl_Swap
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_Swap_update
(JNIEnv * env, jobject swap_obj, jobject picl_obj)
{
  jfieldID swapTotal_fid, swapFree_fid;
  long swapTotal, swapFree;
  int status, i=0;

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

  // Calculate
  struct pginfo p;
  int ret;

  CLASS_SYMBOL cuat;
  struct CuAt paging_ent;
  struct CuAt *pret;
  char buf[1024];
    
  if(odm_initialize() < 0)
  {
      return;
  }

  cuat = odm_open_class(CuAt_CLASS);

  if (cuat == NULL)
  {
      return;
  }

  pret = odm_get_obj(cuat, "value='paging'", &paging_ent, ODM_FIRST);

  while(pret != NULL)
  {
      memset(buf, 0, 1024);

      snprintf(buf, 1024, "%s/%s", "/dev/", paging_ent.name);

      ret = swapqry(buf, &p);

      if (ret == -1)
      {
          return;
      }

      swapTotal += p.size;
      
      swapFree += p.free;


      /* XXX make sure no memory leak occurs somehow when I call this */
      pret = odm_get_obj(cuat, NULL, &paging_ent, ODM_NEXT);
  }

  odm_close_class(cuat);


  if (odm_terminate() < 0)
  {
      return;
  }

  swapTotal *= PAGESIZE;
  swapFree *= PAGESIZE;
      
  // Save into Java Object
  (*env)->SetLongField(env, swap_obj, swapTotal_fid, swapTotal);
  (*env)->SetLongField(env, swap_obj, swapFree_fid, swapFree);
}

#define DIR_NAME_MAX 4096

typedef struct {
  int disk_usage;
} dir_usage_t;

int getDirUsage(const char *dirname, dir_usage_t *dirusage)
{
  int status;
  char name[DIR_NAME_MAX+1];
  int len = strlen(dirname);
  int max = sizeof(name)-len-1;
  char *ptr = name;

  struct dirent *emnt;
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

  while (readdir_r(dirp, &dbuf, &emnt) == 0)
  {
    if (emnt == NULL)
    {
      break;
    }

    if (((emnt->d_name[0] == '.') && (!emnt->d_name[1] || ((emnt->d_name[1] == '.') && !emnt->d_name[2])))) {
      continue;
    }
    
    strncpy(ptr, emnt->d_name, max);
    ptr[max] = '\0';
    
    if (lstat(name, &info) != 0) {
      continue;
    }
    
    dirusage->disk_usage += info.st_size;
    
    if((info.st_mode & S_IFMT) == S_IFDIR)
    {
      getDirUsage(name, dirusage);
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

  dirusage.disk_usage = 0;

  dirname = (*env)->GetStringUTFChars(env, aPath, NULL);

  getDirUsage(dirname, &dirusage);

  // Save into Java Object
  (*env)->SetLongField(env, disk_obj, dirusage_fid, dirusage.disk_usage);

  (*env)->ReleaseStringUTFChars(env, aPath, dirname);
}

#include <mntent.h>
#define DEVICE_INFO_MAX 256
#define DEVICE_MAX 10

typedef enum {
    UNKNOWN,
    NONE,
    LOCAL_DISK,
    NETWORK,
    RAM_DISK,
    CDROM,
    SWAP,
    MAX
}type_e;


typedef struct {
  char dir_name[DIR_NAME_MAX];
  char dev_name[DIR_NAME_MAX];
  char type_name[DEVICE_INFO_MAX];
  type_e type;
} device_t_;

typedef struct {
  unsigned long number;
  unsigned long size;
  device_t_ *data;
} device_list_t_;

int getDeviceList(device_list_t_ *fslist)
{
    int i, size, num;

    char *buf, *mntlist;


    /* get required size */
    if (mntctl(MCTL_QUERY, sizeof(size), (char *)&size) < 0) {

        return errno;

    }


    mntlist = buf = malloc(size);


    if ((num = mntctl(MCTL_QUERY, size, buf)) < 0) {

        free(buf);

        return errno;

    }
    
    fslist->number = 0;

    fslist->size = DEVICE_MAX;

    fslist->data = malloc(sizeof(*(fslist->data))*fslist->size);


    for (i=0; i<num; i++) {

        char *dnme;

        const char *typename = NULL;

        device_t_ *device_fsp;

        struct vmount *emnt = (struct vmount *)mntlist;


        mntlist += emnt->vmt_length;

		int hnme_len;
		int dnme_len;
		int total_len;
		
        if (fslist->number >= fslist->size) {

            fslist->data = realloc(fslist->data, sizeof(*(fslist->data))*(fslist->size + DEVICE_MAX));

            fslist->size += DEVICE_MAX;

        }

        device_fsp = &fslist->data[fslist->number++];

        switch (emnt->vmt_gfstype) {

            case MNT_AIX:

                typename = "aix";

                device_fsp->type = LOCAL_DISK;

                break;

            case MNT_JFS:

                typename = "jfs";

                device_fsp->type = LOCAL_DISK;

                break;

            case MNT_NFS:

            case MNT_NFS3:

                typename = "nfs";

                device_fsp->type = NETWORK;

                break;

            case MNT_CDROM:

                device_fsp->type = CDROM;

                break;

            case MNT_SFS:

            case MNT_CACHEFS:

            case MNT_AUTOFS:

            default:

                if (emnt->vmt_flags & MNT_REMOTE) {

                    device_fsp->type = NETWORK;

                }

                else {

                    device_fsp->type = NONE;

                }

        }
        
        
        strncpy(device_fsp->dir_name, vmt2dataptr(emnt, VMT_STUB), sizeof(device_fsp->dir_name));

        device_fsp->dir_name[sizeof(device_fsp->dir_name)-1] = '\0';

        dnme = vmt2dataptr(emnt, VMT_OBJECT);

        if (device_fsp->type == NETWORK) {
			char *hnme   = vmt2dataptr(emnt, VMT_HOSTNAME);
            #if 0            
            hnme_len = vmt2datasize(emnt, VMT_HOSTNAME)-1;            
            dnme_len  = vmt2datasize(emnt, VMT_OBJECT);            
            #else
            hnme_len = strlen(hnme);
            dnme_len = strlen(dnme) + 1;
            #endif
            total_len = hnme_len + dnme_len + 1;
            
            if (total_len > sizeof(device_fsp->dev_name)) {
             	/* sprintf(device_fsp->dnme, "%s:%s", hnme, dnme) */
                strncpy(device_fsp->dev_name, dnme, sizeof(device_fsp->dev_name));
                device_fsp->dev_name[sizeof(device_fsp->dev_name)-1] = '\0';
            }
            else {
                /* sprintf(device_fsp->dnme, "%s:%s", hnme, dnme) */
                char *place = device_fsp->dev_name;
                memcpy(place, hnme, hnme_len);
                place += hnme_len;
                *place++ = ':';
                memcpy(place, dnme, dnme_len);
            }

        }
        else {
            strncpy(device_fsp->dev_name, dnme, sizeof(device_fsp->dev_name));
            device_fsp->dev_name[sizeof(device_fsp->dev_name)-1] = '\0';
        }
    }

    free(buf);
    
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
    device_t_ *device =  &(device_list.data)[i];
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

int getDiskIo(const char *mountName, disk_load_t *diskLoad)
{
    int i, total, num;
    struct stat buf;
    struct statvfs vbuf;
    char *devName;
    unsigned long numOfBlocks, blockSize;
    
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
    }
    else
    {
        return 0;
    }
    
    perfstat_disk_t *disk;
    perfstat_id_t id;

    total = perfstat_disk(NULL, NULL, sizeof(*disk), 0);

    if (total < 1) {
        return ENOENT;
    }

    disk = malloc(total * sizeof(*disk));
    id.name[0] = '\0';
    num = perfstat_disk(&id, disk, sizeof(*disk), total);

    if (num < 1) {
        free(disk);
        return ENOENT;
    }

    odm_initialize();

    for (i=0; i<num; i++) {

        int check = 1;
                
        char query[256];

        struct CuDv *dv, *ptr;

        struct listinfo info;

        int j;


        snprintf(query, sizeof(query),
                 "parent = '%s'", disk[i].vgname);


        ptr = dv = odm_get_list(CuDv_CLASS, query, &info, 256, 1);

        if ((int)dv == -1) {

            continue;

        }


        for (j=0; j<info.num; j++, ptr++) {

            struct CuAt *attr;

            int num, retval;
            
            struct stat sb;


            if ((attr = getattr(ptr->name, "label", 0, &num))) {

                retval = stat(attr->value, &sb);


                if (retval == 0)
                {
                    if(strncmp(mountName, attr->value, strlen(mountName))==0)
                    {
                        diskLoad->numOfReads  = disk[i].rblks;
                        diskLoad->numOfWrites = disk[i].wblks;
                        diskLoad->read_bytes  = disk[i].rblks * disk[i].bsize;
                        diskLoad->write_bytes = disk[i].wblks * disk[i].bsize;
                        
                        
                        check=0;                        
                    }
                }

                free(attr);

            }

            if(check==0)
                break;
        }


        odm_free_list(dv, &info);

        if(check==0)
            break;
    }

    free(disk);
    odm_terminate();

    return 1;
}

#include <sys/time.h>
#include <unistd.h>

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
(JNIEnv *env, jobject diskload_obj, jobject picl_obj, jstring mountName)
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

  name = (*env)->GetStringUTFChars(env, mountName, 0);

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

  (*env)->ReleaseStringUTFChars(env, mountName, name);

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
    DIR *procDir;
    // A variable to point directory /proc/pid
    struct dirent *entry;
    // A structure to select specific file to use I-node
    struct stat fileStat;
    // A structure to store file information

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


