/* ========================================================================
Copyright (c) 2001-2009 by ZTE Corporation.  All Rights Reserved.        

-------------------------------------------------------------------------------------------------
   Modify History
-------------------------------------------------------------------------------------------------
When           Who        What 
20110211	JIANGFENG			�ػ���綯���Ƶ�appsboot, PM_JIANGFENG_20110211_01
20100611	JIANGFENG			Add ZTE_PLATFORM Micron, BOOT_JIANGFENG_20100611_01
20100413	JIANGFENG			Add ZTE_PLATFORM Micron, BOOT_JIANGFENG_20100413_01
=========================================================================== */
#ifndef ZTE_MEMLOG_H
#define ZTE_MEMLOG_H
#include <mach/msm_iomap.h>

#define SMEM_LOG_INFO_BASE    MSM_SMEM_RAM_PHYS
#define SMEM_LOG_GLOBAL_BASE  (MSM_SMEM_RAM_PHYS + PAGE_SIZE)

#define SMEM_LOG_ENTRY_OFFSET (64*PAGE_SIZE)
#define SMEM_LOG_ENTRY_BASE   (MSM_SMEM_RAM_PHYS + SMEM_LOG_ENTRY_OFFSET)

#define ERR_DATA_MAX_SIZE 0x4000

#define MAX_BBTB_PAGES 8
#define SIZEOF_BBTB_INFO (4 * sizeof(unsigned int))
#define BBTB_BUFFER_SIZE (512*MAX_BBTB_PAGES)
#define MAX_BAD_BLOCKS ((BBTB_BUFFER_SIZE - SIZEOF_BBTB_INFO)/4)
struct bad_block_table {
  unsigned int magic1;
  unsigned int magic2;
  unsigned int version;
  unsigned int num_entries;
  unsigned int bad_block[MAX_BAD_BLOCKS];
};

//PM_JIANGFENG_20110211_01,start
typedef struct
{
	unsigned char display_flag;
	unsigned char img_id;
	unsigned char chg_fulled;
	unsigned char battery_capacity;
	unsigned char battery_valid;
} power_off_supply_status;
//PM_JIANGFENG_20110211_01,end

typedef struct {
  unsigned int ftm;
  unsigned int boot_reason;
  unsigned int reset_reason;
  unsigned int chg_count;
  unsigned int f3log;
  unsigned int err_fatal;
  unsigned int err_dload;
  char err_log[ERR_DATA_MAX_SIZE];
  struct bad_block_table bad_block_table;
  unsigned char flash_id[2];//BOOT_JIANGFENG_20100413_01
  unsigned char sdrem_length;//0:128M; 1:256M; 2:384M; 3:512M; ...		//BOOT_JIANGFENG_20100612_01
//PM_JIANGFENG_20110211_01,start
  unsigned char power_off_charge;	//0:��������;   1:�����Ĺػ����; 2:�ػ���粢��ʾ����
  power_off_supply_status power_off_charge_info;
//PM_JIANGFENG_20110211_01,end
} smem_global;

#endif
