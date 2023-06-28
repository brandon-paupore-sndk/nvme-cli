/*
 * Copyright (c) 2015-2018 Western Digital Corporation or its affiliates.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 *
 *   Author: Chaitanya Kulkarni <chaitanya.kulkarni@hgst.com>,
 *           Dong Ho <dong.ho@hgst.com>,
 *           Jeff Lien <jeff.lien@wdc.com>
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>

#include "linux/nvme_ioctl.h"

#include "nvme.h"
#include "nvme-print.h"
#include "nvme-ioctl.h"
#include "plugin.h"
#include "json.h"

#include "argconfig.h"
#include "suffix.h"
#include <sys/ioctl.h>
#define CREATE_CMD
#include "wdc-nvme.h"
#include "wdc-utils.h"

// TODO: from util/types.c
#include <locale.h>


#define WRITE_SIZE	(sizeof(__u8) * 4096)

#define WDC_NVME_SUBCMD_SHIFT	8

#define WDC_NVME_LOG_SIZE_DATA_LEN			0x08
#define WDC_NVME_LOG_SIZE_HDR_LEN			0x08

/* Device Config */
#define WDC_NVME_VID					0x1c58
#define WDC_NVME_VID_2					0x1b96
#define WDC_NVME_SNDK_VID				0x15b7

#define WDC_NVME_SN100_DEV_ID				0x0003
#define WDC_NVME_SN200_DEV_ID				0x0023
#define WDC_NVME_SN630_DEV_ID				0x2200
#define WDC_NVME_SN630_DEV_ID_1				0x2201
#define WDC_NVME_SN840_DEV_ID				0x2300
#define WDC_NVME_SN840_DEV_ID_1				0x2500
#define WDC_NVME_SN640_DEV_ID				0x2400
#define WDC_NVME_SN640_DEV_ID_1				0x2401
#define WDC_NVME_SN640_DEV_ID_2				0x2402
#define WDC_NVME_SN640_DEV_ID_3				0x2404
#define WDC_NVME_ZN540_DEV_ID				0x2600
#define WDC_NVME_SN540_DEV_ID				0x2610
#define WDC_NVME_SN650_DEV_ID				0x2700
#define WDC_NVME_SN650_DEV_ID_1				0x2701
#define WDC_NVME_SN650_DEV_ID_2				0x2702
#define WDC_NVME_SN650_DEV_ID_3				0x2720
#define WDC_NVME_SN650_DEV_ID_4				0x2721
#define WDC_NVME_SN655_DEV_ID				0x2722
#define WDC_NVME_SN860_DEV_ID				0x2730
#define WDC_NVME_SN660_DEV_ID				0x2704

/* This id's are no longer supported, delete ?? */
#define WDC_NVME_SN550_DEV_ID				0x2708
#define WDC_NVME_SN560_DEV_ID_1				0x2712
#define WDC_NVME_SN560_DEV_ID_2				0x2713
#define WDC_NVME_SN560_DEV_ID_3				0x2714

#define WDC_NVME_SNDK_VID		        0x15b7
#define WDC_NVME_SXSLCL_DEV_ID  		0x2001
#define WDC_NVME_SN520_DEV_ID_1  		0x5003
#define WDC_NVME_SN520_DEV_ID_2  		0x5005
#define WDC_NVME_SN720_DEV_ID   		0x5002
#define WDC_NVME_SN730_DEV_ID   		0x5006
#define WDC_NVME_ZN350_DEV_ID				0x5010
#define WDC_NVME_ZN350_DEV_ID_1				0x5018
#define WDC_NVME_SN820CL_DEV_ID				0x5037

#define WDC_DRIVE_CAP_CAP_DIAG				0x0000000000000001
#define WDC_DRIVE_CAP_INTERNAL_LOG			0x0000000000000002
#define WDC_DRIVE_CAP_C1_LOG_PAGE			0x0000000000000004
#define WDC_DRIVE_CAP_CA_LOG_PAGE			0x0000000000000008
#define WDC_DRIVE_CAP_D0_LOG_PAGE			0x0000000000000010
#define WDC_DRIVE_CAP_DRIVE_STATUS			0x0000000000000020
#define WDC_DRIVE_CAP_CLEAR_ASSERT			0x0000000000000040
#define WDC_DRIVE_CAP_CLEAR_PCIE			0x0000000000000080
#define WDC_DRIVE_CAP_RESIZE				0x0000000000000100
#define WDC_DRIVE_CAP_NAND_STATS			0x0000000000000200
#define WDC_DRIVE_CAP_DRIVE_LOG				0x0000000000000400
#define WDC_DRIVE_CAP_CRASH_DUMP			0x0000000000000800
#define WDC_DRIVE_CAP_PFAIL_DUMP			0x0000000000001000
#define WDC_DRIVE_CAP_FW_ACTIVATE_HISTORY		0x0000000000002000
#define WDC_DRIVE_CAP_CLEAR_FW_ACT_HISTORY		0x0000000000004000
#define WDC_DRVIE_CAP_DISABLE_CTLR_TELE_LOG		0x0000000000008000
#define WDC_DRIVE_CAP_REASON_ID				0x0000000000010000
#define WDC_DRIVE_CAP_LOG_PAGE_DIR			0x0000000000020000
#define WDC_DRIVE_CAP_NS_RESIZE				0x0000000000040000
#define WDC_DRIVE_CAP_INFO				0x0000000000080000
#define WDC_DRIVE_CAP_C0_LOG_PAGE			0x0000000000100000
#define WDC_DRIVE_CAP_TEMP_STATS			0x0000000000200000
#define WDC_DRIVE_CAP_VUC_CLEAR_PCIE			0x0000000000400000
#define WDC_DRIVE_CAP_VU_FID_CLEAR_PCIE			0x0000000000800000
#define WDC_DRIVE_CAP_FW_ACTIVATE_HISTORY_C2		0x0000000001000000
#define WDC_DRIVE_CAP_VU_FID_CLEAR_FW_ACT_HISTORY	0x0000000002000000
#define WDC_DRIVE_CAP_CLOUD_SSD_VERSION			0x0000000004000000
#define WDC_DRIVE_CAP_PCIE_STATS			0x0000000008000000
#define WDC_DRIVE_CAP_HW_REV_LOG_PAGE			0x0000000010000000
#define WDC_DRIVE_CAP_C3_LOG_PAGE			0x0000000020000000
#define WDC_DRIVE_CAP_CLOUD_BOOT_SSD_VERSION		0x0000000040000000
#define WDC_DRIVE_CAP_CLOUD_LOG_PAGE			0x0000000080000000

#define WDC_DRIVE_CAP_DRIVE_ESSENTIALS			0x0000000100000000
#define WDC_DRIVE_CAP_DUI_DATA				0x0000000200000000
#define WDC_SN730B_CAP_VUC_LOG				0x0000000400000000
#define WDC_DRIVE_CAP_DUI				0x0000000800000000
#define WDC_DRIVE_CAP_PURGE				0x0000001000000000
#define WDC_DRIVE_CAP_OCP_C1_LOG_PAGE			0x0000002000000000
#define WDC_DRIVE_CAP_OCP_C4_LOG_PAGE			0x0000004000000000
#define WDC_DRIVE_CAP_OCP_C5_LOG_PAGE			0x0000008000000000
#define WDC_DRIVE_CAP_DEVICE_WAF			0x0000010000000000
#define WDC_DRIVE_CAP_SMART_LOG_MASK			(WDC_DRIVE_CAP_C0_LOG_PAGE | \
							 WDC_DRIVE_CAP_C1_LOG_PAGE | \
							 WDC_DRIVE_CAP_CA_LOG_PAGE | \
							 WDC_DRIVE_CAP_D0_LOG_PAGE)
#define WDC_DRIVE_CAP_CLEAR_PCIE_MASK			(WDC_DRIVE_CAP_CLEAR_PCIE | \
							 WDC_DRIVE_CAP_VUC_CLEAR_PCIE | \
							 WDC_DRIVE_CAP_VU_FID_CLEAR_PCIE)
#define WDC_DRIVE_CAP_FW_ACTIVATE_HISTORY_MASK		(WDC_DRIVE_CAP_FW_ACTIVATE_HISTORY | \
							 WDC_DRIVE_CAP_FW_ACTIVATE_HISTORY_C2)
#define WDC_DRIVE_CAP_CLEAR_FW_ACT_HISTORY_MASK		(WDC_DRIVE_CAP_CLEAR_FW_ACT_HISTORY | \
							 WDC_DRIVE_CAP_VU_FID_CLEAR_FW_ACT_HISTORY)
#define WDC_DRIVE_CAP_INTERNAL_LOG_MASK			(WDC_DRIVE_CAP_INTERNAL_LOG | \
							 WDC_DRIVE_CAP_DUI | \
							 WDC_DRIVE_CAP_DUI_DATA | \
							 WDC_SN730B_CAP_VUC_LOG)

/* Capture Diagnostics */
#define WDC_NVME_CAP_DIAG_HEADER_TOC_SIZE	WDC_NVME_LOG_SIZE_DATA_LEN
#define WDC_NVME_CAP_DIAG_OPCODE			0xE6
#define WDC_NVME_CAP_DIAG_CMD_OPCODE		0xC6
#define WDC_NVME_CAP_DIAG_SUBCMD			0x00
#define WDC_NVME_CAP_DIAG_CMD				0x00

#define WDC_NVME_CRASH_DUMP_TYPE		1
#define WDC_NVME_PFAIL_DUMP_TYPE		2

/* Capture Device Unit Info */
#define WDC_NVME_CAP_DUI_HEADER_SIZE			0x400
#define WDC_NVME_CAP_DUI_OPCODE				0xFA
#define WDC_NVME_CAP_DUI_DISABLE_IO			0x01
#define WDC_NVME_DUI_MAX_SECTION			0x3A
#define WDC_NVME_DUI_MAX_SECTION_V2			0x26
#define WDC_NVME_DUI_MAX_SECTION_V3			0x23
#define WDC_NVME_DUI_MAX_DATA_AREA			0x05
#define WDC_NVME_SN730_SECTOR_SIZE			512

/* Telemtery types for vs-internal-log command */
#define WDC_TELEMETRY_TYPE_NONE				0x0
#define WDC_TELEMETRY_TYPE_HOST				0x1
#define WDC_TELEMETRY_TYPE_CONTROLLER			0x2
#define WDC_TELEMETRY_HEADER_LENGTH			512
#define WDC_TELEMETRY_BLOCK_SIZE			512

/* Crash dump */
#define WDC_NVME_CRASH_DUMP_SIZE_OPCODE		WDC_NVME_CAP_DIAG_CMD_OPCODE
#define WDC_NVME_CRASH_DUMP_SIZE_DATA_LEN	WDC_NVME_LOG_SIZE_DATA_LEN
#define WDC_NVME_CRASH_DUMP_SIZE_NDT		0x02
#define WDC_NVME_CRASH_DUMP_SIZE_CMD		0x20
#define WDC_NVME_CRASH_DUMP_SIZE_SUBCMD		0x03

#define WDC_NVME_CRASH_DUMP_OPCODE			WDC_NVME_CAP_DIAG_CMD_OPCODE
#define WDC_NVME_CRASH_DUMP_CMD				0x20
#define WDC_NVME_CRASH_DUMP_SUBCMD			0x04

/* PFail Crash dump */
#define WDC_NVME_PF_CRASH_DUMP_SIZE_DATA_LEN	WDC_NVME_LOG_SIZE_HDR_LEN
#define WDC_NVME_PF_CRASH_DUMP_SIZE_NDT		0x02
#define WDC_NVME_PF_CRASH_DUMP_SIZE_CMD		0x20
#define WDC_NVME_PF_CRASH_DUMP_SIZE_SUBCMD	0x05

#define WDC_NVME_PF_CRASH_DUMP_OPCODE	    WDC_NVME_CAP_DIAG_CMD_OPCODE
#define WDC_NVME_PF_CRASH_DUMP_CMD	    0x20
#define WDC_NVME_PF_CRASH_DUMP_SUBCMD	    0x06

/* Drive Log */
#define WDC_NVME_DRIVE_LOG_SIZE_OPCODE		WDC_NVME_CAP_DIAG_CMD_OPCODE
#define WDC_NVME_DRIVE_LOG_SIZE_DATA_LEN	WDC_NVME_LOG_SIZE_DATA_LEN
#define WDC_NVME_DRIVE_LOG_SIZE_NDT			0x02
#define WDC_NVME_DRIVE_LOG_SIZE_CMD			0x20
#define WDC_NVME_DRIVE_LOG_SIZE_SUBCMD		0x01

#define WDC_NVME_DRIVE_LOG_OPCODE			WDC_NVME_CAP_DIAG_CMD_OPCODE
#define WDC_NVME_DRIVE_LOG_CMD				WDC_NVME_LOG_SIZE_DATA_LEN
#define WDC_NVME_DRIVE_LOG_SUBCMD			0x00

/* Purge and Purge Monitor */
#define WDC_NVME_PURGE_CMD_OPCODE			0xDD
#define WDC_NVME_PURGE_MONITOR_OPCODE		0xDE
#define WDC_NVME_PURGE_MONITOR_DATA_LEN		0x2F
#define WDC_NVME_PURGE_MONITOR_CMD_CDW10	0x0000000C
#define WDC_NVME_PURGE_MONITOR_TIMEOUT		0x7530
#define WDC_NVME_PURGE_CMD_SEQ_ERR			0x0C
#define WDC_NVME_PURGE_INT_DEV_ERR			0x06

#define WDC_NVME_PURGE_STATE_IDLE			0x00
#define WDC_NVME_PURGE_STATE_DONE			0x01
#define WDC_NVME_PURGE_STATE_BUSY			0x02
#define WDC_NVME_PURGE_STATE_REQ_PWR_CYC	0x03
#define WDC_NVME_PURGE_STATE_PWR_CYC_PURGE	0x04

/* Clear dumps */
#define WDC_NVME_CLEAR_DUMP_OPCODE		0xFF
#define WDC_NVME_CLEAR_CRASH_DUMP_CMD		0x03
#define WDC_NVME_CLEAR_CRASH_DUMP_SUBCMD	0x05
#define WDC_NVME_CLEAR_PF_CRASH_DUMP_SUBCMD	0x06

/* Additional Smart Log */
#define WDC_ADD_LOG_BUF_LEN							0x4000
#define WDC_NVME_ADD_LOG_OPCODE						0xC1
#define WDC_GET_LOG_PAGE_SSD_PERFORMANCE			0x37
#define WDC_NVME_GET_STAT_PERF_INTERVAL_LIFETIME	0x0F

/* C0 EOL Status Log Page */
#define WDC_NVME_GET_EOL_STATUS_LOG_OPCODE		0xC0
#define WDC_NVME_EOL_STATUS_LOG_LEN			0x200
#define WDC_NVME_SMART_CLOUD_ATTR_LEN			0x200

/* C0 SMART Cloud Attributes Log Page*/
#define WDC_NVME_GET_SMART_CLOUD_ATTR_LOG_ID		0xC0

/* CB - FW Activate History Log Page */
#define WDC_NVME_GET_FW_ACT_HISTORY_LOG_ID		0xCB

/* C2 Log Page */
#define WDC_NVME_GET_DEV_MGMNT_LOG_PAGE_OPCODE		0xC2
#define WDC_NVME_GET_DEV_MGMNT_LOG_PAGE_OPCODE_C8	0xC8
#define WDC_C2_LOG_BUF_LEN				0x1000
#define WDC_C2_LOG_PAGES_SUPPORTED_ID			0x08
#define WDC_C2_CUSTOMER_ID_ID				0x15
#define WDC_C2_THERMAL_THROTTLE_STATUS_ID		0x18
#define WDC_C2_ASSERT_DUMP_PRESENT_ID			0x19
#define WDC_C2_USER_EOL_STATUS_ID			0x1A
#define WDC_C2_USER_EOL_STATE_ID			0x1C
#define WDC_C2_SYSTEM_EOL_STATE_ID			0x1D
#define WDC_C2_FORMAT_CORRUPT_REASON_ID			0x1E
#define WDC_EOL_STATUS_NORMAL				cpu_to_le32(0x00000000)
#define WDC_EOL_STATUS_END_OF_LIFE			cpu_to_le32(0x00000001)
#define WDC_EOL_STATUS_READ_ONLY			cpu_to_le32(0x00000002)
#define WDC_ASSERT_DUMP_NOT_PRESENT			cpu_to_le32(0x00000000)
#define WDC_ASSERT_DUMP_PRESENT				cpu_to_le32(0x00000001)
#define WDC_THERMAL_THROTTLING_OFF			cpu_to_le32(0x00000000)
#define WDC_THERMAL_THROTTLING_ON			cpu_to_le32(0x00000001)
#define WDC_THERMAL_THROTTLING_UNAVAILABLE		cpu_to_le32(0x00000002)
#define WDC_FORMAT_NOT_CORRUPT				cpu_to_le32(0x00000000)
#define WDC_FORMAT_CORRUPT_FW_ASSERT			cpu_to_le32(0x00000001)
#define WDC_FORMAT_CORRUPT_UNKNOWN			cpu_to_le32(0x000000FF)


/* D0 Smart Log Page */
#define WDC_NVME_GET_VU_SMART_LOG_OPCODE		0xD0
#define WDC_NVME_VU_SMART_LOG_LEN			0x200

/* CA Log Page */
#define WDC_NVME_GET_DEVICE_INFO_LOG_OPCODE			0xCA
#define WDC_FB_CA_LOG_BUF_LEN				0x80
/* Added 4 padding bytes to resolve build warning messages */
#define WDC_BD_CA_LOG_BUF_LEN				0xA0

#define WDC_CA_LOG_BUF_LEN							0x80
#define WDC_VS_CA_LOG_BUF_LEN 						0x200

/* Customer ID's */
#define WDC_CUSTOMER_ID_GN				0x0001
#define WDC_CUSTOMER_ID_GD				0x0101
#define WDC_CUSTOMER_ID_BD				0x1009

#define WDC_CUSTOMER_ID_0x1005				0x1005

#define WDC_CUSTOMER_ID_0x1004				0x1004
#define WDC_CUSTOMER_ID_0x1008				0x1008
#define WDC_CUSTOMER_ID_0x1304				0x1304
#define WDC_INVALID_CUSTOMER_ID				-1

#define WDC_ALL_PAGE_MASK				0xFFFF
#define WDC_C0_PAGE_MASK				0x0001
#define WDC_C1_PAGE_MASK				0x0002
#define WDC_CA_PAGE_MASK				0x0004
#define WDC_D0_PAGE_MASK				0x0008

/* Clear PCIe Correctable Errors */
#define WDC_NVME_CLEAR_PCIE_CORR_OPCODE  	WDC_NVME_CAP_DIAG_CMD_OPCODE
#define WDC_NVME_CLEAR_PCIE_CORR_CMD		0x22
#define WDC_NVME_CLEAR_PCIE_CORR_SUBCMD		0x04

/* Drive Essentials */
#define WDC_DE_DEFAULT_NUMBER_OF_ERROR_ENTRIES		64
#define WDC_DE_GENERIC_BUFFER_SIZE					80
#define WDC_DE_GLOBAL_NSID							0xFFFFFFFF
#define WDC_DE_DEFAULT_NAMESPACE_ID					0x01
#define WDC_DE_PATH_SEPARATOR						"/"
#define WDC_DE_TAR_FILES							"*.bin"
#define WDC_DE_TAR_FILE_EXTN						".tar.gz"
#define WDC_DE_TAR_CMD								"tar -czf"

/* VU Opcodes */
#define WDC_DE_VU_READ_SIZE_OPCODE					0xC0
#define WDC_DE_VU_READ_BUFFER_OPCODE				0xC2

#define WDC_DE_FILE_HEADER_SIZE                     4
#define WDC_DE_FILE_OFFSET_SIZE                     2
#define WDC_DE_FILE_NAME_SIZE                       32
#define WDC_DE_VU_READ_BUFFER_STANDARD_OFFSET		0x8000
#define WDC_DE_READ_MAX_TRANSFER_SIZE				0x8000

#define WDC_DE_MANUFACTURING_INFO_PAGE_FILE_NAME	"manufacturing_info"  /* Unique log entry page name. */
#define WDC_DE_CORE_DUMP_FILE_NAME					"core_dump"
#define WDC_DE_EVENT_LOG_FILE_NAME					"event_log"
#define WDC_DE_DESTN_SPI							1
#define WDC_DE_DUMPTRACE_DESTINATION				6

/* Miscellanious log pages, for drive capability checking */
#define WDC_ERROR_REC_LOG_ID              0xC1
#define WDC_LATENCY_MON_LOG_ID				0xC3
#define WDC_DEV_CAP_LOG_ID                0xC4
#define WDC_UNSUPPORTED_REQS_LOG_ID       0xC5


#define min(x, y) ((x) > (y) ? (y) : (x))

typedef enum _NVME_FEATURES_SELECT
{
    FS_CURRENT                      = 0,
    FS_DEFAULT                      = 1,
    FS_SAVED                        = 2,
    FS_SUPPORTED_CAPBILITIES        = 3
} NVME_FEATURES_SELECT;

typedef enum _NVME_FEATURE_IDENTIFIERS
{
    FID_ARBITRATION                                 = 0x01,
    FID_POWER_MANAGEMENT                            = 0x02,
    FID_LBA_RANGE_TYPE                              = 0x03,
    FID_TEMPERATURE_THRESHOLD                       = 0x04,
    FID_ERROR_RECOVERY                              = 0x05,
    FID_VOLATILE_WRITE_CACHE                        = 0x06,
    FID_NUMBER_OF_QUEUES                            = 0x07,
    FID_INTERRUPT_COALESCING                        = 0x08,
    FID_INTERRUPT_VECTOR_CONFIGURATION              = 0x09,
    FID_WRITE_ATOMICITY                             = 0x0A,
    FID_ASYNCHRONOUS_EVENT_CONFIGURATION            = 0x0B,
    FID_AUTONOMOUS_POWER_STATE_TRANSITION           = 0x0C,
/*Below FID's are NVM Command Set Specific*/
    FID_SOFTWARE_PROGRESS_MARKER                    = 0x80,
    FID_HOST_IDENTIFIER                             = 0x81,
    FID_RESERVATION_NOTIFICATION_MASK               = 0x82,
    FID_RESERVATION_PERSISTENCE                     = 0x83
} NVME_FEATURE_IDENTIFIERS;

typedef enum
{
	WDC_DE_TYPE_IDENTIFY            = 0x1,
	WDC_DE_TYPE_SMARTATTRIBUTEDUMP  = 0x2,
	WDC_DE_TYPE_EVENTLOG            = 0x4,
	WDC_DE_TYPE_DUMPTRACE           = 0x8,
	WDC_DE_TYPE_DUMPSNAPSHOT        = 0x10,
	WDC_DE_TYPE_ATA_LOGS            = 0x20,
	WDC_DE_TYPE_SMART_LOGS          = 0x40,
	WDC_DE_TYPE_SCSI_LOGS           = 0x80,
	WDC_DE_TYPE_SCSI_MODE_PAGES     = 0x100,
	WDC_DE_TYPE_NVMe_FEATURES       = 0x200,
	WDC_DE_TYPE_DUMPSMARTERRORLOG3  = 0x400,
	WDC_DE_TYPE_DUMPLOG3E           = 0x800,
	WDC_DE_TYPE_DUMPSCRAM           = 0x1000,
	WDC_DE_TYPE_PCU_LOG             = 0x2000,
	WDC_DE_TYPE_DUMP_ERROR_LOGS     = 0x4000,
	WDC_DE_TYPE_FW_SLOT_LOGS        = 0x8000,
	WDC_DE_TYPE_MEDIA_SETTINGS      = 0x10000,
	WDC_DE_TYPE_SMART_DATA          = 0x20000,
	WDC_DE_TYPE_NVME_SETTINGS       = 0x40000,
	WDC_DE_TYPE_NVME_ERROR_LOGS     = 0x80000,
	WDC_DE_TYPE_NVME_LOGS           = 0x100000,
	WDC_DE_TYPE_UART_LOGS           = 0x200000,
	WDC_DE_TYPE_DLOGS_SPI           = 0x400000,
	WDC_DE_TYPE_DLOGS_RAM           = 0x800000,
	WDC_DE_TYPE_NVME_MANF_INFO      = 0x2000000,
	WDC_DE_TYPE_NONE                = 0x1000000,
	WDC_DE_TYPE_ALL                 = 0xFFFFFFF,
} WDC_DRIVE_ESSENTIAL_TYPE;

typedef struct __attribute__((__packed__)) _WDC_DE_VU_FILE_META_DATA
{
    __u8 fileName[WDC_DE_FILE_NAME_SIZE];
    __u16 fileID;
    __u64 fileSize;
} WDC_DE_VU_FILE_META_DATA, *PWDC_DE_VU_FILE_META_DATA;

typedef struct _WDC_DRIVE_ESSENTIALS
{
    WDC_DE_VU_FILE_META_DATA metaData;
    WDC_DRIVE_ESSENTIAL_TYPE essentialType;
} WDC_DRIVE_ESSENTIALS;

typedef struct _WDC_DE_VU_LOG_DIRECTORY
{
    WDC_DRIVE_ESSENTIALS *logEntry;		/* Caller to allocate memory        */
    __u32 maxNumLogEntries; 			/* Caller to input memory allocated */
    __u32 numOfValidLogEntries;			/* API will output this value       */
} WDC_DE_VU_LOG_DIRECTORY,*PWDC_DE_VU_LOG_DIRECTORY;

typedef struct _WDC_DE_CSA_FEATURE_ID_LIST
{
    NVME_FEATURE_IDENTIFIERS featureId;
    __u8 featureName[WDC_DE_GENERIC_BUFFER_SIZE];
} WDC_DE_CSA_FEATURE_ID_LIST;

static WDC_DE_CSA_FEATURE_ID_LIST deFeatureIdList[] =
{
	{0x00                                   , "Dummy Placeholder"},
	{FID_ARBITRATION                        , "Arbitration"},
	{FID_POWER_MANAGEMENT                   , "PowerMgmnt"},
	{FID_LBA_RANGE_TYPE                     , "LbaRangeType"},
	{FID_TEMPERATURE_THRESHOLD              , "TempThreshold"},
	{FID_ERROR_RECOVERY                     , "ErrorRecovery"},
	{FID_VOLATILE_WRITE_CACHE               , "VolatileWriteCache"},
	{FID_NUMBER_OF_QUEUES                   , "NumOfQueues"},
	{FID_INTERRUPT_COALESCING               , "InterruptCoalesing"},
	{FID_INTERRUPT_VECTOR_CONFIGURATION     , "InterruptVectorConfig"},
	{FID_WRITE_ATOMICITY                    , "WriteAtomicity"},
	{FID_ASYNCHRONOUS_EVENT_CONFIGURATION   , "AsynEventConfig"},
	{FID_AUTONOMOUS_POWER_STATE_TRANSITION  , "AutonomousPowerState"},
};

typedef enum _NVME_VU_DE_LOGPAGE_NAMES
{
    NVME_DE_LOGPAGE_E3 = 0x01,
    NVME_DE_LOGPAGE_C0 = 0x02
} NVME_VU_DE_LOGPAGE_NAMES;
typedef struct _NVME_VU_DE_LOGPAGE_LIST
{
	NVME_VU_DE_LOGPAGE_NAMES logPageName;
	__u32	logPageId;
	__u32	logPageLen;
	char	logPageIdStr[5];
} NVME_VU_DE_LOGPAGE_LIST, *PNVME_VU_DE_LOGPAGE_LIST;

typedef struct _WDC_NVME_DE_VU_LOGPAGES
{
    NVME_VU_DE_LOGPAGE_NAMES vuLogPageReqd;
    __u32 numOfVULogPages;
} WDC_NVME_DE_VU_LOGPAGES, *PWDC_NVME_DE_VU_LOGPAGES;

static NVME_VU_DE_LOGPAGE_LIST deVULogPagesList[] =
{
    { NVME_DE_LOGPAGE_E3, 0xE3, 1072, "0xe3"},
    { NVME_DE_LOGPAGE_C0, 0xC0, 512, "0xc0"}
};

static int wdc_get_serial_name(int fd, char *file, size_t len, const char *suffix);
static int wdc_create_log_file(char *file, __u8 *drive_log_data,
		__u32 drive_log_length);
static int wdc_do_clear_dump(int fd, __u8 opcode, __u32 cdw12);
static int wdc_do_dump(int fd, __u32 opcode,__u32 data_len,
		__u32 cdw12, char *file, __u32 xfer_size);
static int wdc_do_crash_dump(int fd, char *file, int type);
static int wdc_crash_dump(int fd, char *file, int type);
static int wdc_get_crash_dump(int argc, char **argv, struct command *command,
		struct plugin *plugin);
static int wdc_do_drive_log(int fd, char *file);
static int wdc_drive_log(int argc, char **argv, struct command *command,
		struct plugin *plugin);
static const char* wdc_purge_mon_status_to_string(__u32 status);
static int wdc_purge(int argc, char **argv,
		struct command *command, struct plugin *plugin);
static int wdc_purge_monitor(int argc, char **argv,
		struct command *command, struct plugin *plugin);
static int wdc_nvme_check_supported_log_page(int fd, __u8 log_id);
static int wdc_clear_pcie_corr(int argc, char **argv, struct command *command,
		struct plugin *plugin);
static int wdc_do_drive_essentials(int fd, char *dir, char *key);
static int wdc_drive_essentials(int argc, char **argv, struct command *command,
		struct plugin *plugin);

/* Drive log data size */
struct wdc_log_size {
	__le32	log_size;
};

/* E6 log header */
struct wdc_e6_log_hdr {
	__le32  eye_catcher;
	__u8	log_size[4];
};

/* DUI log header */
struct wdc_dui_log_section {
	__le16	section_type;
	__le16	data_area_id;
	__le32	section_size;
};

/* DUI log header V2 */
struct __attribute__((__packed__)) wdc_dui_log_section_v2 {
	__le16	section_type;
	__le16	data_area_id;
	__le64	section_size;
};

/* DUI log header V4 */
struct wdc_dui_log_section_v4 {
	__le16	section_type;
	__u8	data_area_id;
	__u8    reserved;
	__le32	section_size_sectors;
};

struct wdc_dui_log_hdr {
	__u8    telemetry_hdr[512];
	__le16	hdr_version;
	__le16	section_count;
	__le32	log_size;
	struct	wdc_dui_log_section log_section[WDC_NVME_DUI_MAX_SECTION];
	__u8    log_data[40];
};

struct  __attribute__((__packed__)) wdc_dui_log_hdr_v2 {
	__u8    telemetry_hdr[512];
	__u8	hdr_version;
	__u8    product_id;
	__le16	section_count;
	__le64	log_size;
	struct	wdc_dui_log_section_v2 log_section[WDC_NVME_DUI_MAX_SECTION_V2];
	__u8    log_data[40];
};

struct  __attribute__((__packed__)) wdc_dui_log_hdr_v3 {
	__u8    telemetry_hdr[512];
	__u8	hdr_version;
	__u8    product_id;
	__le16	section_count;
	__le64	log_size;
	struct	wdc_dui_log_section_v2 log_section[WDC_NVME_DUI_MAX_SECTION_V3];
	__u8    securityNonce[36];
	__u8    log_data[40];
};

struct  __attribute__((__packed__)) wdc_dui_log_hdr_v4 {
	__u8    telemetry_hdr[512];
	__u8	hdr_version;
	__u8    product_id;
	__le16	section_count;
	__le32	log_size_sectors;
	struct	wdc_dui_log_section_v4 log_section[WDC_NVME_DUI_MAX_SECTION];
	__u8    log_data[40];
};

/* Purge monitor response */
struct wdc_nvme_purge_monitor_data {
	__le16 	rsvd1;
	__le16 	rsvd2;
	__le16 	first_erase_failure_cnt;
	__le16 	second_erase_failure_cnt;
	__le16 	rsvd3;
	__le16 	programm_failure_cnt;
	__le32 	rsvd4;
	__le32 	rsvd5;
	__le32 	entire_progress_total;
	__le32 	entire_progress_current;
	__u8   	rsvd6[14];
};

/* Additional Smart Log */
struct wdc_log_page_header {
	uint8_t	num_subpages;
	uint8_t	reserved;
	__le16	total_log_size;
};

struct wdc_log_page_subpage_header {
	uint8_t	spcode;
	uint8_t	pcset;
	__le16	subpage_length;
};

struct wdc_ssd_perf_stats {
	__le64	hr_cmds;		/* Host Read Commands				*/
	__le64	hr_blks;		/* Host Read Blocks					*/
	__le64	hr_ch_cmds;		/* Host Read Cache Hit Commands		*/
	__le64	hr_ch_blks;		/* Host Read Cache Hit Blocks		*/
	__le64	hr_st_cmds;		/* Host Read Stalled Commands		*/
	__le64	hw_cmds;		/* Host Write Commands				*/
	__le64	hw_blks;		/* Host Write Blocks				*/
	__le64	hw_os_cmds;		/* Host Write Odd Start Commands	*/
	__le64	hw_oe_cmds;		/* Host Write Odd End Commands		*/
	__le64	hw_st_cmds;		/* Host Write Commands Stalled		*/
	__le64	nr_cmds;		/* NAND Read Commands				*/
	__le64	nr_blks;		/* NAND Read Blocks					*/
	__le64	nw_cmds;		/* NAND Write Commands				*/
	__le64	nw_blks;		/* NAND Write Blocks				*/
	__le64	nrbw;			/* NAND Read Before Write			*/
};

/* Additional C2 Log Page */
struct wdc_c2_log_page_header {
	__le32	length;
	__le32	version;
};

struct wdc_c2_log_subpage_header {
	__le32	length;
	__le32	entry_id;
	__le32	data;
};

struct wdc_c2_cbs_data {
	__le32	length;
	__u8	data[];
};

struct __attribute__((__packed__)) wdc_bd_ca_log_format {
	__u8	field_id;
	__u8	reserved1[2];
	__u8	normalized_value;
	__u8	raw_value[8];
};

