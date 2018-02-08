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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <libaio.h>
#include "sdm_mgnt_public.h"


#define MAX_CMDS 30
#define MAX_NAME_LEN 100

int print_help(sdm_handle_t *handle, char *data1, char *data2);
int print_api_version(sdm_handle_t *handle, char *data1, char *data2);
int print_supported_features(sdm_handle_t *handle, char *data1, char *data2);
int print_op_ratio(sdm_handle_t *handle, char *data1, char *data2);
int set_op_ratio(sdm_handle_t *handle, char *data1, char *data2);
int print_user_capacity(sdm_handle_t *handle, char *data1, char *data2);
int print_device_capacity(sdm_handle_t *handle, char *data1, char *data2);
int print_erase_block_size(sdm_handle_t *handle, char *data1, char *data2);
int print_free_block_count(sdm_handle_t *handle, char *data1, char *data2);
int print_read_readiness(sdm_handle_t *handle, char *data1, char *data2);
int print_gc_threshold(sdm_handle_t *handle, char *data1, char *data2);
int set_gc_threshold(sdm_handle_t *handle, char *data1, char *data2);
int execute_gc(sdm_handle_t *handle, char *data1, char *data2);
int is_host_triggered_gc_running(sdm_handle_t *handle, char *data1, char *data2);
int print_gc_mode(sdm_handle_t *handle, char *data1, char *data2);
int set_gc_mode(sdm_handle_t *handle, char *data1, char *data2);


sdm_handle_t *g_handle;

typedef struct {
    char letter;
    int (*func) (sdm_handle_t *handle, char *data1, char *data2);
    char buf[MAX_NAME_LEN];
} sdm_command_t;

sdm_command_t sdm_command_table[MAX_CMDS] = \
{  \
    {'h', print_help, "Print This message"},  \
    {'v', print_api_version, "Print API Library version info"},  \
    {'s', print_supported_features, "Print Supported Features"},  \
    {'c', print_user_capacity, "Print User addressable sector count"},  \
    {'d', print_device_capacity, "Print device total sector count"},  \
    {'e', print_erase_block_size, "Print Erase Block Unit Size in sector"},  \
    {'f', print_free_block_count, "Print available free erase block in sector"},  \
    {'r', print_read_readiness, "whether SSD is ready for read?"},  \
    {'t', print_gc_threshold, "Print GC threshold value in sector"},  \
    {'T', set_gc_threshold, "set GC threshold value in sector"},  \
    {'G', execute_gc, "execute host triggered GC"},  \
    {'g', is_host_triggered_gc_running, "is host triggered GC running?"},  \
    {'o', print_op_ratio, "Print Over-Provisioning Ratio in %"},  \
    {'O', set_op_ratio, "Set Over-Provisioning Ratio in %"},  \
    {'m', print_gc_mode, "print GC mode"},  \
    {'M', set_gc_mode, "set GC mode: CONTINUE_GC(0) or STOP_GC(1)"},  \
    {'\0', NULL, "End of Command"} \
};

int print_help(sdm_handle_t *handle, char *data1, char *data2)
{
    int i;

    printf("*** option     ****** comment ******\n");
    
    for (i=0; sdm_command_table[i].letter != '\0'; i++)
    {
        printf("      %c       %s\n", sdm_command_table[i].letter, sdm_command_table[i].buf);      
    }
    printf("*******************************************\n");
    printf("*******************************************\n");

    return 0;
}

int print_api_version(sdm_handle_t *handle, char *data1, char *data2)
{
    sdm_api_version_t vsn;
    sdm_get_api_version(&vsn);
    printf("major version : %d\n", vsn.major);
    printf("minor version : %d\n", vsn.minor);
    return 0;
}

