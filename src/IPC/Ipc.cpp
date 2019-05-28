
#include <string.h>
#include "osa_thr.h"
#include "osa_buf.h"
#include "locale.h"
#include "wchar.h"
#include "ipc_custom_head.hpp"
#include "configtable.h"
#include "msgDriv.h"	// use MSGDRIV_send
#include "app_ctrl.h"		// use app_ctrl_setXXX
#include "app_status.h"		// use MSG_PROC_ID

#define DATAIN_TSK_PRI              (2)
#define DATAIN_TSK_STACK_SIZE       (0)
#define SDK_MEM_MALLOC(size)                                            OSA_memAlloc(size)

extern ALG_CONFIG_Trk gCFG_Trk;
extern ALG_CONFIG_Mtd gCFG_Mtd;
extern OSD_CONFIG_USER gCFG_Osd;
extern OSDSTATUS gSYS_Osd;

int ipc_loop = 1;
int startEnable=0, ipcDbgOn = 0;
OSA_BufCreate msgSendBufCreate;
OSA_BufHndl msgSendBuf;
OSA_ThrHndl thrHandleDataIn_recv;
OSA_ThrHndl thrHandleDataIn_send;

#define FXN_BIGEN if( fxnsCfg != NULL ){
//#define FXN_REG( blkId, feildId, fxn ) ( fxnsCfg[CFGID_BUILD( blkId, feildId )] = fxn )
#define FXN_REG( blkId, fxn ) ( fxnsCfg[blkId] = fxn )
#define FXN_END }
typedef Int32 ( *ConfigFxn )( Int32 blkId, Int32 feildId, void *inprm );
static ConfigFxn *fxnsCfg = NULL;
static int *sysConfig = NULL;
static unsigned char *userConfig = NULL;

extern int IPCSendMsg(CMD_ID cmd, void* prm, int len);
///////////////////////////////////
// cfg_ctrl_xxx() get data from sysconfig for init
// cfg_update_xxx() get data from sysconfig by update and 
int cfg_get_input_bkid(int ich)
{
	if(ich == 0)
		return CFGID_INPUT1_BKID;
	else if(ich == 1)
		return CFGID_INPUT2_BKID;
	else if(ich == 2)
		return CFGID_INPUT3_BKID;
	else if(ich == 3)
		return CFGID_INPUT4_BKID;
	else/* if(ich == 4)*/
		return CFGID_INPUT5_BKID;
}
int cfg_get_input_senid(int blkId)
{
	if(blkId >= CFGID_INPUT5_BKID)
		return 4;
	else if(blkId >= CFGID_INPUT4_BKID)
		return 3;
	else if(blkId >= CFGID_INPUT3_BKID)
		return 2;
	else if(blkId >= CFGID_INPUT2_BKID)
		return 1;
	else/* if(blkId >= CFGID_INPUT1_BKID)*/
		return 0;
}

void cfg_ctrl_sysInit(int * configTab)
{
	int i, BKID;

	///////////////////
	int resolution, usrosdId;
	for(i = 0; i < CAMERACHMAX; i++)
	{
		BKID = cfg_get_input_bkid(i);
		resolution = configTab[CFGID_INPUT_CHRES(BKID)];
		if((0 == resolution)||(1 == resolution) || (5 == resolution))
		{
			vcapWH[i][0] = 1920;
			vcapWH[i][1] = 1080;
			vdisWH[i][0] = 1920;
			vdisWH[i][1] = 1080;
		}
		else if((2 == resolution)||(3 == resolution))
		{
			vcapWH[i][0] = 1280;
			vcapWH[i][1] = 720;
			vdisWH[i][0] = 1280;
			vdisWH[i][1] = 720;
		}
		else
		{
			vcapWH[i][0] = 720;
			vcapWH[i][1] = 576;
			vdisWH[i][0] = 720;
			vdisWH[i][1] = 576;
		}
		usrosdId = configTab[CFGID_INPUT_CHNAME(BKID)];
		gCFG_Osd.items[usrosdId].senbind = 1;
		gCFG_Osd.items[usrosdId].senID = i;
		printf("input init [%d] resolution (%d x %d) usrosdid %d\n", i, vcapWH[i][0], vcapWH[i][1], usrosdId);
	}

	///////////////////
	configTab[CFGID_RTS_mainch] = configTab[CFGID_OUTPUT_dftch];
	configTab[CFGID_RTS_mainch2] = configTab[CFGID_OUTPUT_dftch];
	printf("output init default select channel %d\n", configTab[CFGID_OUTPUT_dftch]);

	///////////////////
	gSYS_Osd.workMode = 4;	// default stat
	gSYS_Osd.osdDrawShow = configTab[CFGID_SYSOSD_biten];
	gSYS_Osd.osdUserShow = configTab[CFGID_USROSD_showen];
	gSYS_Osd.osdDrawColor = 3;	// red color
	gSYS_Osd.algOsdRect = 0;		// auto aimsize
	if(gSYS_Osd.osdDrawShow & 0x01)
		gSYS_Osd.m_crossOsd = 1;
	else
		gSYS_Osd.m_crossOsd = 0;
	if(gSYS_Osd.osdDrawShow & 0x02)
		gSYS_Osd.m_boxOsd = 1;
	else
		gSYS_Osd.m_boxOsd = 0;
	if(gSYS_Osd.osdDrawShow & 0x04)
		gSYS_Osd.m_workOsd = 1;
	else
		gSYS_Osd.m_workOsd = 0;
	gSYS_Osd.m_chidIDOsd = 0;
	gSYS_Osd.m_chidNameOsd = 0;
	gSYS_Osd.m_sceneBoxOsd = 0;
	printf("osd init sysenable %x syscolor %d usrenable %x\n",
			gSYS_Osd.osdDrawShow, gSYS_Osd.osdDrawColor, gSYS_Osd.osdUserShow);

	///////////////////
	memcpy(&gCFG_Trk, &(configTab[CFGID_TRK_BASE]),(4*3*CFGID_FEILD_MAX)/*sizeof(ALG_CONFIG_Trk)*/);
	gCFG_Trk.losttime = (int)gCFG_Trk.flosttime;
	printf("trkPrm init occlusion_thred %f res_area %f losttime %dms\n",
			gCFG_Trk.occlusion_thred, gCFG_Trk.res_area, gCFG_Trk.losttime);

	///////////////////
	memcpy(&gCFG_Mtd, &(configTab[CFGID_MTD_BASE]),(4*2*CFGID_FEILD_MAX)/*sizeof(ALG_CONFIG_Mtd)*/);
	printf("mtdPrm init areaSetBox %d detectNum %d\n",
			gCFG_Mtd.areaSetBox, gCFG_Mtd.detectNum);
	BKID = cfg_get_input_bkid(configTab[CFGID_OUTPUT_dftch]);
	gCFG_Mtd.sensitivityThreshold = configTab[CFGID_INPUT_SENISIVITY(BKID)];
	gCFG_Mtd.detectArea_X = configTab[CFGID_INPUT_DETX(BKID)];
	gCFG_Mtd.detectArea_Y = configTab[CFGID_INPUT_DETY(BKID)];
	gCFG_Mtd.detectArea_wide = configTab[CFGID_INPUT_DETW(BKID)];
	gCFG_Mtd.detectArea_high = configTab[CFGID_INPUT_DETH(BKID)];
	printf("mtdPrm init sensitivity %d detect(%d, %d %d x %d)\n",
			gCFG_Mtd.sensitivityThreshold,
			gCFG_Mtd.detectArea_X, gCFG_Mtd.detectArea_Y,
			gCFG_Mtd.detectArea_wide, gCFG_Mtd.detectArea_high);

}