struct __attribute__((__packed__)) wdc_ssd_ca_perf_stats {
	__le64  nand_bytes_wr_lo;                       /* 0x00 - NAND Bytes Written lo            */
	__le64  nand_bytes_wr_hi;                       /* 0x08 - NAND Bytes Written hi            */
	__le64  nand_bytes_rd_lo;                       /* 0x10 - NAND Bytes Read lo               */
	__le64  nand_bytes_rd_hi;                       /* 0x18 - NAND Bytes Read hi               */
	__le64  nand_bad_block;                         /* 0x20 - NAND Bad Block Count             */
	__le64  uncorr_read_count;                      /* 0x28 - Uncorrectable Read Count         */
	__le64  ecc_error_count;                        /* 0x30 - Soft ECC Error Count             */
	__le32  ssd_detect_count;                       /* 0x38 - SSD End to End Detection Count   */
	__le32  ssd_correct_count;                      /* 0x3C - SSD End to End Correction Count  */
	__u8    data_percent_used;                      /* 0x40 - System Data Percent Used         */
	__le32  data_erase_max;                         /* 0x41 - User Data Erase Counts           */
	__le32  data_erase_min;                         /* 0x45 - User Data Erase Counts           */
	__le64  refresh_count;                          /* 0x49 - Refresh Count                    */
	__le64  program_fail;                           /* 0x51 - Program Fail Count               */
	__le64  user_erase_fail;                        /* 0x59 - User Data Erase Fail Count       */
	__le64  system_erase_fail;                      /* 0x61 - System Area Erase Fail Count     */
	__u8    thermal_throttle_status;                /* 0x69 - Thermal Throttling Status        */
	__u8    thermal_throttle_count;                 /* 0x6A - Thermal Throttling Count         */
	__le64  pcie_corr_error;                        /* 0x6B - pcie Correctable Error Count     */
	__le32  incomplete_shutdown_count;              /* 0x73 - Incomplete Shutdown Count        */
	__u8    percent_free_blocks;                    /* 0x77 - Percent Free Blocks              */
	__u8    rsvd[392];                              /* 0x78 - Reserved bytes 120-511           */
};

/* VS NAND Stats */
#define WDC_NVME_NAND_STATS_LOG_ID			0xFB
#define WDC_NVME_NAND_STATS_SIZE			0x200

static bool get_dev_mgment_cbs_data(int fd,
				    __u8 log_id, void **cbs_data);

struct __attribute__((__packed__)) wdc_nand_stats {
	__u8		nand_write_tlc[16];
	__u8		nand_write_slc[16];
	__le32		nand_prog_failure;
	__le32		nand_erase_failure;
	__le32		bad_block_count;
	__le64		nand_rec_trigger_event;
	__le64		e2e_error_counter;
	__le64		successful_ns_resize_event;
	__u8		rsvd[444];
};

enum {
	SCAO_V1_PMUWT              =  0,	/* Physical media units written TLC */
	SCAO_V1_PMUWS              = 16,	/* Physical media units written SLC */
	SCAO_V1_BUNBN              = 32,	/* Bad user nand blocks normalized */
	SCAO_V1_BUNBR              = 34,	/* Bad user nand blocks raw */
	SCAO_V1_XRC                = 40,	/* XOR recovery count */
	SCAO_V1_UREC               = 48,	/* Uncorrectable read error count */
	SCAO_V1_EECE               = 56,	/* End to end corrected errors */
	SCAO_V1_EEDE               = 64,	/* End to end detected errors */
	SCAO_V1_EEUE               = 72,	/* End to end uncorrected errors */
	SCAO_V1_SDPU               = 80,	/* System data percent used */
	SCAO_V1_MNUDEC             = 84,	/* Min User data erase counts (TLC) */
	SCAO_V1_MXUDEC             = 92,	/* Max User data erase counts (TLC) */
	SCAO_V1_AVUDEC             = 100,	/* Average User data erase counts (TLC) */
	SCAO_V1_MNEC               = 108,	/* Min Erase counts (SLC) */
	SCAO_V1_MXEC               = 116,	/* Max Erase counts (SLC) */
	SCAO_V1_AVEC               = 124,	/* Average Erase counts (SLC) */
	SCAO_V1_PFCN               = 132,	/* Program fail count normalized */
	SCAO_V1_PFCR               = 134,	/* Program fail count raw */
	SCAO_V1_EFCN               = 140,	/* Erase fail count normalized */
	SCAO_V1_EFCR               = 142,	/* Erase fail count raw */
	SCAO_V1_PCEC               = 148,	/* PCIe correctable error count */
	SCAO_V1_PFBU               = 156,	/* Percent free blocks (User) */
	SCAO_V1_SVN                = 160,	/* Security Version Number */
	SCAO_V1_PFBS               = 168,	/* Percent free blocks (System) */
	SCAO_V1_DCC                = 172,	/* Deallocate Commands Completed */
	SCAO_V1_TNU                = 188,	/* Total Namespace Utilization */
	SCAO_V1_FCC                = 196,	/* Format NVM Commands Completed */
	SCAO_V1_BBPG               = 198,	/* Background Back-Pressure Gauge */
	SCAO_V1_SEEC               = 202,	/* Soft ECC error count */
	SCAO_V1_RFSC               = 210,	/* Refresh count */
	SCAO_V1_BSNBN              = 218,	/* Bad system nand blocks normalized */
	SCAO_V1_BSNBR              = 220,	/* Bad system nand blocks raw */
	SCAO_V1_EEST               = 226,	/* Endurance estimate */
	SCAO_V1_TTC                = 242,	/* Thermal throttling count */
	SCAO_V1_UIO                = 244,	/* Unaligned I/O */
	SCAO_V1_PMUR               = 252,	/* Physical media units read */
	SCAO_V1_RTOC               = 268,	/* Read command timeout count */
	SCAO_V1_WTOC               = 272,	/* Write command timeout count */
	SCAO_V1_TTOC               = 276,	/* Trim command timeout count */
	SCAO_V1_PLRC               = 284,	/* PCIe Link Retraining Count */
	SCAO_V1_PSCC               = 292,	/* Power State Change Count */
	SCAO_V1_MAVF               = 300,	/* Boot SSD major version field */
	SCAO_V1_MIVF               = 302,	/* Boot SSD minor version field */
	SCAO_V1_PVF                = 304,	/* Boot SSD point version field */
	SCAO_V1_EVF                = 306,	/* Boot SSD errata version field */
	SCAO_V1_FTLUS              = 308,	/* FTL Unit Size */
	SCAO_V1_TCGOS              = 312,	/* TCG Ownership Status */

	SCAO_V1_LPV                = 494,	/* Log page version - 0x0001 */
	SCAO_V1_LPG                = 496,	/* Log page GUID */
};

#define WDC_C0_GUID_LENGTH              16
#define WDC_SCA_V1_NAND_STATS           0x1
#define WDC_SCA_V1_ALL                  0xF

static __u8 ext_smart_guid[WDC_C0_GUID_LENGTH] = {
	0x65, 0x43, 0x88, 0x78, 0xAC, 0xD8, 0x78, 0xA1,
	0x66, 0x42, 0x1E, 0x0F, 0x92, 0xD7, 0x6D, 0xC4
};

struct __attribute__((__packed__)) wdc_nvme_ext_smart_log {
	__u8  ext_smart_pmuwt[16];			/* 000 Physical media units written TLC */
	__u8  ext_smart_pmuws[16];			/* 016 Physical media units written SLC */
	__u8  ext_smart_bunbc[8];			/* 032 Bad user nand block count */
	__u64 ext_smart_xrc;				/* 040 XOR recovery count */
	__u64 ext_smart_urec;				/* 048 Uncorrectable read error count */
	__u64 ext_smart_eece;				/* 056 End to end corrected errors */
	__u64 ext_smart_eede;				/* 064 End to end detected errors */
	__u64 ext_smart_eeue;				/* 072 End to end uncorrected errors */
	__u8  ext_smart_sdpu;				/* 080 System data percent used */
	__u8  ext_smart_rsvd1[3];			/* 081 reserved */
	__u64 ext_smart_mnudec;				/* 084 Min User data erase counts (TLC) */
	__u64 ext_smart_mxudec;				/* 092 Max User data erase counts (TLC) */
	__u64 ext_smart_avudec;				/* 100 Average User data erase counts (TLC) */
	__u64 ext_smart_mnec;				/* 108 Min Erase counts (SLC) */
	__u64 ext_smart_mxec;				/* 116 Max Erase counts (SLC) */
	__u64 ext_smart_avec;				/* 124 Average Erase counts (SLC) */
	__u8  ext_smart_pfc[8];				/* 132 Program fail count */
	__u8  ext_smart_efc[8];				/* 140 Erase fail count */
	__u64 ext_smart_pcec;				/* 148 PCIe correctable error count */
	__u8  ext_smart_pfbu;				/* 156 Percent free blocks (User) */
	__u8  ext_smart_rsvd2[3];			/* 157 reserved */
	__u64 ext_smart_svn;				/* 160 Security Version Number */
	__u8  ext_smart_pfbs;				/* 168 Percent free blocks (System) */
	__u8  ext_smart_rsvd3[3];			/* 169 reserved */
	__u8  ext_smart_dcc[16];			/* 172 Deallocate Commands Completed */
	__u64 ext_smart_tnu;				/* 188 Total Namespace Utilization  */
	__u16 ext_smart_fcc;				/* 196 Format NVM Commands Completed */
	__u8  ext_smart_bbpg;				/* 198 Background Back-Pressure Gauge */
	__u8  ext_smart_rsvd4[3];			/* 199 reserved */
	__u64 ext_smart_seec;				/* 202 Soft ECC error count */
	__u64 ext_smart_rfsc;				/* 210 Refresh count */
	__u8  ext_smart_bsnbc[8];			/* 218 Bad system nand block count */
	__u8  ext_smart_eest[16];			/* 226 Endurance estimate */
	__u16 ext_smart_ttc;				/* 242 Thermal throttling count */
	__u64 ext_smart_uio;				/* 244 Unaligned I/O */
	__u8  ext_smart_pmur[16];			/* 252 Physical media units read */
	__u32 ext_smart_rtoc;				/* 268 Read command timeout count */
	__u32 ext_smart_wtoc;				/* 272 Write command timeout count */
	__u32 ext_smart_ttoc;				/* 276 Trim command timeout count */
	__u8  ext_smart_rsvd5[4];			/* 280 reserved */
	__u64 ext_smart_plrc;				/* 284 PCIe Link Retraining Count */
	__u64 ext_smart_pscc;				/* 292 Power State Change Count */
	__u16 ext_smart_maj;				/* 300 Boot SSD major version field */
	__u16 ext_smart_min;				/* 302 Boot SSD minor version field */
	__u16 ext_smart_pt;				/* 304 Boot SSD point version field */
	__u16 ext_smart_err;				/* 306 Boot SSD errata version field */
	__u32 ext_smart_ftlus;				/* 308 FTL Unit Size */
	__u32 ext_smart_tcgos;				/* 312 TCG Ownership Status */
	__u8  ext_smart_rsvd6[178];			/* 316 reserved */
	__u16 ext_smart_lpv;				/* 494 Log page version - 0x0001 */
	__u8  ext_smart_lpg[16];			/* 496 Log page GUID */
};

enum {
	SCAO_PMUW               =  0,	/* Physical media units written */
	SCAO_PMUR               = 16,	/* Physical media units read */
	SCAO_BUNBR              = 32,	/* Bad user nand blocks raw */
	SCAO_BUNBN              = 38,	/* Bad user nand blocks normalized */
	SCAO_BSNBR              = 40,	/* Bad system nand blocks raw */
	SCAO_BSNBN              = 46,	/* Bad system nand blocks normalized */
	SCAO_XRC                = 48,	/* XOR recovery count */
	SCAO_UREC               = 56,	/* Uncorrectable read error count */
	SCAO_SEEC               = 64,	/* Soft ecc error count */
	SCAO_EECE               = 72,	/* End to end corrected errors */
	SCAO_EEDC               = 76,	/* End to end detected errors */
	SCAO_SDPU               = 80,	/* System data percent used */
	SCAO_RFSC               = 81,	/* Refresh counts */
	SCAO_MXUDEC             = 88,	/* Max User data erase counts */
	SCAO_MNUDEC             = 92,	/* Min User data erase counts */
	SCAO_NTTE               = 96,	/* Number of Thermal throttling events */
	SCAO_CTS                = 97,	/* Current throttling status */
	SCAO_EVF                = 98,      /* Errata Version Field */
	SCAO_PVF                = 99,      /* Point Version Field */
	SCAO_MIVF               = 101,     /* Minor Version Field */
	SCAO_MAVF               = 103,     /* Major Version Field */
	SCAO_PCEC               = 104,	/* PCIe correctable error count */
	SCAO_ICS                = 112,	/* Incomplete shutdowns */
	SCAO_PFB                = 120,	/* Percent free blocks */
	SCAO_CPH                = 128,	/* Capacitor health */
	SCAO_NEV                = 130,     /* NVMe Errata Version */
	SCAO_UIO                = 136,	/* Unaligned I/O */
	SCAO_SVN                = 144,	/* Security Version Number */
	SCAO_NUSE               = 152,	/* NUSE - Namespace utilization */
	SCAO_PSC                = 160,	/* PLP start count */
	SCAO_EEST               = 176,	/* Endurance estimate */
	SCAO_PLRC               = 192,     /* PCIe Link Retraining Count */
	SCAO_PSCC               = 200,	/* Power State Change Count */
	SCAO_LPV                = 494,	/* Log page version */
	SCAO_LPG                = 496,	/* Log page GUID */
};

static __u8 scao_guid[WDC_C0_GUID_LENGTH] = {
	0xC5, 0xAF, 0x10, 0x28, 0xEA, 0xBF, 0xF2, 0xA4,
	0x9C, 0x4F, 0x6F, 0x7C, 0xC9, 0x14, 0xD5, 0xAF
};

enum {
	EOL_RBC                 = 76,	/* Realloc Block Count */
	EOL_ECCR                = 80,	/* ECC Rate */
	EOL_WRA                 = 84,	/* Write Amp */
	EOL_PLR                 = 88,	/* Percent Life Remaining */
	EOL_RSVBC               = 92,	/* Reserved Block Count */
	EOL_PFC                 = 96,	/* Program Fail Count */
	EOL_EFC                 = 100,	/* Erase Fail Count */
	EOL_RRER                = 108,	/* Raw Read Error Rate */
};

static int wdc_print_c0_cloud_attr_log(void *data, int fmt);

static double safe_div_fp(double numerator, double denominator)
{
	return denominator ? numerator / denominator : 0;
}

static double calc_percent(uint64_t numerator, uint64_t denominator)
{
	return denominator ?
		(uint64_t)(((double)numerator / (double)denominator) * 100) : 0;
}

static long double int128_to_double(__u8 *data)
{
	int i;
	long double result = 0;

	for (i = 0; i < 16; i++) {
		result *= 256;
		result += data[15 - i];
	}
	return result;
}

static int wdc_get_pci_ids(uint32_t *device_id, uint32_t *vendor_id)
{
	int fd, ret = -1;
	char *block, path[512], *id;

	id = calloc(1, 32);
	if (!id) {
		fprintf(stderr, "ERROR : WDC : %s : calloc failed\n", __func__);
		return -1;
	}

	block = nvme_char_from_block((char *)devicename);

	/* read the vendor ID from sys fs  */
	sprintf(path, "/sys/class/nvme/%s/device/vendor", block);

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		sprintf(path, "/sys/class/misc/%s/device/vendor", block);
		fd = open(path, O_RDONLY);
	}
	if (fd < 0) {
		fprintf(stderr, "ERROR : WDC : %s : Open vendor file failed\n", __func__);
		ret = -1;
		goto free_id;
	}

	ret = read(fd, id, 32);
	if (ret < 0) {
		fprintf(stderr, "%s: Read of pci vendor id failed\n", __func__);
		ret = -1;
		goto close_fd;
	} else {
		if (id[strlen(id) - 1] == '\n')
			id[strlen(id) - 1] = '\0';

		/* convert the device id string to an int  */
		*vendor_id = (uint32_t)strtol(&id[2], NULL, 16);
		ret = 0;
	}

	/* read the device ID from sys fs */
	sprintf(path, "/sys/class/nvme/%s/device/device", block);

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		sprintf(path, "/sys/class/misc/%s/device/device", block);
		fd = open(path, O_RDONLY);
	}
	if (fd < 0) {
		fprintf(stderr, "ERROR : WDC : %s : Open device file failed\n", __func__);
		ret = -1;
		goto close_fd;
	}

	ret = read(fd, id, 32);
	if (ret < 0) {
		fprintf(stderr, "%s: Read of pci device id failed\n", __func__);
		ret = -1;
	} else {
		if (id[strlen(id) - 1] == '\n')
			id[strlen(id) - 1] = '\0';

		/* convert the device id string to an int  */
		*device_id = (uint32_t)strtol(&id[2], NULL, 16);
		ret = 0;
	}

close_fd:
	close(fd);
free_id:
	free(id);
	return ret;
}

static int wdc_get_vendor_id(int fd, uint32_t *vendor_id)
{
	int ret;
	struct nvme_id_ctrl ctrl;

	memset(&ctrl, 0, sizeof(struct nvme_id_ctrl));
	ret = nvme_identify_ctrl(fd, &ctrl);
	if (ret) {
		fprintf(stderr, "ERROR: WDC: nvme_identify_ctrl() failed 0x%x\n", ret);
		return -1;
	}

	*vendor_id = (uint32_t) ctrl.vid;

	return ret;
}

static bool wdc_check_device(int fd)
{
	int ret;
	bool supported;
	uint32_t read_device_id = -1, read_vendor_id = -1;

	ret = wdc_get_pci_ids(&read_device_id, &read_vendor_id);
	if (ret < 0) {
		/* Use the identify nvme command to get vendor id due to NVMeOF device. */
		if (wdc_get_vendor_id(fd, &read_vendor_id) < 0)
			return false;
	}

	supported = false;

	if (read_vendor_id == WDC_NVME_VID ||
	    read_vendor_id == WDC_NVME_VID_2 ||
	    read_vendor_id == WDC_NVME_SNDK_VID)
		supported = true;
	else
		fprintf(stderr,
			"ERROR: WDC: unsupported WDC device, Vendor ID = 0x%x, Device ID = 0x%x\n",
			read_vendor_id, read_device_id);

	return supported;
}

static __u32 wdc_get_fw_cust_id(int fd)
{

	__u32 cust_id = WDC_INVALID_CUSTOMER_ID;
	__u32 *cust_id_ptr = NULL;

	if (!get_dev_mgment_cbs_data(fd, WDC_C2_CUSTOMER_ID_ID, (void *)&cust_id_ptr))
		fprintf(stderr, "%s: ERROR: WDC: 0xC2 Log Page entry ID 0x%x not found\n",
			__func__, WDC_C2_CUSTOMER_ID_ID);
	else
		cust_id = *cust_id_ptr;

	free(cust_id_ptr);
	return cust_id;
}

static __u64 wdc_get_enc_drive_capabilities(int fd)
{
	int ret;
	uint32_t read_vendor_id;
	__u64 capabilities = 0;
	__u32 cust_id;

	ret = wdc_get_vendor_id(fd, &read_vendor_id);
	if (ret < 0)
		return capabilities;

	switch (read_vendor_id) {
	case WDC_NVME_VID:
		capabilities = (WDC_DRIVE_CAP_CAP_DIAG | WDC_DRIVE_CAP_INTERNAL_LOG | WDC_DRIVE_CAP_CLEAR_PCIE |
			WDC_DRIVE_CAP_DRIVE_LOG | WDC_DRIVE_CAP_CRASH_DUMP | WDC_DRIVE_CAP_PFAIL_DUMP);

		/* verify the 0xCA log page is supported */
		if (wdc_nvme_check_supported_log_page(fd, WDC_NVME_GET_DEVICE_INFO_LOG_OPCODE) == true)
			capabilities |= WDC_DRIVE_CAP_CA_LOG_PAGE;

		/* verify the 0xC1 log page is supported */
		if (wdc_nvme_check_supported_log_page(fd, WDC_NVME_ADD_LOG_OPCODE) == true)
			capabilities |= WDC_DRIVE_CAP_C1_LOG_PAGE;
		break;
	case WDC_NVME_VID_2:
		capabilities = (WDC_DRIVE_CAP_CAP_DIAG | WDC_DRIVE_CAP_INTERNAL_LOG |
			WDC_DRIVE_CAP_DRIVE_STATUS | WDC_DRIVE_CAP_CLEAR_ASSERT |
			WDC_DRIVE_CAP_RESIZE);

		/* verify the 0xC3 log page is supported */
		if (wdc_nvme_check_supported_log_page(fd, WDC_LATENCY_MON_LOG_ID) == true)
			capabilities |= WDC_DRIVE_CAP_C3_LOG_PAGE;

		/* verify the 0xCB log page is supported */
		if (wdc_nvme_check_supported_log_page(fd, WDC_NVME_GET_FW_ACT_HISTORY_LOG_ID) == true)
			capabilities |= WDC_DRIVE_CAP_FW_ACTIVATE_HISTORY;

		/* verify the 0xCA log page is supported */
		if (wdc_nvme_check_supported_log_page(fd, WDC_NVME_GET_DEVICE_INFO_LOG_OPCODE) == true)
			capabilities |= WDC_DRIVE_CAP_CA_LOG_PAGE;

		/* verify the 0xD0 log page is supported */
		if (wdc_nvme_check_supported_log_page(fd, WDC_NVME_GET_VU_SMART_LOG_OPCODE) == true)
			capabilities |= WDC_DRIVE_CAP_D0_LOG_PAGE;

		cust_id = wdc_get_fw_cust_id(fd);
		if (cust_id == WDC_INVALID_CUSTOMER_ID) {
			fprintf(stderr, "%s: ERROR: WDC: invalid customer id\n", __func__);
			return -1;
		}

		if ((cust_id == WDC_CUSTOMER_ID_0x1004) || (cust_id == WDC_CUSTOMER_ID_0x1008) ||
				(cust_id == WDC_CUSTOMER_ID_0x1005) || (cust_id == WDC_CUSTOMER_ID_0x1304))
			capabilities |= (WDC_DRIVE_CAP_VU_FID_CLEAR_FW_ACT_HISTORY | WDC_DRIVE_CAP_VU_FID_CLEAR_PCIE);
		else
			capabilities |= (WDC_DRIVE_CAP_CLEAR_FW_ACT_HISTORY | WDC_DRIVE_CAP_CLEAR_PCIE);

		break;
	case WDC_NVME_SNDK_VID:
		capabilities = WDC_DRIVE_CAP_DRIVE_ESSENTIALS;
		break;
	default:
		capabilities = 0;
	}

	return capabilities;
}

static __u64 wdc_get_drive_capabilities(int fd)
{
	int ret;
	uint32_t read_device_id = -1, read_vendor_id = -1;
	__u64 capabilities = 0;
	__u32 cust_id;

	ret = wdc_get_pci_ids(&read_device_id, &read_vendor_id);
	if (ret < 0) {
		if (wdc_get_vendor_id(fd, &read_vendor_id) < 0)
			return capabilities;
	}

	/* below check condition is added due in NVMeOF device we dont have device_id so we need to use only vendor_id*/
	if (read_device_id == -1 && read_vendor_id != -1) {
		capabilities = wdc_get_enc_drive_capabilities(fd);
		return capabilities;
	}

	switch (read_vendor_id) {
	case WDC_NVME_VID:
		switch (read_device_id) {
		case WDC_NVME_SN100_DEV_ID:
			capabilities = (WDC_DRIVE_CAP_CAP_DIAG | WDC_DRIVE_CAP_INTERNAL_LOG | WDC_DRIVE_CAP_C1_LOG_PAGE |
					WDC_DRIVE_CAP_DRIVE_LOG | WDC_DRIVE_CAP_CRASH_DUMP | WDC_DRIVE_CAP_PFAIL_DUMP |
					WDC_DRIVE_CAP_PURGE);
			break;
		case WDC_NVME_SN200_DEV_ID:
			capabilities = (WDC_DRIVE_CAP_CAP_DIAG | WDC_DRIVE_CAP_INTERNAL_LOG | WDC_DRIVE_CAP_CLEAR_PCIE |
					WDC_DRIVE_CAP_DRIVE_LOG | WDC_DRIVE_CAP_CRASH_DUMP | WDC_DRIVE_CAP_PFAIL_DUMP |
					WDC_DRIVE_CAP_PURGE);

			/* verify the 0xCA log page is supported */
			if (wdc_nvme_check_supported_log_page(fd,
							      WDC_NVME_GET_DEVICE_INFO_LOG_OPCODE))
				capabilities |= WDC_DRIVE_CAP_CA_LOG_PAGE;

			/* verify the 0xC1 log page is supported */
			if (wdc_nvme_check_supported_log_page(fd,
							      WDC_NVME_ADD_LOG_OPCODE))
				capabilities |= WDC_DRIVE_CAP_C1_LOG_PAGE;
			break;
		default:
			capabilities = 0;
		}
		break;
	case WDC_NVME_VID_2:
		switch (read_device_id) {
		case WDC_NVME_SN630_DEV_ID:
		case WDC_NVME_SN630_DEV_ID_1:
			capabilities = (WDC_DRIVE_CAP_CAP_DIAG | WDC_DRIVE_CAP_INTERNAL_LOG |
					WDC_DRIVE_CAP_DRIVE_STATUS | WDC_DRIVE_CAP_CLEAR_ASSERT |
					WDC_DRIVE_CAP_RESIZE | WDC_DRIVE_CAP_CLEAR_PCIE);
			/* verify the 0xCA log page is supported */
			if (wdc_nvme_check_supported_log_page(fd,
							      WDC_NVME_GET_DEVICE_INFO_LOG_OPCODE))
				capabilities |= WDC_DRIVE_CAP_CA_LOG_PAGE;

			/* verify the 0xD0 log page is supported */
			if (wdc_nvme_check_supported_log_page(fd,
							      WDC_NVME_GET_VU_SMART_LOG_OPCODE))
				capabilities |= WDC_DRIVE_CAP_D0_LOG_PAGE;
			break;
		case WDC_NVME_SN640_DEV_ID:
		case WDC_NVME_SN640_DEV_ID_1:
		case WDC_NVME_SN640_DEV_ID_2:
		case WDC_NVME_SN640_DEV_ID_3:
		case WDC_NVME_SN560_DEV_ID_1:
		case WDC_NVME_SN560_DEV_ID_2:
		case WDC_NVME_SN560_DEV_ID_3:
		case WDC_NVME_SN660_DEV_ID:
			/* verify the 0xC0 log page is supported */
			if (wdc_nvme_check_supported_log_page(fd,
							      WDC_NVME_GET_SMART_CLOUD_ATTR_LOG_ID)
			    == true) {
				capabilities |= WDC_DRIVE_CAP_C0_LOG_PAGE;
			}

			capabilities |= (WDC_DRIVE_CAP_CAP_DIAG | WDC_DRIVE_CAP_INTERNAL_LOG |
					WDC_DRIVE_CAP_DRIVE_STATUS | WDC_DRIVE_CAP_CLEAR_ASSERT |
					WDC_DRIVE_CAP_RESIZE | WDC_DRIVE_CAP_FW_ACTIVATE_HISTORY |
					WDC_DRVIE_CAP_DISABLE_CTLR_TELE_LOG | WDC_DRIVE_CAP_REASON_ID |
					WDC_DRIVE_CAP_LOG_PAGE_DIR);

			/* verify the 0xC1 (OCP Error Recovery) log page is supported */
			if (wdc_nvme_check_supported_log_page(fd,
							      WDC_ERROR_REC_LOG_ID))
				capabilities |= WDC_DRIVE_CAP_OCP_C1_LOG_PAGE;

			/* verify the 0xC3 (OCP Latency Monitor) log page is supported */
			if (wdc_nvme_check_supported_log_page(fd,
							      WDC_LATENCY_MON_LOG_ID))
				capabilities |= WDC_DRIVE_CAP_C3_LOG_PAGE;

			/* verify the 0xC4 (OCP Device Capabilities) log page is supported */
			if (wdc_nvme_check_supported_log_page(fd,
							      WDC_DEV_CAP_LOG_ID))
				capabilities |= WDC_DRIVE_CAP_OCP_C4_LOG_PAGE;

			/* verify the 0xC5 (OCP Unsupported Requirments) log page is supported */
			if (wdc_nvme_check_supported_log_page(fd,
							      WDC_UNSUPPORTED_REQS_LOG_ID))
				capabilities |= WDC_DRIVE_CAP_OCP_C5_LOG_PAGE;

			/* verify the 0xCA log page is supported */
			if (wdc_nvme_check_supported_log_page(fd,
							      WDC_NVME_GET_DEVICE_INFO_LOG_OPCODE))
				capabilities |= WDC_DRIVE_CAP_CA_LOG_PAGE;

			/* verify the 0xD0 log page is supported */
			if (wdc_nvme_check_supported_log_page(fd,
							      WDC_NVME_GET_VU_SMART_LOG_OPCODE))
				capabilities |= WDC_DRIVE_CAP_D0_LOG_PAGE;

			cust_id = wdc_get_fw_cust_id(fd);
			if (cust_id == WDC_INVALID_CUSTOMER_ID) {
				fprintf(stderr, "%s: ERROR: WDC: invalid customer id\n", __func__);
				return -1;
			}

			if ((cust_id == WDC_CUSTOMER_ID_0x1004) || (cust_id == WDC_CUSTOMER_ID_0x1008) ||
					(cust_id == WDC_CUSTOMER_ID_0x1005) || (cust_id == WDC_CUSTOMER_ID_0x1304))
				capabilities |= (WDC_DRIVE_CAP_VU_FID_CLEAR_FW_ACT_HISTORY | WDC_DRIVE_CAP_VU_FID_CLEAR_PCIE |
						WDC_DRIVE_CAP_INFO | WDC_DRIVE_CAP_CLOUD_SSD_VERSION);
			else
				capabilities |= (WDC_DRIVE_CAP_CLEAR_FW_ACT_HISTORY | WDC_DRIVE_CAP_CLEAR_PCIE);

			break;
		case WDC_NVME_SN840_DEV_ID:
		case WDC_NVME_SN840_DEV_ID_1:
		case WDC_NVME_SN860_DEV_ID:
			/* verify the 0xC0 log page is supported */
			if (wdc_nvme_check_supported_log_page(fd,
							      WDC_NVME_GET_EOL_STATUS_LOG_OPCODE))
				capabilities |= WDC_DRIVE_CAP_C0_LOG_PAGE;
		case WDC_NVME_ZN540_DEV_ID:
		case WDC_NVME_SN540_DEV_ID:
			capabilities |= (WDC_DRIVE_CAP_CAP_DIAG | WDC_DRIVE_CAP_INTERNAL_LOG |
					WDC_DRIVE_CAP_DRIVE_STATUS | WDC_DRIVE_CAP_CLEAR_ASSERT |
					WDC_DRIVE_CAP_RESIZE | WDC_DRIVE_CAP_CLEAR_PCIE |
					WDC_DRIVE_CAP_FW_ACTIVATE_HISTORY | WDC_DRIVE_CAP_CLEAR_FW_ACT_HISTORY |
					WDC_DRVIE_CAP_DISABLE_CTLR_TELE_LOG | WDC_DRIVE_CAP_REASON_ID |
					WDC_DRIVE_CAP_LOG_PAGE_DIR);

			/* verify the 0xCA log page is supported */
			if (wdc_nvme_check_supported_log_page(fd,
							      WDC_NVME_GET_DEVICE_INFO_LOG_OPCODE))
				capabilities |= WDC_DRIVE_CAP_CA_LOG_PAGE;

			/* verify the 0xD0 log page is supported */
			if (wdc_nvme_check_supported_log_page(fd,
							      WDC_NVME_GET_VU_SMART_LOG_OPCODE))
				capabilities |= WDC_DRIVE_CAP_D0_LOG_PAGE;
			break;
		case WDC_NVME_SN650_DEV_ID:
		case WDC_NVME_SN650_DEV_ID_1:
		case WDC_NVME_SN650_DEV_ID_2:
		case WDC_NVME_SN650_DEV_ID_3:
		case WDC_NVME_SN650_DEV_ID_4:
		case WDC_NVME_SN655_DEV_ID:
		case WDC_NVME_SN550_DEV_ID:
			/* verify the 0xC0 log page is supported */
			if (wdc_nvme_check_supported_log_page(fd,
							      WDC_NVME_GET_SMART_CLOUD_ATTR_LOG_ID))
				capabilities |= WDC_DRIVE_CAP_C0_LOG_PAGE;

			/* verify the 0xC1 (OCP Error Recovery) log page is supported */
			if (wdc_nvme_check_supported_log_page(fd, WDC_ERROR_REC_LOG_ID))
				capabilities |= WDC_DRIVE_CAP_OCP_C1_LOG_PAGE;

			/* verify the 0xC3 (OCP Latency Monitor) log page is supported */
			if (wdc_nvme_check_supported_log_page(fd, WDC_LATENCY_MON_LOG_ID))
				capabilities |= WDC_DRIVE_CAP_C3_LOG_PAGE;

			/* verify the 0xC4 (OCP Device Capabilities) log page is supported */
			if (wdc_nvme_check_supported_log_page(fd, WDC_DEV_CAP_LOG_ID))
				capabilities |= WDC_DRIVE_CAP_OCP_C4_LOG_PAGE;

			/* verify the 0xC5 (OCP Unsupported Requirments) log page is supported */
			if (wdc_nvme_check_supported_log_page(fd, WDC_UNSUPPORTED_REQS_LOG_ID))
				capabilities |= WDC_DRIVE_CAP_OCP_C5_LOG_PAGE;

			capabilities |= (WDC_DRIVE_CAP_CAP_DIAG | WDC_DRIVE_CAP_INTERNAL_LOG |
					 WDC_DRIVE_CAP_DRIVE_STATUS | WDC_DRIVE_CAP_CLEAR_ASSERT |
					 WDC_DRIVE_CAP_RESIZE | WDC_DRIVE_CAP_FW_ACTIVATE_HISTORY |
					 WDC_DRVIE_CAP_DISABLE_CTLR_TELE_LOG |
					 WDC_DRIVE_CAP_REASON_ID | WDC_DRIVE_CAP_LOG_PAGE_DIR);

			cust_id = wdc_get_fw_cust_id(fd);
			if (cust_id == WDC_INVALID_CUSTOMER_ID) {
				fprintf(stderr, "%s: ERROR: WDC: invalid customer id\n", __func__);
				return -1;
			}

			if ((cust_id == WDC_CUSTOMER_ID_0x1004) ||
			    (cust_id == WDC_CUSTOMER_ID_0x1008) ||
			    (cust_id == WDC_CUSTOMER_ID_0x1005) ||
			    (cust_id == WDC_CUSTOMER_ID_0x1304))
				capabilities |= (WDC_DRIVE_CAP_VU_FID_CLEAR_FW_ACT_HISTORY |
						 WDC_DRIVE_CAP_VU_FID_CLEAR_PCIE |
						 WDC_DRIVE_CAP_INFO |
						 WDC_DRIVE_CAP_CLOUD_SSD_VERSION);
			else
				capabilities |= (WDC_DRIVE_CAP_CLEAR_FW_ACT_HISTORY |
						 WDC_DRIVE_CAP_CLEAR_PCIE);

			break;
		default:
			capabilities = 0;
		}
		break;
	case WDC_NVME_SNDK_VID:
		switch (read_device_id) {
		case WDC_NVME_SXSLCL_DEV_ID:
			capabilities = WDC_DRIVE_CAP_DRIVE_ESSENTIALS;
			break;
		case WDC_NVME_SN520_DEV_ID_1:
		case WDC_NVME_SN520_DEV_ID_2:
			capabilities = WDC_DRIVE_CAP_DUI_DATA;
			break;
		case WDC_NVME_SN720_DEV_ID:
			capabilities = WDC_DRIVE_CAP_DUI_DATA | WDC_DRIVE_CAP_NAND_STATS |
				       WDC_DRIVE_CAP_NS_RESIZE;
			break;
		case WDC_NVME_ZN350_DEV_ID:
		case WDC_NVME_ZN350_DEV_ID_1:
			capabilities = WDC_DRIVE_CAP_DUI_DATA | WDC_DRIVE_CAP_VU_FID_CLEAR_PCIE |
				       WDC_DRIVE_CAP_C0_LOG_PAGE |
				       WDC_DRIVE_CAP_VU_FID_CLEAR_FW_ACT_HISTORY |
				       WDC_DRIVE_CAP_FW_ACTIVATE_HISTORY_C2 | WDC_DRIVE_CAP_INFO |
				       WDC_DRIVE_CAP_CLOUD_SSD_VERSION | WDC_DRIVE_CAP_LOG_PAGE_DIR;
			break;
		default:
			capabilities = 0;
		}
		break;
	default:
		capabilities = 0;
	}

	return capabilities;
}