int print_supported_features(sdm_handle_t *handle, char *data1, char *data2)
{
    sdm_supp_features_t feature;

    sdm_get_supported_features(handle, &feature);
    printf("opti_rd_mgmt:%d\n", feature.opti_rd_mgmt);
    printf("gc_mgmt     :%d\n", feature.gc_mgmt);
    printf("op_mgmt     :%d\n", feature.op_mgmt);

    return 0;
}

int print_user_capacity(sdm_handle_t *handle, char *data1, char *data2)
{
    uint64_t user_capacity;
    sdm_get_user_capacity(handle, &user_capacity);

    printf("Total number of user addressable Sectors = %llu\n",user_capacity);
    return 0;
}

int print_device_capacity(sdm_handle_t *handle, char *data1, char *data2)
{
    uint64_t device_capacity;
    sdm_get_device_capacity(handle, &device_capacity);

    printf("Total number of Sectors in device = %llu\n",device_capacity);
    return 0;
}

int print_erase_block_size(sdm_handle_t *handle, char *data1, char *data2)
{
    uint32_t erase_blk_size;
    sdm_get_current_eb_size(handle, &erase_blk_size);

    printf("Erase Block Size in sector= %d\n", erase_blk_size);
    return 0;
}

int print_free_block_count(sdm_handle_t *handle, char *data1, char *data2)
{
    uint64_t free_blk_count;
    sdm_get_free_block_count(handle, &free_blk_count);

    printf("Currently available Free erase blocks in sector= %llu\n", free_blk_count);
    return 0;
}

int print_read_readiness(sdm_handle_t *handle, char *data1, char *data2)
{
    uint32_t is_ready_for_read;
    sdm_is_ready_for_read(handle, &is_ready_for_read);

    printf("read readiness = %d\n", is_ready_for_read);
    return 0;
}

int print_gc_threshold(sdm_handle_t *handle, char *data1, char *data2)
{
    uint32_t curr_low_wm, curr_high_wm;
    uint32_t max_low_wm, max_high_wm;
    sdm_get_gc_threshold(handle, &curr_low_wm, &curr_high_wm, &max_low_wm, &max_high_wm);
    printf("curr_low_wm  = %d sectors\n", curr_low_wm);
    printf("curr_high_wm = %d sectors\n", curr_high_wm);
    printf("max_low_wm  = %d sectors\n", max_low_wm);
    printf("max_high_wm = %d sectors\n", max_high_wm);
    return 0;
}

int set_gc_threshold(sdm_handle_t *handle, char *data1, char *data2)
{
    uint32_t low_wm, high_wm;

    if (data1 == NULL || data2 == NULL)
    {
        printf("please provide 2 more arguments for low_wm and high_wm\n");
        return 0;
    }    
    low_wm = atoi(data1);
    high_wm = atoi(data2);
    sdm_set_gc_threshold(handle, low_wm, high_wm);
    return 0;
}

int execute_gc(sdm_handle_t *handle, char *data1, char *data2)
{
    uint32_t gc_time_ms;

    if (data1 == NULL)
    {
        printf("please provide 1 more arguments for gc_time_ms\n");
        return 0;
    }    
    gc_time_ms = atoi(data1);
    sdm_run_gc(handle, gc_time_ms);
    return 0;
}

int is_host_triggered_gc_running(sdm_handle_t *handle, char *data1, char *data2)
{
    uint32_t is_gc_running;
    sdm_is_gc_running(handle, &is_gc_running);

    printf("host triggered GC state : %d\n", is_gc_running);
    return 0;
}

int print_op_ratio(sdm_handle_t *handle, char *data1, char *data2)
{
    uint32_t curr_op;
    sdm_get_overprovision(handle, &curr_op);
    
    printf("OP ratio = %d percentage\n", curr_op);
    return 0;
}