void cfg_ctrl_osdInit(int * configTab, unsigned char *configUser)
{
	int id;
	for(id=0; id<CFGID_USEROSD_MAX; id++)
	{
		if(id<16)
		{
			memcpy(&gCFG_Osd.items[id], &(configTab[CFGID_OSD_BASE(id)]), (4*1*CFGID_FEILD_MAX)/*sizeof(OSD_CONFIG_ITEM)*/);
		}
		else
		{
			memcpy(&gCFG_Osd.items[id], &(configTab[CFGID_OSD2_BASE(id)]), (4*1*CFGID_FEILD_MAX)/*sizeof(OSD_CONFIG_ITEM)*/);
		}
		setlocale(LC_ALL, "zh_CN.UTF-8");
		swprintf(gCFG_Osd.disBuf[id], 33, L"%s", (char *)&(configUser[id*USEROSD_LENGTH]));
		//printf("usrosd[%d] init show %d pos %d,%d font %d size %d\n",
		//	id, gCFG_Osd.items[id].ctrl,
		//	gCFG_Osd.items[id].posx, gCFG_Osd.items[id].posy,
		//	gCFG_Osd.items[id].font, gCFG_Osd.items[id].fontsize);
	}
}

void cfg_ctrl_acqReset(void *inprm)
{
	int i, BKID;
	int * configTab = sysConfig;
	CMD_EXT * pIStuts = (CMD_EXT *)inprm;
	///////////////////
	int type;
	for(i=0; i<CAMERACHMAX; i++)
	{
		BKID = cfg_get_input_bkid(i);
		type = configTab[CFGID_INPUT_AIMTYPE(BKID)];
		if(type == 2)	// switch level
		{
			int swlv = configTab[CFGID_INPUT_AIMLV(BKID)];
			pIStuts->AcqRectW[i] = configTab[CFGID_INPUT_SWAIMW(BKID, swlv)];
			pIStuts->AcqRectH[i] = configTab[CFGID_INPUT_SWAIMH(BKID, swlv)];
		}
		else
		{
			pIStuts->AcqRectW[i] = configTab[CFGID_INPUT_FIXAIMW(BKID)];
			pIStuts->AcqRectH[i] = configTab[CFGID_INPUT_FIXAIMH(BKID)];
		}
		pIStuts->AimW[i] = pIStuts->AcqRectW[i];
		pIStuts->AimH[i] = pIStuts->AcqRectH[i];
		configTab[CFGID_INPUT_CURAIMW(BKID)] = pIStuts->AcqRectW[i];
		configTab[CFGID_INPUT_CURAIMH(BKID)] = pIStuts->AcqRectH[i];
		printf("input[%d] acqtype %d rect(%d x %d)\n", i, type, pIStuts->AcqRectW[i], pIStuts->AcqRectH[i]);
		if(i == configTab[CFGID_RTS_mainch])
			gSYS_Osd.algOsdRect = (type)?0:1;
	}

}

void cfg_ctrl_setSensor(int * configTab)
{
	int ich, BKID;
	ich = configTab[CFGID_RTS_mainch];
	BKID = cfg_get_input_bkid(ich);

	// mtd
	gCFG_Mtd.sensitivityThreshold = configTab[CFGID_INPUT_SENISIVITY(BKID)];
	gCFG_Mtd.detectArea_X = configTab[CFGID_INPUT_DETX(BKID)];
	gCFG_Mtd.detectArea_Y = configTab[CFGID_INPUT_DETY(BKID)];
	gCFG_Mtd.detectArea_wide = configTab[CFGID_INPUT_DETW(BKID)];
	gCFG_Mtd.detectArea_high = configTab[CFGID_INPUT_DETH(BKID)];
	MSGDRIV_send(MSGID_EXT_MVDETECTAERA, 0);

}

Int32 cfg_update_trk( Int32 blkId, Int32 feildId, void *inprm )
{
	int * configTab = sysConfig;
	/////////////////////////
	if(feildId == 0xFF)
	{
		memcpy(&gCFG_Trk, &(configTab[CFGID_TRK_BASE]),(4*3*CFGID_FEILD_MAX)/*sizeof(ALG_CONFIG_Trk)*/);
		gCFG_Trk.losttime = (int)gCFG_Trk.flosttime;
	}
	if(CFGID_BUILD(blkId, feildId) == CFGID_TRK_assitime)
	{
		memcpy(&gCFG_Trk.flosttime, &configTab[CFGID_TRK_assitime], 4);
		gCFG_Trk.losttime = (int)gCFG_Trk.flosttime;
		printf("trkPrm update losttime %dms\n", gCFG_Trk.losttime);
	}
	return 0;
}

Int32 cfg_update_output( Int32 blkId, Int32 feildId, void *inprm )
{
	return 0;
}

Int32 cfg_update_sys( Int32 blkId, Int32 feildId, void *inprm )
{
	int i;
	int * configTab = sysConfig;
	/////////////////////////
	if(feildId == 0xFF || CFGID_BUILD(blkId, feildId) == CFGID_SYSOSD_biten)
	{
		gSYS_Osd.osdDrawShow = configTab[CFGID_SYSOSD_biten];
		if(gSYS_Osd.osdDrawShow & 0x01)
			gSYS_Osd.m_crossOsd = 1;
		else
			gSYS_Osd.m_crossOsd = 0;
		if(gSYS_Osd.osdDrawShow & 0x02)
			gSYS_Osd.m_boxOsd = 1;
		else
			gSYS_Osd.m_boxOsd = 0;
		if(gSYS_Osd.osdDrawShow & 0x04)
			gSYS_Osd.m_workOsd = 1;
		else
			gSYS_Osd.m_workOsd = 0;
		MSGDRIV_send(MSGID_EXT_UPDATE_OSD, 0);		// sys ctrl part
	}
	if(feildId == 0xFF || CFGID_BUILD(blkId, feildId) == CFGID_USROSD_showen)
	{
		gSYS_Osd.osdUserShow = configTab[CFGID_USROSD_showen];
		MSGDRIV_send(MSGID_EXT_UPDATE_OSD, 0);		// sys ctrl part
	}
	return 0;
}