static bool wdc_check_device_match(int fd, int vendor_id, int device_id)
{
	int ret;
	uint32_t read_device_id, read_vendor_id;

	ret = wdc_get_pci_ids(&read_device_id, &read_vendor_id);
	if (ret < 0)
		return false;

	/* WDC : Use PCI Vendor and Device ID's to identify WDC Devices */
	if ((le32_to_cpu(read_vendor_id) == vendor_id) &&
		(le32_to_cpu(read_device_id) == device_id))
		return true;
	else
		return false;
}

static int wdc_get_serial_name(int fd, char *file, size_t len, const char *suffix)
{
	int i;
	int ret;
	int res_len = 0;
	char orig[PATH_MAX] = {0};
	struct nvme_id_ctrl ctrl;
	int ctrl_sn_len = sizeof (ctrl.sn);

	i = sizeof (ctrl.sn) - 1;
	strncpy(orig, file, PATH_MAX - 1);
	memset(file, 0, len);
	memset(&ctrl, 0, sizeof (struct nvme_id_ctrl));
	ret = nvme_identify_ctrl(fd, &ctrl);
	if (ret) {
		fprintf(stderr, "ERROR : WDC : nvme_identify_ctrl() failed "
				"0x%x\n", ret);
		return -1;
	}
	/* Remove trailing spaces from the name */
	while (i && ctrl.sn[i] == ' ') {
		ctrl.sn[i] = '\0';
		i--;
	}

	if (ctrl.sn[sizeof (ctrl.sn) - 1] == '\0') {
		ctrl_sn_len = strlen(ctrl.sn);
	}

	res_len = snprintf(file, len, "%s%.*s%s", orig, ctrl_sn_len, ctrl.sn, suffix);
	if (len <= res_len) {
		fprintf(stderr, "ERROR : WDC : cannot format serial number due to data "
				"of unexpected length\n");
		return -1;
	}

	return 0;
}

static int wdc_create_log_file(char *file, __u8 *drive_log_data,
		__u32 drive_log_length)
{
	int fd;
	int ret;

	if (drive_log_length == 0) {
		fprintf(stderr, "ERROR : WDC: invalid log file length\n");
		return -1;
	}

	fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd < 0) {
		fprintf(stderr, "ERROR : WDC: open : %s\n", strerror(errno));
		return -1;
	}

	while (drive_log_length > WRITE_SIZE) {
		ret = write(fd, drive_log_data, WRITE_SIZE);
		if (ret < 0) {
			fprintf (stderr, "ERROR : WDC: write : %s\n", strerror(errno));
			return -1;
		}
		drive_log_data += WRITE_SIZE;
		drive_log_length -= WRITE_SIZE;
	}

	ret = write(fd, drive_log_data, drive_log_length);
	if (ret < 0) {
		fprintf(stderr, "ERROR : WDC : write : %s\n", strerror(errno));
		return -1;
	}

	if (fsync(fd) < 0) {
		fprintf(stderr, "ERROR : WDC : fsync : %s\n", strerror(errno));
		return -1;
	}
	close(fd);
	return 0;
}

static int wdc_nvme_check_supported_log_page(int fd, __u8 log_id)
{
	int i;
	bool found = false;
	struct wdc_c2_cbs_data *cbs_data = NULL;

	if (get_dev_mgment_cbs_data(fd, WDC_C2_LOG_PAGES_SUPPORTED_ID, (void *)&cbs_data)) {
		if (cbs_data) {
			for (i = 0; i < le32_to_cpu(cbs_data->length); i++) {
				if (log_id == cbs_data->data[i]) {
					found = true;
					break;
				}
			}

#ifdef WDC_NVME_CLI_DEBUG
			if (!found) {
				fprintf(stderr, "ERROR: WDC: Log Page 0x%x not supported\n", log_id);
				fprintf(stderr, "WDC: Supported Log Pages:\n");
				/* print the supported pages */
				d((__u8 *)cbs_data->data, le32_to_cpu(cbs_data->length), 16, 1);
			}
#endif
			free(cbs_data);
		} else {
			fprintf(stderr, "ERROR: WDC: cbs_data ptr = NULL\n");
		}
	} else {
		fprintf(stderr, "ERROR: WDC: 0xC2 Log Page entry ID 0x%x not found\n",
			WDC_C2_LOG_PAGES_SUPPORTED_ID);
	}

	return found;
}

static int wdc_do_clear_dump(int fd, __u8 opcode, __u32 cdw12)
{
	int ret;
	struct nvme_admin_cmd admin_cmd;

	memset(&admin_cmd, 0, sizeof (struct nvme_admin_cmd));
	admin_cmd.opcode = opcode;
	admin_cmd.cdw12 = cdw12;
	ret = nvme_submit_passthru(fd, NVME_IOCTL_ADMIN_CMD, &admin_cmd);
	if (ret != 0) {
		fprintf(stdout, "ERROR : WDC : Crash dump erase failed\n");
	}
	fprintf(stderr, "NVMe Status:%s(%x)\n", nvme_status_to_string(ret), ret);
	return ret;
}

static __u32 wdc_dump_length(int fd, __u32 opcode, __u32 cdw10, __u32 cdw12, __u32 *dump_length)
{
	int ret;
	__u8 buf[WDC_NVME_LOG_SIZE_DATA_LEN] = {0};
	struct wdc_log_size *l;
	struct nvme_admin_cmd admin_cmd;

	l = (struct wdc_log_size *) buf;
	memset(&admin_cmd, 0, sizeof (struct nvme_admin_cmd));
	admin_cmd.opcode = opcode;
	admin_cmd.addr = (__u64)(uintptr_t)buf;
	admin_cmd.data_len = WDC_NVME_LOG_SIZE_DATA_LEN;
	admin_cmd.cdw10 = cdw10;
	admin_cmd.cdw12 = cdw12;

	ret = nvme_submit_passthru(fd, NVME_IOCTL_ADMIN_CMD, &admin_cmd);
	if (ret != 0) {
		l->log_size = 0;
		ret = -1;
		fprintf(stderr, "ERROR : WDC : reading dump length failed\n");
		fprintf(stderr, "NVMe Status:%s(%x)\n", nvme_status_to_string(ret), ret);
		return ret;
	}

	if (opcode == WDC_NVME_CAP_DIAG_OPCODE)
		*dump_length = buf[0x04] << 24 | buf[0x05] << 16 | buf[0x06] << 8 | buf[0x07];
	else
		*dump_length = le32_to_cpu(l->log_size);
	return ret;
}

static __u32 wdc_dump_length_e6(int fd, __u32 opcode, __u32 cdw10, __u32 cdw12, struct wdc_e6_log_hdr *dump_hdr)
{
	int ret;
	struct nvme_admin_cmd admin_cmd;

	memset(&admin_cmd, 0, sizeof (struct nvme_admin_cmd));
	admin_cmd.opcode = opcode;
	admin_cmd.addr = (__u64)(uintptr_t)dump_hdr;
	admin_cmd.data_len = WDC_NVME_LOG_SIZE_HDR_LEN;
	admin_cmd.cdw10 = cdw10;
	admin_cmd.cdw12 = cdw12;

	ret = nvme_submit_passthru(fd, NVME_IOCTL_ADMIN_CMD, &admin_cmd);
	if (ret != 0) {
		fprintf(stderr, "ERROR : WDC : reading dump length failed\n");
		fprintf(stderr, "NVMe Status:%s(%x)\n", nvme_status_to_string(ret), ret);
	}

	return ret;
}

static int wdc_do_dump(int fd, __u32 opcode,__u32 data_len,
		__u32 cdw12, char *file, __u32 xfer_size)
{
	int ret = 0;
	__u8 *dump_data;
	__u32 curr_data_offset, curr_data_len;
	int i;
	struct nvme_admin_cmd admin_cmd;
	__u32 dump_length = data_len;

	dump_data = (__u8 *) malloc(sizeof (__u8) * dump_length);
	if (dump_data == NULL) {
		fprintf(stderr, "%s: ERROR : malloc : %s\n", __func__, strerror(errno));
		return -1;
	}
	memset(dump_data, 0, sizeof (__u8) * dump_length);
	memset(&admin_cmd, 0, sizeof (struct nvme_admin_cmd));
	curr_data_offset = 0;
	curr_data_len = xfer_size;
	i = 0;

	admin_cmd.opcode = opcode;
	admin_cmd.addr = (__u64)(uintptr_t)dump_data;
	admin_cmd.data_len = curr_data_len;
	admin_cmd.cdw10 = curr_data_len >> 2;
	admin_cmd.cdw12 = cdw12;
	admin_cmd.cdw13 = curr_data_offset;

	while (curr_data_offset < data_len) {
		ret = nvme_submit_passthru(fd, NVME_IOCTL_ADMIN_CMD, &admin_cmd);
		if (ret != 0) {
			fprintf(stderr, "%s: ERROR : WDC : NVMe Status:%s(%x)\n",
				__func__, nvme_status_to_string(ret), ret);
			fprintf(stderr, "%s: ERROR : WDC : Get chunk %d, size = 0x%x, offset = 0x%x, addr = 0x%lx\n",
				__func__, i, admin_cmd.data_len, curr_data_offset, (long unsigned int)admin_cmd.addr);
			break;
		}

		if ((curr_data_offset + xfer_size) <= data_len)
			curr_data_len = xfer_size;
		else
			curr_data_len = data_len - curr_data_offset;   // last transfer

		curr_data_offset += curr_data_len;
		admin_cmd.addr = (__u64)(uintptr_t)dump_data + (__u64)curr_data_offset;
		admin_cmd.data_len = curr_data_len;
		admin_cmd.cdw10 = curr_data_len >> 2;
		admin_cmd.cdw13 = curr_data_offset >> 2;
		i++;
	}

	if (ret == 0) {
		fprintf(stderr, "%s:  NVMe Status:%s(%x)\n", __func__, nvme_status_to_string(ret), ret);
		ret = wdc_create_log_file(file, dump_data, dump_length);
	}
	free(dump_data);
	return ret;
}

static int wdc_do_dump_e6(int fd, __u32 opcode,__u32 data_len,
		__u32 cdw12, char *file, __u32 xfer_size, __u8 *log_hdr)
{
	int ret = 0;
	__u8 *dump_data;
	__u32 curr_data_offset, log_size;
	int i;
	struct nvme_admin_cmd admin_cmd;

	dump_data = (__u8 *) malloc(sizeof (__u8) * data_len);

	if (dump_data == NULL) {
		fprintf(stderr, "%s: ERROR : malloc : %s\n", __func__, strerror(errno));
		return -1;
	}
	memset(dump_data, 0, sizeof (__u8) * data_len);
	memset(&admin_cmd, 0, sizeof (struct nvme_admin_cmd));
	curr_data_offset = WDC_NVME_LOG_SIZE_HDR_LEN;
	i = 0;

	/* copy the 8 byte header into the dump_data buffer */
	memcpy(dump_data, log_hdr, WDC_NVME_LOG_SIZE_HDR_LEN);

	admin_cmd.opcode = opcode;
	admin_cmd.cdw12 = cdw12;

	log_size = data_len;
	while (log_size > 0) {
		xfer_size = min(xfer_size, log_size);

		admin_cmd.addr = (__u64)(uintptr_t)dump_data + (__u64)curr_data_offset;
		admin_cmd.data_len = xfer_size;
		admin_cmd.cdw10 = xfer_size >> 2;
		admin_cmd.cdw13 = curr_data_offset >> 2;

		ret = nvme_submit_passthru(fd, NVME_IOCTL_ADMIN_CMD, &admin_cmd);
		if (ret != 0) {
			fprintf(stderr, "%s: ERROR : WDC : NVMe Status:%s(%x)\n", __func__, nvme_status_to_string(ret), ret);
			fprintf(stderr, "%s: ERROR : WDC : Get chunk %d, size = 0x%x, offset = 0x%x, addr = 0x%lx\n",
					__func__, i, admin_cmd.data_len, curr_data_offset, (long unsigned int)admin_cmd.addr);
			break;
		}

		log_size         -= xfer_size;
		curr_data_offset += xfer_size;
		i++;
	}

	if (ret == 0) {
		fprintf(stderr, "%s:  NVMe Status:%s(%x)\n", __func__, nvme_status_to_string(ret), ret);
		ret = wdc_create_log_file(file, dump_data, data_len);
	}
	free(dump_data);
	return ret;
}

// static int wdc_do_cap_telemetry_log(int fd, char *file, __u32 bs, int type,
// 				    int data_area)
// {
// 	struct nvme_telemetry_log *log;
// 	size_t full_size = 0;
// 	int err = 0, output;
// 	__u32 host_gen = 1;
// 	int ctrl_init = 0;
// 	__u32 result;
// 	void *buf = NULL;
// 	__u8 *data_ptr = NULL;
// 	int data_written = 0, data_remaining = 0;
// 	struct nvme_id_ctrl ctrl;
// 	__u64 capabilities = 0;

// 	memset(&ctrl, 0, sizeof(struct nvme_id_ctrl));
// 	err = nvme_identify_ctrl(fd, &ctrl);
// 	if (err) {
// 		fprintf(stderr, "ERROR: WDC: nvme_identify_ctrl() failed 0x%x\n", err);
// 		return err;
// 	}

// 	if (!(ctrl.lpa & 0x8)) {
// 		fprintf(stderr, "Telemetry Host-Initiated and Telemetry Controller-Initiated log pages not supported\n");
// 		return -EINVAL;
// 	}

// 	capabilities = wdc_get_drive_capabilities(fd);

// 	if (type == WDC_TELEMETRY_TYPE_HOST) {
// 		host_gen = 1;
// 		ctrl_init = 0;
// 	} else if (type == WDC_TELEMETRY_TYPE_CONTROLLER) {
// 		if ((capabilities & WDC_DRIVE_CAP_INTERNAL_LOG) == WDC_DRIVE_CAP_INTERNAL_LOG) {
// 			/* Verify the Controller Initiated Option is enabled */
// 			err = nvme_get_features_data(fd,
// 						 WDC_VU_DISABLE_CNTLR_TELEMETRY_OPTION_FEATURE_ID,
// 						 0, 4, buf, &result);
// 			if (!err) {
// 				if (!result) {
// 					/* enabled */
// 					host_gen = 0;
// 					ctrl_init = 1;
// 				} else {
// 					fprintf(stderr, "%s: Controller initiated option telemetry log page disabled\n", __func__);
// 					return -EINVAL;
// 				}
// 			} else {
// 				fprintf(stderr, "ERROR: WDC: Get telemetry option feature failed.");
// 				nvme_show_status(err);
// 				return -EPERM;
// 			}
// 		} else {
// 			host_gen = 0;
// 			ctrl_init = 1;
// 		}
// 	} else {
// 		fprintf(stderr, "%s: Invalid type parameter; type = %d\n", __func__, type);
// 		return -EINVAL;
// 	}

// 	if (!file) {
// 		fprintf(stderr, "%s: Please provide an output file!\n", __func__);
// 		return -EINVAL;
// 	}

// 	output = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
// 	if (output < 0) {
// 		fprintf(stderr, "%s: Failed to open output file %s: %s!\n",
// 				__func__, file, strerror(errno));
// 		return output;
// 	}

// 	if (ctrl_init)
// 		err = nvme_get_ctrl_telemetry(dev_fd(dev), true, &log,
// 					  data_area, &full_size);
// 	else if (host_gen)
// 		err = nvme_get_new_host_telemetry(dev_fd(dev), &log,
// 						  data_area, &full_size);
// 	else
// 		err = nvme_get_host_telemetry(dev_fd(dev), &log, data_area,
// 					  &full_size);

// 	if (err < 0) {
// 		perror("get-telemetry-log");
// 		goto close_output;
// 	} else if (err > 0) {
// 		nvme_show_status(err);
// 		fprintf(stderr, "%s: Failed to acquire telemetry header!\n", __func__);
// 		goto close_output;
// 	}

// 	/*
// 	 *Continuously pull data until the offset hits the end of the last
// 	 *block.
// 	 */
// 	data_written = 0;
// 	data_remaining = full_size;
// 	data_ptr = (__u8 *)log;

// 	while (data_remaining) {
// 		data_written = write(output, data_ptr, data_remaining);

// 		if (data_written < 0) {
// 			data_remaining = data_written;
// 			break;
// 		} else if (data_written <= data_remaining) {
// 			data_remaining -= data_written;
// 			data_ptr += data_written;
// 		} else {
// 			/* Unexpected overwrite */
// 			fprintf(stderr, "Failure: Unexpected telemetry log overwrite - data_remaining = 0x%x, data_written = 0x%x\n",
// 					data_remaining, data_written);
// 			break;
// 		}
// 	}

// 	if (fsync(output) < 0) {
// 		fprintf(stderr, "ERROR: %s: fsync: %s\n", __func__, strerror(errno));
// 		err = -1;
// 	}

// 	free(log);
// close_output:
// 	close(output);
// 	return err;
// }

static int wdc_do_cap_diag(int fd, char *file, __u32 xfer_size, int type,
			   int data_area)
{
	int ret = -1;
	__u32 e6_log_hdr_size = WDC_NVME_CAP_DIAG_HEADER_TOC_SIZE;
	struct wdc_e6_log_hdr *log_hdr;
	__u32 cap_diag_length;

	log_hdr = (struct wdc_e6_log_hdr *) malloc(e6_log_hdr_size);
	if (log_hdr == NULL) {
		fprintf(stderr, "%s: ERROR : malloc : %s\n", __func__, strerror(errno));
		return -1;
	}
	memset(log_hdr, 0, e6_log_hdr_size);

	if (type == WDC_TELEMETRY_TYPE_NONE) {
		ret = wdc_dump_length_e6(fd, WDC_NVME_CAP_DIAG_OPCODE,
					WDC_NVME_CAP_DIAG_HEADER_TOC_SIZE >> 2,
					0x00, log_hdr);
		if (ret == -1) {
			ret = -1;
			goto out;
		}

		cap_diag_length = (log_hdr->log_size[0] << 24 | log_hdr->log_size[1] << 16 |
				log_hdr->log_size[2] << 8 | log_hdr->log_size[3]);

		if (!cap_diag_length) {
			fprintf(stderr, "INFO: WDC: Capture Diagnostics log is empty\n");
		} else {
			ret = wdc_do_dump_e6(fd,
					WDC_NVME_CAP_DIAG_OPCODE,
					cap_diag_length,
					(WDC_NVME_CAP_DIAG_SUBCMD << WDC_NVME_SUBCMD_SHIFT) | WDC_NVME_CAP_DIAG_CMD,
					file, xfer_size, (__u8 *)log_hdr);

			fprintf(stderr, "INFO: WDC: Capture Diagnostics log, length = 0x%x\n", cap_diag_length);
		}
	// } else if ((type == WDC_TELEMETRY_TYPE_HOST) ||
	// 		(type == WDC_TELEMETRY_TYPE_CONTROLLER)) {
	// 	/* Get the desired telemetry log page */
	// 	ret = wdc_do_cap_telemetry_log(fd, file, xfer_size, type, data_area);
	} else {
		fprintf(stderr, "%s: ERROR: Invalid type : %d\n", __func__, type);
	}

out:
	free(log_hdr);
	return ret;
}

static int wdc_cap_diag(int argc, char **argv, struct command *command,
		struct plugin *plugin)
{
	char *desc = "Capture Diagnostics Log.";
	char *file = "Output file pathname.";
	char *size = "Data retrieval transfer size.";
	__u64 capabilities = 0;
	char f[PATH_MAX] = {0};
	int fd;
	__u32 xfer_size = 0;
	int ret = 0;

	struct config {
		char *file;
		__u32 xfer_size;
	};

	struct config cfg = {
		.file = NULL,
		.xfer_size = 0x10000
	};

	const struct argconfig_commandline_options command_line_options[] = {
		{"output-file", 'o', "FILE", CFG_STRING, &cfg.file, required_argument, file},
		{"transfer-size", 's', "NUM", CFG_POSITIVE, &cfg.xfer_size, required_argument, size},
		{ NULL, '\0', NULL, CFG_NONE, NULL, no_argument, desc},
		{NULL}
	};

	fd = parse_and_open(argc, argv, desc, command_line_options, NULL, 0);
	if (fd < 0)
		return fd;

	if (cfg.file)
		strncpy(f, cfg.file, PATH_MAX - 1);
	if (cfg.xfer_size)
		xfer_size = cfg.xfer_size;
	ret = wdc_get_serial_name(fd, f, PATH_MAX, "cap_diag");
	if (ret) {
		fprintf(stderr, "ERROR: WDC: failed to generate file name\n");
		goto out;
	}
	if (!cfg.file) {
		if (strlen(f) > PATH_MAX - 5) {
			fprintf(stderr, "ERROR: WDC: file name overflow\n");
			ret = -1;
			goto out;
		}
		strcat(f, ".bin");
	}

	capabilities = wdc_get_drive_capabilities(fd);
	if ((capabilities & WDC_DRIVE_CAP_CAP_DIAG) == WDC_DRIVE_CAP_CAP_DIAG)
		ret = wdc_do_cap_diag(fd, f, xfer_size, 0, 0);
	else
		fprintf(stderr, "ERROR: WDC: unsupported device for this command\n");
out:
	close(fd);
	return ret;
}

static __u32 wdc_dump_dui_data(int fd, __u32 dataLen, __u32 offset, __u8 *dump_data, bool last_xfer)
{
	int ret;
	struct nvme_passthru_cmd admin_cmd;

	memset(&admin_cmd, 0, sizeof(struct nvme_passthru_cmd));
	admin_cmd.opcode = WDC_NVME_CAP_DUI_OPCODE;
	admin_cmd.nsid = 0xFFFFFFFF;
	admin_cmd.addr = (__u64)(uintptr_t)dump_data;
	admin_cmd.data_len = dataLen;
	admin_cmd.cdw10 = ((dataLen >> 2) - 1);
	admin_cmd.cdw12 = offset;
	if (last_xfer)
		admin_cmd.cdw14 = 0;
	else
		admin_cmd.cdw14 = WDC_NVME_CAP_DUI_DISABLE_IO;


	ret = nvme_submit_passthru(fd, NVME_IOCTL_ADMIN_CMD, &admin_cmd);
	if (ret) {
		fprintf(stderr, "ERROR: WDC: reading DUI data failed\n");
		fprintf(stderr, "NVMe Status:%s(%x)\n", nvme_status_to_string(ret), ret);
	}

	return ret;
}

static __u32 wdc_dump_dui_data_v2(int fd, __u32 dataLen, __u64 offset, __u8 *dump_data, bool last_xfer)
{
	int ret;
	struct nvme_passthru_cmd admin_cmd;
	__u64 offset_lo, offset_hi;

	memset(&admin_cmd, 0, sizeof(struct nvme_passthru_cmd));
	admin_cmd.opcode = WDC_NVME_CAP_DUI_OPCODE;
	admin_cmd.nsid = 0xFFFFFFFF;
	admin_cmd.addr = (__u64)(uintptr_t)dump_data;
	admin_cmd.data_len = dataLen;
	admin_cmd.cdw10 = ((dataLen >> 2) - 1);
	offset_lo = offset & 0x00000000FFFFFFFF;
	offset_hi = ((offset & 0xFFFFFFFF00000000) >> 32);
	admin_cmd.cdw12 = (__u32)offset_lo;
	admin_cmd.cdw13 = (__u32)offset_hi;

	if (last_xfer)
		admin_cmd.cdw14 = 0;
	else
		admin_cmd.cdw14 = WDC_NVME_CAP_DUI_DISABLE_IO;

	ret = nvme_submit_passthru(fd, NVME_IOCTL_ADMIN_CMD, &admin_cmd);
	if (ret) {
		fprintf(stderr, "ERROR: WDC: reading DUI data V2 failed\n");
		fprintf(stderr, "NVMe Status:%s(%x)\n", nvme_status_to_string(ret), ret);
	}

	return ret;
}

static int wdc_do_cap_dui_v1(int fd, char *file, __u32 xfer_size, int data_area, int verbose,
			     struct wdc_dui_log_hdr *log_hdr, __s64 *total_size)
{
	__s32 log_size = 0;
	__u32 cap_dui_length = le32_to_cpu(log_hdr->log_size);
	__u32 curr_data_offset = 0;
	__u8 *buffer_addr;
	__u8 *dump_data = NULL;
	bool last_xfer = false;
	int err;
	int i;
	int j;
	int output;
	int ret = 0;

	if (verbose) {
		fprintf(stderr, "INFO: WDC: Capture V1 Device Unit Info log, data area = %d\n",
			data_area);
		fprintf(stderr, "INFO: WDC: DUI Header Version = 0x%x\n", log_hdr->hdr_version);
	}

	if (!cap_dui_length) {
		fprintf(stderr, "INFO: WDC: Capture V1 Device Unit Info log is empty\n");
		return 0;
	}

	/* parse log header for all sections up to specified data area inclusively */
	if (data_area != WDC_NVME_DUI_MAX_DATA_AREA) {
		for (j = 0; j < WDC_NVME_DUI_MAX_SECTION; j++) {
			if (log_hdr->log_section[j].data_area_id <= data_area &&
					log_hdr->log_section[j].data_area_id) {
				log_size += log_hdr->log_section[j].section_size;
				if (verbose)
					fprintf(stderr,
						"%s: Data area ID %d : section size 0x%x, total size = 0x%x\n",
						__func__, log_hdr->log_section[j].data_area_id,
						(unsigned int)log_hdr->log_section[j].section_size,
						(unsigned int)log_size);

			} else {
				if (verbose)
					fprintf(stderr, "%s: break, total size = 0x%x\n", __func__,
						(unsigned int)log_size);
				break;
			}
		}
	} else {
		log_size = cap_dui_length;
	}

	*total_size = log_size;

	dump_data = (__u8 *)malloc(sizeof(__u8) * xfer_size);
	if (!dump_data) {
		fprintf(stderr, "%s: ERROR: dump data V1 malloc failed : status %s, size = 0x%x\n",
			__func__, strerror(errno), (unsigned int)xfer_size);
		return -1;
	}
	memset(dump_data, 0, sizeof(__u8) * xfer_size);

	output = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (output < 0) {
		fprintf(stderr, "%s: Failed to open output file %s: %s!\n", __func__, file,
			strerror(errno));
		free(dump_data);
		return output;
	}

	/* write the telemetry and log headers into the dump_file */
	err = write(output, (void *)log_hdr, WDC_NVME_CAP_DUI_HEADER_SIZE);
	if (err != WDC_NVME_CAP_DUI_HEADER_SIZE) {
		fprintf(stderr, "%s: Failed to flush header data to file!\n", __func__);
		goto free_mem;
	}

	log_size -= WDC_NVME_CAP_DUI_HEADER_SIZE;
	curr_data_offset = WDC_NVME_CAP_DUI_HEADER_SIZE;
	i = 0;
	buffer_addr = dump_data;

	for (; log_size > 0; log_size -= xfer_size) {
		xfer_size = min(xfer_size, log_size);

		if (log_size <= xfer_size)
			last_xfer = true;

		ret = wdc_dump_dui_data(fd, xfer_size, curr_data_offset, buffer_addr, last_xfer);
		if (ret) {
			fprintf(stderr,
				"%s: ERROR: WDC: Get chunk %d, size = 0x%"PRIx64", offset = 0x%x, addr = %p\n",
				__func__, i, (uint64_t)log_size, curr_data_offset, buffer_addr);
			fprintf(stderr, "%s: ERROR: WDC: ", __func__);
			fprintf(stderr, "NVMe Status:%s(%x)\n", nvme_status_to_string(ret), ret);
			break;
		}

		/* write the dump data into the file */
		err = write(output, (void *)buffer_addr, xfer_size);
		if (err != xfer_size) {
			fprintf(stderr,
				"%s: ERROR: WDC: Failed to flush DUI data to file! chunk %d, err = 0x%x, xfer_size = 0x%x\n",
				__func__, i, err, xfer_size);
			ret = -1;
			goto free_mem;
		}

		curr_data_offset += xfer_size;
		i++;
	}

free_mem:
	close(output);
	free(dump_data);
	return ret;
}

static int wdc_do_cap_dui_v2_v3(int fd, char *file, __u32 xfer_size, int data_area, int verbose,
				struct wdc_dui_log_hdr *log_hdr, __s64 *total_size, __u64 file_size,
				__u64 offset)
{
	__u64 cap_dui_length_v3;
	__u64 curr_data_offset = 0;
	__s64 log_size = 0;
	__u64 xfer_size_long = (__u64)xfer_size;
	__u8 *buffer_addr;
	__u8 *dump_data = NULL;
	bool last_xfer = false;
	int err;
	int i;
	int j;
	int output;
	int ret = 0;
	struct wdc_dui_log_hdr_v3 *log_hdr_v3 = (struct wdc_dui_log_hdr_v3 *)log_hdr;

	cap_dui_length_v3 = le64_to_cpu(log_hdr_v3->log_size);

	if (verbose) {
		fprintf(stderr,
			"INFO: WDC: Capture V2 or V3 Device Unit Info log, data area = %d\n",
			data_area);

		fprintf(stderr, "INFO: WDC: DUI Header Version = 0x%x\n",
			log_hdr_v3->hdr_version);
		if ((log_hdr->hdr_version & 0xFF) == 0x03)
			fprintf(stderr, "INFO: WDC: DUI Product ID = 0x%x/%c\n",
				log_hdr_v3->product_id, log_hdr_v3->product_id);
	}

	if (!cap_dui_length_v3) {
		fprintf(stderr, "INFO: WDC: Capture V2 or V3 Device Unit Info log is empty\n");
		return 0;
	}

	/* parse log header for all sections up to specified data area inclusively */
	if (data_area != WDC_NVME_DUI_MAX_DATA_AREA) {
		for (j = 0; j < WDC_NVME_DUI_MAX_SECTION_V3; j++) {
			if (log_hdr_v3->log_section[j].data_area_id <= data_area &&
					log_hdr_v3->log_section[j].data_area_id) {
				log_size += log_hdr_v3->log_section[j].section_size;
				if (verbose)
					fprintf(stderr,
						"%s: Data area ID %d : section size 0x%x, total size = 0x%"PRIx64"\n",
						__func__, log_hdr_v3->log_section[j].data_area_id,
						(unsigned int)log_hdr_v3->log_section[j].section_size,
						(uint64_t)log_size);
			} else {
				if (verbose)
					fprintf(stderr, "%s: break, total size = 0x%"PRIx64"\n",
						__func__, (uint64_t)log_size);
				break;
			}
		}
	} else {
		log_size = cap_dui_length_v3;
	}

	*total_size = log_size;

	if (offset >= *total_size) {
		fprintf(stderr,
			"%s: INFO: WDC: Offset 0x%"PRIx64" exceeds total size 0x%"PRIx64", no data retrieved\n",
			__func__, (uint64_t)offset, (uint64_t)*total_size);
		return -1;
	}

	dump_data = (__u8 *)malloc(sizeof(__u8) * xfer_size_long);
	if (!dump_data) {
		fprintf(stderr,
			"%s: ERROR: dump data v3 malloc failed : status %s, size = 0x%"PRIx64"\n",
			__func__, strerror(errno), (uint64_t)xfer_size_long);
		return -1;
	}
	memset(dump_data, 0, sizeof(__u8) * xfer_size_long);

	output = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (output < 0) {
		fprintf(stderr, "%s: Failed to open output file %s: %s!\n",
				__func__, file, strerror(errno));
		free(dump_data);
		return output;
	}

	curr_data_offset = 0;

	if (file_size) {
		/* Write the DUI data based on the passed in file size */
		if ((offset + file_size) > *total_size)
			log_size = min((*total_size - offset), file_size);
		else
			log_size = min(*total_size, file_size);

		if (verbose)
			fprintf(stderr,
				"%s: INFO: WDC: Offset 0x%"PRIx64", file size 0x%"PRIx64", total size 0x%"PRIx64", log size 0x%"PRIx64"\n",
				__func__, (uint64_t)offset,
				(uint64_t)file_size, (uint64_t)*total_size, (uint64_t)log_size);

		curr_data_offset = offset;
	}

	i = 0;
	buffer_addr = dump_data;

	for (; log_size > 0; log_size -= xfer_size_long) {
		xfer_size_long = min(xfer_size_long, log_size);

		if (log_size <= xfer_size_long)
			last_xfer = true;

		ret = wdc_dump_dui_data_v2(fd, (__u32)xfer_size_long, curr_data_offset, buffer_addr,
					   last_xfer);
		if (ret) {
			fprintf(stderr,
				"%s: ERROR: WDC: Get chunk %d, size = 0x%"PRIx64", offset = 0x%"PRIx64", addr = %p\n",
				__func__, i, (uint64_t)*total_size, (uint64_t)curr_data_offset,
				buffer_addr);
			fprintf(stderr, "%s: ERROR: WDC: ", __func__);
			fprintf(stderr, "NVMe Status:%s(%x)\n", nvme_status_to_string(ret), ret);
			break;
		}

		/* write the dump data into the file */
		err = write(output, (void *)buffer_addr, xfer_size_long);
		if (err != xfer_size_long) {
			fprintf(stderr,
				"%s: ERROR: WDC: Failed to flush DUI data to file! chunk %d, err = 0x%x, xfer_size = 0x%"PRIx64"\n",
				__func__, i, err, (uint64_t)xfer_size_long);
			ret = -1;
			goto free_mem;
		}

		curr_data_offset += xfer_size_long;
		i++;
	}

free_mem:
	close(output);
	free(dump_data);
	return ret;
}

