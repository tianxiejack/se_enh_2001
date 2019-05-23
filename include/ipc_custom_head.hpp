/*
 * ipc_custom_head.hpp
 *
 *  Created on: 2019年3月14日
 *      Author: alex
 */

#ifndef IPC_CUSTOM_HEAD_HPP_
#define IPC_CUSTOM_HEAD_HPP_

#include "Ipcctl.h"

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
	workmode,
	Iris,
	focus,
	exit_IrisAndFocus,
	exit_img,

	read_shm_config,
	read_shm_block,
	read_shm_single,
	read_shm_usrosd,
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
	unsigned int intPrm[PARAMLEN/4];
}IPC_PRM_INT;

static void Ipc_init()
{
	IpcHandl.resize(IPC_MAX);

	IpcHandl[IPC_TOIMG_MSG].name = DEFAULTPATH;
	IpcHandl[IPC_TOIMG_MSG].Identify = IPC_TOIMG_MSG;
	IpcHandl[IPC_TOIMG_MSG].Class = IPC_Class_MSG;
	IpcHandl[IPC_TOIMG_MSG].IPCID = 0;
	IpcHandl[IPC_TOIMG_MSG].length = 0;
	IpcHandl[IPC_TOIMG_MSG].ptr = NULL;
	
	IpcHandl[IPC_FRIMG_MSG].name = DEFAULTPATH;
	IpcHandl[IPC_FRIMG_MSG].Identify = IPC_FRIMG_MSG;
	IpcHandl[IPC_FRIMG_MSG].Class = IPC_Class_MSG;
	IpcHandl[IPC_FRIMG_MSG].IPCID = 0;
	IpcHandl[IPC_FRIMG_MSG].length = 0;
	IpcHandl[IPC_FRIMG_MSG].ptr = NULL;
	
	IpcHandl[IPC_IMG_SHA].name = DEFAULTPATH;
	IpcHandl[IPC_IMG_SHA].Identify = IPC_IMG_SHA;
	IpcHandl[IPC_IMG_SHA].Class=IPC_Class_SHA;
	IpcHandl[IPC_IMG_SHA].IPCID = 0;
	IpcHandl[IPC_IMG_SHA].length = 4*CFGID_FEILD_MAX*CFGID_BLOCK_MAX;
	IpcHandl[IPC_IMG_SHA].RWmode = shm_rdwr;
	IpcHandl[IPC_IMG_SHA].ptr = NULL;

	IpcHandl[IPC_USER_SHA].name = DEFAULTPATH;
	IpcHandl[IPC_USER_SHA].Identify = IPC_USER_SHA;
	IpcHandl[IPC_USER_SHA].Class=IPC_Class_SHA;
	IpcHandl[IPC_USER_SHA].IPCID = 0;
	IpcHandl[IPC_USER_SHA].length = USEROSD_LENGTH*CFGID_USEROSD_MAX;
	IpcHandl[IPC_USER_SHA].RWmode = shm_rdwr;
	IpcHandl[IPC_USER_SHA].ptr = NULL;

	IpcHandl[IPC_SEM].name = DEFAULTPATH;
	IpcHandl[IPC_SEM].Identify = IPC_SEM;
	IpcHandl[IPC_SEM].Class = IPC_Class_SEM;	
	IpcHandl[IPC_SEM].IPCID = 0;
	IpcHandl[IPC_SEM].ptr = NULL;

	return;
}

//static int *sysConfig = NULL;
//static unsigned char *userConfig = NULL;
static void *ipc_getSharedMem(IPC_MTYPE itype)
{
	if(itype == IPC_IMG_SHA || itype == IPC_USER_SHA)
		return (void*)IpcHandl[itype].ptr;
	else
		return NULL;
}

#endif /* IPC_CUSTOM_HEAD_HPP_ */