Int32 cfg_update_mtd( Int32 blkId, Int32 feildId, void *inprm )
{
	int i;
	int * configTab = sysConfig;
	/////////////////////////
	if(feildId == 0xFF)
	{
		memcpy(&gCFG_Mtd, &(configTab[CFGID_MTD_BASE]),(4*2*CFGID_FEILD_MAX)/*sizeof(ALG_CONFIG_Mtd)*/);
		MSGDRIV_send(MSGID_EXT_MVDETECTUPDATE, 0);
	}
	if(CFGID_BUILD(blkId, feildId) == CFGID_MTD_upspd)
	{
		gCFG_Mtd.tmpUpdateSpeed = configTab[CFGID_MTD_upspd];
		MSGDRIV_send(MSGID_EXT_MVDETECTUPDATE, 0);
	}
	return 0;
}

Int32 cfg_update_usrosd( Int32 blkId, Int32 feildId, void *inprm )
{
	int * configTab = sysConfig;
	unsigned char *configUser = userConfig;
	/////////////////////////
	int id, i, addrbase;
	if(blkId >= CFGID_OSD2_BKID)
	{
		id = blkId - CFGID_OSD2_BKID + 16;
		addrbase = CFGID_OSD2_BASE(id);
	}
	else
	{
		id = blkId - CFGID_OSD_BKID;
		addrbase = CFGID_OSD_BASE(id);
	}

	IPC_PRM_INT *pPrm = (IPC_PRM_INT *)&gCFG_Osd.items[id];
	if(feildId == 0xFF)
	{
		for(i=0;i<9;i++)
			pPrm->intPrm[i] = (int)configTab[addrbase+i];
		setlocale(LC_ALL, "zh_CN.UTF-8");
		swprintf(gCFG_Osd.disBuf[id], 33, L"%s", (char *)&(configUser[id*USEROSD_LENGTH]));
	}
	else if(feildId < 9)
	{
		pPrm->intPrm[feildId] = (int)configTab[addrbase+feildId];
	}

	return 0;
}

Int32 cfg_update_input( Int32 blkId, Int32 feildId, void *inprm )
{
	int ich, BKID, configId, usrosdId;
	int cfgIdtype, cfgIdswlv;
	int type, swlv;
	int * configTab = sysConfig;
	CMD_EXT * pInCmd = (CMD_EXT *)inprm;

	ich = cfg_get_input_senid(blkId);
	BKID = cfg_get_input_bkid(ich);
	configId = CFGID_BUILD(blkId, feildId);
	cfgIdtype = CFGID_INPUT_AIMTYPE(BKID);
	type = configTab[cfgIdtype];
	cfgIdswlv = CFGID_INPUT_AIMLV(BKID);
	swlv = configTab[cfgIdswlv];

	if(feildId == 0xFF || configId == cfgIdtype || configId == cfgIdswlv || 
		configId == CFGID_INPUT_SWAIMW(BKID, swlv) || configId == CFGID_INPUT_SWAIMH(BKID, swlv) ||
		configId == CFGID_INPUT_FIXAIMW(BKID) || configId == CFGID_INPUT_FIXAIMH(BKID))
	{
		if(type == 2)
		{
			pInCmd->AcqRectW[ich] = configTab[CFGID_INPUT_SWAIMW(BKID, swlv)];
			pInCmd->AcqRectH[ich] = configTab[CFGID_INPUT_SWAIMH(BKID, swlv)];
		}
		else
		{
			pInCmd->AcqRectW[ich] = configTab[CFGID_INPUT_FIXAIMW(BKID)];
			pInCmd->AcqRectH[ich] = configTab[CFGID_INPUT_FIXAIMH(BKID)];
		}
		pInCmd->AimW[ich] = pInCmd->AcqRectW[ich];
		pInCmd->AimH[ich] = pInCmd->AcqRectH[ich];
		configTab[CFGID_INPUT_CURAIMW(BKID)] = pInCmd->AcqRectW[ich];
		configTab[CFGID_INPUT_CURAIMH(BKID)] = pInCmd->AcqRectH[ich];

		if(ich == configTab[CFGID_RTS_mainch])
		{
			gSYS_Osd.algOsdRect = (type)?0:1;
		}
		app_ctrl_setAcqRect(pInCmd);
		app_ctrl_setAimSize(pInCmd);
	}
	if(feildId == 0xFF || configId == CFGID_INPUT_SENISIVITY(BKID) ||
		configId == CFGID_INPUT_DETX(BKID) || configId == CFGID_INPUT_DETY(BKID) ||
		configId == CFGID_INPUT_DETW(BKID) || configId == CFGID_INPUT_DETH(BKID))
	{
		if(ich == configTab[CFGID_RTS_mainch])
			cfg_ctrl_setSensor(configTab);
	}
	if(feildId == 0xFF || configId == CFGID_INPUT_CHNAME(BKID))
	{
		// clear old
		for(usrosdId=0;usrosdId<CFGID_USEROSD_MAX;usrosdId++)
		{
			if(gCFG_Osd.items[usrosdId].senbind && gCFG_Osd.items[usrosdId].senID == ich)
			{
				gCFG_Osd.items[usrosdId].senbind = 0;
				break;
			}
		}
		// set new
		usrosdId = configTab[CFGID_INPUT_CHNAME(BKID)];
		gCFG_Osd.items[usrosdId].senID = ich;
		gCFG_Osd.items[usrosdId].senbind = 1;
	}

	return 0;
}

void cfg_init(void)
{
	sysConfig = (int *)ipc_getSharedMem(IPC_IMG_SHA);
	userConfig = (unsigned char *)ipc_getSharedMem(IPC_USER_SHA);
	if(sysConfig == NULL || userConfig == NULL)
	{
		printf("error : give error sharemem \n");
		exit(EXIT_FAILURE);
	}
	printf("====== ConfigTable[ver %d] init date: %s %s======\n", SYSCONFIG_VERSION, __DATE__, __TIME__);

	int i;
	fxnsCfg = (ConfigFxn*)OSA_memAlloc( sizeof( ConfigFxn ) * CFGID_BKID_MAX );
	FXN_BIGEN
	for(i=0; i<3; i++)
		FXN_REG(CFGID_TRK_BKID+i, cfg_update_trk);
	FXN_REG(CFGID_OUTPUT_BKID, cfg_update_output);
	FXN_REG(CFGID_SYS_BKID, cfg_update_sys);
	for(i=0; i<2; i++)
		FXN_REG(CFGID_MTD_BKID+i, cfg_update_mtd);
	for(i=0; i<16; i++)
	{
		FXN_REG(CFGID_OSD_BKID+i, cfg_update_usrosd);
		FXN_REG(CFGID_OSD2_BKID+i, cfg_update_usrosd);
	}
	for(i=0; i<7; i++)
	{
		FXN_REG(CFGID_INPUT1_BKID+i, cfg_update_input);
		FXN_REG(CFGID_INPUT2_BKID+i, cfg_update_input);
		FXN_REG(CFGID_INPUT3_BKID+i, cfg_update_input);
		FXN_REG(CFGID_INPUT4_BKID+i, cfg_update_input);
		FXN_REG(CFGID_INPUT5_BKID+i, cfg_update_input);
	}
	FXN_END;

	return ;
}

