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
 * SDM public header file
 */

#ifndef __SDM_MGNT_PUBLIC_H__
#define __SDM_MGNT_PUBLIC_H__


/* C++ wrapper */
#ifdef __cplusplus
    extern "C" {
#endif

#include "sdm_types_public.h"
#include "sdm_errno_public.h"


/**
  * definitions
  *
  */
#define API_VERSION_MAJOR_NUMBER 1
#define API_VERSION_MINOR_NUMBER 0

#define CONTINUE_GC 0
#define STOP_GC     1

#define FEATURES_OPTIMIZED_READ_MGMT 0x0001
#define FEATURES_GARBAGE_COLLECTION_MGMT 0x0002
#define FEATURES_OVER_PROVISION_MGMT 0x0004

#define SET_OVER_PROVISION_TRIAL_QUERY 0
#define SET_OVER_PROVISION_FORCE_OPERATION 1


/**
  * public structure definitions
  *
  */
typedef struct {
    uint32_t major;
    uint32_t minor;
    uint64_t reserved;
} sdm_api_version_t;

typedef struct {
    int id;
    uint64_t reserved;
    sdm_api_version_t hd_api_ver;
} sdm_handle_t;
 
typedef struct {
    uint64_t start_lba;
    uint64_t len_sector;
} sdm_block_range_t;

typedef struct {
    /* Starting LBA address */
    uint64_t start_lba;
    /* Starting address of input buffer for each read */
    void *buf;
    /* Number of bytes to read */
    uint32_t len_byte;
} sdm_iovec_t;

typedef struct {
    /* optimized read management */
    uint8_t opti_rd_mgmt:1;
    uint8_t gc_mgmt:1;
    uint8_t op_mgmt:1;
    uint8_t reserved1: 5;
    /* reserved for future use */
    uint8_t reserved2[31];
} sdm_supp_features_t;


/**
  * Smart SSD V1.0 API prototypes start here
  *
  */

/********************          Basic APIs          ***************************/
sdm_error_t sdm_get_api_version(sdm_api_version_t *api_ver);

sdm_error_t sdm_open(const char *pathname, sdm_handle_t **handle, 
                     sdm_api_version_t *api_ver);

sdm_error_t sdm_get_supported_features(sdm_handle_t *handle, 
                                       sdm_supp_features_t *supp_features);

sdm_error_t sdm_close(sdm_handle_t *handle);

sdm_error_t sdm_get_user_capacity(sdm_handle_t *handle, 
                                  uint64_t *user_capacity);

sdm_error_t sdm_get_device_capacity(sdm_handle_t *handle, 
                                    uint64_t *device_capacity);

sdm_error_t sdm_get_current_eb_size (sdm_handle_t *handle, uint32_t *eb_size);

sdm_error_t sdm_get_free_block_count(sdm_handle_t *handle, uint64_t *count);



/********************        Optimized Read        ***************************/
sdm_error_t sdm_is_ready_for_read(sdm_handle_t *handle, 
                                  uint32_t *ready_for_read);



/********************        Garbage Collection    ***************************/
sdm_error_t sdm_get_gc_threshold(sdm_handle_t *handle, uint32_t *curr_low_wm, 
                                 uint32_t *curr_high_wm, uint32_t *max_low_wm, 
                                 uint32_t *max_high_wm);

sdm_error_t sdm_set_gc_threshold(sdm_handle_t *handle, uint32_t low_wm, 
                                 uint32_t high_wm);

sdm_error_t sdm_get_gc_mode(sdm_handle_t *handle, uint32_t *gc_mode);

sdm_error_t sdm_set_gc_mode(sdm_handle_t *handle, uint32_t gc_mode);

sdm_error_t sdm_run_gc(sdm_handle_t *handle, uint32_t gc_time_ms);

sdm_error_t sdm_is_gc_running(sdm_handle_t *handle, uint32_t *is_gc_running);



/**********************      Over Provisioning     ***************************/
sdm_error_t sdm_get_overprovision(sdm_handle_t *handle, uint32_t *curr_op);

sdm_error_t sdm_set_overprovision(sdm_handle_t *handle, uint32_t new_op, 
                                  uint32_t force_op, 
                                  uint64_t *user_visible_cap);
















/**
  * Smart SSD V1.x or higher API prototypes start here
  *
  */

int sdm_get_supported_eb_size_count(sdm_handle_t *handle, uint32_t *count);
int sdm_get_supported_eb_sizes(sdm_handle_t *handle, uint32_t eb_sizes[], uint32_t count);
int sdm_set_eb_size (sdm_handle_t *handle, uint32_t eb_size);
int sdm_allocate_new_block(sdm_handle_t *handle);
int sdm_register_gc_alert_handler(sdm_handle_t *handle, void *cb, uint32_t free_block_percentage, uint32_t poll_interval_ms);

#define TRIM_DEFAULT 0x00
#define TRIM_OPPORTUNISTIC 0x01
#define TRIM_SECURE 0x02
int sdm_set_trim_mode(sdm_handle_t *handle, uint32_t trim_mode);
int sdm_get_trim_mode(sdm_handle_t *handle, uint32_t *trim_mode);
int sdm_trim(sdm_handle_t *handle, sdm_block_range_t trim_ranges[], uint32_t range_count);

#define IO_MODE_LEGACY 0 /* Legacy write I/O mode */
#define IO_MODE_ATOMIC_WRITE 1 /* Atomic write I/O */
int sdm_set_write_io_mode(sdm_handle_t *handle, uint32_t write_io_mode);
int sdm_get_write_io_mode(sdm_handle_t *handle, uint32_t *write_io_mode);
int sdm_get_supported_aw_range_count(sdm_handle_t *handle, uint64_t *count);
int sdm_set_aw_range(sdm_handle_t *handle, uint64_t start_lba, uint64_t len_sector, uint32_t reset);
int sdm_get_aw_range_count(sdm_handle_t *handle, uint64_t *count);
int sdm_get_aw_ranges(sdm_handle_t *handle, sdm_block_range_t aw_ranges[], uint64_t count);
int sdm_get_nonzero_ranges(sdm_handle_t *handle, sdm_block_range_t block_range, \
                           sdm_block_range_t *nonzero_data_ranges, \
                           uint32_t max_range_count, uint32_t *range_count);

int sdm_write_if_same(sdm_handle_t *handle, void *buf, uint64_t buf_len_byte, \
                      void *pattern, uint64_t pattern_len_bytes, \
                      uint64_t start_lba, uint64_t pattern_byte_offset);

int sdm_sub_sector_write(sdm_handle_t *handle, void *buf, uint64_t lba, \
                         uint64_t byte_offset, uint64_t len_byte);

int sdm_get_matching_lbas (sdm_handle_t *handle, uint64_t matching_lbas[], \
                           sdm_block_range_t block, uint8_t *patterns[], \
                           uint64_t pattern_count, uint64_t pattern_size_byte, \
                           uint64_t *matching_lba_count);

int sdm_get_supported_hot_data_range_count(sdm_handle_t *handle, uint64_t *count);

int sdm_set_hot_data_range(sdm_handle_t *handle, uint64_t start_lba, \
                           uint64_t len_sector, uint32_t reset);

int sdm_get_hot_data_range_count(sdm_handle_t *handle, uint64_t *count);

int sdm_get_hot_data_ranges(sdm_handle_t *handle, \
                            sdm_block_range_t *hot_data_ranges, 
                            uint32_t max_range_count, uint32_t *range_count);

int sdm_read_vector(sdm_handle_t *handle, sdm_iovec_t iov[], uint32_t iov_count);




#ifdef __cplusplus
    }
#endif

#endif // __SDM_MGNT_PUBLIC_H__