static int wdc_do_cap_dui_v4(int fd, char *file, __u32 xfer_size, int data_area, int verbose,
			     struct wdc_dui_log_hdr *log_hdr, __s64 *total_size, __u64 file_size,
			     __u64 offset)
{
	__s64 log_size = 0;
	__s64 section_size_bytes = 0;
	__s64 xfer_size_long = (__s64)xfer_size;
	__u64 cap_dui_length_v4;
	__u64 curr_data_offset = 0;
	__u8 *buffer_addr;
	__u8 *dump_data = NULL;
	int err;
	int i;
	int j;
	int output;
	int ret = 0;
	bool last_xfer = false;
	struct wdc_dui_log_hdr_v4 *log_hdr_v4 = (struct wdc_dui_log_hdr_v4 *)log_hdr;

	cap_dui_length_v4 = le64_to_cpu(log_hdr_v4->log_size_sectors) * WDC_NVME_SN730_SECTOR_SIZE;

	if (verbose) {
		fprintf(stderr, "INFO: WDC: Capture V4 Device Unit Info log, data area = %d\n", data_area);
		fprintf(stderr, "INFO: WDC: DUI Header Version = 0x%x\n", log_hdr_v4->hdr_version);
		fprintf(stderr, "INFO: WDC: DUI Product ID = 0x%x/%c\n", log_hdr_v4->product_id, log_hdr_v4->product_id);
		fprintf(stderr, "INFO: WDC: DUI log size sectors = 0x%x\n", log_hdr_v4->log_size_sectors);
		fprintf(stderr, "INFO: WDC: DUI cap_dui_length = 0x%"PRIx64"\n", (uint64_t)cap_dui_length_v4);
	}

	if (!cap_dui_length_v4) {
		fprintf(stderr, "INFO: WDC: Capture V4 Device Unit Info log is empty\n");
		return 0;
	}

	/* parse log header for all sections up to specified data area inclusively */
	if (data_area != WDC_NVME_DUI_MAX_DATA_AREA) {
		for (j = 0; j < WDC_NVME_DUI_MAX_SECTION; j++) {
			if (log_hdr_v4->log_section[j].data_area_id <= data_area &&
			    log_hdr_v4->log_section[j].data_area_id) {
				section_size_bytes = ((__s64)log_hdr_v4->log_section[j].section_size_sectors * WDC_NVME_SN730_SECTOR_SIZE);
				log_size += section_size_bytes;
				if (verbose)
					fprintf(stderr,
						"%s: Data area ID %d : section size 0x%x sectors, section size 0x%"PRIx64" bytes, total size = 0x%"PRIx64"\n",
						__func__, log_hdr_v4->log_section[j].data_area_id,
						log_hdr_v4->log_section[j].section_size_sectors,
						(uint64_t)section_size_bytes, (uint64_t)log_size);
			} else {
				if (verbose)
					fprintf(stderr, "%s: break, total size = 0x%"PRIx64"\n", __func__, (uint64_t)log_size);
				break;
			}
		}
	} else {
		log_size = cap_dui_length_v4;
	}

	*total_size = log_size;

	if (offset >= *total_size) {
		fprintf(stderr,
			"%s: INFO: WDC: Offset 0x%"PRIx64" exceeds total size 0x%"PRIx64", no data retrieved\n",
			__func__, (uint64_t)offset, (uint64_t)*total_size);
		return -1;
	}

	dump_data = (__u8 *)malloc(sizeof(__u8) * xfer_size_long);
	if (!dump_data) {
		fprintf(stderr, "%s: ERROR: dump data V4 malloc failed : status %s, size = 0x%x\n",
			__func__, strerror(errno), (unsigned int)xfer_size_long);
		return -1;
	}
	memset(dump_data, 0, sizeof(__u8) * xfer_size_long);

	output = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (output < 0) {
		fprintf(stderr, "%s: Failed to open output file %s: %s!\n", __func__, file,
			strerror(errno));
		free(dump_data);
		return output;
	}

	curr_data_offset = 0;

	if (file_size) {
		/* Write the DUI data based on the passed in file size */
		if ((offset + file_size) > *total_size)
			log_size = min((*total_size - offset), file_size);
		else
			log_size = min(*total_size, file_size);

		if (verbose)
			fprintf(stderr,
				"%s: INFO: WDC: Offset 0x%"PRIx64", file size 0x%"PRIx64", total size 0x%"PRIx64", log size 0x%"PRIx64"\n",
				__func__, (uint64_t)offset, (uint64_t)file_size,
				(uint64_t)*total_size, (uint64_t)log_size);

		curr_data_offset = offset;
	}

	i = 0;
	buffer_addr = dump_data;

	for (; log_size > 0; log_size -= xfer_size_long) {
		xfer_size_long = min(xfer_size_long, log_size);

		if (log_size <= xfer_size_long)
			last_xfer = true;

		ret = wdc_dump_dui_data_v2(fd, (__u32)xfer_size_long, curr_data_offset, buffer_addr, last_xfer);
		if (ret) {
			fprintf(stderr,
				"%s: ERROR: WDC: Get chunk %d, size = 0x%"PRIx64", offset = 0x%"PRIx64", addr = %p\n",
				__func__, i, (uint64_t)log_size, (uint64_t)curr_data_offset,
				buffer_addr);
			fprintf(stderr, "%s: ERROR: WDC:", __func__);
			fprintf(stderr, "NVMe Status:%s(%x)\n", nvme_status_to_string(ret), ret);
			break;
		}

		/* write the dump data into the file */
		err = write(output, (void *)buffer_addr, xfer_size_long);
		if (err != xfer_size_long) {
			fprintf(stderr,
				"%s: ERROR: WDC: Failed to flush DUI data to file! chunk %d, err = 0x%x, xfer_size_long = 0x%"PRIx64"\n",
				__func__, i, err, (uint64_t)xfer_size_long);
			ret = -1;
			goto free_mem;
		}

		curr_data_offset += xfer_size_long;
		i++;
	}

free_mem:
	close(output);
	free(dump_data);
	return ret;
}

static int wdc_do_cap_dui(int fd, char *file, __u32 xfer_size, int data_area, int verbose,
			  __u64 file_size, __u64 offset)
{
	int ret = 0;
	__u32 dui_log_hdr_size = WDC_NVME_CAP_DUI_HEADER_SIZE;
	struct wdc_dui_log_hdr *log_hdr;
	__s64 total_size = 0;
	bool last_xfer = false;

	log_hdr = (struct wdc_dui_log_hdr *) malloc(dui_log_hdr_size);
	if (log_hdr == NULL) {
		fprintf(stderr, "%s: ERROR : malloc : %s\n", __func__, strerror(errno));
		return -1;
	}
	memset(log_hdr, 0, dui_log_hdr_size);

	/* get the dui telemetry and log headers */
	ret = wdc_dump_dui_data(fd, WDC_NVME_CAP_DUI_HEADER_SIZE, 0x00,	(__u8 *)log_hdr, last_xfer);
	if (ret) {
		fprintf(stderr, "%s: ERROR: WDC: Get DUI headers failed\n", __func__);
		fprintf(stderr, "%s: ERROR: WDC: ", __func__);
		fprintf(stderr, "NVMe Status:%s(%x)\n", nvme_status_to_string(ret), ret);
		goto out;
	}

	/* Check the Log Header version */
	if ((log_hdr->hdr_version & 0xFF) == 0x00 || (log_hdr->hdr_version & 0xFF) == 0x01) {
		ret = wdc_do_cap_dui_v1(fd, file, xfer_size, data_area, verbose, log_hdr,
					&total_size);
		if (ret)
			goto out;
	} else if ((log_hdr->hdr_version & 0xFF) == 0x02 ||
		   (log_hdr->hdr_version & 0xFF) == 0x03) {
		/* Process Version 2 or 3 header */
		ret = wdc_do_cap_dui_v2_v3(fd, file, xfer_size, data_area, verbose, log_hdr,
					   &total_size, file_size, offset);
		if (ret)
			goto out;
	} else if ((log_hdr->hdr_version & 0xFF) == 0x04) {
		ret = wdc_do_cap_dui_v4(fd, file, xfer_size, data_area, verbose, log_hdr,
					&total_size, file_size, offset);
		if (ret)
			goto out;
	} else {
		fprintf(stderr, "INFO: WDC: Unsupported header version = 0x%x\n",
			log_hdr->hdr_version);
		goto out;
	}
	
	fprintf(stderr, "NVMe Status:%s(%x)\n", nvme_status_to_string(ret), ret);
	if (verbose)
		fprintf(stderr, "INFO: WDC: Capture Device Unit Info log, length = 0x%"PRIx64"\n",
			(uint64_t)total_size);
out:
	free(log_hdr);
	return ret;
}

static int wdc_internal_fw_log(int argc, char **argv, struct command *command,
               struct plugin *plugin)
{
	char *desc = "Internal Firmware Log.";
	char *file = "Output file pathname.";
	char *size = "Data retrieval transfer size.";
	char *data_area = "Data area to retrieve up to. Currently only supported on the SN340, SN640, SN730, and SN840 devices.";
	char *file_size = "Output file size.  Currently only supported on the SN340 device.";
	char *offset = "Output file data offset. Currently only supported on the SN340 device.";
	char *type = "Telemetry type - NONE, HOST, or CONTROLLER. Currently only supported on the SN530, SN640, SN730, SN740, SN810, SN840 and ZN350 devices.";
	char *verbose = "Display more debug messages.";
	char f[PATH_MAX] = {0};
	char fileSuffix[PATH_MAX] = {0};
	__u32 xfer_size = 0;
	int fd;
	int telemetry_type = 0, telemetry_data_area = 0;
	UtilsTimeInfo             timeInfo;
	__u8                      timeStamp[MAX_PATH_LEN];
	__u64 capabilities = 0;
	int ret = -1;

	struct config {
		char *file;
		__u32 xfer_size;
		int data_area;
		__u64 file_size;
		__u64 offset;
		char *type;
		bool verbose;
	};

	struct config cfg = {
		.file = NULL,
		.xfer_size = 0x10000,
		.data_area = 0,
		.file_size = 0,
		.offset = 0,
		.type = NULL,
		.verbose = false,
	};

	const struct argconfig_commandline_options command_line_options[] = {
		{"output-file", 'o', "FILE", CFG_STRING, &cfg.file, required_argument, file},
		{"transfer-size", 's', "NUM", CFG_POSITIVE, &cfg.xfer_size, required_argument, size},
		{"data-area", 'd', "NUM", CFG_POSITIVE, &cfg.data_area, required_argument, data_area},
		{"file-size", 'f', "NUM", CFG_LONG, &cfg.file_size, required_argument, file_size},
		{"offset", 'e', "NUM", CFG_LONG, &cfg.offset, required_argument, offset},
		{"type", 't', "FILE", CFG_STRING, &cfg.type, required_argument, type},
		{"verbose", 'v', NULL, CFG_NONE, &cfg.verbose, no_argument, verbose},
		{ NULL, '\0', NULL, CFG_NONE, NULL, no_argument, desc},
		{NULL}
	};

	fd = parse_and_open(argc, argv, desc, command_line_options, NULL, 0);
	if (fd < 0)
		return fd;

	if (cfg.xfer_size) {
		xfer_size = cfg.xfer_size;
	} else {
		fprintf(stderr, "ERROR: WDC: Invalid length\n");
		goto out;
	}

	if (cfg.file) {
		int verify_file;

		/* verify the passed in file name and path is valid before getting the dump data */
		verify_file = open(cfg.file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if (verify_file < 0) {
			fprintf(stderr, "ERROR: WDC: open: %s\n", strerror(errno));
			goto out;
		}
		close(verify_file);
		strncpy(f, cfg.file, PATH_MAX - 1);
	} else {
		wdc_UtilsGetTime(&timeInfo);
		memset(timeStamp, 0, sizeof(timeStamp));
		wdc_UtilsSnprintf((char *)timeStamp, MAX_PATH_LEN, "%02u%02u%02u_%02u%02u%02u",
			timeInfo.year, timeInfo.month, timeInfo.dayOfMonth,
			timeInfo.hour, timeInfo.minute, timeInfo.second);
		snprintf(fileSuffix, PATH_MAX, "_internal_fw_log_%s", (char *)timeStamp);

		ret = wdc_get_serial_name(fd, f, PATH_MAX, fileSuffix);
		if (ret) {
			fprintf(stderr, "ERROR: WDC: failed to generate file name\n");
			goto out;
		}
	}

	if (!cfg.file) {
		if (strlen(f) > PATH_MAX - 5) {
			fprintf(stderr, "ERROR: WDC: file name overflow\n");
			ret = -1;
			goto out;
		}
		strcat(f, ".bin");
	}
	fprintf(stderr, "%s: filename = %s\n", __func__, f);

	if (cfg.data_area) {
		if (cfg.data_area > 5 || cfg.data_area < 1) {
			fprintf(stderr, "ERROR: WDC: Data area must be 1-5\n");
			ret = -1;
			goto out;
		}
	}

	if (!cfg.type || !strcmp(cfg.type, "NONE") || !strcmp(cfg.type, "none")) {
		telemetry_type = WDC_TELEMETRY_TYPE_NONE;
		data_area = 0;
	// } else if (!strcmp(cfg.type, "HOST") || !strcmp(cfg.type, "host")) {
	// 	telemetry_type = WDC_TELEMETRY_TYPE_HOST;
	// 	telemetry_data_area = cfg.data_area;
	// } else if (!strcmp(cfg.type, "CONTROLLER") || !strcmp(cfg.type, "controller")) {
	// 	telemetry_type = WDC_TELEMETRY_TYPE_CONTROLLER;
	// 	telemetry_data_area = cfg.data_area;
	} else {
		fprintf(stderr, "ERROR: WDC: Invalid type - Must be NONE, telemetry hasn't been backported. \n");
		ret = -1;
		goto out;
	}

	capabilities = wdc_get_drive_capabilities(fd);
	if ((capabilities & WDC_DRIVE_CAP_INTERNAL_LOG) == WDC_DRIVE_CAP_INTERNAL_LOG) {
		if (!telemetry_data_area)
			telemetry_data_area = 3;       /* Set the default DA to 3 if not specified */

		ret = wdc_do_cap_diag(fd, f, xfer_size,
				  telemetry_type, telemetry_data_area);
		goto out;
	}
	if ((capabilities & WDC_DRIVE_CAP_DUI) == WDC_DRIVE_CAP_DUI) {
		// if ((telemetry_type == WDC_TELEMETRY_TYPE_HOST) ||
		// 	(telemetry_type == WDC_TELEMETRY_TYPE_CONTROLLER)) {
		// 	if (!telemetry_data_area)
		// 		telemetry_data_area = 3;       /* Set the default DA to 3 if not specified */
		// 	/* Get the desired telemetry log page */
		// 	ret = wdc_do_cap_telemetry_log(dev, f, xfer_size,
		// 			telemetry_type, telemetry_data_area);
		// 	goto out;
		// } else {
			if (!cfg.data_area)
				cfg.data_area = 1;

			/* FW requirement - xfer size must be 256k for data area 4 */
			if (cfg.data_area >= 4)
				xfer_size = 0x40000;
			ret = wdc_do_cap_dui(fd, f, xfer_size,
					 cfg.data_area,
					 cfg.verbose, cfg.file_size,
					 cfg.offset);
			goto out;
		// }
	}
	if ((capabilities & WDC_DRIVE_CAP_DUI_DATA) == WDC_DRIVE_CAP_DUI_DATA) {
		// if ((telemetry_type == WDC_TELEMETRY_TYPE_HOST) ||
		// 	(telemetry_type == WDC_TELEMETRY_TYPE_CONTROLLER)) {
		// 	if (!telemetry_data_area)
		// 		telemetry_data_area = 3;       /* Set the default DA to 3 if not specified */
		// 	/* Get the desired telemetry log page */
		// 	ret = wdc_do_cap_telemetry_log(dev, f, xfer_size,
		// 			telemetry_type, telemetry_data_area);
		// 	goto out;
		// } else {
			ret = wdc_do_cap_dui(fd, f, xfer_size,
					     WDC_NVME_DUI_MAX_DATA_AREA,
					     cfg.verbose, 0, 0);
			goto out;
		// }
	}
	// if ((capabilities & WDC_SN730B_CAP_VUC_LOG) == WDC_SN730B_CAP_VUC_LOG) {
	// 	ret = wdc_do_sn730_get_and_tar(dev_fd(dev), f);
	// } else {
	// 	fprintf(stderr, "ERROR: WDC: unsupported device for this command\n");
	// 	ret = -1;
	// }
out:
	close(fd);
	return ret;
}

static int wdc_do_crash_dump(int fd, char *file, int type)
{
	int ret;
	__u32 crash_dump_length;
	__u32 opcode;
	__u32 cdw12;
	__u32 cdw10_size;
	__u32 cdw12_size;
	__u32 cdw12_clear;

	if (type == WDC_NVME_PFAIL_DUMP_TYPE) {
		/* set parms to get the PFAIL Crash Dump */
		opcode = WDC_NVME_PF_CRASH_DUMP_OPCODE;
		cdw10_size = WDC_NVME_PF_CRASH_DUMP_SIZE_NDT;
		cdw12_size = ((WDC_NVME_PF_CRASH_DUMP_SIZE_SUBCMD << WDC_NVME_SUBCMD_SHIFT) |
			WDC_NVME_PF_CRASH_DUMP_SIZE_CMD);

		cdw12 = (WDC_NVME_PF_CRASH_DUMP_SUBCMD << WDC_NVME_SUBCMD_SHIFT) |
			WDC_NVME_PF_CRASH_DUMP_CMD;

		cdw12_clear = ((WDC_NVME_CLEAR_PF_CRASH_DUMP_SUBCMD << WDC_NVME_SUBCMD_SHIFT) |
			WDC_NVME_CLEAR_CRASH_DUMP_CMD);

	} else {
		/* set parms to get the Crash Dump */
		opcode = WDC_NVME_CRASH_DUMP_OPCODE;
		cdw10_size = WDC_NVME_CRASH_DUMP_SIZE_NDT;
		cdw12_size = ((WDC_NVME_CRASH_DUMP_SIZE_SUBCMD << WDC_NVME_SUBCMD_SHIFT) |
			WDC_NVME_CRASH_DUMP_SIZE_CMD);

		cdw12 = (WDC_NVME_CRASH_DUMP_SUBCMD << WDC_NVME_SUBCMD_SHIFT) |
			WDC_NVME_CRASH_DUMP_CMD;

		cdw12_clear = ((WDC_NVME_CLEAR_CRASH_DUMP_SUBCMD << WDC_NVME_SUBCMD_SHIFT) |
			WDC_NVME_CLEAR_CRASH_DUMP_CMD);
	}

	ret = wdc_dump_length(fd,
		opcode,
		cdw10_size,
		cdw12_size,
		&crash_dump_length);

	if (ret == -1) {
		if (type == WDC_NVME_PFAIL_DUMP_TYPE)
		    fprintf(stderr, "INFO : WDC: Pfail dump get size failed\n");
		else
		    fprintf(stderr, "INFO : WDC: Crash dump get size failed\n");

		return -1;
	}

	if (crash_dump_length == 0) {
		if (type == WDC_NVME_PFAIL_DUMP_TYPE)
		    fprintf(stderr, "INFO : WDC: Pfail dump is empty\n");
		else
		    fprintf(stderr, "INFO : WDC: Crash dump is empty\n");
	} else {
		ret = wdc_do_dump(fd,
			opcode,
			crash_dump_length,
			cdw12,
			file,
			crash_dump_length);

		if (ret == 0)
			ret = wdc_do_clear_dump(fd, WDC_NVME_CLEAR_DUMP_OPCODE, cdw12_clear);
	}
	return ret;
}

static int wdc_crash_dump(int fd, char *file, int type)
{
	char f[PATH_MAX] = {0};
	const char *dump_type;

	if (file != NULL) {
		strncpy(f, file, PATH_MAX - 1);
	}

	if (type == WDC_NVME_PFAIL_DUMP_TYPE)
		dump_type = "_pfail_dump";
	else
		dump_type = "_crash_dump";

	if (wdc_get_serial_name(fd, f, PATH_MAX, dump_type) == -1) {
		fprintf(stderr, "ERROR : WDC : failed to generate file name\n");
		return -1;
	}
	return wdc_do_crash_dump(fd, f, type);
}

static int wdc_do_drive_log(int fd, char *file)
{
	int ret;
	__u8 *drive_log_data;
	__u32 drive_log_length;
	struct nvme_admin_cmd admin_cmd;

	ret = wdc_dump_length(fd, WDC_NVME_DRIVE_LOG_SIZE_OPCODE,
			WDC_NVME_DRIVE_LOG_SIZE_NDT,
			(WDC_NVME_DRIVE_LOG_SIZE_SUBCMD <<
			WDC_NVME_SUBCMD_SHIFT | WDC_NVME_DRIVE_LOG_SIZE_CMD),
			&drive_log_length);
	if (ret == -1) {
		return -1;
	}

	drive_log_data = (__u8 *) malloc(sizeof (__u8) * drive_log_length);
	if (drive_log_data == NULL) {
		fprintf(stderr, "ERROR : WDC : malloc : %s\n", strerror(errno));
		return -1;
	}

	memset(drive_log_data, 0, sizeof (__u8) * drive_log_length);
	memset(&admin_cmd, 0, sizeof (struct nvme_admin_cmd));
	admin_cmd.opcode = WDC_NVME_DRIVE_LOG_OPCODE;
	admin_cmd.addr = (__u64)(uintptr_t)drive_log_data;
	admin_cmd.data_len = drive_log_length;
	admin_cmd.cdw10 = drive_log_length;
	admin_cmd.cdw12 = ((WDC_NVME_DRIVE_LOG_SUBCMD <<
				WDC_NVME_SUBCMD_SHIFT) | WDC_NVME_DRIVE_LOG_SIZE_CMD);

	ret = nvme_submit_passthru(fd, NVME_IOCTL_ADMIN_CMD, &admin_cmd);
	fprintf(stderr, "NVMe Status:%s(%x)\n", nvme_status_to_string(ret),
			ret);
	if (ret == 0) {
		ret = wdc_create_log_file(file, drive_log_data, drive_log_length);
	}
	free(drive_log_data);
	return ret;
}

static int wdc_drive_log(int argc, char **argv, struct command *command,
		struct plugin *plugin)
{
	const char *desc = "Capture Drive Log.";
	const char *file = "Output file pathname.";
	char f[PATH_MAX] = {0};
	int fd;
	struct config {
		char *file;
	};

	struct config cfg = {
		.file = NULL
	};

	const struct argconfig_commandline_options command_line_options[] = {
		{"output-file", 'o', "FILE", CFG_STRING, &cfg.file, required_argument, file},
		{ NULL, '\0', NULL, CFG_NONE, NULL, no_argument, desc},
		{NULL}
	};

	fd = parse_and_open(argc, argv, desc, command_line_options, NULL, 0);
	if (fd < 0)
		return fd;

	wdc_check_device(fd);
	if (cfg.file != NULL) {
		strncpy(f, cfg.file, PATH_MAX - 1);
	}
	if (wdc_get_serial_name(fd, f, PATH_MAX, "drive_log") == -1) {
		fprintf(stderr, "ERROR : WDC : failed to generate file name\n");
		return -1;
	}
	return wdc_do_drive_log(fd, f);
}

static int wdc_get_crash_dump(int argc, char **argv, struct command *command,
		struct plugin *plugin)
{
	const char *desc = "Get Crash Dump.";
	const char *file = "Output file pathname.";
	int fd;
	struct config {
		char *file;
	};

	struct config cfg = {
		.file = NULL,
	};

	const struct argconfig_commandline_options command_line_options[] = {
		{"output-file", 'o', "FILE", CFG_STRING, &cfg.file, required_argument, file},
		{ NULL, '\0', NULL, CFG_NONE, NULL, no_argument, desc},
		{NULL}
	};

	fd = parse_and_open(argc, argv, desc, command_line_options, NULL, 0);
	if (fd < 0)
		return fd;

	if (wdc_check_device_match(fd, WDC_NVME_VID, WDC_NVME_SN100_DEV_ID) ||
		wdc_check_device_match(fd, WDC_NVME_VID, WDC_NVME_SN200_DEV_ID) ) {
		return wdc_crash_dump(fd, cfg.file, WDC_NVME_CRASH_DUMP_TYPE);
	} else {
		fprintf(stderr, "ERROR : WDC: unsupported device for get-crash-dump command\n");
		return -1;
	}
}

static int wdc_get_pfail_dump(int argc, char **argv, struct command *command,
	struct plugin *plugin)
{
	char *desc = "Get Pfail Crash Dump.";
	char *file = "Output file pathname.";
	int fd;
	struct config {
		char *file;
	};

	struct config cfg = {
		.file = NULL,
	};

	const struct argconfig_commandline_options command_line_options[] = {
		{"output-file", 'o', "FILE", CFG_STRING, &cfg.file, required_argument, file},
		{ NULL, '\0', NULL, CFG_NONE, NULL, no_argument, desc},
		{NULL}
	};

	fd = parse_and_open(argc, argv, desc, command_line_options, NULL, 0);
	if (fd < 0)
		return fd;

	if (wdc_check_device_match(fd, WDC_NVME_VID, WDC_NVME_SN100_DEV_ID) ||
		wdc_check_device_match(fd, WDC_NVME_VID, WDC_NVME_SN200_DEV_ID) ) {
		return wdc_crash_dump(fd, cfg.file, WDC_NVME_PFAIL_DUMP_TYPE);
	} else {
		fprintf(stderr, "ERROR : WDC: unsupported device for get-pfail-dump command\n");
		return -1;
	}
}

static void wdc_do_id_ctrl(__u8 *vs, struct json_object *root)
{
	char vsn[24] = {0};
	int base = 3072;
	int vsn_start = 3081;

	memcpy(vsn, &vs[vsn_start - base], sizeof(vsn));
	if (root)
		json_object_add_value_string(root, "wdc vsn", strlen(vsn) > 1 ? vsn : "NULL");
	else
		printf("wdc vsn : %s\n", strlen(vsn) > 1 ? vsn : "NULL");
}

static int wdc_id_ctrl(int argc, char **argv, struct command *cmd, struct plugin *plugin)
{
	return __id_ctrl(argc, argv, cmd, plugin, wdc_do_id_ctrl);
}

static const char* wdc_purge_mon_status_to_string(__u32 status)
{
	const char *str;

	switch (status) {
	case WDC_NVME_PURGE_STATE_IDLE:
		str = "Purge State Idle.";
		break;
	case WDC_NVME_PURGE_STATE_DONE:
		str = "Purge State Done.";
		break;
	case WDC_NVME_PURGE_STATE_BUSY:
		str = "Purge State Busy.";
		break;
	case WDC_NVME_PURGE_STATE_REQ_PWR_CYC:
		str = "Purge Operation resulted in an error that requires "
			"power cycle.";
		break;
	case WDC_NVME_PURGE_STATE_PWR_CYC_PURGE:
		str = "The previous purge operation was interrupted by a power "
			"cycle\nor reset interruption. Other commands may be "
			"rejected until\nPurge Execute is issued and "
			"completed.";
		break;
	default:
		str = "Unknown.";
	}
	return str;
}

static int wdc_purge(int argc, char **argv,
		struct command *command, struct plugin *plugin)
{
	const char *desc = "Send a Purge command.";
	char *err_str;
	int fd;
	int ret;
	struct nvme_passthru_cmd admin_cmd;
	const struct argconfig_commandline_options command_line_options[] = {
		{ NULL, '\0', NULL, CFG_NONE, NULL, no_argument, desc },
		{NULL}
	};

	err_str = "";
	memset(&admin_cmd, 0, sizeof (admin_cmd));
	admin_cmd.opcode = WDC_NVME_PURGE_CMD_OPCODE;

	fd = parse_and_open(argc, argv, desc, command_line_options, NULL, 0);
	if (fd < 0)
		return fd;

	wdc_check_device(fd);
	ret = nvme_submit_passthru(fd, NVME_IOCTL_ADMIN_CMD, &admin_cmd);
	if (ret > 0) {
		switch (ret) {
		case WDC_NVME_PURGE_CMD_SEQ_ERR:
			err_str = "ERROR : WDC : Cannot execute purge, "
					"Purge operation is in progress.\n";
			break;
		case WDC_NVME_PURGE_INT_DEV_ERR:
			err_str = "ERROR : WDC : Internal Device Error.\n";
			break;
		default:
			err_str = "ERROR : WDC\n";
		}
	}
	fprintf(stderr, "%s", err_str);
	fprintf(stderr, "NVMe Status:%s(%x)\n", nvme_status_to_string(ret), ret);
	return ret;
}

static int wdc_purge_monitor(int argc, char **argv,
		struct command *command, struct plugin *plugin)
{
	const char *desc = "Send a Purge Monitor command.";
	int fd;
	int ret;
	__u8 output[WDC_NVME_PURGE_MONITOR_DATA_LEN];
	double progress_percent;
	struct nvme_passthru_cmd admin_cmd;
	struct wdc_nvme_purge_monitor_data *mon;
	const struct argconfig_commandline_options command_line_options[] = {
		{ NULL, '\0', NULL, CFG_NONE, NULL, no_argument, desc },
		{NULL}
	};

	memset(output, 0, sizeof (output));
	memset(&admin_cmd, 0, sizeof (struct nvme_admin_cmd));
	admin_cmd.opcode = WDC_NVME_PURGE_MONITOR_OPCODE;
	admin_cmd.addr = (__u64)(uintptr_t)output;
	admin_cmd.data_len = WDC_NVME_PURGE_MONITOR_DATA_LEN;
	admin_cmd.cdw10 = WDC_NVME_PURGE_MONITOR_CMD_CDW10;
	admin_cmd.timeout_ms = WDC_NVME_PURGE_MONITOR_TIMEOUT;

	fd = parse_and_open(argc, argv, desc, command_line_options, NULL, 0);
	if (fd < 0)
		return fd;