void cfg_uninit(void)
{
	if(fxnsCfg != NULL)
		OSA_memFree(fxnsCfg);
}

int cfg_set_outSensor(unsigned int outsen, unsigned int outsen2)
{
	static unsigned int outsenPrev = 0, outsen2Prev = 0;
	sysConfig[CFGID_RTS_mainch] = outsen;
	sysConfig[CFGID_RTS_mainch2] = outsen2;
	if(outsenPrev != outsen)	// report
	{
		int configId = CFGID_RTS_mainch;
		IPCSendMsg(read_shm_single, &configId, 4);
		OSA_printf("OUT-SEN: mainch from %d to %d\n", outsenPrev, outsen);
		outsenPrev = outsen;
	}
	if(outsen2Prev != outsen2)	// report
	{
		int configId = CFGID_RTS_mainch2;
		IPCSendMsg(read_shm_single, &configId, 4);
		OSA_printf("OUT-SEN: mainch2 from %d to %d\n", outsen2Prev, outsen2);
		outsen2Prev = outsen2;
	}
	return 0;
}

int cfg_set_trkMode(unsigned int bTrack, unsigned int bScene)
{
	static unsigned int bTrackPrev = 0, bScenePrev = 0;
	sysConfig[CFGID_RTS_trken] = bTrack;
	sysConfig[CFGID_RTS_trkmode] = bScene;

	if(!bTrack)	// clean
	{
		sysConfig[CFGID_RTS_trkstat] = 0;
		sysConfig[CFGID_RTS_trkerrx] = 0;
		sysConfig[CFGID_RTS_trkerry] = 0;
	}
	if(bTrackPrev != bTrack)	// report
	{
		int configId = CFGID_RTS_trken;
		IPCSendMsg(read_shm_single, &configId, 4);
		OSA_printf("ALG-TRK: enable from %d to %d\n", bTrackPrev, bTrack);
		bTrackPrev = bTrack;
	}
	if(bScenePrev != bScene)	// report
	{
		//int configId = CFGID_RTS_trkmode;
		//IPCSendMsg(read_shm_single, &configId, 4);
		OSA_printf("ALG-TRK: scene from %d to %d\n", bScenePrev, bScene);
		bScenePrev = bScene;
	}
	return 0;
}

int cfg_set_trkSecStat(unsigned int bSecTrk)
{
	static unsigned int bSecTrkPrev = 0;
	sysConfig[CFGID_RTS_sectrkstat] = bSecTrk;
	if(bSecTrkPrev != bSecTrk)	// report
	{
		int configId = CFGID_RTS_sectrkstat;
		IPCSendMsg(read_shm_single, &configId, 4);
		OSA_printf("ALG-TRK: sectrk from %d to %d\n", bSecTrkPrev, bSecTrk);
		bSecTrkPrev = bSecTrk;
	}
	return 0;
}

int cfg_set_trkFeedback(unsigned int trackstatus, float trackposx, float trackposy)
{
	static unsigned int bTrkStatPrev = 0;
	sysConfig[CFGID_RTS_trkstat] = trackstatus;
	//sysConfig[CFGID_RTS_trkerrx] = trackposx;
	//sysConfig[CFGID_RTS_trkerry] = trackposy;
	memcpy(&sysConfig[CFGID_RTS_trkerrx], &trackposx, 4);
	memcpy(&sysConfig[CFGID_RTS_trkerry], &trackposy, 4);
	// report always
	int configId = CFGID_RTS_trkstat;
	IPCSendMsg(read_shm_single, &configId, 4);
	if(bTrkStatPrev != trackstatus)
	{
		OSA_printf("ALG-TRK: stat from %d to %d\n", bTrkStatPrev, trackstatus);
		bTrkStatPrev = trackstatus;
	}
	return 0;
}

int cfg_set_mtdFeedback(unsigned int bMtd, unsigned int bMtdDetect)
{
	static unsigned int bMtdPrev = 0, bMtdDetectPrev = 0;
	sysConfig[CFGID_RTS_mtden] = bMtd;
	sysConfig[CFGID_RTS_mtddet] = bMtdDetect;
	if(bMtdPrev != bMtd)	// report
	{
		int configId = CFGID_RTS_mtden;
		IPCSendMsg(read_shm_single, &configId, 4);
		OSA_printf("ALG-MTD: enable from %d to %d\n", bMtdPrev, bMtd);
		bMtdPrev = bMtd;
	}
	if(bMtdDetectPrev != bMtdDetect)	// report
	{
		int configId = CFGID_RTS_mtddet;
		IPCSendMsg(read_shm_single, &configId, 4);
		OSA_printf("ALG-MTD: stat from %d to %d\n", bMtdDetectPrev, bMtdDetect);
		bMtdDetectPrev = bMtdDetect;
	}
	return 0;
}
/////////////////////////////////////////////////////

void createSendBuf()
{
    int status;
    int i=0;
    msgSendBufCreate.numBuf = OSA_BUF_NUM_MAX;
    for (i = 0; i < msgSendBufCreate.numBuf; i++)
    {
        msgSendBufCreate.bufVirtAddr[i] = SDK_MEM_MALLOC(64);
        OSA_assert(msgSendBufCreate.bufVirtAddr[i] != NULL);
        memset(msgSendBufCreate.bufVirtAddr[i], 0, 64);
    }
    OSA_bufCreate(&msgSendBuf, &msgSendBufCreate);
}

int IPCSendMsg(CMD_ID cmd, void* prm, int len)
{
	if(len >= PARAMLEN || (prm == NULL && len > 0))
		return -1;

	int bufId = 0;
	SENDST *pSendData=NULL;
	OSA_bufGetEmpty(&(msgSendBuf), &bufId, OSA_TIMEOUT_NONE);
	if(bufId >= 0)
	{
		pSendData=(SENDST *)msgSendBuf.bufInfo[bufId].virtAddr;
		memset(pSendData->param, 0, PARAMLEN);
		pSendData->cmd_ID = cmd & 0xFF;
		if(len > 0)
			memcpy(pSendData->param, prm, len);
		OSA_bufPutFull(&(msgSendBuf), bufId);
	}
	return 0;
}

