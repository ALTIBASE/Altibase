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

int getDirUsage(const char *dirname, dir_usage_t *dirusage)
{
  int status;
  char name[DIR_NAME_MAX+1];
  int len = strlen(dirname);
  int max = sizeof(name)-len-1;
  char *ptr = name;

  struct dirent *ent;
  struct stat info;
  struct dirent dbuf;

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
    return;
  }

  // To copy dirname into name buffer
  strncpy(name, dirname, sizeof(name));

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
  while (readdir_r(dirp, &dbuf, &ent) == 0)
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
    dirusage->disk_usage += info.st_size;

    // If the subdirectory type is directory, we call the function getDirUsage recursively.
    if((info.st_mode & S_IFMT) == S_IFDIR)
    {
      getDirUsage(name, dirusage);
    }
  }

  closedir(dirp);

  return 1;
}

int getDeviceList(device_list_t_ *device_list)
{
  struct mntent *ent;

  FILE *fp;

  device_t_ *device;

  if(!(fp = setmntent(MNT_CHECKLIST, "r")))
  {
    return errno;
  }

  device_list->number = 0;
  device_list->size = DEVICE_MAX;
  device_list->data = malloc(sizeof(*(device_list->data))*device_list->size);

  while ((ent = getmntent(fp)))
  {
    // To skip other device type
    if (((*(ent->mnt_type) == 's') && (strcmp(ent->mnt_type, "swap") == 0))
	|| (*(ent->mnt_type) == 'i') || (*(ent->mnt_type) == 'a')
	|| (*(ent->mnt_type) == 's') || (*(ent->mnt_type) == 'n')
	|| (*(ent->mnt_type) == 'c'))
    {
      continue;
    }

    if (device_list->number >= device_list->size) {
      device_list->data = realloc(device_list->data, sizeof(*(device_list->data))*(device_list->size + DEVICE_MAX));
      device_list->size += DEVICE_MAX;
    }

    device = &device_list->data[device_list->number++];

    // To get device name and mount name
    strncpy(device->dir_name, ent->mnt_dir, sizeof(device->dir_name));
    device->dir_name[sizeof(device->dir_name)-1] = '\0';
    strncpy(device->dev_name, ent->mnt_fsname, sizeof(device->dev_name));
    device->dev_name[sizeof(device->dev_name)-1] = '\0';
  }

  endmntent(fp);

  return 1;
}