	wdc_check_device(fd);
	ret = nvme_submit_passthru(fd, NVME_IOCTL_ADMIN_CMD, &admin_cmd);
	if (ret == 0) {
		mon = (struct wdc_nvme_purge_monitor_data *) output;
		printf("Purge state = 0x%0x\n", admin_cmd.result);
		printf("%s\n", wdc_purge_mon_status_to_string(admin_cmd.result));
		if (admin_cmd.result == WDC_NVME_PURGE_STATE_BUSY) {
			progress_percent =
				((double)le32_to_cpu(mon->entire_progress_current) * 100) /
				le32_to_cpu(mon->entire_progress_total);
			printf("Purge Progress = %f%%\n", progress_percent);
		}
	}
	fprintf(stderr, "NVMe Status:%s(%x)\n", nvme_status_to_string(ret), ret);
	return ret;
}

static void wdc_print_log_normal(struct wdc_ssd_perf_stats *perf)
{
	printf("  C1 Log Page Performance Statistics :- \n");
	printf("  Host Read Commands                             %20"PRIu64"\n",
			(uint64_t)le64_to_cpu(perf->hr_cmds));
	printf("  Host Read Blocks                               %20"PRIu64"\n",
			(uint64_t)le64_to_cpu(perf->hr_blks));
	printf("  Average Read Size                              %20lf\n",
			safe_div_fp((le64_to_cpu(perf->hr_blks)), (le64_to_cpu(perf->hr_cmds))));
	printf("  Host Read Cache Hit Commands                   %20"PRIu64"\n",
			(uint64_t)le64_to_cpu(perf->hr_ch_cmds));
	printf("  Host Read Cache Hit_Percentage                 %20"PRIu64"%%\n",
			(uint64_t) calc_percent(le64_to_cpu(perf->hr_ch_cmds), le64_to_cpu(perf->hr_cmds)));
	printf("  Host Read Cache Hit Blocks                     %20"PRIu64"\n",
			(uint64_t)le64_to_cpu(perf->hr_ch_blks));
	printf("  Average Read Cache Hit Size                    %20f\n",
			safe_div_fp((le64_to_cpu(perf->hr_ch_blks)), (le64_to_cpu(perf->hr_ch_cmds))));
	printf("  Host Read Commands Stalled                     %20"PRIu64"\n",
			(uint64_t)le64_to_cpu(perf->hr_st_cmds));
	printf("  Host Read Commands Stalled Percentage          %20"PRIu64"%%\n",
			(uint64_t)calc_percent((le64_to_cpu(perf->hr_st_cmds)), le64_to_cpu(perf->hr_cmds)));
	printf("  Host Write Commands                            %20"PRIu64"\n",
			(uint64_t)le64_to_cpu(perf->hw_cmds));
	printf("  Host Write Blocks                              %20"PRIu64"\n",
			(uint64_t)le64_to_cpu(perf->hw_blks));
	printf("  Average Write Size                             %20f\n",
			safe_div_fp((le64_to_cpu(perf->hw_blks)), (le64_to_cpu(perf->hw_cmds))));
	printf("  Host Write Odd Start Commands                  %20"PRIu64"\n",
			(uint64_t)le64_to_cpu(perf->hw_os_cmds));
	printf("  Host Write Odd Start Commands Percentage       %20"PRIu64"%%\n",
			(uint64_t)calc_percent((le64_to_cpu(perf->hw_os_cmds)), (le64_to_cpu(perf->hw_cmds))));
	printf("  Host Write Odd End Commands                    %20"PRIu64"\n",
			(uint64_t)le64_to_cpu(perf->hw_oe_cmds));
	printf("  Host Write Odd End Commands Percentage         %20"PRIu64"%%\n",
			(uint64_t)calc_percent((le64_to_cpu(perf->hw_oe_cmds)), (le64_to_cpu((perf->hw_cmds)))));
	printf("  Host Write Commands Stalled                    %20"PRIu64"\n",
		(uint64_t)le64_to_cpu(perf->hw_st_cmds));
	printf("  Host Write Commands Stalled Percentage         %20"PRIu64"%%\n",
		(uint64_t)calc_percent((le64_to_cpu(perf->hw_st_cmds)), (le64_to_cpu(perf->hw_cmds))));
	printf("  NAND Read Commands                             %20"PRIu64"\n",
		(uint64_t)le64_to_cpu(perf->nr_cmds));
	printf("  NAND Read Blocks Commands                      %20"PRIu64"\n",
		(uint64_t)le64_to_cpu(perf->nr_blks));
	printf("  Average NAND Read Size                         %20f\n",
		safe_div_fp((le64_to_cpu(perf->nr_blks)), (le64_to_cpu((perf->nr_cmds)))));
	printf("  Nand Write Commands                            %20"PRIu64"\n",
			(uint64_t)le64_to_cpu(perf->nw_cmds));
	printf("  NAND Write Blocks                              %20"PRIu64"\n",
			(uint64_t)le64_to_cpu(perf->nw_blks));
	printf("  Average NAND Write Size                        %20f\n",
			safe_div_fp((le64_to_cpu(perf->nw_blks)), (le64_to_cpu(perf->nw_cmds))));
	printf("  NAND Read Before Write                         %20"PRIu64"\n",
			(uint64_t)le64_to_cpu(perf->nrbw));
}

static void wdc_print_log_json(struct wdc_ssd_perf_stats *perf)
{
	struct json_object *root;

	root = json_create_object();
	json_object_add_value_int(root, "Host Read Commands", le64_to_cpu(perf->hr_cmds));
	json_object_add_value_int(root, "Host Read Blocks", le64_to_cpu(perf->hr_blks));
	json_object_add_value_int(root, "Average Read Size",
			safe_div_fp((le64_to_cpu(perf->hr_blks)), (le64_to_cpu(perf->hr_cmds))));
	json_object_add_value_int(root, "Host Read Cache Hit Commands",
			(uint64_t)le64_to_cpu(perf->hr_ch_cmds));
	json_object_add_value_int(root, "Host Read Cache Hit Percentage",
			(uint64_t) calc_percent(le64_to_cpu(perf->hr_ch_cmds), le64_to_cpu(perf->hr_cmds)));
	json_object_add_value_int(root, "Host Read Cache Hit Blocks",
			(uint64_t)le64_to_cpu(perf->hr_ch_blks));
	json_object_add_value_int(root, "Average Read Cache Hit Size",
			safe_div_fp((le64_to_cpu(perf->hr_ch_blks)), (le64_to_cpu(perf->hr_ch_cmds))));
	json_object_add_value_int(root, "Host Read Commands Stalled",
			(uint64_t)le64_to_cpu(perf->hr_st_cmds));
	json_object_add_value_int(root, "Host Read Commands Stalled Percentage",
			(uint64_t)calc_percent((le64_to_cpu(perf->hr_st_cmds)), le64_to_cpu(perf->hr_cmds)));
	json_object_add_value_int(root, "Host Write Commands",
			(uint64_t)le64_to_cpu(perf->hw_cmds));
	json_object_add_value_int(root, "Host Write Blocks",
			(uint64_t)le64_to_cpu(perf->hw_blks));
	json_object_add_value_int(root, "Average Write Size",
			safe_div_fp((le64_to_cpu(perf->hw_blks)), (le64_to_cpu(perf->hw_cmds))));
	json_object_add_value_int(root, "Host Write Odd Start Commands",
			(uint64_t)le64_to_cpu(perf->hw_os_cmds));
	json_object_add_value_int(root, "Host Write Odd Start Commands Percentage",
			(uint64_t)calc_percent((le64_to_cpu(perf->hw_os_cmds)), (le64_to_cpu(perf->hw_cmds))));
	json_object_add_value_int(root, "Host Write Odd End Commands",
			(uint64_t)le64_to_cpu(perf->hw_oe_cmds));
	json_object_add_value_int(root, "Host Write Odd End Commands Percentage",
			(uint64_t)calc_percent((le64_to_cpu(perf->hw_oe_cmds)), (le64_to_cpu((perf->hw_cmds)))));
	json_object_add_value_int(root, "Host Write Commands Stalled",
		(uint64_t)le64_to_cpu(perf->hw_st_cmds));
	json_object_add_value_int(root, "Host Write Commands Stalled Percentage",
		(uint64_t)calc_percent((le64_to_cpu(perf->hw_st_cmds)), (le64_to_cpu(perf->hw_cmds))));
	json_object_add_value_int(root, "NAND Read Commands",
		(uint64_t)le64_to_cpu(perf->nr_cmds));
	json_object_add_value_int(root, "NAND Read Blocks Commands",
		(uint64_t)le64_to_cpu(perf->nr_blks));
	json_object_add_value_int(root, "Average NAND Read Size",
		safe_div_fp((le64_to_cpu(perf->nr_blks)), (le64_to_cpu((perf->nr_cmds)))));
	json_object_add_value_int(root, "Nand Write Commands",
			(uint64_t)le64_to_cpu(perf->nw_cmds));
	json_object_add_value_int(root, "NAND Write Blocks",
			(uint64_t)le64_to_cpu(perf->nw_blks));
	json_object_add_value_int(root, "Average NAND Write Size",
			safe_div_fp((le64_to_cpu(perf->nw_blks)), (le64_to_cpu(perf->nw_cmds))));
	json_object_add_value_int(root, "NAND Read Before Written",
			(uint64_t)le64_to_cpu(perf->nrbw));
	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

static int wdc_print_log(struct wdc_ssd_perf_stats *perf, int fmt)
{
	if (!perf) {
		fprintf(stderr, "ERROR : WDC : Invalid buffer to read perf stats\n");
		return -1;
	}
	switch (fmt) {
	case NORMAL:
		wdc_print_log_normal(perf);
		break;
	case JSON:
		wdc_print_log_json(perf);
		break;
	}
	return 0;
}

static int wdc_get_ca_log_page(int fd, char *format);

static int wdc_get_c1_log_page(int fd, char *format, uint8_t interval)
{
	int ret = 0;
	int fmt = -1;
	__u8 *data;
	__u8 *p;
	int i;
	int skip_cnt = 4;
	int total_subpages;
	struct wdc_log_page_header *l;
	struct wdc_log_page_subpage_header *sph;
	struct wdc_ssd_perf_stats *perf;

	
	if (!wdc_check_device(fd))
		return -1;
	fmt = validate_output_format(format);
	if (fmt < 0) {
		fprintf(stderr, "ERROR : WDC : invalid output format\n");
		return fmt;
	}

	if (interval < 1 || interval > 15) {
		fprintf(stderr, "ERROR : WDC : interval out of range [1-15]\n");
		return -1;
	}

	data = (__u8 *)malloc(sizeof(__u8) * WDC_ADD_LOG_BUF_LEN);
	if (!data) {
		fprintf(stderr, "ERROR : WDC : malloc : %s\n", strerror(errno));
		return -1;
	}
	memset(data, 0, sizeof (__u8) * WDC_ADD_LOG_BUF_LEN);

	ret = nvme_get_log(fd, NVME_NSID_ALL, WDC_NVME_ADD_LOG_OPCODE, 0,
			   false, WDC_ADD_LOG_BUF_LEN, data);
	if (strcmp(format, "json"))
		fprintf(stderr, "NVMe Status:%s(%x)\n", nvme_status_to_string(ret), ret);
	if (!ret) {
		l = (struct wdc_log_page_header*)data;
		total_subpages = l->num_subpages + WDC_NVME_GET_STAT_PERF_INTERVAL_LIFETIME - 1;
		for (i = 0, p = data + skip_cnt; i < total_subpages; i++, p += skip_cnt) {
			sph = (struct wdc_log_page_subpage_header *) p;
			if (sph->spcode == WDC_GET_LOG_PAGE_SSD_PERFORMANCE) {
				if (sph->pcset == interval) {
					perf = (struct wdc_ssd_perf_stats *) (p + 4);
					ret = wdc_print_log(perf, fmt);
					break;
				}
			}
			skip_cnt = le16_to_cpu(sph->subpage_length) + 4;
		}
		if (ret) {
			fprintf(stderr, "ERROR : WDC : Unable to read data from buffer\n");
		}
	}
	free(data);
	return ret;
}

static int wdc_smart_add_log(int argc, char **argv, struct command *command,
		struct plugin *plugin)
{
	const char *desc = "Retrieve additional performance statistics.";
	const char *interval = "Interval to read the statistics from [1, 15].";
	int fd;
	int ret;

	struct config {
		uint8_t interval;
		int   vendor_specific;
		char *output_format;
	};

	struct config cfg = {
		.interval = 14,
		.output_format = "normal",
	};

	const struct argconfig_commandline_options command_line_options[] = {
		{"interval", 'i', "NUM", CFG_POSITIVE, &cfg.interval, required_argument, interval},
		{"output-format", 'o', "FMT", CFG_STRING, &cfg.output_format, required_argument, "Output Format: normal|json" },
		{NULL}
	};

	fd = parse_and_open(argc, argv, desc, command_line_options, NULL, 0);
	if (fd < 0)
		return fd;


	if (wdc_check_device_match(fd, WDC_NVME_VID, WDC_NVME_SN100_DEV_ID)) {
		// Get the C1 Log Page
		ret = wdc_get_c1_log_page(fd, cfg.output_format, cfg.interval);

		if (ret) {
			fprintf(stderr, "ERROR : WDC : Unable to read C1 Log Page data from buffer\n");
			return ret;
		}
	}
	else if (wdc_check_device_match(fd, WDC_NVME_VID, WDC_NVME_SN200_DEV_ID)) {
		// Get the CA and C1 Log Page
		ret = wdc_get_ca_log_page(fd, cfg.output_format);
		if (ret) {
			fprintf(stderr, "ERROR : WDC : Unable to read CA Log Page data from buffer\n");
			return ret;
		}

		ret = wdc_get_c1_log_page(fd, cfg.output_format, cfg.interval);
		if (ret) {
			fprintf(stderr, "ERROR : WDC : Unable to read C1 Log Page data from buffer\n");
			return ret;
		}
	}
	else {
		fprintf(stderr, "INFO : WDC : Command not supported in this device\n");
	}

	return 0;
}

static int wdc_clear_pcie_corr(int argc, char **argv, struct command *command,
		struct plugin *plugin)
{
	char *desc = "Clear PCIE Correctable Errors.";
	int fd;
	int ret;
	struct nvme_passthru_cmd admin_cmd;
	const struct argconfig_commandline_options command_line_options[] = {
		{ NULL, '\0', NULL, CFG_NONE, NULL, no_argument, desc },
		{NULL}
	};

	fd = parse_and_open(argc, argv, desc, command_line_options, NULL, 0);
	if (fd < 0)
		return fd;

	wdc_check_device(fd);

	memset(&admin_cmd, 0, sizeof (admin_cmd));
	admin_cmd.opcode = WDC_NVME_CLEAR_PCIE_CORR_OPCODE;
	admin_cmd.cdw12 = ((WDC_NVME_CLEAR_PCIE_CORR_SUBCMD << WDC_NVME_SUBCMD_SHIFT) |
			WDC_NVME_CLEAR_PCIE_CORR_CMD);

	ret = nvme_submit_passthru(fd, NVME_IOCTL_ADMIN_CMD, &admin_cmd);
	fprintf(stderr, "NVMe Status:%s(%x)\n", nvme_status_to_string(ret), ret);
	return ret;
}

static int wdc_get_serial_and_fw_rev(int fd, char *sn, char *fw_rev)
{
	int i;
	int ret;
	struct nvme_id_ctrl ctrl;

	i = sizeof (ctrl.sn) - 1;
	memset(sn, 0, WDC_SERIAL_NO_LEN);
	memset(fw_rev, 0, WDC_NVME_FIRMWARE_REV_LEN);
	memset(&ctrl, 0, sizeof (struct nvme_id_ctrl));
	ret = nvme_identify_ctrl(fd, &ctrl);
	if (ret) {
		fprintf(stderr, "ERROR : WDC : nvme_identify_ctrl() failed "
				"0x%x\n", ret);
		return -1;
	}
	/* Remove trailing spaces from the name */
	while (i && ctrl.sn[i] == ' ') {
		ctrl.sn[i] = '\0';
		i--;
	}
	snprintf(sn, WDC_SERIAL_NO_LEN, "%s", ctrl.sn);
	snprintf(fw_rev, WDC_NVME_FIRMWARE_REV_LEN, "%s", ctrl.fr);

	return 0;
}

static int wdc_get_max_transfer_len(int fd, __u32 *maxTransferLen)
{
	int ret = 0;
	struct nvme_id_ctrl ctrl;

	__u32 maxTransferLenDevice = 0;

	memset(&ctrl, 0, sizeof (struct nvme_id_ctrl));
	ret = nvme_identify_ctrl(fd, &ctrl);
	if (ret) {
		fprintf(stderr, "ERROR : WDC : nvme_identify_ctrl() failed 0x%x\n", ret);
		return -1;
	}

	maxTransferLenDevice = (1 << ctrl.mdts) * getpagesize();
	*maxTransferLen = maxTransferLenDevice;

	return ret;
}

static int wdc_de_VU_read_size(int fd, __u32 fileId, __u16 spiDestn, __u32* logSize)
{
	int ret = WDC_STATUS_FAILURE;
	struct nvme_admin_cmd cmd;

	if(!fd || !logSize )
	{
		ret = WDC_STATUS_INVALID_PARAMETER;
		goto end;
	}

	memset(&cmd,0,sizeof(struct nvme_admin_cmd));
	cmd.opcode = WDC_DE_VU_READ_SIZE_OPCODE;
	cmd.nsid = WDC_DE_DEFAULT_NAMESPACE_ID;
	cmd.cdw13 = fileId<<16;
	cmd.cdw14 = spiDestn;

	ret = nvme_submit_passthru(fd, NVME_IOCTL_ADMIN_CMD, &cmd);

	if (!ret && logSize)
		*logSize = cmd.result;
	if( ret != WDC_STATUS_SUCCESS)
		fprintf(stderr, "ERROR : WDC : VUReadSize() failed, status:%s(0x%x)\n", nvme_status_to_string(ret), ret);

	end:
	return ret;
}

static int wdc_de_VU_read_buffer(int fd, __u32 fileId, __u16 spiDestn, __u32 offsetInDwords, __u8* dataBuffer, __u32* bufferSize)
{
	int ret = WDC_STATUS_FAILURE;
	struct nvme_admin_cmd cmd;
	__u32 noOfDwordExpected = 0;

	if(!fd || !dataBuffer || !bufferSize)
	{
		ret = WDC_STATUS_INVALID_PARAMETER;
		goto end;
	}

	memset(&cmd,0,sizeof(struct nvme_admin_cmd));
	noOfDwordExpected = *bufferSize/sizeof(__u32);
	cmd.opcode = WDC_DE_VU_READ_BUFFER_OPCODE;
	cmd.nsid = WDC_DE_DEFAULT_NAMESPACE_ID;
	cmd.cdw10 = noOfDwordExpected;
	cmd.cdw13 = fileId<<16;
	cmd.cdw14 = spiDestn;
	cmd.cdw15 = offsetInDwords;

	cmd.addr = (__u64)(__u64)(uintptr_t)dataBuffer;
	cmd.data_len = *bufferSize;

	ret = nvme_submit_passthru(fd, NVME_IOCTL_ADMIN_CMD, &cmd);

	if( ret != WDC_STATUS_SUCCESS)
		fprintf(stderr, "ERROR : WDC : VUReadBuffer() failed, status:%s(0x%x)\n", nvme_status_to_string(ret), ret);

	end:
	return ret;
}

static int wdc_get_log_dir_max_entries(int fd, __u32* maxNumOfEntries)
{
	int     		ret = WDC_STATUS_FAILURE;
	__u32           headerPayloadSize = 0;
	__u8*           fileIdOffsetsBuffer = NULL;
	__u32           fileIdOffsetsBufferSize = 0;
	__u32           fileNum = 0;
	__u16           fileOffset = 0;


	if (!fd || !maxNumOfEntries)
	{
		ret = WDC_STATUS_INVALID_PARAMETER;
		return ret;
	}
	/* 1.Get log directory first four bytes */
	if (WDC_STATUS_SUCCESS != (ret = wdc_de_VU_read_size(fd, 0, 5, (__u32*)&headerPayloadSize)))
	{
		fprintf(stderr, "ERROR : WDC : %s: Failed to get headerPayloadSize from file directory 0x%x\n",
				__func__, ret);
		goto end;
	}

	fileIdOffsetsBufferSize = WDC_DE_FILE_HEADER_SIZE + (headerPayloadSize * WDC_DE_FILE_OFFSET_SIZE);
	fileIdOffsetsBuffer = (__u8*)calloc(1, fileIdOffsetsBufferSize);

	/* 2.Read to get file offsets */
	if (WDC_STATUS_SUCCESS != (ret = wdc_de_VU_read_buffer(fd, 0, 5, 0, fileIdOffsetsBuffer, &fileIdOffsetsBufferSize)))
	{
		fprintf(stderr, "ERROR : WDC : %s: Failed to get fileIdOffsets from file directory 0x%x\n",
				__func__, ret);
		goto end;
	}
	/* 3.Determine valid entries */
	for (fileNum = 0; fileNum < (headerPayloadSize - WDC_DE_FILE_HEADER_SIZE) / WDC_DE_FILE_OFFSET_SIZE; fileNum++)
	{
		fileOffset = (fileIdOffsetsBuffer[WDC_DE_FILE_HEADER_SIZE + (fileNum * WDC_DE_FILE_OFFSET_SIZE)] << 8) +
				fileIdOffsetsBuffer[WDC_DE_FILE_HEADER_SIZE + (fileNum * WDC_DE_FILE_OFFSET_SIZE) + 1];
		if (!fileOffset)
			continue;
		(*maxNumOfEntries)++;
	}
	end:
	if (!fileIdOffsetsBuffer)
		free(fileIdOffsetsBuffer);
	return ret;
}

static WDC_DRIVE_ESSENTIAL_TYPE wdc_get_essential_type(__u8 fileName[])
{
	WDC_DRIVE_ESSENTIAL_TYPE essentialType = WDC_DE_TYPE_NONE;

	if (wdc_UtilsStrCompare((char*)fileName, WDC_DE_CORE_DUMP_FILE_NAME) == 0)
	{
		essentialType = WDC_DE_TYPE_DUMPSNAPSHOT;
	}
	else if (wdc_UtilsStrCompare((char*)fileName, WDC_DE_EVENT_LOG_FILE_NAME) == 0)
	{
		essentialType = WDC_DE_TYPE_EVENTLOG;
	}
	else if (wdc_UtilsStrCompare((char*)fileName, WDC_DE_MANUFACTURING_INFO_PAGE_FILE_NAME) == 0)
	{
		essentialType = WDC_DE_TYPE_NVME_MANF_INFO;
	}

	return essentialType;
}

static int wdc_fetch_log_directory(int fd, PWDC_DE_VU_LOG_DIRECTORY directory)
{
	int             ret = WDC_STATUS_FAILURE;
	__u8            *fileOffset = NULL;
	__u8            *fileDirectory = NULL;
	__u32           headerSize = 0;
	__u32           fileNum = 0, startIdx = 0;
	__u16           fileOffsetTemp = 0;
	__u32           entryId = 0;
	__u32           fileDirectorySize = 0;

	if (!fd || !directory)
	{
		ret = WDC_STATUS_INVALID_PARAMETER;
		goto end;
	}

	if (WDC_STATUS_SUCCESS != (ret = wdc_de_VU_read_size(fd, 0, 5, &fileDirectorySize)))
	{
		fprintf(stderr, "ERROR : WDC : %s: Failed to get filesystem directory size, ret = %d\n",
				__func__, ret);
		goto end;
	}

	fileDirectory = (__u8*)calloc(1, fileDirectorySize);
	if (WDC_STATUS_SUCCESS != (ret = wdc_de_VU_read_buffer(fd, 0, 5, 0, fileDirectory, &fileDirectorySize)))
	{
		fprintf(stderr, "ERROR : WDC : %s: Failed to get filesystem directory, ret = %d\n",
				__func__, ret);
		goto end;
	}

	/* First four bytes of header directory is headerSize */
	memcpy(&headerSize, fileDirectory, WDC_DE_FILE_HEADER_SIZE);

	if (directory->maxNumLogEntries == 0) //minimum buffer for 1 entry is required
	{
		ret = WDC_STATUS_INVALID_PARAMETER;
		goto end;
	}

	for (fileNum = 0; fileNum < (headerSize - WDC_DE_FILE_HEADER_SIZE) / WDC_DE_FILE_OFFSET_SIZE; fileNum++)
	{
		if (entryId >= directory->maxNumLogEntries)
			break;
		startIdx = WDC_DE_FILE_HEADER_SIZE + (fileNum * WDC_DE_FILE_OFFSET_SIZE);
		memcpy(&fileOffsetTemp, fileDirectory + startIdx, sizeof(fileOffsetTemp));
		fileOffset = fileDirectory + fileOffsetTemp;

		if (0 == fileOffsetTemp)
		{
			continue;
		}

		memset(&directory->logEntry[entryId], 0, sizeof(WDC_DRIVE_ESSENTIALS));
		memcpy(&directory->logEntry[entryId].metaData, fileOffset, sizeof(WDC_DE_VU_FILE_META_DATA));
		directory->logEntry[entryId].metaData.fileName[WDC_DE_FILE_NAME_SIZE - 1] = '\0';
		wdc_UtilsDeleteCharFromString((char*)directory->logEntry[entryId].metaData.fileName, WDC_DE_FILE_NAME_SIZE, ' ');
		if (0 == directory->logEntry[entryId].metaData.fileID)
		{
			continue;
		}
		directory->logEntry[entryId].essentialType = wdc_get_essential_type(directory->logEntry[entryId].metaData.fileName);
		/*fprintf(stderr, "WDC : %s: NVMe VU Log Entry %d, fileName = %s, fileSize = 0x%lx, fileId = 0x%x\n",
			__func__, entryId, directory->logEntry[entryId].metaData.fileName,
			(long unsigned int)directory->logEntry[entryId].metaData.fileSize, directory->logEntry[entryId].metaData.fileID);
		 */
		entryId++;
	}
	directory->numOfValidLogEntries = entryId;
	end:
	if (fileDirectory != NULL)
		free(fileDirectory);

	return ret;
}

static int wdc_fetch_log_file_from_device(int fd, __u32 fileId, __u16 spiDestn, __u64 fileSize, __u8* dataBuffer)
{
	int ret = WDC_STATUS_FAILURE;
	__u32                     chunckSize = WDC_DE_VU_READ_BUFFER_STANDARD_OFFSET;
	__u32                     maximumTransferLength = 0;
	__u32                     buffSize = 0;
	__u64                     offsetIdx = 0;

	if (!fd || !dataBuffer || !fileSize)
	{
		ret = WDC_STATUS_INVALID_PARAMETER;
		goto end;
	}

	wdc_get_max_transfer_len(fd, &maximumTransferLength);

	/* Fetch Log File Data */
	if ((fileSize >= maximumTransferLength) || (fileSize > 0xffffffff))
	{
		chunckSize = WDC_DE_VU_READ_BUFFER_STANDARD_OFFSET;
		if (maximumTransferLength < WDC_DE_VU_READ_BUFFER_STANDARD_OFFSET)
			chunckSize = maximumTransferLength;

		buffSize = chunckSize;
		for (offsetIdx = 0; (offsetIdx * chunckSize) < fileSize; offsetIdx++)
		{
			if (((offsetIdx * chunckSize) + buffSize) > fileSize)
				buffSize = (__u32)(fileSize - (offsetIdx * chunckSize));
			/* Limitation in VU read buffer - offsetIdx and bufferSize are not greater than u32 */
			ret = wdc_de_VU_read_buffer(fd, fileId, spiDestn,
					(__u32)((offsetIdx * chunckSize) / sizeof(__u32)), dataBuffer + (offsetIdx * chunckSize), &buffSize);
			if (ret != WDC_STATUS_SUCCESS)
			{
				fprintf(stderr, "ERROR : WDC : %s: wdc_de_VU_read_buffer failed with ret = %d, fileId = 0x%x, fileSize = 0x%lx\n",
						__func__, ret, fileId, (long unsigned int)fileSize);
				break;
			}
		}
	} else {
		buffSize = (__u32)fileSize;
		ret = wdc_de_VU_read_buffer(fd, fileId, spiDestn,
				(__u32)((offsetIdx * chunckSize) / sizeof(__u32)), dataBuffer, &buffSize);
		if (ret != WDC_STATUS_SUCCESS)
		{
			fprintf(stderr, "ERROR : WDC : %s: wdc_de_VU_read_buffer failed with ret = %d, fileId = 0x%x, fileSize = 0x%lx\n",
					__func__, ret, fileId, (long unsigned int)fileSize);
		}
	}

	end:
	return ret;
}

static int wdc_de_get_dump_trace(int fd, char * filePath, __u16 binFileNameLen, char *binFileName)
{
	int                     ret = WDC_STATUS_FAILURE;
	__u8                    *readBuffer = NULL;
	__u32                   readBufferLen = 0;
	__u32                   lastPktReadBufferLen = 0;
	__u32                   maxTransferLen = 0;
	__u32                   dumptraceSize = 0;
	__u32                   chunkSize = 0;
	__u32                   chunks = 0;
	__u32                   offset = 0;
	__u8                    loop = 0;
	__u16					i = 0;
	__u32                   maximumTransferLength = 0;

	if (!fd || !binFileName || !filePath)
	{
		ret = WDC_STATUS_INVALID_PARAMETER;
		return ret;
	}

	wdc_get_max_transfer_len(fd, &maximumTransferLength);

	do
	{
		/* Get dumptrace size */
		ret = wdc_de_VU_read_size(fd, 0, WDC_DE_DUMPTRACE_DESTINATION, &dumptraceSize);
		if (ret != WDC_STATUS_SUCCESS)
		{
			fprintf(stderr, "ERROR : WDC : %s: wdc_de_VU_read_size failed with ret = %d\n",
					__func__, ret);
			break;
		}

		/* Make sure the size requested is greater than dword */
		if (dumptraceSize < 4)
		{
			ret = WDC_STATUS_FAILURE;
			fprintf(stderr, "ERROR : WDC : %s: wdc_de_VU_read_size failed, read size is less than 4 bytes, dumptraceSize = 0x%x\n",
					__func__, dumptraceSize);
			break;
		}

		/* Choose the least max transfer length */
		maxTransferLen = maximumTransferLength < WDC_DE_READ_MAX_TRANSFER_SIZE ? maximumTransferLength : WDC_DE_READ_MAX_TRANSFER_SIZE;

		/* Comment from  FW Team:
		 * The max non - block transfer size is 0xFFFF (16 bits allowed as the block size).Use 0x8000
		 * to keep it on a word - boundary.
		 * max_xfer = int(pow(2, id_data['MDTS'])) * 4096 # 4k page size as reported in pcie capabiltiies
		 */
		chunkSize = dumptraceSize < maxTransferLen ? dumptraceSize : maxTransferLen;
		chunks = (dumptraceSize / maxTransferLen) + ((dumptraceSize % maxTransferLen) ? 1 : 0);

		readBuffer = (unsigned char *)calloc(dumptraceSize, sizeof(unsigned char));
		readBufferLen = chunkSize;
		lastPktReadBufferLen = (dumptraceSize % maxTransferLen) ? (dumptraceSize % maxTransferLen) : chunkSize;

		if (readBuffer == NULL)
		{
			fprintf(stderr, "ERROR : WDC : %s: readBuffer calloc failed\n", __func__);
			ret = WDC_STATUS_INSUFFICIENT_MEMORY;
			break;
		}

		for (i = 0; i < chunks; i++)
		{
			offset = ((i*chunkSize) / 4);

			/* Last loop call, Assign readBufferLen to read only left over bytes */
			if (i == (chunks - 1))
			{
				readBufferLen = lastPktReadBufferLen;
			}

			ret = wdc_de_VU_read_buffer(fd, 0, WDC_DE_DUMPTRACE_DESTINATION, 0, readBuffer + offset, &readBufferLen);
			if (ret != WDC_STATUS_SUCCESS)
			{
				fprintf(stderr, "ERROR : WDC : %s: wdc_de_VU_read_buffer failed, ret = %d on offset 0x%x\n",
						__func__, ret, offset);
				break;
			}
		}
	} while (loop);

	if (ret == WDC_STATUS_SUCCESS)
	{
		ret = wdc_WriteToFile(binFileName, (char*)readBuffer, dumptraceSize);
		if (ret != WDC_STATUS_SUCCESS)
			fprintf(stderr, "ERROR : WDC : %s: wdc_WriteToFile failed, ret = %d\n", __func__, ret);
	} else {
		fprintf(stderr, "ERROR : WDC : %s: Read Buffer Loop failed, ret = %d\n", __func__, ret);
	}

	if (readBuffer)
	{
		free(readBuffer);
	}

	return ret;
}

static int wdc_do_drive_essentials(int fd, char *dir, char *key)
{
	int ret = 0;
	void *retPtr;
	char                      fileName[MAX_PATH_LEN];
	__s8                      bufferFolderPath[MAX_PATH_LEN];
	char                      bufferFolderName[MAX_PATH_LEN];
	char                      tarFileName[MAX_PATH_LEN];
	char                      tarFiles[MAX_PATH_LEN];
	char                      tarCmd[MAX_PATH_LEN+MAX_PATH_LEN];
	UtilsTimeInfo             timeInfo;
	__u8                      timeString[MAX_PATH_LEN];
	__u8                      serialNo[WDC_SERIAL_NO_LEN];
	__u8                      firmwareRevision[WDC_NVME_FIRMWARE_REV_LEN];
	__u8                      idSerialNo[WDC_SERIAL_NO_LEN];
	__u8                      idFwRev[WDC_NVME_FIRMWARE_REV_LEN];
	__u8                      featureIdBuff[4];
	char                      currDir[MAX_PATH_LEN];
	char                      *dataBuffer     = NULL;
	__u32 					  elogNumEntries, elogBufferSize;
	__u32 					  dataBufferSize;
	__u32                     listIdx = 0;
	__u32                     vuLogIdx = 0;
	__u32 					  result;
	__u32                     maxNumOfVUFiles = 0;
	struct nvme_id_ctrl ctrl;
	struct nvme_id_ns ns;
	struct nvme_error_log_page *elogBuffer;
	struct nvme_smart_log smart_log;
	struct nvme_firmware_log_page fw_log;
	PWDC_NVME_DE_VU_LOGPAGES vuLogInput = NULL;
	WDC_DE_VU_LOG_DIRECTORY deEssentialsList;

	memset(bufferFolderPath,0,sizeof(bufferFolderPath));
	memset(bufferFolderName,0,sizeof(bufferFolderName));
	memset(tarFileName,0,sizeof(tarFileName));
	memset(tarFiles,0,sizeof(tarFiles));
	memset(tarCmd,0,sizeof(tarCmd));
	memset(&timeInfo,0,sizeof(timeInfo));
	memset(&vuLogInput, 0, sizeof(vuLogInput));

	if (wdc_get_serial_and_fw_rev(fd, (char *)idSerialNo, (char *)idFwRev))
	{
		fprintf(stderr, "ERROR : WDC : get serial # and fw revision failed\n");
		return -1;
	} else {
		fprintf(stderr, "Get Drive Essentials Data for device serial #: %s and fw revision: %s\n",
				idSerialNo, idFwRev);
	}

	/* Create Drive Essentials directory  */
	wdc_UtilsGetTime(&timeInfo);
	memset(timeString, 0, sizeof(timeString));
	wdc_UtilsSnprintf((char*)timeString, MAX_PATH_LEN, "%02u%02u%02u_%02u%02u%02u",
			timeInfo.year, timeInfo.month, timeInfo.dayOfMonth,
			timeInfo.hour, timeInfo.minute, timeInfo.second);

	wdc_UtilsSnprintf((char*)serialNo,WDC_SERIAL_NO_LEN,(char*)idSerialNo);
	/* Remove any space form serialNo */
	wdc_UtilsDeleteCharFromString((char*)serialNo, WDC_SERIAL_NO_LEN, ' ');

	memset(firmwareRevision, 0, sizeof(firmwareRevision));
	wdc_UtilsSnprintf((char*)firmwareRevision, WDC_NVME_FIRMWARE_REV_LEN, (char*)idFwRev);
	/* Remove any space form FirmwareRevision */
	wdc_UtilsDeleteCharFromString((char*)firmwareRevision, WDC_NVME_FIRMWARE_REV_LEN, ' ');

	wdc_UtilsSnprintf((char*)bufferFolderName, MAX_PATH_LEN, "%s_%s_%s_%s",
			"DRIVE_ESSENTIALS", (char*)serialNo, (char*)firmwareRevision, (char*)timeString);

	if (dir != NULL) {
		wdc_UtilsSnprintf((char*)bufferFolderPath, MAX_PATH_LEN, "%s%s%s",
				(char *)dir, WDC_DE_PATH_SEPARATOR, (char *)bufferFolderName);
	} else {
		retPtr = getcwd((char*)currDir, MAX_PATH_LEN);
		if (retPtr != NULL)
			wdc_UtilsSnprintf((char*)bufferFolderPath, MAX_PATH_LEN, "%s%s%s",
					(char *)currDir, WDC_DE_PATH_SEPARATOR, (char *)bufferFolderName);
		else {
			fprintf(stderr, "ERROR : WDC : get current working directory failed\n");
			return -1;
		}
	}

	ret = wdc_UtilsCreateDir((char*)bufferFolderPath);
	if (ret != 0)
	{
		fprintf(stderr, "ERROR : WDC : create directory failed, ret = %d, dir = %s\n", ret, bufferFolderPath);
		return -1;
	} else {
		fprintf(stderr, "Store Drive Essentials bin files in directory: %s\n", bufferFolderPath);
	}

	/* Get Identify Controller Data */
	memset(&ctrl, 0, sizeof (struct nvme_id_ctrl));
	ret = nvme_identify_ctrl(fd, &ctrl);
	if (ret) {
		fprintf(stderr, "ERROR : WDC : nvme_identify_ctrl() failed, ret = %d\n", ret);
		return -1;
	} else {
		wdc_UtilsSnprintf(fileName, MAX_PATH_LEN, "%s%s%s_%s_%s.bin", (char*)bufferFolderPath, WDC_DE_PATH_SEPARATOR,
				"IdentifyController", (char*)serialNo, (char*)timeString);
		wdc_WriteToFile(fileName, (char*)&ctrl, sizeof (struct nvme_id_ctrl));
	}

	memset(&ns, 0, sizeof (struct nvme_id_ns));
	ret = nvme_identify_ns(fd, 1, 0, &ns);
	if (ret) {
		fprintf(stderr, "ERROR : WDC : nvme_identify_ns() failed, ret = %d\n", ret);
	} else {
		wdc_UtilsSnprintf(fileName, MAX_PATH_LEN, "%s%s%s_%s_%s.bin", (char*)bufferFolderPath, WDC_DE_PATH_SEPARATOR,
				"IdentifyNamespace", (char*)serialNo, (char*)timeString);
		wdc_WriteToFile(fileName, (char*)&ns, sizeof (struct nvme_id_ns));
	}

	/* Get Log Pages (0x01, 0x02, 0x03, 0xC0 and 0xE3) */
	elogNumEntries = WDC_DE_DEFAULT_NUMBER_OF_ERROR_ENTRIES;
	elogBufferSize = elogNumEntries*sizeof(struct nvme_error_log_page);
	dataBuffer = calloc(1, elogBufferSize);
	elogBuffer = (struct nvme_error_log_page *)dataBuffer;

	ret = nvme_error_log(fd, elogNumEntries, elogBuffer);
	if (ret) {
		fprintf(stderr, "ERROR : WDC : nvme_error_log() failed, ret = %d\n", ret);
	} else {
		wdc_UtilsSnprintf(fileName, MAX_PATH_LEN, "%s%s%s_%s_%s.bin", (char*)bufferFolderPath, WDC_DE_PATH_SEPARATOR,
				"ErrorLog", (char*)serialNo, (char*)timeString);
		wdc_WriteToFile(fileName, (char*)elogBuffer, elogBufferSize);
	}

	free(dataBuffer);
	dataBuffer = NULL;

	/* Get Smart log page  */
	memset(&smart_log, 0, sizeof (struct nvme_smart_log));
	ret = nvme_smart_log(fd, NVME_NSID_ALL, &smart_log);
	if (ret) {
		fprintf(stderr, "ERROR : WDC : nvme_smart_log() failed, ret = %d\n", ret);
	} else {
		wdc_UtilsSnprintf(fileName, MAX_PATH_LEN, "%s%s%s_%s_%s.bin", (char*)bufferFolderPath, WDC_DE_PATH_SEPARATOR,
				"SmartLog", (char*)serialNo, (char*)timeString);
		wdc_WriteToFile(fileName, (char*)&smart_log, sizeof(struct nvme_smart_log));
	}

	/* Get FW Slot log page  */
	memset(&fw_log, 0, sizeof (struct nvme_firmware_log_page));
	ret = nvme_fw_log(fd, &fw_log);
	if (ret) {
		fprintf(stderr, "ERROR : WDC : nvme_fw_log() failed, ret = %d\n", ret);
	} else {
		wdc_UtilsSnprintf(fileName, MAX_PATH_LEN, "%s%s%s_%s_%s.bin", (char*)bufferFolderPath, WDC_DE_PATH_SEPARATOR,
				"FwSLotLog", (char*)serialNo, (char*)timeString);
		wdc_WriteToFile(fileName, (char*)&fw_log, sizeof(struct nvme_firmware_log_page));
	}

	/* Get VU log pages  */
	/* define inputs for vendor unique log pages */
	vuLogInput = (PWDC_NVME_DE_VU_LOGPAGES)calloc(1, sizeof(WDC_NVME_DE_VU_LOGPAGES));
	vuLogInput->numOfVULogPages = sizeof(deVULogPagesList) / sizeof(deVULogPagesList[0]);

	for (vuLogIdx = 0; vuLogIdx < vuLogInput->numOfVULogPages; vuLogIdx++)
	{
		dataBufferSize = deVULogPagesList[vuLogIdx].logPageLen;
		dataBuffer = calloc(1, dataBufferSize);
		memset(dataBuffer, 0, dataBufferSize);

		ret = nvme_get_log(fd, WDC_DE_GLOBAL_NSID, deVULogPagesList[vuLogIdx].logPageId, 0,
				   false, dataBufferSize, dataBuffer);
		if (ret) {
			fprintf(stderr, "ERROR : WDC : nvme_get_log() for log page 0x%x failed, ret = %d\n",
					deVULogPagesList[vuLogIdx].logPageId, ret);
		} else {
			wdc_UtilsDeleteCharFromString((char*)deVULogPagesList[vuLogIdx].logPageIdStr, 4, ' ');
			wdc_UtilsSnprintf(fileName, MAX_PATH_LEN, "%s%s%s_%s_%s_%s.bin", (char*)bufferFolderPath, WDC_DE_PATH_SEPARATOR,
					"LogPage", (char*)&deVULogPagesList[vuLogIdx].logPageIdStr, (char*)serialNo, (char*)timeString);
			wdc_WriteToFile(fileName, (char*)dataBuffer, dataBufferSize);
		}

		free(dataBuffer);
		dataBuffer = NULL;
	}

	free(vuLogInput);

	/* Get NVMe Features (0x01, 0x02, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C) */
	for (listIdx = 1; listIdx < (sizeof(deFeatureIdList) / sizeof(deFeatureIdList[0])); listIdx++)
	{
		memset(featureIdBuff, 0, sizeof(featureIdBuff));
		/* skipping  LbaRangeType as it is an optional nvme command and not supported */
		if (deFeatureIdList[listIdx].featureId == FID_LBA_RANGE_TYPE)
			continue;
		ret = nvme_get_feature(fd, WDC_DE_GLOBAL_NSID, deFeatureIdList[listIdx].featureId, FS_CURRENT, 0,
				sizeof(featureIdBuff), &featureIdBuff, &result);

		if (ret) {
			fprintf(stderr, "ERROR : WDC : nvme_get_feature id 0x%x failed, ret = %d\n",
					deFeatureIdList[listIdx].featureId, ret);
		} else {
			wdc_UtilsSnprintf(fileName, MAX_PATH_LEN, "%s%s%s0x%x_%s_%s_%s.bin", (char*)bufferFolderPath, WDC_DE_PATH_SEPARATOR,
					"FEATURE_ID_", deFeatureIdList[listIdx].featureId,
					deFeatureIdList[listIdx].featureName, serialNo, timeString);
			wdc_WriteToFile(fileName, (char*)featureIdBuff, sizeof(featureIdBuff));
		}
	}

	/* Read Debug Directory */
	ret = wdc_get_log_dir_max_entries(fd, &maxNumOfVUFiles);
	if (ret == WDC_STATUS_SUCCESS)
	{
		memset(&deEssentialsList, 0, sizeof(deEssentialsList));
		deEssentialsList.logEntry = (WDC_DRIVE_ESSENTIALS*)calloc(1, sizeof(WDC_DRIVE_ESSENTIALS)*maxNumOfVUFiles);
		deEssentialsList.maxNumLogEntries = maxNumOfVUFiles;

		/* Fetch VU File Directory */
		ret = wdc_fetch_log_directory(fd, &deEssentialsList);
		if (ret == WDC_STATUS_SUCCESS)
		{
			/* Get Debug Data Files */
			for (listIdx = 0; listIdx < deEssentialsList.numOfValidLogEntries; listIdx++)
			{
				if (0 == deEssentialsList.logEntry[listIdx].metaData.fileSize)
				{
					fprintf(stderr, "ERROR : WDC : File Size for %s is 0\n",
							deEssentialsList.logEntry[listIdx].metaData.fileName);
					ret = WDC_STATUS_FILE_SIZE_ZERO;
				} else {
					/* Fetch Log File Data */
					dataBuffer = (char *)calloc(1, (size_t)deEssentialsList.logEntry[listIdx].metaData.fileSize);
					ret = wdc_fetch_log_file_from_device(fd, deEssentialsList.logEntry[listIdx].metaData.fileID, WDC_DE_DESTN_SPI, deEssentialsList.logEntry[listIdx].metaData.fileSize,
							(__u8 *)dataBuffer);

					/* Write databuffer to file */
					if (ret == WDC_STATUS_SUCCESS)
					{
						memset(fileName, 0, sizeof(fileName));
						wdc_UtilsSnprintf(fileName, MAX_PATH_LEN, "%s%s%s_%s_%s.bin", bufferFolderPath, WDC_DE_PATH_SEPARATOR,
								deEssentialsList.logEntry[listIdx].metaData.fileName, serialNo, timeString);
						if (deEssentialsList.logEntry[listIdx].metaData.fileSize > 0xffffffff)
						{
							wdc_WriteToFile(fileName, dataBuffer, 0xffffffff);
							wdc_WriteToFile(fileName, dataBuffer + 0xffffffff, (__u32)(deEssentialsList.logEntry[listIdx].metaData.fileSize - 0xffffffff));
						} else {
							wdc_WriteToFile(fileName, dataBuffer, (__u32)deEssentialsList.logEntry[listIdx].metaData.fileSize);
						}
					} else {
						fprintf(stderr, "ERROR : WDC : wdc_fetch_log_file_from_device: %s failed, ret = %d\n",
								deEssentialsList.logEntry[listIdx].metaData.fileName, ret);
					}
					free(dataBuffer);
					dataBuffer = NULL;
				}
			}
		} else {
			fprintf(stderr, "WDC : wdc_fetch_log_directory failed, ret = %d\n", ret);
		}

		free(deEssentialsList.logEntry);
		deEssentialsList.logEntry = NULL;
	} else {
		fprintf(stderr, "WDC : wdc_get_log_dir_max_entries failed, ret = %d\n", ret);
	}

	/* Get Dump Trace Data */
	wdc_UtilsSnprintf(fileName, MAX_PATH_LEN, "%s%s%s_%s_%s.bin", (char*)bufferFolderPath, WDC_DE_PATH_SEPARATOR, "dumptrace", serialNo, timeString);
	if (WDC_STATUS_SUCCESS != (ret = wdc_de_get_dump_trace(fd, (char*)bufferFolderPath, 0, fileName)))
	{
		fprintf(stderr, "ERROR : WDC : wdc_de_get_dump_trace failed, ret = %d\n", ret);
	}

	/* Tar the Drive Essentials directory */
	wdc_UtilsSnprintf(tarFileName, sizeof(tarFileName), "%s%s", (char*)bufferFolderPath, WDC_DE_TAR_FILE_EXTN);
	if (dir != NULL) {
		wdc_UtilsSnprintf(tarFiles, sizeof(tarFiles), "%s%s%s%s%s",
				(char*)dir, WDC_DE_PATH_SEPARATOR, (char*)bufferFolderName, WDC_DE_PATH_SEPARATOR, WDC_DE_TAR_FILES);
	} else {
		wdc_UtilsSnprintf(tarFiles, sizeof(tarFiles), "%s%s%s", (char*)bufferFolderName, WDC_DE_PATH_SEPARATOR, WDC_DE_TAR_FILES);
	}
	wdc_UtilsSnprintf(tarCmd, sizeof(tarCmd), "%s %s %s", WDC_DE_TAR_CMD, (char*)tarFileName, (char*)tarFiles);

	ret = system(tarCmd);

	if (ret) {
		fprintf(stderr, "ERROR : WDC : Tar of Drive Essentials data failed, ret = %d\n", ret);
	}

	fprintf(stderr, "Get of Drive Essentials data successful\n");
	return 0;
}

static int wdc_drive_essentials(int argc, char **argv, struct command *command,
		struct plugin *plugin)
{
	char *desc = "Capture Drive Essentials.";
	char *dirName = "Output directory pathname.";

	char d[PATH_MAX] = {0};
	char k[PATH_MAX] = {0};
	char *d_ptr;
	int fd;
	struct config {
		char *dirName;
	};

	struct config cfg = {
			.dirName = NULL,
	};

	const struct argconfig_commandline_options command_line_options[] = {
			{"dir-name", 'd', "DIRECTORY", CFG_STRING, &cfg.dirName, required_argument, dirName},
			{ NULL, '\0', NULL, CFG_NONE, NULL, no_argument, desc},
			{NULL}
	};

	fd = parse_and_open(argc, argv, desc, command_line_options, NULL, 0);
	if (fd < 0)
		return fd;

	if (!wdc_check_device_match(fd, WDC_NVME_SNDK_VID, WDC_NVME_SXSLCL_DEV_ID)) {
		fprintf(stderr, "WARNING : WDC : Device not supported\n");
		return -1;
	}

	if (cfg.dirName != NULL) {
		strncpy(d, cfg.dirName, PATH_MAX - 1);
		d_ptr = d;
	} else {
		d_ptr = NULL;
	}

	return wdc_do_drive_essentials(fd, d_ptr, k);
}

static void wdc_print_nand_stats_normal(struct wdc_nand_stats *data)
{
	printf("  NAND Statistics :- \n");
	printf("  NAND Writes TLC (Bytes)		         %.0Lf\n",
			int128_to_double(data->nand_write_tlc));
	printf("  NAND Writes SLC (Bytes)		         %.0Lf\n",
			int128_to_double(data->nand_write_slc));
	printf("  NAND Program Failures			  	 %"PRIu32"\n",
			(uint32_t)le32_to_cpu(data->nand_prog_failure));
	printf("  NAND Erase Failures				 %"PRIu32"\n",
			(uint32_t)le32_to_cpu(data->nand_erase_failure));
	printf("  Bad Block Count			         %"PRIu32"\n",
			(uint32_t)le32_to_cpu(data->bad_block_count));
	printf("  NAND XOR/RAID Recovery Trigger Events		 %"PRIu64"\n",
			(long unsigned int)le64_to_cpu(data->nand_rec_trigger_event));
	printf("  E2E Error Counter                    		 %"PRIu64"\n",
			(long unsigned int)le64_to_cpu(data->e2e_error_counter));
	printf("  Number Successful NS Resizing Events		 %"PRIu64"\n",
			(long unsigned int)le64_to_cpu(data->successful_ns_resize_event));
}

static void wdc_print_nand_stats_json(struct wdc_nand_stats *data)
{
	struct json_object *root;

	root = json_create_object();
	json_object_add_value_float(root, "NAND Writes TLC (Bytes)",
			int128_to_double(data->nand_write_tlc));
	json_object_add_value_float(root, "NAND Writes SLC (Bytes)",
			int128_to_double(data->nand_write_slc));
	json_object_add_value_uint(root, "NAND Program Failures",
			le32_to_cpu(data->nand_prog_failure));
	json_object_add_value_uint(root, "NAND Erase Failures",
			le32_to_cpu(data->nand_erase_failure));
	json_object_add_value_uint(root, "Bad Block Count",
			le32_to_cpu(data->bad_block_count));
	json_object_add_value_uint(root, "NAND XOR/RAID Recovery Trigger Events",
			le64_to_cpu(data->nand_rec_trigger_event));
	json_object_add_value_uint(root, "E2E Error Counter",
			le64_to_cpu(data->e2e_error_counter));
	json_object_add_value_uint(root, "Number Successful NS Resizing Events",
			le64_to_cpu(data->successful_ns_resize_event));

	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

static int wdc_do_vs_nand_stats(int fd, char *format)
{
	int ret;
	int fmt = -1;
	uint8_t *output = NULL;
	struct wdc_nand_stats *nand_stats;

	if ((output = (uint8_t*)calloc(WDC_NVME_NAND_STATS_SIZE, sizeof(uint8_t))) == NULL) {
		fprintf(stderr, "ERROR : WDC : calloc : %s\n", strerror(errno));
		ret = -1;
		goto out;
	}

	ret = nvme_get_log(fd, 0xFFFFFFFF, WDC_NVME_NAND_STATS_LOG_ID, 0,
			   false, WDC_NVME_NAND_STATS_SIZE, (void*)output);
	if (ret) {
		fprintf(stderr, "ERROR : WDC : %s : Failed to retreive NAND stats\n", __func__);
		goto out;
	} else {
		fmt = validate_output_format(format);
		if (fmt < 0) {
			fprintf(stderr, "ERROR : WDC : invalid output format\n");
			ret = fmt;
			goto out;
		}

		/* parse the data */
		nand_stats = (struct wdc_nand_stats *)(output);
		switch (fmt) {
		case NORMAL:
			wdc_print_nand_stats_normal(nand_stats);
			break;
		case JSON:
			wdc_print_nand_stats_json(nand_stats);
			break;
		}
	}

out:
	free(output);
	return ret;
}


static int wdc_vs_nand_stats(int argc, char **argv, struct command *command,
		struct plugin *plugin)
{
	const char *desc = "Retrieve NAND statistics.";
	int fd;
	int ret = 0;

	struct config {
		char *output_format;
	};

	struct config cfg = {
		.output_format = "normal",
	};

	const struct argconfig_commandline_options command_line_options[] = {
			{"output-format", 'o', "FMT", CFG_STRING, &cfg.output_format, required_argument, "Output Format: normal|json" },
			{ NULL, '\0', NULL, CFG_NONE, NULL, no_argument, desc},
			{NULL}
	};

	fd = parse_and_open(argc, argv, desc, command_line_options, NULL, 0);
	if (fd < 0)
		return fd;


	ret = wdc_do_vs_nand_stats(fd, cfg.output_format);
	if (ret)
		fprintf(stderr, "ERROR : WDC : Failure reading NAND statistics, ret = %d\n", ret);

	return ret;
}




static void wdc_print_fb_ca_log_normal(struct wdc_ssd_ca_perf_stats *perf)
{
	uint64_t converted = 0;

	printf("  CA Log Page Performance Statistics :- \n");
	printf("  NAND Bytes Written                             %20"PRIu64 "%20"PRIu64"\n",
			le64_to_cpu(perf->nand_bytes_wr_hi), le64_to_cpu(perf->nand_bytes_wr_lo));
	printf("  NAND Bytes Read                                %20"PRIu64 "%20"PRIu64"\n",
			le64_to_cpu(perf->nand_bytes_rd_hi), le64_to_cpu(perf->nand_bytes_rd_lo));

	converted = le64_to_cpu(perf->nand_bad_block);
	printf("  NAND Bad Block Count (Normalized)              %20"PRIu64"\n",
			converted & 0xFFFF);
	printf("  NAND Bad Block Count (Raw)                     %20"PRIu64"\n",
			converted >> 16);

	printf("  Uncorrectable Read Count                       %20"PRIu64"\n",
			le64_to_cpu(perf->uncorr_read_count));
	printf("  Soft ECC Error Count                           %20"PRIu64"\n",
			le64_to_cpu(perf->ecc_error_count));
	printf("  SSD End to End Detected Correction Count       %20"PRIu32"\n",
			(uint32_t)le32_to_cpu(perf->ssd_detect_count));
	printf("  SSD End to End Corrected Correction Count      %20"PRIu32"\n",
			(uint32_t)le32_to_cpu(perf->ssd_correct_count));
	printf("  System Data Percent Used                       %20"PRIu32"%%\n",
			perf->data_percent_used);
	printf("  User Data Erase Counts Max                     %20"PRIu32"\n",
			(uint32_t)le32_to_cpu(perf->data_erase_max));
	printf("  User Data Erase Counts Min                     %20"PRIu32"\n",
			(uint32_t)le32_to_cpu(perf->data_erase_min));
	printf("  Refresh Count                                  %20"PRIu64"\n",
			le64_to_cpu(perf->refresh_count));

	converted = le64_to_cpu(perf->program_fail);
	printf("  Program Fail Count (Normalized)                %20"PRIu64"\n",
			converted & 0xFFFF);
	printf("  Program Fail Count (Raw)                       %20"PRIu64"\n",
			converted >> 16);

	converted = le64_to_cpu(perf->user_erase_fail);
	printf("  User Data Erase Fail Count (Normalized)        %20"PRIu64"\n",
			converted & 0xFFFF);
	printf("  User Data Erase Fail Count (Raw)               %20"PRIu64"\n",
			converted >> 16);

	converted = le64_to_cpu(perf->system_erase_fail);
	printf("  System Area Erase Fail Count (Normalized)      %20"PRIu64"\n",
			converted & 0xFFFF);
	printf("  System Area Erase Fail Count (Raw)             %20"PRIu64"\n",
			converted >> 16);

	printf("  Thermal Throttling Status                      %20"PRIu8"\n",
			perf->thermal_throttle_status);
	printf("  Thermal Throttling Count                       %20"PRIu8"\n",
			perf->thermal_throttle_count);
	printf("  PCIe Correctable Error Count                   %20"PRIu64"\n",
			le64_to_cpu(perf->pcie_corr_error));
	printf("  Incomplete Shutdown Count                      %20"PRIu32"\n",
			(uint32_t)le32_to_cpu(perf->incomplete_shutdown_count));
	printf("  Percent Free Blocks                            %20"PRIu32"%%\n",
			perf->percent_free_blocks);
}

static void wdc_print_fb_ca_log_json(struct wdc_ssd_ca_perf_stats *perf)
{
	struct json_object *root;
	uint64_t converted = 0;

	root = json_create_object();
	json_object_add_value_int(root, "NAND Bytes Written Hi", le64_to_cpu(perf->nand_bytes_wr_hi));
	json_object_add_value_int(root, "NAND Bytes Written Lo", le64_to_cpu(perf->nand_bytes_wr_lo));
	json_object_add_value_int(root, "NAND Bytes Read Hi", le64_to_cpu(perf->nand_bytes_rd_hi));
	json_object_add_value_int(root, "NAND Bytes Read Lo", le64_to_cpu(perf->nand_bytes_rd_lo));

	converted = le64_to_cpu(perf->nand_bad_block);
	json_object_add_value_int(root, "NAND Bad Block Count (Normalized)",
			converted & 0xFFFF);
	json_object_add_value_int(root, "NAND Bad Block Count (Raw)",
			converted >> 16);

	json_object_add_value_int(root, "Uncorrectable Read Count", le64_to_cpu(perf->uncorr_read_count));
	json_object_add_value_int(root, "Soft ECC Error Count",	le64_to_cpu(perf->ecc_error_count));
	json_object_add_value_int(root, "SSD End to End Detected Correction Count",
			le32_to_cpu(perf->ssd_detect_count));
	json_object_add_value_int(root, "SSD End to End Corrected Correction Count",
			le32_to_cpu(perf->ssd_correct_count));
	json_object_add_value_int(root, "System Data Percent Used",
			perf->data_percent_used);
	json_object_add_value_int(root, "User Data Erase Counts Max",
			le32_to_cpu(perf->data_erase_max));
	json_object_add_value_int(root, "User Data Erase Counts Min",
			le32_to_cpu(perf->data_erase_min));
	json_object_add_value_int(root, "Refresh Count", le64_to_cpu(perf->refresh_count));

	converted = le64_to_cpu(perf->program_fail);
	json_object_add_value_int(root, "Program Fail Count (Normalized)",
			converted & 0xFFFF);
	json_object_add_value_int(root, "Program Fail Count (Raw)",
			converted >> 16);

	converted = le64_to_cpu(perf->user_erase_fail);
	json_object_add_value_int(root, "User Data Erase Fail Count (Normalized)",
			converted & 0xFFFF);
	json_object_add_value_int(root, "User Data Erase Fail Count (Raw)",
			converted >> 16);

	converted = le64_to_cpu(perf->system_erase_fail);
	json_object_add_value_int(root, "System Area Erase Fail Count (Normalized)",
			converted & 0xFFFF);
	json_object_add_value_int(root, "System Area Erase Fail Count (Raw)",
			converted >> 16);

	json_object_add_value_int(root, "Thermal Throttling Status",
			perf->thermal_throttle_status);
	json_object_add_value_int(root, "Thermal Throttling Count",
			perf->thermal_throttle_count);
	json_object_add_value_int(root, "PCIe Correctable Error", le64_to_cpu(perf->pcie_corr_error));
	json_object_add_value_int(root, "Incomplete Shutdown Counte", le32_to_cpu(perf->incomplete_shutdown_count));
	json_object_add_value_int(root, "Percent Free Blocks", perf->percent_free_blocks);
	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

static int wdc_print_fb_ca_log(struct wdc_ssd_ca_perf_stats *perf, int fmt)
{
	if (!perf) {
		fprintf(stderr, "ERROR : WDC : Invalid buffer to read perf stats\n");
		return -1;
	}
	switch (fmt) {
	case NORMAL:
		wdc_print_fb_ca_log_normal(perf);
		break;
	case JSON:
		wdc_print_fb_ca_log_json(perf);
		break;
	}
	return 0;
}

static void wdc_print_bd_ca_log_normal(void *data)
{
	struct wdc_bd_ca_log_format *bd_data = (struct wdc_bd_ca_log_format *)data;
	__u64 *raw;
	__u16 *word_raw1, *word_raw2, *word_raw3;
	__u32  *dword_raw;
	__u8  *byte_raw;

	if (bd_data->field_id == 0x00) {
		raw = (__u64 *)&bd_data->raw_value[0];
		printf("Additional Smart Log for NVME device: namespace-id:%x\n",
		       WDC_DE_GLOBAL_NSID);
		printf("key                               normalized raw\n");
		printf("program_fail_count              : %3"PRIu8"%%       %"PRIu64"\n",
		       bd_data->normalized_value, le64_to_cpu(*raw & 0x00FFFFFFFFFFFFFF));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x01) {
		raw = (__u64 *)&bd_data->raw_value[0];
		printf("erase_fail_count                : %3"PRIu8"%%       %"PRIu64"\n",
				bd_data->normalized_value, le64_to_cpu(*raw & 0x00FFFFFFFFFFFFFF));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x02) {
		word_raw1 = (__u16 *)&bd_data->raw_value[1];
		word_raw2 = (__u16 *)&bd_data->raw_value[3];
		word_raw3 = (__u16 *)&bd_data->raw_value[5];
		printf("wear_leveling                   : %3"PRIu8"%%       min: %"PRIu16", max: %"PRIu16", avg: %"PRIu16"\n",
				bd_data->normalized_value,
				le16_to_cpu(*word_raw1),
				le16_to_cpu(*word_raw2),
				le16_to_cpu(*word_raw3));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x03) {
		raw = (__u64 *)&bd_data->raw_value[0];
		printf("end_to_end_error_detection_count: %3"PRIu8"%%       %"PRIu64"\n",
				bd_data->normalized_value, le64_to_cpu(*raw & 0x00FFFFFFFFFFFFFF));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x04) {
		raw = (__u64 *)&bd_data->raw_value[0];
		printf("crc_error_count                 : %3"PRIu8"%%       %"PRIu64"\n",
			   bd_data->normalized_value, le64_to_cpu(*raw & 0x00FFFFFFFFFFFFFF));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x05) {
		raw = (__u64 *)&bd_data->raw_value[0];
		printf("timed_workload_media_wear       : %3"PRIu8"%%       %-.3f%%\n",
		       bd_data->normalized_value, safe_div_fp((*raw & 0x00FFFFFFFFFFFFFF), 1024.0));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x06) {
		raw = (__u64 *)&bd_data->raw_value[0];
		printf("timed_workload_host_reads       : %3"PRIu8"%%       %"PRIu64"%%\n",
		       bd_data->normalized_value, le64_to_cpu(*raw & 0x00FFFFFFFFFFFFFF));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x07) {
		raw = (__u64 *)&bd_data->raw_value[0];
		printf("timed_workload_timer            : %3"PRIu8"%%       %"PRIu64"\n",
		       bd_data->normalized_value, le64_to_cpu(*raw & 0x00FFFFFFFFFFFFFF));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x08) {
		byte_raw = (__u8 *)&bd_data->raw_value[1];
		dword_raw = (__u32 *)&bd_data->raw_value[2];
		printf("thermal_throttle_status         : %3"PRIu8"%%       %"PRIu16"%%, cnt: %"PRIu16"\n",
		       bd_data->normalized_value, *byte_raw, le32_to_cpu(*dword_raw));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x09) {
		raw = (__u64 *)&bd_data->raw_value[0];
		printf("retry_buffer_overflow_count     : %3"PRIu8"%%       %"PRIu64"\n",
		       bd_data->normalized_value, le64_to_cpu(*raw & 0x00FFFFFFFFFFFFFF));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x0A) {
		raw = (__u64 *)&bd_data->raw_value[0];
		printf("pll_lock_loss_count             : %3"PRIu8"%%       %"PRIu64"\n",
			   bd_data->normalized_value, le64_to_cpu(*raw & 0x00FFFFFFFFFFFFFF));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x0B) {
		raw = (__u64 *)&bd_data->raw_value[0];
		printf("nand_bytes_written              : %3"PRIu8"%%       sectors: %.f\n",
			   bd_data->normalized_value, safe_div_fp((*raw & 0x00FFFFFFFFFFFFFF), 0xFFFF));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x0C) {
		raw = (__u64 *)&bd_data->raw_value[0];
		printf("host_bytes_written              : %3"PRIu8"%%       sectors: %.f\n",
			   bd_data->normalized_value, safe_div_fp((*raw & 0x00FFFFFFFFFFFFFF), 0xFFFF));
	} else {
		goto invalid_id;
	}

	goto done;

invalid_id:
	printf("  Invalid Field ID = %d\n", bd_data->field_id);

done:
	return;

}

static void wdc_print_bd_ca_log_json(void *data)
{
	struct wdc_bd_ca_log_format *bd_data = (struct wdc_bd_ca_log_format *)data;
	__u64 *raw;
	__u16 *word_raw;
	__u32  *dword_raw;
	__u8  *byte_raw;
	struct json_object *root = json_create_object();

	if (bd_data->field_id == 0x00) {
		raw = (__u64 *)&bd_data->raw_value[0];
		json_object_add_value_int(root, "program_fail_count normalized",
				bd_data->normalized_value);
		json_object_add_value_int(root, "program_fail_count raw",
				le64_to_cpu(*raw & 0x00FFFFFFFFFFFFFF));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x01) {
		raw = (__u64 *)&bd_data->raw_value[0];
		json_object_add_value_int(root, "erase_fail_count normalized",
				bd_data->normalized_value);
		json_object_add_value_int(root, "erase_fail_count raw",
				le64_to_cpu(*raw & 0x00FFFFFFFFFFFFFF));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x02) {
		word_raw = (__u16 *)&bd_data->raw_value[1];
		json_object_add_value_int(root, "wear_leveling normalized",	bd_data->normalized_value);
		json_object_add_value_int(root, "wear_leveling min", le16_to_cpu(*word_raw));
		word_raw = (__u16 *)&bd_data->raw_value[3];
		json_object_add_value_int(root, "wear_leveling max", le16_to_cpu(*word_raw));
		word_raw = (__u16 *)&bd_data->raw_value[5];
		json_object_add_value_int(root, "wear_leveling avg", le16_to_cpu(*word_raw));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x03) {
		raw = (__u64 *)&bd_data->raw_value[0];
		json_object_add_value_int(root, "end_to_end_error_detection_count normalized",
				bd_data->normalized_value);
		json_object_add_value_int(root, "end_to_end_error_detection_count raw",
				le64_to_cpu(*raw & 0x00FFFFFFFFFFFFFF));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x04) {
		raw = (__u64 *)&bd_data->raw_value[0];
		json_object_add_value_int(root, "crc_error_count normalized",
				bd_data->normalized_value);
		json_object_add_value_int(root, "crc_error_count raw",
				le64_to_cpu(*raw & 0x00FFFFFFFFFFFFFF));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x05) {
		raw = (__u64 *)&bd_data->raw_value[0];
		json_object_add_value_int(root, "timed_workload_media_wear normalized",
				bd_data->normalized_value);
		json_object_add_value_float(root, "timed_workload_media_wear raw",
				(float)safe_div_fp((*raw & 0x00FFFFFFFFFFFFFF), 1024.0));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x06) {
		raw = (__u64 *)&bd_data->raw_value[0];
		json_object_add_value_int(root, "timed_workload_host_reads normalized",
				bd_data->normalized_value);
		json_object_add_value_int(root, "timed_workload_host_reads raw",
				le64_to_cpu(*raw & 0x00000000000000FF));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x07) {
		raw = (__u64 *)&bd_data->raw_value[0];
		json_object_add_value_int(root, "timed_workload_timer normalized",
					  bd_data->normalized_value);
		json_object_add_value_int(root, "timed_workload_timer",
					  le64_to_cpu(*raw & 0x00FFFFFFFFFFFFFF));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x08) {
		byte_raw = (__u8 *)&bd_data->raw_value[1];
		json_object_add_value_int(root, "thermal_throttle_status normalized",
				bd_data->normalized_value);
		json_object_add_value_int(root, "thermal_throttle_status", *byte_raw);
		dword_raw = (__u32 *)&bd_data->raw_value[2];
		json_object_add_value_int(root, "thermal_throttle_cnt",	le32_to_cpu(*dword_raw));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x09) {
		raw = (__u64 *)&bd_data->raw_value[0];
		json_object_add_value_int(root, "retry_buffer_overflow_count normalized",
				bd_data->normalized_value);
		json_object_add_value_int(root, "retry_buffer_overflow_count raw",
				le64_to_cpu(*raw & 0x00FFFFFFFFFFFFFF));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x0A) {
		raw = (__u64 *)&bd_data->raw_value[0];
		json_object_add_value_int(root, "pll_lock_loss_count normalized",
				bd_data->normalized_value);
		json_object_add_value_int(root, "pll_lock_loss_count raw",
				le64_to_cpu(*raw & 0x00FFFFFFFFFFFFFF));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x0B) {
		raw = (__u64 *)&bd_data->raw_value[0];
		json_object_add_value_int(root, "nand_bytes_written normalized",
				bd_data->normalized_value);
		json_object_add_value_float(root, "nand_bytes_written raw",
				(float)safe_div_fp((*raw & 0x00FFFFFFFFFFFFFF), 0xFFFF));
	} else {
		goto invalid_id;
	}
	bd_data++;
	if (bd_data->field_id == 0x0C) {
		raw = (__u64 *)&bd_data->raw_value[0];
		json_object_add_value_int(root, "host_bytes_written normalized",
				bd_data->normalized_value);
		json_object_add_value_float(root, "host_bytes_written raw",
				(float)safe_div_fp((*raw & 0x00FFFFFFFFFFFFFF), 0xFFFF));
	} else {
		goto invalid_id;
	}

	goto done;

invalid_id:
	printf("  Invalid Field ID = %d\n", bd_data->field_id);

done:
	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);

	return;

}

static int wdc_print_bd_ca_log(void *bd_data, int fmt)
{
	if (!bd_data) {
		fprintf(stderr, "ERROR: WDC: Invalid buffer to read data\n");
		return -1;
	}
	switch (fmt) {
	case NORMAL:
		wdc_print_bd_ca_log_normal(bd_data);
		break;
	case JSON:
		wdc_print_bd_ca_log_json(bd_data);
		break;
	default:
		fprintf(stderr, "ERROR: WDC: Unknown format - %d\n", fmt);
		return -1;
	}
	return 0;
}

static int wdc_get_ca_log_page(int fd, char *format)
{
	int ret = 0;
	int fmt = -1;
	__u8 *data;
	struct wdc_ssd_ca_perf_stats *perf;
	uint32_t read_device_id, read_vendor_id;
	__u32 cust_id;

	if (!wdc_check_device(fd))
		return -1;
	fmt = validate_output_format(format);
	if (fmt < 0) {
		fprintf(stderr, "ERROR : WDC : invalid output format\n");
		return fmt;
	}

	/* verify the 0xCA log page is supported */
	if (wdc_nvme_check_supported_log_page(fd, WDC_NVME_GET_DEVICE_INFO_LOG_OPCODE) == false) {
		fprintf(stderr, "ERROR : WDC : 0xCA Log Page not supported\n");
		return -1;
	}

	/* get the FW customer id */
	cust_id = wdc_get_fw_cust_id(fd);
	if (cust_id == WDC_INVALID_CUSTOMER_ID) {
		fprintf(stderr, "%s: ERROR: WDC: invalid customer id\n", __func__);
		return -1;
	}

	ret = wdc_get_pci_ids(&read_device_id, &read_vendor_id);

	switch (read_device_id) {
	case WDC_NVME_SN200_DEV_ID:
		if (cust_id == WDC_CUSTOMER_ID_0x1005) {
			data = (__u8 *)malloc(sizeof(__u8) * WDC_FB_CA_LOG_BUF_LEN);
			if (!data) {
				fprintf(stderr, "ERROR: WDC: malloc: %s\n", strerror(errno));
				return -1;
			}

			memset(data, 0, sizeof(__u8) * WDC_FB_CA_LOG_BUF_LEN);

			ret = nvme_get_log(fd, 0xFFFFFFFF, WDC_NVME_GET_DEVICE_INFO_LOG_OPCODE, 0,
				   false, WDC_FB_CA_LOG_BUF_LEN, data);

			if (!ret) {
				/* parse the data */
				perf = (struct wdc_ssd_ca_perf_stats *)(data);
				ret = wdc_print_fb_ca_log(perf, fmt);
			} else {
				fprintf(stderr, "ERROR: WDC: Unable to read CA Log Page data\n");
				ret = -1;
			}
		} else {

			fprintf(stderr, "ERROR: WDC: Unsupported Customer id, id = 0x%x\n", cust_id);
			return -1;
		}
		break;
	case WDC_NVME_SN640_DEV_ID:
	case WDC_NVME_SN640_DEV_ID_1:
	case WDC_NVME_SN640_DEV_ID_2:
	case WDC_NVME_SN640_DEV_ID_3:
	case WDC_NVME_SN840_DEV_ID:
	case WDC_NVME_SN840_DEV_ID_1:
	case WDC_NVME_SN860_DEV_ID:
		if (cust_id == WDC_CUSTOMER_ID_0x1005) {
			data = (__u8 *)malloc(sizeof(__u8) * WDC_FB_CA_LOG_BUF_LEN);
			if (!data) {
				fprintf(stderr, "ERROR: WDC: malloc: %s\n", strerror(errno));
				return -1;
			}

			memset(data, 0, sizeof(__u8) * WDC_FB_CA_LOG_BUF_LEN);

			ret = nvme_get_log(fd, 0xFFFFFFFF, WDC_NVME_GET_DEVICE_INFO_LOG_OPCODE, 0,
				   false, WDC_FB_CA_LOG_BUF_LEN, data);

			if (!ret) {
				/* parse the data */
				perf = (struct wdc_ssd_ca_perf_stats *)(data);
				ret = wdc_print_fb_ca_log(perf, fmt);
			} else {
				fprintf(stderr, "ERROR: WDC: Unable to read CA Log Page data\n");
				ret = -1;
			}
		} else if ((cust_id == WDC_CUSTOMER_ID_GN) || (cust_id == WDC_CUSTOMER_ID_GD) ||
				(cust_id == WDC_CUSTOMER_ID_BD)) {
			data = (__u8 *)malloc(sizeof(__u8) * WDC_BD_CA_LOG_BUF_LEN);
			if (!data) {
				fprintf(stderr, "ERROR: WDC: malloc: %s\n", strerror(errno));
				return -1;
			}

			memset(data, 0, sizeof(__u8) * WDC_BD_CA_LOG_BUF_LEN);

			ret = nvme_get_log(fd, 0xFFFFFFFF, WDC_NVME_GET_DEVICE_INFO_LOG_OPCODE, 0,
				   false, WDC_BD_CA_LOG_BUF_LEN, data);

			if (!ret) {
				/* parse the data */
				ret = wdc_print_bd_ca_log(data, fmt);
			} else {
				fprintf(stderr, "ERROR: WDC: Unable to read CA Log Page data\n");
				ret = -1;
			}
		} else {
			fprintf(stderr, "ERROR: WDC: Unsupported Customer id, id = 0x%x\n", cust_id);
			return -1;
		}
		break;
	default:
		fprintf(stderr, "ERROR: WDC: Log page 0xCA not supported for this device\n");
		return -1;
	}

	free(data);
	return ret;
}


static void wdc_print_eol_c0_normal(void *data)
{

	__u8 *log_data = (__u8 *)data;

	printf("  End of Life Log Page 0xC0 :-\n");

	printf("  Realloc Block Count			%"PRIu32"\n",
			(uint32_t)le32_to_cpu(log_data[EOL_RBC]));
	printf("  ECC Rate				%"PRIu32"\n",
			(uint32_t)le32_to_cpu(log_data[EOL_ECCR]));
	printf("  Write Amp				%"PRIu32"\n",
			(uint32_t)le32_to_cpu(log_data[EOL_WRA]));
	printf("  Percent Life Remaining		%"PRIu32"\n",
			(uint32_t)le32_to_cpu(log_data[EOL_PLR]));
	printf("  Program Fail Count			%"PRIu32"\n",
			(uint32_t)le32_to_cpu(log_data[EOL_PFC]));
	printf("  Erase Fail Count			%"PRIu32"\n",
			(uint32_t)le32_to_cpu(log_data[EOL_EFC]));
	printf("  Raw Read Error Rate			%"PRIu32"\n",
			(uint32_t)le32_to_cpu(log_data[EOL_RRER]));

}

static void wdc_print_eol_c0_json(void *data)
{
	__u8 *log_data = (__u8 *)data;
	struct json_object *root = json_create_object();

	json_object_add_value_uint(root, "Realloc Block Count",
			(uint32_t)le32_to_cpu(log_data[EOL_RBC]));
	json_object_add_value_uint(root, "ECC Rate",
			(uint32_t)le32_to_cpu(log_data[EOL_ECCR]));
	json_object_add_value_uint(root, "Write Amp",
			(uint32_t)le32_to_cpu(log_data[EOL_WRA]));
	json_object_add_value_uint(root, "Percent Life Remaining",
			(uint32_t)le32_to_cpu(log_data[EOL_PLR]));
	json_object_add_value_uint(root, "Program Fail Count",
			(uint32_t)le32_to_cpu(log_data[EOL_PFC]));
	json_object_add_value_uint(root, "Erase Fail Count",
			(uint32_t)le32_to_cpu(log_data[EOL_EFC]));
	json_object_add_value_uint(root, "Raw Read Error Rate",
			(uint32_t)le32_to_cpu(log_data[EOL_RRER]));

	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

static int wdc_print_c0_eol_log(void *data, int fmt)
{
	if (!data) {
		fprintf(stderr, "ERROR: WDC: Invalid buffer to read 0xC0 log\n");
		return -1;
	}
	switch (fmt) {
	case NORMAL:
		wdc_print_eol_c0_normal(data);
		break;
	case JSON:
		wdc_print_eol_c0_json(data);
		break;
	}
	return 0;
}


static int wdc_get_c0_log_page_sn_customer_id_0x100X(int fd, int uuid_index,
						     char *format, __u32 namespace_id, int fmt)
{
	int ret;
	__u8 *data;
	int i;

	if (!uuid_index) {
		data = (__u8 *)malloc(sizeof(__u8) * WDC_NVME_SMART_CLOUD_ATTR_LEN);
		if (!data) {
			fprintf(stderr, "ERROR: WDC: malloc: %s\n", strerror(errno));
			return -1;
		}

		if (namespace_id == NVME_NSID_ALL) {
			ret = nvme_get_nsid_new(fd, &namespace_id);
			if (ret < 0)
				namespace_id = NVME_NSID_ALL;
		}

		/* Get the 0xC0 log data */
		ret = nvme_get_log(fd, namespace_id, 
			WDC_NVME_GET_SMART_CLOUD_ATTR_LOG_ID, uuid_index, 
			false, WDC_NVME_SMART_CLOUD_ATTR_LEN, data);

		if (!ret) {
			/* Verify GUID matches */
			for (i = 0; i < 16; i++) {
				if (scao_guid[i] != data[SCAO_LPG + i]) {
					fprintf(stderr, "ERROR: WDC: Unknown GUID in C0 Log Page data\n");
					int j;

					fprintf(stderr, "ERROR: WDC: Expected GUID:  0x");
					for (j = 0; j < 16; j++)
						fprintf(stderr, "%x", scao_guid[j]);
					fprintf(stderr, "\nERROR: WDC: Actual GUID:    0x");
					for (j = 0; j < 16; j++)
						fprintf(stderr, "%x", data[SCAO_LPG + j]);
					fprintf(stderr, "\n");

					ret = -1;
					break;
				}
			}

			if (!ret)
				/* parse the data */
				wdc_print_c0_cloud_attr_log(data, fmt);
		} else {
			fprintf(stderr, "ERROR: WDC: Unable to read C0 Log Page data\n");
			ret = -1;
		}

		free(data);
	} else if (uuid_index == 1) {
		data = (__u8 *)malloc(sizeof(__u8) * WDC_NVME_EOL_STATUS_LOG_LEN);
		if (!data) {
			fprintf(stderr, "ERROR: WDC: malloc: %s\n", strerror(errno));
			return -1;
		}

		/* Get the 0xC0 log data */
		ret = nvme_get_log(fd, NVME_NSID_ALL, 
			WDC_NVME_GET_EOL_STATUS_LOG_OPCODE, uuid_index, 
			false, WDC_NVME_EOL_STATUS_LOG_LEN, data);

		if (!ret) {
			/* parse the data */
			wdc_print_c0_eol_log(data, fmt);
		} else {
			fprintf(stderr, "ERROR: WDC: Unable to read C0 Log Page data\n");
			ret = -1;
		}

		free(data);
	} else {
		fprintf(stderr, "ERROR: WDC: Unknown uuid index\n");
		ret = -1;
	}

	return ret;
}

static int wdc_get_c0_log_page_sn(int fd, int uuid_index, char *format,
				  __u32 namespace_id, int fmt)
{
	int ret = 0;
	__u32 cust_id;
	__u8 *data;

	cust_id = wdc_get_fw_cust_id(fd);
	if (cust_id == WDC_INVALID_CUSTOMER_ID) {
		fprintf(stderr, "%s: ERROR: WDC: invalid customer id\n", __func__);
		return -1;
	}

	if ((cust_id == WDC_CUSTOMER_ID_0x1004) || (cust_id == WDC_CUSTOMER_ID_0x1008) ||
	    (cust_id == WDC_CUSTOMER_ID_0x1005)) {
		ret = wdc_get_c0_log_page_sn_customer_id_0x100X(fd, uuid_index, format,
								namespace_id, fmt);
	} else {
		data = (__u8 *)malloc(sizeof(__u8) * WDC_NVME_EOL_STATUS_LOG_LEN);
		if (!data) {
			fprintf(stderr, "ERROR: WDC: malloc: %s\n", strerror(errno));
			return -1;
		}

		/* Get the 0xC0 log data */
		ret = nvme_get_log(fd, namespace_id, 
			WDC_NVME_GET_EOL_STATUS_LOG_OPCODE, uuid_index, 
			false, WDC_NVME_EOL_STATUS_LOG_LEN, data);

		if (!ret) {
			/* parse the data */
			wdc_print_c0_eol_log(data, fmt);
		} else {
			fprintf(stderr, "ERROR: WDC: Unable to read C0 Log Page data\n");
			ret = -1;
		}

		free(data);
	}

	return ret;
}

// TODO: from util/types.h
/* uint128_t is not always available, define our own. */
union nvme_uint128 {
        __u8  bytes[16];
	__u32 words[4]; /* [0] is most significant word */
};

typedef union nvme_uint128 nvme_uint128_t;

// TODO: from util/types.c
static char *__uint128_t_to_string(nvme_uint128_t val, bool l10n)
{
	static char str[60];
	int idx = 60;
	__u64 div, rem;
	char *sep = NULL;
	int i, len = 0;

	if (l10n) {
		sep = localeconv()->thousands_sep;
		len = strlen(sep);
	}

	/* terminate at the end, and build up from the ones */
	str[--idx] = '\0';

	do {
		if (len && !((sizeof(str) - idx) % (3 + len))) {
			for (i = 0; i < len; i++)
				str[--idx] = sep[i];
		}

		rem = val.words[0];

		div = rem / 10;
		rem = ((rem - div * 10) << 32) + val.words[1];
		val.words[0] = div;

		div = rem / 10;
		rem = ((rem - div * 10) << 32) + val.words[2];
		val.words[1] = div;

		div = rem / 10;
		rem = ((rem - div * 10) << 32) + val.words[3];
		val.words[2] = div;

		div = rem / 10;
		rem = rem - div * 10;
		val.words[3] = div;

		str[--idx] = '0' + rem;
	} while (val.words[0] || val.words[1] || val.words[2] || val.words[3]);

	return str + idx;
}

char *uint128_t_to_string(nvme_uint128_t val)
{
	return __uint128_t_to_string(val, false);
}

nvme_uint128_t le128_to_cpu(__u8 *data)
{
	nvme_uint128_t u;
	nvme_uint128_t tmp;

	memcpy(tmp.bytes, data, 16);
	u.words[0] = le32_to_cpu(tmp.words[3]);
	u.words[1] = le32_to_cpu(tmp.words[2]);
	u.words[2] = le32_to_cpu(tmp.words[1]);
	u.words[3] = le32_to_cpu(tmp.words[0]);
	return u;
}
// TODO: End of code from types.c

static void wdc_print_smart_cloud_attr_C0_normal(void *data)
{
	__u8 *log_data = (__u8 *)data;
	uint16_t smart_log_ver = 0;

	printf("  SMART Cloud Attributes :-\n");

	printf("  Physical media units written			: %s\n",
	       uint128_t_to_string(le128_to_cpu(&log_data[SCAO_PMUW])));
	printf("  Physical media units read			: %s\n",
	       uint128_t_to_string(le128_to_cpu(&log_data[SCAO_PMUR])));
	printf("  Bad user nand blocks Raw			: %"PRIu64"\n",
	       (uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_BUNBR] & 0x0000FFFFFFFFFFFF));
	printf("  Bad user nand blocks Normalized		: %d\n",
	       (uint16_t)le16_to_cpu(*(uint16_t *)&log_data[SCAO_BUNBN]));
	printf("  Bad system nand blocks Raw			: %"PRIu64"\n",
	       (uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_BSNBR] & 0x0000FFFFFFFFFFFF));
	printf("  Bad system nand blocks Normalized		: %d\n",
	       (uint16_t)le16_to_cpu(*(uint16_t *)&log_data[SCAO_BSNBN]));
	printf("  XOR recovery count				: %"PRIu64"\n",
	       (uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_XRC]));
	printf("  Uncorrectable read error count		: %"PRIu64"\n",
	       (uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_UREC]));
	printf("  Soft ecc error count				: %"PRIu64"\n",
	       (uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_SEEC]));
	printf("  End to end corrected errors			: %"PRIu32"\n",
	       (uint32_t)le32_to_cpu(*(uint32_t *)&log_data[SCAO_EECE]));
	printf("  End to end detected errors			: %"PRIu32"\n",
	       (uint32_t)le32_to_cpu(*(uint32_t *)&log_data[SCAO_EEDC]));
	printf("  System data percent used			: %d\n", (__u8)log_data[SCAO_SDPU]);
	printf("  Refresh counts				: %"PRIu64"\n",
	       (uint64_t)(le64_to_cpu(*(uint64_t *)&log_data[SCAO_RFSC]) & 0x00FFFFFFFFFFFFFF));
	printf("  Max User data erase counts			: %"PRIu32"\n",
	       (uint32_t)le32_to_cpu(*(uint32_t *)&log_data[SCAO_MXUDEC]));
	printf("  Min User data erase counts			: %"PRIu32"\n",
	       (uint32_t)le32_to_cpu(*(uint32_t *)&log_data[SCAO_MNUDEC]));
	printf("  Number of Thermal throttling events		: %d\n", (__u8)log_data[SCAO_NTTE]);
	printf("  Current throttling status			: 0x%x\n", (__u8)log_data[SCAO_CTS]);
	printf("  PCIe correctable error count			: %"PRIu64"\n",
	       (uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_PCEC]));
	printf("  Incomplete shutdowns				: %"PRIu32"\n",
	       (uint32_t)le32_to_cpu(*(uint32_t *)&log_data[SCAO_ICS]));
	printf("  Percent free blocks				: %d\n", (__u8)log_data[SCAO_PFB]);
	printf("  Capacitor health				: %"PRIu16"\n",
	       (uint16_t)le16_to_cpu(*(uint16_t *)&log_data[SCAO_CPH]));
	printf("  Unaligned I/O					: %"PRIu64"\n",
	       (uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_UIO]));
	printf("  Securqity Version Number			: %"PRIu64"\n",
	       (uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_SVN]));
	printf("  NUSE Namespace utilization			: %"PRIu64"\n",
	       (uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_NUSE]));
	printf("  PLP start count				: %s\n",
	       uint128_t_to_string(le128_to_cpu(&log_data[SCAO_PSC])));
	printf("  Endurance estimate				: %s\n",
	       uint128_t_to_string(le128_to_cpu(&log_data[SCAO_EEST])));
	smart_log_ver = (uint16_t)le16_to_cpu(*(uint16_t *)&log_data[SCAO_LPV]);
	printf("  Log page version				: %"PRIu16"\n", smart_log_ver);
	printf("  Log page GUID					: 0x");
	printf("%"PRIx64"%"PRIx64"\n",
	       (uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_LPG + 8]),
	       (uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_LPG]));
	if (smart_log_ver > 2) {
		printf("  Errata Version Field				: %d\n",
		       (__u8)log_data[SCAO_EVF]);
		printf("  Point Version Field				: %"PRIu16"\n",
		       (uint16_t)log_data[SCAO_PVF]);
		printf("  Minor Version Field				: %"PRIu16"\n",
		       (uint16_t)log_data[SCAO_MIVF]);
		printf("  Major Version Field				: %d\n",
		       (__u8)log_data[SCAO_MAVF]);
		printf("  NVMe Errata Version				: %d\n",
		       (__u8)log_data[SCAO_NEV]);
		printf("  PCIe Link Retraining Count			: %"PRIu64"\n",
		       (uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_PLRC]));
	}
	if (smart_log_ver > 3) {
		printf("  Power State Change Count				: %"PRIu64"\n",
		       (uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_PSCC]));
	}
	printf("\n");
}