int set_op_ratio(sdm_handle_t *handle, char *data1, char *data2)
{
    uint32_t op_ratio;
    uint64_t cap_after;

    if (data1 == NULL)
    {
        printf("please provide 1 more arguments for op_ratio in percentage\n");
        printf("Ex.) \"utest /dev/sdb 10\" ");
        printf("    ---> set 10 percentage OP ratio in simulation mode\n");
        printf("Ex.) \"utest /dev/sdb 10 f\" ");
        printf("  ---> set 10 percentage OP ratio in actual device\n");
        return 0;
    }

    op_ratio = (uint32_t) atoi(data1);

    if ( (data2 != NULL) && (data2[0] == 'f') )
    {   /* configure actual device */
        printf("set op ratio:%d percentage in real device\n", op_ratio);
        sdm_set_overprovision(handle, op_ratio, 1, &cap_after);
    }
    else
    {   /* don't configure actual device */
        printf("simulate op ratio:%d percentage without setting it\n", op_ratio);
        sdm_set_overprovision(handle, op_ratio, 0, &cap_after);
    }
    printf("new user capacity is %llu sectors in total\n", cap_after);
    return 0;
}

int print_gc_mode(sdm_handle_t *handle, char *data1, char *data2)
{
    uint32_t gc_mode;
    sdm_get_gc_mode(handle, &gc_mode);
    printf("GC mode is %d\n", gc_mode); 
    return 0;
}

int set_gc_mode(sdm_handle_t *handle, char *data1, char *data2)
{
    uint32_t gc_mode;

    if (data1 == NULL)
    {
        printf("please provide 1 more arguments for gc_mode\n");
        printf("      : 0 for CONTINUE_GC\n");
        printf("      : 1 for STOP_GC\n");
        return 0;
    }

    gc_mode = (uint32_t) atoi(data1);
    
    if (gc_mode == 0)
    {
        gc_mode = CONTINUE_GC;
    }
    else
    {
        gc_mode = STOP_GC;
    }

    sdm_set_gc_mode(handle, gc_mode);
    return 0;
}

int process_command(sdm_handle_t *handle, const char *command, char *data1, char *data2)
{
    int i;
    char cmd_letter = command[0];
    
    for (i=0; sdm_command_table[i].letter != '\0'; i++)
    {
        if ( (sdm_command_table[i].letter == cmd_letter) &&
             (sdm_command_table[i].func != NULL) )
        {
            sdm_command_table[i].func(handle, data1, data2);
            break;
        } 
    }

    if (sdm_command_table[i].letter == '\0')
    {
        printf("command '%c' is not supported\n", *command);
        printf("please use option 'h' to see list of supported commands\n");
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int ret;
    sdm_api_version_t version;

    if ( (argc < 3) || (argv[2][0] == 'h') )
    {
        printf("usage:   utest <device node name> <option>\n\n");
        printf("example usage:   utest /dev/sdb h\n\n");
        print_help(NULL, NULL, NULL);
        
        return 0;
    }

    version.major = API_VERSION_MAJOR_NUMBER;
    version.minor = API_VERSION_MINOR_NUMBER;

    printf("trying to open %s\n", argv[1]);
    if (ret = sdm_open((const char *)argv[1], &g_handle, &version))
    {
        printf("cannot open device %s: %d returned\n", argv[1], ret);
        return 0;
    }
    printf("device %s is opened with fd=%d\n", argv[1], g_handle->id);

    if (argc == 3)
    {
        /* command with no parameter */
        process_command(g_handle, argv[2], NULL, NULL);
    }
    else if (argc == 4)
    {
        /* command with one parameter */
        process_command(g_handle, argv[2], argv[3], NULL);
    } 
    else if (argc == 5)
    {
        /* command with two parameters */
        process_command(g_handle, argv[2], argv[3], argv[4]);
    } 
    else if (argc > 5)
    {
        printf("two many parameters for command %s\n", argv[2]);
    }

    printf("closing the device\n");
    if (ret = sdm_close(g_handle))
    {
        printf("cannot close handle %p: %d returned\n", g_handle, ret);
    }
    
    return 0;
}