int  send_msgpth(SENDST * pData)
{
	int bufId=0;
	int sendLen=0;
	OSA_bufGetFull(&msgSendBuf, &bufId, OSA_TIMEOUT_FOREVER);
	memcpy(pData, msgSendBuf.bufInfo[bufId].virtAddr,sizeof(SENDST));
	OSA_bufPutEmpty(&msgSendBuf, bufId);
	{
		IPC_PRM_INT *pIn=NULL;
		pIn = (IPC_PRM_INT *)pData->param;
		if(ipcDbgOn)
		{
			printf("[IPC-SEND] cmdID : %d (%08x %08x %08x %08x)\n",
				pData->cmd_ID,
				pIn->intPrm[0],pIn->intPrm[1],
				pIn->intPrm[2],pIn->intPrm[3]);
		}
	}
	return 0;
}

void* recv_msgpth(SENDST *pInData)
{
	IPC_PRM_INT *pIn=NULL;
	int configId, blkId, feildId;

	if(pInData==NULL)
	{
		return NULL ;
	}

	CMD_EXT inCtrl, *pMsg = NULL;
	pMsg = &inCtrl;
	memset(pMsg,0,sizeof(CMD_EXT));
	if(startEnable)
		app_ctrl_getSysData(pMsg);

	pIn = (IPC_PRM_INT *)pInData->param;
	if(ipcDbgOn)
	{
		printf("[IPC-RECV] cmdID : %d (%08x %08x %08x %08x)\n",
			pInData->cmd_ID,
			pIn->intPrm[0],pIn->intPrm[1],
			pIn->intPrm[2],pIn->intPrm[3]);
	}
	switch(pInData->cmd_ID)
	{
		case read_shm_config:
			//if(!startEnable)
			{
				cfg_ctrl_sysInit(sysConfig);
				MSGDRIV_send(MSGID_EXT_UPDATE_OSD, 0);		// sys osd part
				MSGDRIV_send(MSGID_EXT_MVDETECTAERA, 0);
				MSGDRIV_send(MSGID_EXT_MVDETECTUPDATE, 0);
				cfg_ctrl_osdInit(sysConfig, userConfig);
				startEnable = 1;
			}
			break;

		case read_shm_block:
			{
				configId = pIn->intPrm[0];
				blkId = CFGID_blkId(configId);
				if(fxnsCfg[blkId] != NULL)
					fxnsCfg[blkId](blkId, 0xFF, (void *)pMsg);
			}
			break;
		case read_shm_single:
			{
				configId = pIn->intPrm[0];
				blkId = CFGID_blkId(configId);
				feildId = CFGID_feildId(configId);
				if(fxnsCfg[blkId] != NULL)
					fxnsCfg[blkId](blkId, feildId, (void *)pMsg);
			}
			break;
		case read_shm_usrosd:
			{
				configId = pIn->intPrm[0];
				configId = (configId < 16)?(CFGID_OSD_BKID+configId):(CFGID_OSD2_BKID+(configId-16));
				cfg_update_usrosd(configId, 0xFF, NULL);
			}
			break;

	///////////////////////////////////////////
		case sensor:
			if(!pMsg->AvtTrkStat)
			{
				pMsg->SensorStat = pIn->intPrm[0];
				app_ctrl_setSensor(pMsg);
				cfg_set_outSensor(pMsg->SensorStat, 0/*pMsg->SensorStat2*/);
				cfg_ctrl_setSensor(sysConfig);
			}
			break;

		case BoresightPos:
			//if(!pMsg->AvtTrkStat)
			{
				unsigned int BoresightPos_x = pIn->intPrm[0];
				unsigned int BoresightPos_y = pIn->intPrm[1];
				pMsg->opticAxisPosX[pMsg->SensorStat] = BoresightPos_x;
				pMsg->opticAxisPosY[pMsg->SensorStat] = BoresightPos_y;
				pMsg->AxisPosX[pMsg->SensorStat] = pMsg->opticAxisPosX[pMsg->SensorStat];
				pMsg->AxisPosY[pMsg->SensorStat] = pMsg->opticAxisPosY[pMsg->SensorStat];

				pMsg->AvtPosX[pMsg->SensorStat] = pMsg->AxisPosX[pMsg->SensorStat];
				pMsg->AvtPosY[pMsg->SensorStat] = pMsg->AxisPosY[pMsg->SensorStat];
				
				app_ctrl_setBoresightPos(pMsg);
			}
			break;

		case AcqPos:
			{
				unsigned int AcqStat = pIn->intPrm[0];
				unsigned int BoresightPos_x = pIn->intPrm[1];
				unsigned int BoresightPos_y = pIn->intPrm[2];
				if( pMsg->MtdState[pMsg->SensorStat])
					break;
				if(2 == AcqStat){
					pMsg->AxisPosX[pMsg->SensorStat] = BoresightPos_x;
					pMsg->AxisPosY[pMsg->SensorStat] = BoresightPos_y;
					pMsg->AvtTrkStat = eTrk_mode_search;
					int width = 0,height = 0;
					if((pMsg->SensorStat == video_pal)||(pMsg->SensorStat == video_gaoqing0)||(pMsg->SensorStat == video_gaoqing)||(pMsg->SensorStat == video_gaoqing2)||(pMsg->SensorStat == video_gaoqing3)){
						width  = vdisWH[pMsg->SensorStat][0];
						height = vdisWH[pMsg->SensorStat][1];
					}
					if(pMsg->AxisPosX[pMsg->SensorStat] + pMsg->crossAxisWidth[pMsg->SensorStat]/2 > width)
						pMsg->AxisPosX[pMsg->SensorStat] = width - pMsg->crossAxisWidth[pMsg->SensorStat]/2;
					if(pMsg->AxisPosY[pMsg->SensorStat] + pMsg->crossAxisHeight[pMsg->SensorStat]/2 > height)
						pMsg->AxisPosY[pMsg->SensorStat] = height - pMsg->crossAxisHeight[pMsg->SensorStat]/2;

					if(pMsg->AxisPosX[pMsg->SensorStat] <  pMsg->crossAxisWidth[pMsg->SensorStat]/2)
						pMsg->AxisPosX[pMsg->SensorStat] =  pMsg->crossAxisWidth[pMsg->SensorStat]/2;
					if(pMsg->AxisPosY[pMsg->SensorStat]  <  pMsg->crossAxisHeight[pMsg->SensorStat]/2)
						pMsg->AxisPosY[pMsg->SensorStat] =  pMsg->crossAxisHeight[pMsg->SensorStat]/2;
					app_ctrl_setAxisPos(pMsg);

				}
				else if(1 == AcqStat){
					pMsg->AvtTrkStat = eTrk_mode_acqmove;
					pMsg->AvtPosX[pMsg->SensorStat] = BoresightPos_x;
					pMsg->AvtPosY[pMsg->SensorStat] = BoresightPos_y;
					app_ctrl_setTrkStat(pMsg);
					pMsg->AxisPosX[pMsg->SensorStat] = pMsg->opticAxisPosX[pMsg->SensorStat];
					pMsg->AxisPosY[pMsg->SensorStat] = pMsg->opticAxisPosY[pMsg->SensorStat];
					app_ctrl_setAxisPos(pMsg);
				}
				else if(0 == AcqStat){
					pMsg->AxisPosX[pMsg->SensorStat] = BoresightPos_x;
					pMsg->AxisPosY[pMsg->SensorStat] = BoresightPos_y;
					pMsg->AvtTrkStat = eTrk_mode_acq;
					app_ctrl_setAxisPos(pMsg);
				}
			}
			break;

		case sceneTrk:
			{
				unsigned int AvtTrkStat = pIn->intPrm[0];
				if(AvtTrkStat == 0x1)
					pMsg->SceneAvtTrkStat =eTrk_mode_target;
				else
					pMsg->SceneAvtTrkStat = eTrk_mode_acq;

				app_ctrl_setSceneTrk(pMsg); 
			}
			break;

		case trk:	
			{
				unsigned int AvtTrkStat = pIn->intPrm[0];
				if(AvtTrkStat == 0x1)
					pMsg->AvtTrkStat =eTrk_mode_target;
				else
					pMsg->AvtTrkStat = eTrk_mode_acq;

				if(pMsg->AvtTrkStat == eTrk_mode_acq)
				{
					pMsg->AxisPosX[pMsg->SensorStat] = pMsg->opticAxisPosX[pMsg->SensorStat];
					pMsg->AxisPosY[pMsg->SensorStat] = pMsg->opticAxisPosY[pMsg->SensorStat];
					pMsg->AvtPosX[pMsg->SensorStat]  = pMsg->AxisPosX[pMsg->SensorStat];
					pMsg->AvtPosY[pMsg->SensorStat]  = pMsg->AxisPosY[pMsg->SensorStat];
					pMsg->AimW[pMsg->SensorStat]  = pMsg->AcqRectW[pMsg->SensorStat];
					pMsg->AimH[pMsg->SensorStat]  = pMsg->AcqRectH[pMsg->SensorStat];
					app_ctrl_setAimPos(pMsg);
					app_ctrl_setAxisPos(pMsg);
					app_ctrl_setAimSize(pMsg);
				}

				app_ctrl_setTrkStat(pMsg); 
				cfg_set_trkSecStat(0);	// set sectrk close when exit trk 
			}
			break;
	
		case sectrk:
			{
				unsigned int SecAcqStat = pIn->intPrm[0];
				unsigned int ImgPixelX = pIn->intPrm[1];
				unsigned int ImgPixelY = pIn->intPrm[2];
				if(pMsg->AvtTrkStat != eTrk_mode_acq)
				{
					if(1 == SecAcqStat){
						pMsg->AxisPosX[pMsg->SensorStat] = ImgPixelX;
						pMsg->AxisPosY[pMsg->SensorStat] = ImgPixelY;

						int width = 0,height = 0;
						if((pMsg->SensorStat == video_pal)||(pMsg->SensorStat == video_gaoqing0)||(pMsg->SensorStat == video_gaoqing)||(pMsg->SensorStat == video_gaoqing2)||(pMsg->SensorStat == video_gaoqing3)){
							width  = vdisWH[pMsg->SensorStat][0];
							height = vdisWH[pMsg->SensorStat][1];
						}
						if(pMsg->AxisPosX[pMsg->SensorStat] + pMsg->crossAxisWidth[pMsg->SensorStat]/2 > width)
							pMsg->AxisPosX[pMsg->SensorStat] = width - pMsg->crossAxisWidth[pMsg->SensorStat]/2;
						if(pMsg->AxisPosY[pMsg->SensorStat] + pMsg->crossAxisHeight[pMsg->SensorStat]/2 > height)
							pMsg->AxisPosY[pMsg->SensorStat] = height - pMsg->crossAxisHeight[pMsg->SensorStat]/2;

						if(pMsg->AxisPosX[pMsg->SensorStat] <  pMsg->crossAxisWidth[pMsg->SensorStat]/2)
							pMsg->AxisPosX[pMsg->SensorStat] =  pMsg->crossAxisWidth[pMsg->SensorStat]/2;
						if(pMsg->AxisPosY[pMsg->SensorStat]  <  pMsg->crossAxisHeight[pMsg->SensorStat]/2)
							pMsg->AxisPosY[pMsg->SensorStat] =  pMsg->crossAxisHeight[pMsg->SensorStat]/2;
		
						pMsg->AvtTrkStat =eTrk_mode_search;
						app_ctrl_setTrkStat(pMsg);
						app_ctrl_setAxisPos(pMsg);
					}
					else if(2 == SecAcqStat){
						pMsg->AvtTrkStat = eTrk_mode_sectrk;
						pMsg->AvtPosX[pMsg->SensorStat] = ImgPixelX;
						pMsg->AvtPosY[pMsg->SensorStat] = ImgPixelY;				
						app_ctrl_setTrkStat(pMsg);
						pMsg->AxisPosX[pMsg->SensorStat] = pMsg->opticAxisPosX[pMsg->SensorStat];
						pMsg->AxisPosY[pMsg->SensorStat] = pMsg->opticAxisPosY[pMsg->SensorStat];
						app_ctrl_setAxisPos(pMsg);
					}
					else
					{
						pMsg->AvtTrkStat = eTrk_mode_target;
						app_ctrl_setTrkStat(pMsg);
					}
					cfg_set_trkSecStat(SecAcqStat);	// set sectrk open/entertrk/cancel
				}
			}
			break;

		case posmove:
			{
				unsigned int AvtMoveX = pIn->intPrm[0];
				unsigned int AvtMoveY = pIn->intPrm[1];
				//pMsg->aimRectMoveStepX = AvtMoveX;
				//pMsg->aimRectMoveStepY = AvtMoveY;
				if(AvtMoveX==eTrk_ref_left)
				{
					pMsg->aimRectMoveStepX=-1;
				}
				else if(AvtMoveX==eTrk_ref_right)
				{
					pMsg->aimRectMoveStepX=1;
				}
				else//for dbg
				{
					pMsg->aimRectMoveStepX=AvtMoveX;
				}
				
				if(AvtMoveY==eTrk_ref_up)
				{
					pMsg->aimRectMoveStepY=-1;
				}
				else if(AvtMoveY==eTrk_ref_down)
				{
					pMsg->aimRectMoveStepY=1;
				}
				else//for dbg
				{
					pMsg->aimRectMoveStepY=AvtMoveY;
				}
				app_ctrl_setAimPos(pMsg);
			}
			break;	

#if __MMT__
		case mmt:
			{
				unsigned int ImgMtdStat = pIn->intPrm[0];
				if(ImgMtdStat == 0x01)
				{
					pMsg->MmtStat[pMsg->SensorStat] = eImgAlg_Enable;	
				}
				else if(ImgMtdStat == 0x00)
				{
					pMsg->MmtStat[pMsg->SensorStat] = eImgAlg_Disable;
				}
				app_ctrl_setMMT(pMsg);
			}
			break;

		case mmtselect:
			{
				unsigned int ImgMmtSelect = pIn->intPrm[0];
				ImgMmtSelect--;
				if(ImgMmtSelect > 5)
					ImgMmtSelect = 5;
				app_ctrl_setMmtSelect(pMsg,ImgMmtSelect);	
				pMsg->MmtStat[pMsg->SensorStat] = eImgAlg_Disable;
				app_ctrl_setMMT(pMsg);
			}
			break;
#endif
#if __ENH__
		case enh:
			{
				unsigned int ImgEnhStat = pIn->intPrm[0];
				if(ImgEnhStat == 1){
					pMsg->ImgEnhStat[pMsg->SensorStat] = eImgAlg_Enable;
				}
				else if(ImgEnhStat == 0){
					pMsg->ImgEnhStat[pMsg->SensorStat] = eImgAlg_Disable;
				}	
				app_ctrl_setEnhance(pMsg);
			}
			break;
#endif
#if __MOVE_DETECT__
		case mtd:
			{
				unsigned int ImgMtdStat = pIn->intPrm[0];
				if(ImgMtdStat == 1){
					pMsg->MtdState[pMsg->SensorStat] = eImgAlg_Enable;
				}
				else if(ImgMtdStat == 0){
					pMsg->MtdState[pMsg->SensorStat] = eImgAlg_Disable;
				}
				app_ctrl_setMtdStat(pMsg);
			}
			break;

		case mtdSelect:
			{
				unsigned int ImgMmtSelect = pIn->intPrm[0];
				pMsg->MtdSelect[pMsg->SensorStat] = ImgMmtSelect;
				app_ctrl_setMtdSelect(pMsg);
			}
			break;
			
#endif
		case workmode:
			gSYS_Osd.workMode = pIn->intPrm[0];
			break;

		case Iris:
			gSYS_Osd.IrisAndFocusAndExit = Enable_Iris;
			memcpy(&gSYS_Osd.cmd_triangle, (void *)pIn->intPrm, sizeof(CMD_triangle));
			break;

		case focus:
			gSYS_Osd.IrisAndFocusAndExit = Enable_Focus;
			memcpy(&gSYS_Osd.cmd_triangle, (void *)pIn->intPrm, sizeof(CMD_triangle));
			break;

		case exit_IrisAndFocus:
			gSYS_Osd.IrisAndFocusAndExit = Disable;
			memcpy(&gSYS_Osd.cmd_triangle, (void *)pIn->intPrm, sizeof(CMD_triangle));
			break;

		case exit_img:
			ipc_loop = 0;			
			break;

		default:
			break;
	}
}