// static void wdc_print_smart_cloud_attr_C0_json(void *data)
// {
// 	__u8 *log_data = (__u8 *)data;
// 	struct json_object *root = json_create_object();
// 	uint16_t smart_log_ver = 0;

// 	json_object_add_value_uint(root, "Physical media units written",
// 			le128_to_cpu(&log_data[SCAO_PMUW]));
// 	json_object_add_value_uint(root, "Physical media units read",
// 			le128_to_cpu(&log_data[SCAO_PMUR]));
// 	json_object_add_value_uint(root, "Bad user nand blocks - Raw",
// 			(uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_BUNBR] & 0x0000FFFFFFFFFFFF));
// 	json_object_add_value_uint(root, "Bad user nand blocks - Normalized",
// 			(uint16_t)le16_to_cpu(*(uint16_t *)&log_data[SCAO_BUNBN]));
// 	json_object_add_value_uint(root, "Bad system nand blocks - Raw",
// 			(uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_BSNBR] & 0x0000FFFFFFFFFFFF));
// 	json_object_add_value_uint(root, "Bad system nand blocks - Normalized",
// 			(uint16_t)le16_to_cpu(*(uint16_t *)&log_data[SCAO_BSNBN]));
// 	json_object_add_value_uint(root, "XOR recovery count",
// 			(uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_XRC]));
// 	json_object_add_value_uint(root, "Uncorrectable read error count",
// 			(uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_UREC]));
// 	json_object_add_value_uint(root, "Soft ecc error count",
// 			(uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_SEEC]));
// 	json_object_add_value_uint(root, "End to end corrected errors",
// 			(uint32_t)le32_to_cpu(*(uint32_t *)&log_data[SCAO_EECE]));
// 	json_object_add_value_uint(root, "End to end detected errors",
// 			(uint32_t)le32_to_cpu(*(uint32_t *)&log_data[SCAO_EEDC]));
// 	json_object_add_value_uint(root, "System data percent used",
// 			(__u8)log_data[SCAO_SDPU]);
// 	json_object_add_value_uint(root, "Refresh counts",
// 	    (uint64_t)(le64_to_cpu(*(uint64_t *)&log_data[SCAO_RFSC]) & 0x00FFFFFFFFFFFFFF));
// 	json_object_add_value_uint(root, "Max User data erase counts",
// 			(uint32_t)le32_to_cpu(*(uint32_t *)&log_data[SCAO_MXUDEC]));
// 	json_object_add_value_uint(root, "Min User data erase counts",
// 			(uint32_t)le32_to_cpu(*(uint32_t *)&log_data[SCAO_MNUDEC]));
// 	json_object_add_value_uint(root, "Number of Thermal throttling events",
// 			(__u8)log_data[SCAO_NTTE]);
// 	json_object_add_value_uint(root, "Current throttling status",
// 			(__u8)log_data[SCAO_CTS]);
// 	json_object_add_value_uint(root, "PCIe correctable error count",
// 			(uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_PCEC]));
// 	json_object_add_value_uint(root, "Incomplete shutdowns",
// 			(uint32_t)le32_to_cpu(*(uint32_t *)&log_data[SCAO_ICS]));
// 	json_object_add_value_uint(root, "Percent free blocks",
// 			(__u8)log_data[SCAO_PFB]);
// 	json_object_add_value_uint(root, "Capacitor health",
// 			(uint16_t)le16_to_cpu(*(uint16_t *)&log_data[SCAO_CPH]));
// 	json_object_add_value_uint(root, "Unaligned I/O",
// 			(uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_UIO]));
// 	json_object_add_value_uint(root, "Security Version Number",
// 			(uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_SVN]));
// 	json_object_add_value_uint(root, "NUSE - Namespace utilization",
// 			(uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_NUSE]));
// 	json_object_add_value_uint(root, "PLP start count",
// 			le128_to_cpu(&log_data[SCAO_PSC]));
// 	json_object_add_value_uint(root, "Endurance estimate",
// 			le128_to_cpu(&log_data[SCAO_EEST]));
// 	smart_log_ver = (uint16_t)le16_to_cpu(*(uint16_t *)&log_data[SCAO_LPV]);
// 	json_object_add_value_uint(root, "Log page version", smart_log_ver);
// 	char guid[40];

