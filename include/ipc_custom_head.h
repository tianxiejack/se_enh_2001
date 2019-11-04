/*
 * ipc_custom_head.h
 *
 *  Created on: 2019骞?鏈?4鏃?
 *      Author: alex
 */

#ifndef IPC_CUSTOM_HEAD_HPP_
#define IPC_CUSTOM_HEAD_HPP_

#ifdef __cplusplus
extern "C" {
#endif

#include "Ipcctl.h"
#include "stdlib.h"
#include <string.h>

#define CFGID_FEILD_MAX	16
#define CFGID_BLOCK_MAX	128
#define CFGID_USEROSD_MAX	32
#define USEROSD_LENGTH	128

#define DEFAULTPATH "/"

typedef enum 
{
	sensor,  /*0*/
	BoresightPos,
	AcqPos,
	sceneTrk,
	trk,
	mtd,
	mtdSelect,
	sectrk,
	posmove,

	read_shm_config,
	read_shm_block,
	read_shm_single,
	read_shm_usrosd,

	changeSensor,
	trkMtdId,
	settrktime,
	sendMtdcoord,

	mmt,
	mmtselect,

	workmode,
	exit_img,
}CMD_ID;

typedef enum 
{
	IPC_TOIMG_MSG,  		// SERVER TO CLIENT
	IPC_FRIMG_MSG,		// CLIENT TO SERCER
	IPC_IMG_SHA,
	IPC_USER_SHA,
	IPC_SEM,
	IPC_MAX
}IPC_MTYPE;

typedef enum
{
	shm_rdonly,
	shm_rdwr
}shm_perms;

typedef struct
{
	unsigned int x;
	unsigned int y;
}IPC_PIXEL_T;

typedef struct
{
	int chid;
	IPC_PIXEL_T target[5];
}IPC_MTD_COORD_T;


typedef struct
{
	unsigned int intPrm[PARAMLEN/4];
}IPC_PRM_INT;

static IPC_Handl_t * tmpIpc;

static void Ipc_init()
{
	tmpIpc = (IPC_Handl_t*)malloc(IPC_MAX* sizeof(IPC_Handl_t));
	memset(tmpIpc,0,sizeof(IPC_Handl_t)*IPC_MAX);
	char tmp[256] = {"/"};
	memcpy(tmpIpc[IPC_TOIMG_MSG].name,tmp,sizeof(tmp));
	tmpIpc[IPC_TOIMG_MSG].Identify = IPC_TOIMG_MSG;
	tmpIpc[IPC_TOIMG_MSG].Class = IPC_Class_MSG;
	tmpIpc[IPC_TOIMG_MSG].IPCID = IPC_MAX;
	tmpIpc[IPC_TOIMG_MSG].length = 0;
	tmpIpc[IPC_TOIMG_MSG].ptr = NULL;
	
	memcpy(tmpIpc[IPC_FRIMG_MSG].name,tmp,sizeof(tmp));
	tmpIpc[IPC_FRIMG_MSG].Identify = IPC_FRIMG_MSG;
	tmpIpc[IPC_FRIMG_MSG].Class = IPC_Class_MSG;
	tmpIpc[IPC_FRIMG_MSG].IPCID = IPC_MAX;
	tmpIpc[IPC_FRIMG_MSG].length = 0;
	tmpIpc[IPC_FRIMG_MSG].ptr = NULL;
	
	memcpy(tmpIpc[IPC_IMG_SHA].name,tmp,sizeof(tmp));
	tmpIpc[IPC_IMG_SHA].Identify = IPC_IMG_SHA;
	tmpIpc[IPC_IMG_SHA].Class=IPC_Class_SHA;
	tmpIpc[IPC_IMG_SHA].IPCID = IPC_MAX;
	tmpIpc[IPC_IMG_SHA].length = 4*CFGID_FEILD_MAX*CFGID_BLOCK_MAX;
	tmpIpc[IPC_IMG_SHA].RWmode = shm_rdwr;
	tmpIpc[IPC_IMG_SHA].ptr = NULL;

	memcpy(tmpIpc[IPC_USER_SHA].name,tmp,sizeof(tmp));
	tmpIpc[IPC_USER_SHA].Identify = IPC_USER_SHA;
	tmpIpc[IPC_USER_SHA].Class=IPC_Class_SHA;
	tmpIpc[IPC_USER_SHA].IPCID = IPC_MAX;
	tmpIpc[IPC_USER_SHA].length = USEROSD_LENGTH*CFGID_USEROSD_MAX;
	tmpIpc[IPC_USER_SHA].RWmode = shm_rdwr;
	tmpIpc[IPC_USER_SHA].ptr = NULL;

	memcpy(tmpIpc[IPC_SEM].name,tmp,sizeof(tmp));
	tmpIpc[IPC_SEM].Identify = IPC_SEM;
	tmpIpc[IPC_SEM].Class = IPC_Class_SEM;
	tmpIpc[IPC_SEM].IPCID = IPC_MAX;
	tmpIpc[IPC_SEM].ptr = NULL;

	return;
}

//static int *sysConfig = NULL;
//static unsigned char *userConfig = NULL;
static void *ipc_getSharedMem(IPC_MTYPE itype)
{
	if(itype == IPC_IMG_SHA || itype == IPC_USER_SHA)
		return ipc_getSharedAddress(itype);
		//return (void*)IpcHandl[itype].ptr;
	else
		return NULL;
}

#ifdef __cplusplus
}
#endif

#endif /* IPC_CUSTOM_HEAD_HPP_ */