static void * ipc_dataRecv(Void * prm)
{
	SENDST test;
	while(ipc_loop)
	{
		ipc_recvmsg(&test,IPC_TOIMG_MSG);
		recv_msgpth(&test);
	}
}

static void * ipc_dataSend(Void * prm)
{
	SENDST test;
	while(ipc_loop)
	{	
		send_msgpth(&test);
		ipc_sendmsg(&test,IPC_FRIMG_MSG);
	}
}

void Ipc_pthread_start(void)
{
	Ipc_init();
	int ret = Ipc_create();
	if(-1 == ret)
	{
		printf("error : give error param \n");
	}

	cfg_init();
	createSendBuf();
	OSA_thrCreate(&thrHandleDataIn_recv,
                      ipc_dataRecv,
                      DATAIN_TSK_PRI,
                      DATAIN_TSK_STACK_SIZE,
                      NULL);
       OSA_thrCreate(&thrHandleDataIn_send,
                      ipc_dataSend,
                      DATAIN_TSK_PRI,
                      DATAIN_TSK_STACK_SIZE,
                      NULL);
}

void Ipc_pthread_stop(void)
{
	OSA_thrDelete(&thrHandleDataIn_recv);
	OSA_thrDelete(&thrHandleDataIn_send);

	Ipc_destory();
	cfg_uninit();

}