// 	memset((void *)guid, 0, 40);
// 	sprintf((char *)guid, "0x%"PRIx64"%"PRIx64"",
// 		(uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_LPG + 8]),
// 		(uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_LPG]));
// 	json_object_add_value_string(root, "Log page GUID", guid);
// 	if (smart_log_ver > 2) {
// 		json_object_add_value_uint(root, "Errata Version Field",
// 				(__u8)log_data[SCAO_EVF]);
// 		json_object_add_value_uint(root, "Point Version Field",
// 				(uint16_t)log_data[SCAO_PVF]);
// 		json_object_add_value_uint(root, "Minor Version Field",
// 				(uint16_t)log_data[SCAO_MIVF]);
// 		json_object_add_value_uint(root, "Major Version Field",
// 				(__u8)log_data[SCAO_MAVF]);
// 		json_object_add_value_uint(root, "NVMe Errata Version",
// 				(__u8)log_data[SCAO_NEV]);
// 		json_object_add_value_uint(root, "PCIe Link Retraining Count",
// 				(uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_PLRC]));
// 	}
// 	if (smart_log_ver > 3) {
// 		json_object_add_value_uint(root, "Power State Change Count",
// 				(uint64_t)le64_to_cpu(*(uint64_t *)&log_data[SCAO_PSCC]));
// 	}
// 	json_print_object(root, NULL);
// 	printf("\n");
// 	json_free_object(root);
// }

static int wdc_print_c0_cloud_attr_log(void *data, int fmt)
{
	if (!data) {
		fprintf(stderr, "ERROR: WDC: Invalid buffer to read 0xC0 log\n");
		return -1;
	}
	switch (fmt) {
	case NORMAL:
		wdc_print_smart_cloud_attr_C0_normal(data);
		break;
	case JSON:
		// wdc_print_smart_cloud_attr_C0_json(data);
		fprintf(stderr, "ERROR: JSON output format support not backported \n");
		break;
	}
	return 0;
}

static int nvme_get_ext_smart_cloud_log(int fd, __u8 **data, int uuid_index, __u32 namespace_id)
{
	int ret, i;
	__u8 *log_ptr = NULL;

	log_ptr = (__u8 *)malloc(sizeof(__u8) * WDC_NVME_SMART_CLOUD_ATTR_LEN);
	if (!log_ptr) {
		fprintf(stderr, "ERROR: WDC: malloc: %s\n", strerror(errno));
		return -1;
	}

	/* Get the 0xC0 log data */
	
	/* Get the 0xC0 log data */
	ret = nvme_get_log(fd, namespace_id, 
		WDC_NVME_GET_SMART_CLOUD_ATTR_LOG_ID, uuid_index, 
		false, WDC_NVME_SMART_CLOUD_ATTR_LEN, data);

	if (!ret) {
		/* Verify GUID matches */
		for (i = 0; i < WDC_C0_GUID_LENGTH; i++) {
			if (ext_smart_guid[i] != *&log_ptr[SCAO_V1_LPG + i]) {
				fprintf(stderr, "ERROR: WDC: Unknown GUID in C0 Log Page V1 data\n");
				int j;

				fprintf(stderr, "ERROR: WDC: Expected GUID:  0x");
				for (j = 0; j < WDC_C0_GUID_LENGTH; j++)
					fprintf(stderr, "%x", ext_smart_guid[j]);
				fprintf(stderr, "\nERROR: WDC: Actual GUID:    0x");
				for (j = 0; j < WDC_C0_GUID_LENGTH; j++)
					fprintf(stderr, "%x", *&log_ptr[SCAO_V1_LPG + j]);
				fprintf(stderr, "\n");

				ret = -1;
				break;
			}
		}
	}

	*data = log_ptr;

	return ret;
}


static void wdc_print_ext_smart_cloud_log_normal(void *data, int mask)
{
	int i;
	struct __attribute__((__packed__)) wdc_nvme_ext_smart_log * 
		ext_smart_log_ptr = (struct __attribute__((__packed__)) 
		wdc_nvme_ext_smart_log *)data;

	if (mask == WDC_SCA_V1_NAND_STATS)
		printf("  NAND Statistics :-\n");
	else
		printf("  SMART Cloud Attributes :-\n");

	printf("  Physical Media Units Written TLC (Bytes): %s\n",
		uint128_t_to_string(le128_to_cpu(
					ext_smart_log_ptr->ext_smart_pmuwt)));
	printf("  Physical Media Units Written SLC (Bytes): %s\n",
		uint128_t_to_string(le128_to_cpu(
					ext_smart_log_ptr->ext_smart_pmuws)));
	printf("  Bad User NAND Block Count (Normalized) (Int) : %d\n",
			le16_to_cpu(*(uint16_t *)ext_smart_log_ptr->ext_smart_bunbc));
	printf("  Bad User NAND Block Count (Raw) (Int)	: %"PRIu64"\n",
			le64_to_cpu(*(uint64_t *)ext_smart_log_ptr->ext_smart_bunbc & 0xFFFFFFFFFFFF0000));
	printf("  XOR Recovery Count (Int) : %"PRIu64"\n",
			le64_to_cpu(ext_smart_log_ptr->ext_smart_xrc));
	printf("  Uncorrectable Read Error Count (Int) : %"PRIu64"\n",
			le64_to_cpu(ext_smart_log_ptr->ext_smart_urec));
	if (mask == WDC_SCA_V1_ALL) {
		printf("  SSD End to End correction counts (Corrected Errors) (Int) : %"PRIu64"\n",
			le64_to_cpu(ext_smart_log_ptr->ext_smart_eece));
		printf("  SSD End to End correction counts (Detected Errors) (Int) : %"PRIu64"\n",
			le64_to_cpu(ext_smart_log_ptr->ext_smart_eede));
		printf("  SSD End to End correction counts (Uncorrected E2E Errors) (Int) : %"PRIu64"\n",
			le64_to_cpu(ext_smart_log_ptr->ext_smart_eeue));
		printf("  System Data %% life-used : %d %%\n",
			ext_smart_log_ptr->ext_smart_sdpu);
	}
	printf("  User data erase counts (Minimum TLC) (Int) : %"PRIu64"\n",
			le64_to_cpu(ext_smart_log_ptr->ext_smart_mnudec));
	printf("  User data erase counts (Maximum TLC) (Int) : %"PRIu64"\n",
			le64_to_cpu(ext_smart_log_ptr->ext_smart_mxudec));
	printf("  User data erase counts (Minimum SLC) (Int) : %"PRIu64"\n",
			le64_to_cpu(ext_smart_log_ptr->ext_smart_mnec));
	printf("  User data erase counts (Maximum SLC) (Int) : %"PRIu64"\n",
			le64_to_cpu(ext_smart_log_ptr->ext_smart_mxec));
	printf("  User data erase counts (Average SLC) (Int) : %"PRIu64"\n",
			le64_to_cpu(ext_smart_log_ptr->ext_smart_avec));
	printf("  User data erase counts (Average TLC) (Int) : %"PRIu64"\n",
			le64_to_cpu(ext_smart_log_ptr->ext_smart_avudec));
	printf("  Program Fail Count (Normalized) (Int) : %d\n",
			le16_to_cpu(*(uint16_t *)ext_smart_log_ptr->ext_smart_pfc));
	printf("  Program Fail Count (Raw) (Int) : %"PRIu64"\n",
			le64_to_cpu(*(uint64_t *)ext_smart_log_ptr->ext_smart_pfc & 0xFFFFFFFFFFFF0000));
	printf("  Erase Fail Count (Normalized) (Int) : %d\n",
			le16_to_cpu(*(uint16_t *)ext_smart_log_ptr->ext_smart_efc));
	printf("  Erase Fail Count (Raw) (Int) : %"PRIu64"\n",
			le64_to_cpu(*(uint64_t *)ext_smart_log_ptr->ext_smart_efc & 0xFFFFFFFFFFFF0000));
	if (mask == WDC_SCA_V1_ALL) {
		printf("  PCIe Correctable Error Count (Int) : %"PRIu64"\n",
			le64_to_cpu(ext_smart_log_ptr->ext_smart_pcec));
		printf("  %% Free Blocks (User) (Int) : %d %%\n",
			ext_smart_log_ptr->ext_smart_pfbu);
		printf("  Security Version Number (Int) : %"PRIu64"\n",
			le64_to_cpu(ext_smart_log_ptr->ext_smart_svn));
		printf("  %% Free Blocks (System) (Int)	: %d %%\n",
			ext_smart_log_ptr->ext_smart_pfbs);
		printf("  NVMe Stats (# Data Set Management/TRIM Commands Completed) (Int): %s\n",
			uint128_t_to_string(le128_to_cpu(
						ext_smart_log_ptr->ext_smart_dcc)));
		printf("  Total Namespace Utilization (nvme0n1 NUSE) (Bytes) : %"PRIu64"\n",
			le64_to_cpu(ext_smart_log_ptr->ext_smart_tnu));
		printf("  NVMe Stats (# NVMe Format Commands Completed) (Int) : %d\n",
			le16_to_cpu(ext_smart_log_ptr->ext_smart_fcc));
		printf("  Background Back-Pressure Gauge(%%) (Int) : %d\n",
			ext_smart_log_ptr->ext_smart_bbpg);
	}
	printf("  Total # of Soft ECC Error Count (Int)	: %"PRIu64"\n",
			le64_to_cpu(ext_smart_log_ptr->ext_smart_seec));
	if (mask == WDC_SCA_V1_ALL) {
		printf("  Total # of Read Refresh Count (Int) : %"PRIu64"\n",
			le64_to_cpu(ext_smart_log_ptr->ext_smart_rfsc));
	}
	printf("  Bad System NAND Block Count (Normalized) (Int) : %d\n",
			le16_to_cpu(*(uint16_t *)ext_smart_log_ptr->ext_smart_bsnbc));
	printf("  Bad System NAND Block Count (Raw) (Int) : %"PRIu64"\n",
			le64_to_cpu(*(uint64_t *)ext_smart_log_ptr->ext_smart_bsnbc & 0xFFFFFFFFFFFF0000));
	printf("  Endurance Estimate (Total Writable Lifetime Bytes) (Bytes) :  %s\n",
		uint128_t_to_string(
			le128_to_cpu(ext_smart_log_ptr->ext_smart_eest)));
	if (mask == WDC_SCA_V1_ALL) {
		printf("  Thermal Throttling Status & Count (Number of thermal throttling events) (Int)	: %d\n",
			le16_to_cpu(ext_smart_log_ptr->ext_smart_ttc));
		printf("  Total # Unaligned I/O (Int) : %"PRIu64"\n",
			le64_to_cpu(ext_smart_log_ptr->ext_smart_uio));
	}
	printf("  Total Physical Media Units Read (Bytes) (Int)	:  %s\n",
		uint128_t_to_string(
			le128_to_cpu(ext_smart_log_ptr->ext_smart_pmur)));
	if (mask == WDC_SCA_V1_ALL) {
		printf("  Command Timeout (# of READ Commands > 5 Seconds) (Int) : %"PRIu32"\n",
			le32_to_cpu(ext_smart_log_ptr->ext_smart_rtoc));
		printf("  Command Timeout (# of WRITE Commands > 5 Seconds) (Int) : %"PRIu32"\n",
			le32_to_cpu(ext_smart_log_ptr->ext_smart_wtoc));
		printf("  Command Timeout (# of TRIM Commands > 5 Seconds) (Int) : %"PRIu32"\n",
			le32_to_cpu(ext_smart_log_ptr->ext_smart_ttoc));
		printf("  Total PCIe Link Retraining Count (Int) : %"PRIu64"\n",
			le64_to_cpu(ext_smart_log_ptr->ext_smart_plrc));
		printf("  Active Power State Change Count (Int)	: %"PRIu64"\n",
			le64_to_cpu(ext_smart_log_ptr->ext_smart_pscc));
	}
	printf("  Cloud Boot SSD Spec Version (Int) : %d.%d.%d.%d\n",
			le16_to_cpu(ext_smart_log_ptr->ext_smart_maj),
			le16_to_cpu(ext_smart_log_ptr->ext_smart_min),
			le16_to_cpu(ext_smart_log_ptr->ext_smart_pt),
			le16_to_cpu(ext_smart_log_ptr->ext_smart_err));
	printf("  Cloud Boot SSD HW Revision (Int) : %d.%d.%d.%d\n",
			0, 0, 0, 0);
	if (mask == WDC_SCA_V1_ALL) {
		printf("  FTL Unit Size	: %"PRIu32"\n",
			le32_to_cpu(ext_smart_log_ptr->ext_smart_ftlus));
		printf("  TCG Ownership Status : %"PRIu32"\n",
			le32_to_cpu(ext_smart_log_ptr->ext_smart_tcgos));
		printf("  Log Page Version (Int) : %d\n",
			le16_to_cpu(ext_smart_log_ptr->ext_smart_lpv));
		printf("  Log page GUID	(Hex) : 0x");
		for (i = WDC_C0_GUID_LENGTH; i > 0; i--)
			printf("%02x", ext_smart_log_ptr->ext_smart_lpg[i-1]);
		printf("\n");
	}
	printf("\n");
}

