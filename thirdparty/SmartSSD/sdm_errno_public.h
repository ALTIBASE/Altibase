/******************************************************************************/
/*                                                                            */
/*            COPYRIGHT 2009-2020 SAMSUNG ELECTRONICS CO., LTD.               */
/*                          ALL RIGHTS RESERVED                               */
/*                                                                            */
/* Permission is hereby granted to licensees of Samsung Electronics Co., Ltd. */
/* products to use or abstract this computer program only in  accordance with */
/* the terms of the NAND FLASH MEMORY SOFTWARE LICENSE AGREEMENT for the sole */
/* purpose of implementing  a product based  on Samsung Electronics Co., Ltd. */
/* products. No other rights to  reproduce, use, or disseminate this computer */
/* program, whether in part or in whole, are granted.                         */
/*                                                                            */
/* Samsung Electronics Co., Ltd. makes no  representation or warranties  with */
/* respect to  the performance  of  this computer  program,  and specifically */
/* disclaims any  responsibility for  any damages,  special or consequential, */
/* connected with the use of this program.                                    */
/*                                                                            */
/******************************************************************************/

/* 
 * SDM Error Number Definitions
 */

#ifndef __SDM_ERRNO_PUBLIC_H__
#define __SDM_ERRNO_PUBLIC_H__

#include <errno.h>

typedef int sdm_error_t;
#define    SUCCESS            0
#define    ENOSUP             0x10000 /* not supported API */
#define    EINVALID_HANDLE    0x10001 /* invalid sdm_handle */

#endif //__SDM_ERRNO_PUBLIC_H__