//////////////////////////////////////////////
// self debug part
/////////////////////////////////////////////
void cfg_dbg_setDefault(int * configTab)
{
	int i, BKID;
	memset(configTab, 0, CFGID_BKID_MAX*CFGID_FEILD_MAX*4);

	//CFGID_JOS_BKID
	//CFGID_PTZ_BKID
	//CFGID_TRK_BKID
	{
		#if 0
		ALG_CONFIG_Trk *pCFG_Trk = (ALG_CONFIG_Trk *)&configTab[CFGID_TRK_BASE];
		pCFG_Trk->occlusion_thred = 0.28;
		pCFG_Trk->retry_acq_thred = 0.38;
		pCFG_Trk->up_factor = 0.0125;
		pCFG_Trk->res_distance = 80;
		pCFG_Trk->res_area = 5000;
		pCFG_Trk->gapframe = 10;
		pCFG_Trk->enhEnable = 0;	
		pCFG_Trk->cliplimit = 4.0;
		pCFG_Trk->dictEnable = 0;
		pCFG_Trk->moveX = 20;
		pCFG_Trk->moveY = 10;
		pCFG_Trk->moveX2 = 30;
		pCFG_Trk->moveY2 = 20;
		pCFG_Trk->segPixelX = 600;
		pCFG_Trk->segPixelY = 450;
		pCFG_Trk->losttime = 5000;
		pCFG_Trk->ScalerLarge = 256;
		pCFG_Trk->ScalerMid = 128;
		pCFG_Trk->ScalerSmall = 64;
		pCFG_Trk->Scatter = 10;
		pCFG_Trk->ratio = 1.0;
		pCFG_Trk->FilterEnable = 0;
		pCFG_Trk->BigSecEnable = 0;
		pCFG_Trk->SalientThred = 40;
		pCFG_Trk->ScalerEnable = 0;
		pCFG_Trk->DynamicRatioEnable = 0;
		pCFG_Trk->acqSize_width = 8;
		pCFG_Trk->acqSize_height = 8;
		pCFG_Trk->TrkAim43_Enable = 0;
		pCFG_Trk->SceneMVEnable = 0;
		pCFG_Trk->BackTrackEnable = 0;
		pCFG_Trk->bAveTrkPos = 0;
		pCFG_Trk->fTau = 0.5;
		pCFG_Trk->buildFrms = 500;
		pCFG_Trk->LostFrmThred = 30;
		pCFG_Trk->histMvThred = 1.0;
		pCFG_Trk->detectFrms = 30;
		pCFG_Trk->stillFrms = 50;
		pCFG_Trk->stillThred = 0.1;
		pCFG_Trk->bKalmanFilter = 0;
		pCFG_Trk->xMVThred = 3.0;
		pCFG_Trk->yMVThred = 2.0;
		pCFG_Trk->xStillThred = 0.5;
		pCFG_Trk->yStillThred = 0.3;
		pCFG_Trk->slopeThred = 0.08;
		pCFG_Trk->kalmanHistThred = 2.5;
		pCFG_Trk->kalmanCoefQ = 0.00001;
		pCFG_Trk->kalmanCoefR = 0.0025;
		#else
		IPC_PRM_INT *pPrm;
		pPrm = (IPC_PRM_INT *)&configTab[CFGID_TRK_BASE];
		pPrm = (IPC_PRM_INT *)&configTab[CFGID_TRK_BASE+1];
		pPrm = (IPC_PRM_INT *)&configTab[CFGID_TRK_BASE+2];
		//memcpy();
		//memcpy();
		//memcpy();
		#endif
	}
	//CFGID_OSD_BKID
	for(i=0; i<16; i++)
	{
		configTab[CFGID_OSD_COLOR(i)] = 0x02;
		configTab[CFGID_OSD_FONT(i)] = 0x02;
		configTab[CFGID_OSD_SIZE(i)] = 0x02;
	}
	//CFGID_INPUT1_BKID ~ CFGID_INPUT5_BKID
	for(i=0; i<CAMERACHMAX; i++)
	{
		BKID = cfg_get_input_bkid(i);
		configTab[CFGID_INPUT_SENISIVITY(BKID)] = 16;
		if(i < 4)
		{
			configTab[CFGID_INPUT_CHRES(BKID)] = 0;
			configTab[CFGID_INPUT_DETX(BKID)] = 960;
			configTab[CFGID_INPUT_DETY(BKID)] = 540;
			configTab[CFGID_INPUT_DETW(BKID)] = 960;
			configTab[CFGID_INPUT_DETH(BKID)] = 540;
		}
		else
		{
			configTab[CFGID_INPUT_CHRES(BKID)] = 6;
			configTab[CFGID_INPUT_DETX(BKID)] = 360;
			configTab[CFGID_INPUT_DETY(BKID)] = 288;
			configTab[CFGID_INPUT_DETW(BKID)] = 360;
			configTab[CFGID_INPUT_DETH(BKID)] = 288;
		}
		configTab[CFGID_INPUT_AIMLV(BKID)] = 1;
		configTab[CFGID_INPUT_FIXAIMW(BKID)] = 40;
		configTab[CFGID_INPUT_FIXAIMH(BKID)] = 40;
		for(int SWLV=0; SWLV<3; SWLV++)
		{
			configTab[CFGID_INPUT_SWAIMW(BKID, SWLV)] = 40;
			configTab[CFGID_INPUT_SWAIMH(BKID, SWLV)] = 40;
		}
	}
	//CFGID_OSD2_BKID
	for(i=16; i<32; i++)
	{
		configTab[CFGID_OSD2_COLOR(i)] = 0x02;
		configTab[CFGID_OSD2_FONT(i)] = 0x02;
		configTab[CFGID_OSD2_SIZE(i)] = 0x02;
	}
	//CFGID_OUTPUT_BKID
	{
		configTab[CFGID_OUTPUT1_res] = 0x01;
		configTab[CFGID_OUTPUT2_res] = 0x01;
	}
	//CFGID_SYS_BKID
	{
		configTab[CFGID_SYSOSD_biten] = 1;
	}
	//CFGID_MTD_BKID
	{
		ALG_CONFIG_Mtd *pCFG_Mtd = (ALG_CONFIG_Mtd *)&configTab[CFGID_MTD_BASE];
		pCFG_Mtd->areaSetBox = 1;
		pCFG_Mtd->detectNum = 5;
		pCFG_Mtd->tmpUpdateSpeed = 30;
		pCFG_Mtd->tmpMaxPixel = 40000;
		pCFG_Mtd->tmpMinPixel = 100;
		pCFG_Mtd->detectSpeed = 80;
		pCFG_Mtd->TrkMaxTime = 10;
		pCFG_Mtd->priority = 1;
		pCFG_Mtd->alarm_delay = 10;
	}

}