// static void wdc_print_ext_smart_cloud_log_json(void *data, int mask)
// {
// 	struct __packed wdc_nvme_ext_smart_log * ext_smart_log_ptr =
// 		(struct __packed wdc_nvme_ext_smart_log *)data;
// 	struct json_object *root = json_create_object();

// 	json_object_add_value_uint128(root, "physical_media_units_bytes_tlc",
// 				      le128_to_cpu(ext_smart_log_ptr->ext_smart_pmuwt));
// 	json_object_add_value_uint128(root, "physical_media_units_bytes_slc",
// 				      le128_to_cpu(ext_smart_log_ptr->ext_smart_pmuws));
// 	json_object_add_value_uint(root, "bad_user_blocks_normalized",
// 				   le16_to_cpu(*(uint16_t *)ext_smart_log_ptr->ext_smart_bunbc));
// 	json_object_add_value_uint64(root, "bad_user_blocks_raw",
// 	    le64_to_cpu(*(uint64_t *)ext_smart_log_ptr->ext_smart_bunbc & 0xFFFFFFFFFFFF0000));
// 	json_object_add_value_uint64(root, "xor_recovery_count",
// 				     le64_to_cpu(ext_smart_log_ptr->ext_smart_xrc));
// 	json_object_add_value_uint64(root, "uncorrectable_read_errors",
// 				     le64_to_cpu(ext_smart_log_ptr->ext_smart_urec));
// 	if (mask == WDC_SCA_V1_ALL) {
// 		json_object_add_value_uint64(root, "corrected_e2e_errors",
// 					     le64_to_cpu(ext_smart_log_ptr->ext_smart_eece));
// 		json_object_add_value_uint64(root, "detected_e2e_errors",
// 					     le64_to_cpu(ext_smart_log_ptr->ext_smart_eede));
// 		json_object_add_value_uint64(root, "uncorrected_e2e_errors",
// 					     le64_to_cpu(ext_smart_log_ptr->ext_smart_eeue));
// 		json_object_add_value_uint(root, "system_data_life_used_pct",
// 					   (__u8)ext_smart_log_ptr->ext_smart_sdpu);
// 	}
// 	json_object_add_value_uint64(root, "min_slc_user_data_erase_count",
// 				     le64_to_cpu(ext_smart_log_ptr->ext_smart_mnec));
// 	json_object_add_value_uint64(root, "min_tlc_user_data_erase_count",
// 				     le64_to_cpu(ext_smart_log_ptr->ext_smart_mnudec));
// 	json_object_add_value_uint64(root, "max_slc_user_data_erase_count",
// 				     le64_to_cpu(ext_smart_log_ptr->ext_smart_mxec));
// 	json_object_add_value_uint64(root, "max_tlc_user_data_erase_count",
// 				     le64_to_cpu(ext_smart_log_ptr->ext_smart_mxudec));
// 	json_object_add_value_uint64(root, "avg_slc_user_data_erase_count",
// 				     le64_to_cpu(ext_smart_log_ptr->ext_smart_avec));
// 	json_object_add_value_uint64(root, "avg_tlc_user_data_erase_count",
// 				     le64_to_cpu(ext_smart_log_ptr->ext_smart_avudec));
// 	json_object_add_value_uint(root, "program_fail_count_normalized",
// 				   le16_to_cpu(*(uint16_t *)ext_smart_log_ptr->ext_smart_pfc));
// 	json_object_add_value_uint64(root, "program_fail_count_raw",
// 	    le64_to_cpu(*(uint64_t *)ext_smart_log_ptr->ext_smart_pfc & 0xFFFFFFFFFFFF0000));
// 	json_object_add_value_uint(root, "erase_fail_count_normalized",
// 				   le16_to_cpu(*(uint16_t *)ext_smart_log_ptr->ext_smart_efc));
// 	json_object_add_value_uint64(root, "erase_fail_count_raw",
// 	    le64_to_cpu(*(uint64_t *)ext_smart_log_ptr->ext_smart_efc & 0xFFFFFFFFFFFF0000));
// 	if (mask == WDC_SCA_V1_ALL) {
// 		json_object_add_value_uint64(root, "pcie_correctable_errors",
// 					   le64_to_cpu(ext_smart_log_ptr->ext_smart_pcec));
// 		json_object_add_value_uint(root, "pct_free_blocks_user",
// 					   (__u8)ext_smart_log_ptr->ext_smart_pfbu);
// 		json_object_add_value_uint64(root, "security_version",
// 					     le64_to_cpu(ext_smart_log_ptr->ext_smart_svn));
// 		json_object_add_value_uint(root, "pct_free_blocks_system",
// 					   (__u8)ext_smart_log_ptr->ext_smart_pfbs);
// 		json_object_add_value_uint128(root, "num_of_trim_commands",
// 					      le128_to_cpu(ext_smart_log_ptr->ext_smart_dcc));
// 		json_object_add_value_uint64(root, "total_nuse_bytes",
// 					   le64_to_cpu(ext_smart_log_ptr->ext_smart_tnu));
// 		json_object_add_value_uint(root, "num_of_format_commands",
// 					   le16_to_cpu(ext_smart_log_ptr->ext_smart_fcc));
// 		json_object_add_value_uint(root, "background_pressure_gauge",
// 					   (__u8)ext_smart_log_ptr->ext_smart_bbpg);
// 	}
// 	json_object_add_value_uint64(root, "soft_ecc_error_count",
// 				     le64_to_cpu(ext_smart_log_ptr->ext_smart_seec));
// 	if (mask == WDC_SCA_V1_ALL)
// 		json_object_add_value_uint64(root, "read_refresh_count",
// 					     le64_to_cpu(ext_smart_log_ptr->ext_smart_rfsc));
// 	json_object_add_value_uint(root, "bad_system_block_normalized",
// 				      le16_to_cpu(*(uint16_t *)ext_smart_log_ptr->ext_smart_bsnbc));
// 	json_object_add_value_uint64(root, "bad_system_block_raw",
// 	    le64_to_cpu(*(uint64_t *)ext_smart_log_ptr->ext_smart_bsnbc & 0xFFFFFFFFFFFF0000));
// 	json_object_add_value_uint128(root, "endurance_est_bytes",
// 				      le128_to_cpu(ext_smart_log_ptr->ext_smart_eest));
// 	if (mask == WDC_SCA_V1_ALL) {
// 		json_object_add_value_uint(root, "num_throttling_events",
// 					   le16_to_cpu(ext_smart_log_ptr->ext_smart_ttc));
// 		json_object_add_value_uint64(root, "total_unaligned_io",
// 					     le64_to_cpu(ext_smart_log_ptr->ext_smart_uio));
// 	}
// 	json_object_add_value_uint128(root, "physical_media_units_read_bytes",
// 				      le128_to_cpu(ext_smart_log_ptr->ext_smart_pmur));
// 	if (mask == WDC_SCA_V1_ALL) {
// 		json_object_add_value_uint(root, "num_read_timeouts",
// 					   le32_to_cpu(ext_smart_log_ptr->ext_smart_rtoc));
// 		json_object_add_value_uint(root, "num_write_timeouts",
// 					   le32_to_cpu(ext_smart_log_ptr->ext_smart_wtoc));
// 		json_object_add_value_uint(root, "num_trim_timeouts",
// 					   le32_to_cpu(ext_smart_log_ptr->ext_smart_ttoc));
// 		json_object_add_value_uint64(root, "pcie_link_retrain_count",
// 					   le64_to_cpu(ext_smart_log_ptr->ext_smart_plrc));
// 		json_object_add_value_uint64(root, "active_power_state_change_count",
// 					   le64_to_cpu(ext_smart_log_ptr->ext_smart_pscc));
// 	}
// 	char vers_str[40];

// 	memset((void *)vers_str, 0, 40);
// 	sprintf((char *)vers_str, "%d.%d.%d.%d",
// 		le16_to_cpu(ext_smart_log_ptr->ext_smart_maj),
// 		le16_to_cpu(ext_smart_log_ptr->ext_smart_min),
// 		le16_to_cpu(ext_smart_log_ptr->ext_smart_pt),
// 		le16_to_cpu(ext_smart_log_ptr->ext_smart_err));
// 	json_object_add_value_string(root, "cloud_boot_ssd_spec_ver", vers_str);
// 	memset((void *)vers_str, 0, 40);
// 	sprintf((char *)vers_str, "%d.%d.%d.%d", 0, 0, 0, 0);
// 	json_object_add_value_string(root, "cloud_boot_ssd_hw_ver", vers_str);

// 	if (mask == WDC_SCA_V1_ALL) {
// 		json_object_add_value_uint(root, "ftl_unit_size",
// 					   le32_to_cpu(ext_smart_log_ptr->ext_smart_ftlus));
// 		json_object_add_value_uint(root, "tcg_ownership_status",
// 					   le32_to_cpu(ext_smart_log_ptr->ext_smart_tcgos));
// 		json_object_add_value_uint(root, "log_page_ver",
// 					   le16_to_cpu(ext_smart_log_ptr->ext_smart_lpv));
// 		char guid[40];

// 		memset((void *)guid, 0, 40);
// 		sprintf((char *)guid, "0x%"PRIx64"%"PRIx64"",
// 			le64_to_cpu(*(uint64_t *)&ext_smart_log_ptr->ext_smart_lpg[8]),
// 			le64_to_cpu(*(uint64_t *)&ext_smart_log_ptr->ext_smart_lpg[0]));
// 		json_object_add_value_string(root, "log_page_guid", guid);
// 	}

// 	json_print_object(root, NULL);
// 	printf("\n");
// 	json_free_object(root);
// }

static int wdc_print_ext_smart_cloud_log(void *data, int fmt)
{
	if (!data) {
		fprintf(stderr, "ERROR: WDC: Invalid buffer to read 0xC0 V1 log\n");
		return -1;
	}
	switch (fmt) {
	case NORMAL:
		wdc_print_ext_smart_cloud_log_normal(data, WDC_SCA_V1_ALL);
		break;
	// case JSON:
	// 	wdc_print_ext_smart_cloud_log_json(data, WDC_SCA_V1_ALL);
	// 	break;
	}
	return 0;
}

static int wdc_get_c0_log_page(int fd, char *format, int uuid_index,
			       __u32 namespace_id)
{
	int ret = 0;
	int fmt = -1;
	__u8 *data;
	uint32_t device_id, read_vendor_id;

	if (!wdc_check_device(fd))
		return -1;
	fmt = validate_output_format(format);
	if (fmt < 0) {
		fprintf(stderr, "ERROR: WDC: invalid output format\n");
		return fmt;
	}

	ret = wdc_get_pci_ids(&device_id, &read_vendor_id);

	switch (device_id) {
	case WDC_NVME_SN640_DEV_ID:
	case WDC_NVME_SN640_DEV_ID_1:
	case WDC_NVME_SN640_DEV_ID_2:
	case WDC_NVME_SN640_DEV_ID_3:
	case WDC_NVME_SN840_DEV_ID:
	case WDC_NVME_SN840_DEV_ID_1:
	case WDC_NVME_SN860_DEV_ID:
	case WDC_NVME_SN650_DEV_ID:
	case WDC_NVME_SN650_DEV_ID_1:
	case WDC_NVME_SN650_DEV_ID_2:
	case WDC_NVME_SN650_DEV_ID_3:
	case WDC_NVME_SN650_DEV_ID_4:
	case WDC_NVME_SN655_DEV_ID:
	case WDC_NVME_SN560_DEV_ID_1:
	case WDC_NVME_SN560_DEV_ID_2:
	case WDC_NVME_SN560_DEV_ID_3:
	case WDC_NVME_SN550_DEV_ID:
		ret = wdc_get_c0_log_page_sn(fd, uuid_index, format, namespace_id, fmt);
		break;
	case WDC_NVME_ZN350_DEV_ID:
	case WDC_NVME_ZN350_DEV_ID_1:
		data = (__u8 *)malloc(sizeof(__u8) * WDC_NVME_SMART_CLOUD_ATTR_LEN);
		if (!data) {
			fprintf(stderr, "ERROR: WDC: malloc: %s\n", strerror(errno));
			return -1;
		}

		/* Get the 0xC0 log data */
		ret = nvme_get_log(fd, namespace_id, 
			WDC_NVME_GET_SMART_CLOUD_ATTR_LOG_ID, uuid_index, 
			false, WDC_NVME_SMART_CLOUD_ATTR_LEN, data);

		if (!ret) {
			/* parse the data */
			wdc_print_c0_cloud_attr_log(data, fmt);
		} else {
			fprintf(stderr, "ERROR: WDC: Unable to read C0 Log Page data\n");
			ret = -1;
		}

		free(data);
		break;
	case WDC_NVME_SN820CL_DEV_ID:
		/* Get the 0xC0 Extended Smart Cloud Attribute log data */
		data = NULL;
		ret = nvme_get_ext_smart_cloud_log(fd, &data,
						   uuid_index, namespace_id);

		if (!ret) {
			/* parse the data */
			wdc_print_ext_smart_cloud_log(data, fmt);
		} else {
			fprintf(stderr, "ERROR: WDC: Unable to read C0 Log Page V1 data\n");
			ret = -1;
		}

		if (data)
			free(data);
		break;
	default:
		fprintf(stderr, "ERROR: WDC: Unknown device id - 0x%x\n", device_id);
		ret = -1;
		break;

	}

	return ret;
}

static int wdc_vs_smart_add_log(int argc, char **argv, struct command *command,
		struct plugin *plugin)
{
	const char *desc = "Retrieve additional performance statistics.";
	const char *interval = "Interval to read the statistics from [1, 15].";
	const char *log_page_version = "Log Page Version: 0 = vendor, 1 = WDC";
	const char *log_page_mask = "Log Page Mask, comma separated list: 0xC1, 0xCA";
	const char *namespace_id = "desired namespace id";
	int fd;
	int uuid_index = 0;
	int ret = 0;
	int page_mask = 0, num, i;
	int log_page_list[16];
	__u64 capabilities = 0;

	struct config {
		uint8_t interval;
		char *output_format;
		__u8  log_page_version;
		char *log_page_mask;
		__u32 namespace_id;
	};

	struct config cfg = {
		.interval = 14,
		.output_format = "normal",
		.log_page_version   = 0,
		.log_page_mask   = "",
		.namespace_id = NVME_NSID_ALL,
	};

	const struct argconfig_commandline_options command_line_options[] = {
		{"interval", 'i', "NUM", CFG_POSITIVE, &cfg.interval, required_argument, interval},
		{"output-format", 'o', "FMT", CFG_STRING, &cfg.output_format, required_argument, "Output Format: normal|json" },
		{"log-page-version", 'l', "NUM", CFG_BYTE, &cfg.log_page_version, required_argument, log_page_version},
		{"log-page-mask", 'p', "LIST", CFG_STRING, &cfg.log_page_mask, required_argument, log_page_mask},
		{"namespace-id", 'n', "NUM", CFG_STRING, &cfg.namespace_id, required_argument, namespace_id},
		{NULL}
	};

	fd = parse_and_open(argc, argv, desc, command_line_options, NULL, 0);
	if (fd < 0)
		return fd;
	if (!cfg.log_page_version) {
		uuid_index = 0;
	} else if (cfg.log_page_version == 1) {
		uuid_index = 1;
	}
	 else {
		fprintf(stderr, "ERROR: WDC: unsupported log page version for this command\n");
		ret = -1;
		goto out;
	}

	num = argconfig_parse_comma_sep_array(cfg.log_page_mask, log_page_list, 16);

	if (num == -1) {
		fprintf(stderr, "ERROR: WDC: log page list is malformed\n");
		ret = -1;
		goto out;
	}

	if (!num) {
		page_mask |= WDC_ALL_PAGE_MASK;
	} else {
		for (i = 0; i < num; i++) {
			if (log_page_list[i] == 0xc0)
				page_mask |= WDC_C0_PAGE_MASK;
			if (log_page_list[i] == 0xc1)
				page_mask |= WDC_C1_PAGE_MASK;
			if (log_page_list[i] == 0xca)
				page_mask |= WDC_CA_PAGE_MASK;
			if (log_page_list[i] == 0xd0)
				page_mask |= WDC_D0_PAGE_MASK;
		}
	}

	if (!page_mask)
		fprintf(stderr, "ERROR: WDC: Unknown log page mask - %s\n", cfg.log_page_mask);

	capabilities = wdc_get_drive_capabilities(fd);

	if (((capabilities & WDC_DRIVE_CAP_C0_LOG_PAGE) == WDC_DRIVE_CAP_C0_LOG_PAGE) &&
	    (page_mask & WDC_C0_PAGE_MASK)) {
		/* Get 0xC0 log page if possible. */
		ret = wdc_get_c0_log_page(fd, cfg.output_format,
					  uuid_index, cfg.namespace_id);
		if (ret)
			fprintf(stderr, "ERROR: WDC: Failure reading the C0 Log Page, ret = %d\n", ret);
	}
	if (((capabilities & (WDC_DRIVE_CAP_CA_LOG_PAGE)) == (WDC_DRIVE_CAP_CA_LOG_PAGE)) &&
	    (page_mask & WDC_CA_PAGE_MASK)) {
		/* Get the CA Log Page */
		ret = wdc_get_ca_log_page(fd, cfg.output_format);
		if (ret)
			fprintf(stderr, "ERROR: WDC: Failure reading the CA Log Page, ret = %d\n", ret);
	}
	if (((capabilities & WDC_DRIVE_CAP_C1_LOG_PAGE) == WDC_DRIVE_CAP_C1_LOG_PAGE) &&
	    (page_mask & WDC_C1_PAGE_MASK)) {
		/* Get the C1 Log Page */
		ret = wdc_get_c1_log_page(fd, cfg.output_format,
					  cfg.interval);
		if (ret)
			fprintf(stderr, "ERROR: WDC: Failure reading the C1 Log Page, ret = %d\n", ret);
	}
	// if (((capabilities & WDC_DRIVE_CAP_D0_LOG_PAGE) == WDC_DRIVE_CAP_D0_LOG_PAGE) &&
	//     (page_mask & WDC_D0_PAGE_MASK)) {
	// 	/* Get the D0 Log Page */
	// 	ret = wdc_get_d0_log_page(fd, cfg.output_format);
	// 	if (ret)
	// 		fprintf(stderr, "ERROR: WDC: Failure reading the D0 Log Page, ret = %d\n", ret);
	// }

out:
	close(fd);
	return ret;
}

bool wdc_get_dev_mng_log_entry(__u32 log_length, __u32 entry_id,
			       struct wdc_c2_log_page_header *p_log_hdr,
			       struct wdc_c2_log_subpage_header **p_p_found_log_entry)
{
	__u32 remaining_len = 0;
	__u32 log_entry_hdr_size = sizeof(struct wdc_c2_log_subpage_header) - 1;
	__u32 log_entry_size = 0;
	__u32 size = 0;
	bool valid_log;
	__u32 current_data_offset = 0;
	struct wdc_c2_log_subpage_header *p_next_log_entry = NULL;

	if (!*p_p_found_log_entry) {
		fprintf(stderr, "ERROR: WDC - %s: No ppLogEntry pointer.\n", __func__);
		return false;
	}

	*p_p_found_log_entry = NULL;

	/* Ensure log data is large enough for common header */
	if (log_length < sizeof(struct wdc_c2_log_page_header)) {
		fprintf(stderr,
		    "ERROR: WDC - %s: Buffer is not large enough for the common header. BufSize: 0x%x  HdrSize: %"PRIxPTR"\n",
		    __func__, log_length, sizeof(struct wdc_c2_log_page_header));
		return false;
	}

	/* Get pointer to first log Entry */
	size = sizeof(struct wdc_c2_log_page_header);
	current_data_offset = size;
	p_next_log_entry = (struct wdc_c2_log_subpage_header *)((__u8 *)p_log_hdr + current_data_offset);
	remaining_len = log_length - size;
	valid_log = false;

	/*
	 * Walk the entire structure. Perform a sanity check to make sure this is a
	 * standard version of the structure. This means making sure each entry looks
	 * valid. But allow for the data to overflow the allocated
	 * buffer (we don't want a false negative because of a FW formatting error)
	 */

	/* Proceed only if there is at least enough data to read an entry header */
	while (remaining_len >= log_entry_hdr_size) {
		/* Get size of the next entry */
		log_entry_size = p_next_log_entry->length;

		/*
		 * If log entry size is 0 or the log entry goes past the end
		 * of the data, we must be at the end of the data
		 */
		if (!log_entry_size || log_entry_size > remaining_len) {
			fprintf(stderr, "ERROR: WDC: %s: Detected unaligned end of the data. ",
				__func__);
			fprintf(stderr, "Data Offset: 0x%x Entry Size: 0x%x, ",
				current_data_offset, log_entry_size);
			fprintf(stderr, "Remaining Log Length: 0x%x Entry Id: 0x%x\n",
				remaining_len, p_next_log_entry->entry_id);

			/* Force the loop to end */
			remaining_len = 0;
		} else if (!p_next_log_entry->entry_id || p_next_log_entry->entry_id > 200) {
			/* Invalid entry - fail the search */
			fprintf(stderr, "ERROR: WDC: %s: Invalid entry found at offset: 0x%x ",
				__func__, current_data_offset);
			fprintf(stderr, "Entry Size: 0x%x, Remaining Log Length: 0x%x ",
				log_entry_size, remaining_len);
			fprintf(stderr, "Entry Id: 0x%x\n", p_next_log_entry->entry_id);

			/* Force the loop to end */
			remaining_len = 0;
			valid_log = false;

			/* The struture is invalid, so any match that was found is invalid. */
			*p_p_found_log_entry = NULL;
		} else {
			/* Structure must have at least one valid entry to be considered valid */
			valid_log = true;
			if (p_next_log_entry->entry_id == entry_id)
				/* A potential match. */
				*p_p_found_log_entry = p_next_log_entry;

			remaining_len -= log_entry_size;

			if (remaining_len > 0) {
				/* Increment the offset counter */
				current_data_offset += log_entry_size;

				/* Get the next entry */
				p_next_log_entry = (struct wdc_c2_log_subpage_header *)(((__u8 *)p_log_hdr) + current_data_offset);
			}
		}
	}

	return valid_log;
}

static bool get_dev_mgment_cbs_data(int fd,
				    __u8 log_id, void **cbs_data)
{
	int ret = -1;
	void* data;
	struct wdc_c2_log_page_header *hdr_ptr;
	struct wdc_c2_log_subpage_header *sph;
	__u32 length = 0;
	bool found = false;
	__u8 uuid_ix = 1;
	__u8 lid = 0;
	*cbs_data = NULL;
	__u32 device_id, read_vendor_id;

	ret = wdc_get_pci_ids(&device_id, &read_vendor_id);
	if (device_id == WDC_NVME_ZN350_DEV_ID || device_id == WDC_NVME_ZN350_DEV_ID_1) {
		lid = WDC_NVME_GET_DEV_MGMNT_LOG_PAGE_OPCODE_C8;
		uuid_ix = 0;
	} else {
		lid = WDC_NVME_GET_DEV_MGMNT_LOG_PAGE_OPCODE;
	}

	data = (__u8 *)malloc(sizeof(__u8) * WDC_C2_LOG_BUF_LEN);
	if (!data) {
		fprintf(stderr, "ERROR: WDC: malloc: %s\n", strerror(errno));
		return false;
	}
	memset(data, 0, sizeof (__u8) * WDC_C2_LOG_BUF_LEN);

	/* get the log page length */
	ret = nvme_get_log(fd, 0xFFFFFFFF, lid, uuid_ix, false, WDC_C2_LOG_BUF_LEN, data);
	if (ret) {
		fprintf(stderr, "ERROR : WDC : Unable to get C2 Log Page length, ret = %d\n", ret);
		goto end;
	}

	hdr_ptr = (struct wdc_c2_log_page_header *)data;
	length = le32_to_cpu(hdr_ptr->length);

	if (length > WDC_C2_LOG_BUF_LEN) {
		/* Log Page buffer too small, free and reallocate the necessary size */
		free(data);
		data = calloc(length, sizeof(__u8));
		if (!data) {
			fprintf(stderr, "ERROR: WDC: malloc: %s\n", strerror(errno));
			return false;
		}
	}

	/* get the log page data */
	ret = nvme_get_log(fd, 0xFFFFFFFF, lid, uuid_ix, false, length, data);
	if (ret) {
		fprintf(stderr, "ERROR: WDC: Unable to read 0x%x Log Page data, ret = 0x%x\n", lid, ret);
		goto end;
	}

	/* Check the log data to see if the WD version of log page ID's is found */

	length = sizeof(struct wdc_c2_log_page_header);
	hdr_ptr = (struct wdc_c2_log_page_header *)data;
	sph = (struct wdc_c2_log_subpage_header *)(data + length);
	found = wdc_get_dev_mng_log_entry(le32_to_cpu(hdr_ptr->length), log_id, hdr_ptr, &sph);
	if (found) {
		*cbs_data = calloc(le32_to_cpu(sph->length), sizeof(__u8));
		if (*cbs_data == NULL) {
			fprintf(stderr, "ERROR : WDC : calloc : %s\n", strerror(errno));
			goto end;
		}
		memcpy((void *)*cbs_data, (void *)&sph->data, le32_to_cpu(sph->length));
	} else {

		/* WD version not found  */
		fprintf(stderr, "ERROR : WDC : Unable to find correct version of page 0x%x, entry id = %d\n", lid, log_id);
	}
end:
	free(data);
	return found;
}

static bool wdc_nvme_get_dev_status_log_data(int fd,
					     __le32 *ret_data,
						 __u8 log_id)
{
	__u32 *cbs_data = NULL;

	if (get_dev_mgment_cbs_data(fd, log_id, (void *)&cbs_data)) {
		if (cbs_data != NULL) {
			memcpy((void *)ret_data, (void *)cbs_data, 4);
			free(cbs_data);

			return true;
		}
	}

	*ret_data = 0;
	return false;
}

static int wdc_drive_status(int argc, char **argv, struct command *command,
		struct plugin *plugin)
{
	char *desc = "Get Drive Status.";
	int ret = 0;
	int fd = 0;

	__le32 system_eol_state;
	__le32 user_eol_state;
	__le32 format_corrupt_reason = cpu_to_le32(0xFFFFFFFF);
	__le32 eol_status;
	__le32 assert_status = cpu_to_le32(0xFFFFFFFF);
	__le32 thermal_status = cpu_to_le32(0xFFFFFFFF);

	const struct argconfig_commandline_options command_line_options[] = {
		{NULL}
	};

	fd = parse_and_open(argc, argv, desc, command_line_options, NULL, 0);
	if (fd < 0)
		return fd;

	/* verify the 0xC2 Device Manageability log page is supported */
	if (wdc_nvme_check_supported_log_page(fd,
					      WDC_NVME_GET_DEV_MGMNT_LOG_PAGE_OPCODE) == false) {
		fprintf(stderr, "ERROR : WDC : 0xC2 Log Page not supported\n");
		ret = -1;
		goto out;
	}

	/* Get the assert dump present status */
	if (!wdc_nvme_get_dev_status_log_data(fd, &assert_status,
			WDC_C2_ASSERT_DUMP_PRESENT_ID))
		fprintf(stderr, "ERROR : WDC : Get Assert Status Failed\n");

	/* Get the thermal throttling status */
	if (!wdc_nvme_get_dev_status_log_data(fd, &thermal_status,
			WDC_C2_THERMAL_THROTTLE_STATUS_ID))
		fprintf(stderr, "ERROR : WDC : Get Thermal Throttling Status Failed\n");

	/* Get EOL status */
	if (!wdc_nvme_get_dev_status_log_data(fd, &eol_status,
			WDC_C2_USER_EOL_STATUS_ID)) {
		fprintf(stderr, "ERROR : WDC : Get User EOL Status Failed\n");
		eol_status = cpu_to_le32(-1);
	}

	/* Get Customer EOL state */
	if (!wdc_nvme_get_dev_status_log_data(fd, &user_eol_state,
			WDC_C2_USER_EOL_STATE_ID))
		fprintf(stderr, "ERROR : WDC : Get User EOL State Failed\n");

	/* Get System EOL state*/
	if (!wdc_nvme_get_dev_status_log_data(fd, &system_eol_state,
			WDC_C2_SYSTEM_EOL_STATE_ID))
		fprintf(stderr, "ERROR : WDC : Get System EOL State Failed\n");

	/* Get format corrupt reason*/
	if (!wdc_nvme_get_dev_status_log_data(fd, &format_corrupt_reason,
			WDC_C2_FORMAT_CORRUPT_REASON_ID))
		fprintf(stderr, "ERROR : WDC : Get Format Corrupt Reason Failed\n");

	printf("  Drive Status :- \n");
	if ((int)le32_to_cpu(eol_status) >= 0) {
		printf("  Percent Used:				%"PRIu32"%%\n",
				le32_to_cpu(eol_status));
	}
	else
		printf("  Percent Used:				Unknown\n");
	if (system_eol_state == WDC_EOL_STATUS_NORMAL && user_eol_state == WDC_EOL_STATUS_NORMAL)
		printf("  Drive Life Status:			Normal\n");
	else if (system_eol_state == WDC_EOL_STATUS_END_OF_LIFE || user_eol_state == WDC_EOL_STATUS_END_OF_LIFE)
		printf("  Drive Life Status:	  		End Of Life\n");
	else if (system_eol_state == WDC_EOL_STATUS_READ_ONLY || user_eol_state == WDC_EOL_STATUS_READ_ONLY)
		printf("  Drive Life Status:	  		Read Only\n");
	else
		printf("  Drive Life Status:			Unknown : 0x%08x/0x%08x\n",
				le32_to_cpu(user_eol_state), le32_to_cpu(system_eol_state));

	if (assert_status == WDC_ASSERT_DUMP_PRESENT)
		printf("  Assert Dump Status:			Present\n");
	else if (assert_status == WDC_ASSERT_DUMP_NOT_PRESENT)
		printf("  Assert Dump Status:			Not Present\n");
	else
		printf("  Assert Dump Status:			Unknown : 0x%08x\n", le32_to_cpu(assert_status));

	if (thermal_status == WDC_THERMAL_THROTTLING_OFF)
		printf("  Thermal Throttling Status:		Off\n");
	else if (thermal_status == WDC_THERMAL_THROTTLING_ON)
		printf("  Thermal Throttling Status:		On\n");
	else if (thermal_status == WDC_THERMAL_THROTTLING_UNAVAILABLE)
		printf("  Thermal Throttling Status:		Unavailable\n");
	else
		printf("  Thermal Throttling Status:		Unknown : 0x%08x\n", le32_to_cpu(thermal_status));

	if (format_corrupt_reason == WDC_FORMAT_NOT_CORRUPT)
		printf("  Format Corrupt Reason:		Format Not Corrupted\n");
	else if (format_corrupt_reason == WDC_FORMAT_CORRUPT_FW_ASSERT)
		printf("  Format Corrupt Reason:	        Format Corrupt due to FW Assert\n");
	else if (format_corrupt_reason == WDC_FORMAT_CORRUPT_UNKNOWN)
		printf("  Format Corrupt Reason:	        Format Corrupt for Unknown Reason\n");
	else
		printf("  Format Corrupt Reason:	        Unknown : 0x%08x\n", le32_to_cpu(format_corrupt_reason));

out:
	return ret;
}