void cfg_dbg_getDefault(int * configTab, unsigned char *configUser)
{
	if(configTab != NULL)
		memcpy(sysConfig, configTab, CFGID_BKID_MAX*CFGID_FEILD_MAX*4);
	if(configUser != NULL)
		memcpy(userConfig, configUser, USEROSD_LENGTH*CFGID_USEROSD_MAX);

	SENDST test;
	memset(&test, 0, sizeof(test));
	test.cmd_ID = read_shm_config;
	ipc_sendmsg(&test,IPC_TOIMG_MSG);

}

void cfg_dbg_setCmd(int cmd, int prm)
{
	SENDST test;
	IPC_PRM_INT *pInt = (IPC_PRM_INT *)test.param;
	/////////////////////
	memset(&test, 0, sizeof(test));
	test.cmd_ID = cmd;
	if(cmd == sensor || cmd == sceneTrk || cmd == trk || cmd == mtd || cmd == mtdSelect || cmd == workmode)
	{
		pInt->intPrm[0] = prm;
	}
	else if(cmd == BoresightPos)
	{
		if(prm)
		{
			pInt->intPrm[0] = 350;
			pInt->intPrm[1] = 600;
		}
		else
		{
			pInt->intPrm[0] = 960;
			pInt->intPrm[1] = 540;
		}
	}
	else if(cmd == AcqPos)
	{
		pInt->intPrm[0] = prm;
		if(prm)
		{
			pInt->intPrm[1] = 700;
			pInt->intPrm[2] = 350;
		}
		else
		{
			pInt->intPrm[1] = 960;
			pInt->intPrm[2] = 540;
		}
	}
	else if(cmd == sectrk)
	{
		pInt->intPrm[0] = prm;
		pInt->intPrm[1] = 500;
		pInt->intPrm[2] = 450;
	}
	else if(cmd == posmove)
	{
		if(prm)
		{
			pInt->intPrm[0] = 20;	// 1
			pInt->intPrm[1] = 20;
		}
		else
		{
			pInt->intPrm[0] = -20;	// 2
			pInt->intPrm[1] = -20;
		}
	}
	else if(cmd == Iris || cmd == focus || cmd == exit_IrisAndFocus)
	{
		if(cmd == exit_IrisAndFocus)
		{
			pInt->intPrm[0] = 0;	// CMD_triangle.dir
			pInt->intPrm[1] = 0;	// CMD_triangle.alpha
		}
		else
		{
			if(prm == 0)
			{
				pInt->intPrm[0] = 1;	// CMD_triangle.dir
				pInt->intPrm[1] = 255;	// CMD_triangle.alpha
			}
			else if(prm == 1)
			{
				pInt->intPrm[0] = -1;	// CMD_triangle.dir
				pInt->intPrm[1] = 255;	// CMD_triangle.alpha
			}
			else
			{
				pInt->intPrm[0] = 1;	// CMD_triangle.dir
				pInt->intPrm[1] = 0;	// CMD_triangle.alpha
			}
		}
	}
	else
		return ;

	ipc_sendmsg(&test, IPC_TOIMG_MSG);
	return ;
}


