#include "Ipcctl.h"
#include <string.h>
#include "osa_thr.h"
#include "osa_buf.h"
#include "app_status.h"
#include "app_ctrl.h"
#include "configable.h"
#include "osd_text.hpp"
#include "msgDriv.h"
#include "Ipcctl.h"
#include "locale.h"
#include "wchar.h"

#define DATAIN_TSK_PRI              (2)
#define DATAIN_TSK_STACK_SIZE       (0)
#define SDK_MEM_MALLOC(size)                                            OSA_memAlloc(size)

extern  bool startEnable;
extern OSDSTATUS gConfig_Osd_param ;
extern UTCTRKSTATUS gConfig_Alg_param;
int IrisAndFocusAndExit = 0;
CMD_triangle cmd_triangle;
OSD_param m_osd;
int ipc_loop = 1;

extern void inputtmp(unsigned char cmdid);

extern osdbuffer_t disOsdBuf[32];
extern osdbuffer_t disOsdBufbak[32];
extern wchar_t disOsd[32][33];
extern int glosttime;
OSA_BufCreate msgSendBufCreate;
OSA_BufHndl msgSendBuf;

OSA_ThrHndl thrHandleDataIn_recv;
OSA_ThrHndl thrHandleDataIn_send;

#if LINKAGE_FUNC
	extern SingletonSysParam* g_sysParam;

#endif
void initmessage()
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

void MSGAPI_msgsend(int cmdID)
{
   int bufId = 0;
   unsigned char *quebuf;

   OSA_bufGetEmpty(&(msgSendBuf), &bufId, OSA_TIMEOUT_NONE);
   quebuf=(unsigned char *)msgSendBuf.bufInfo[bufId].virtAddr;

   //msgSendBuf.bufInfo[bufId].size = 7;
   quebuf[0]=cmdID & 0xFF;	

   OSA_bufPutFull(&(msgSendBuf), bufId);
}

int  send_msgpth(SENDST * RS422)
{
	memset(RS422,0,sizeof(char)*(PARAMLEN+1));
	int bufId=0;
	int sendLen=0;
	OSA_bufGetFull(&msgSendBuf, &bufId, OSA_TIMEOUT_FOREVER);
	memcpy(RS422, msgSendBuf.bufInfo[bufId].virtAddr,sizeof(SENDST));
	OSA_bufPutEmpty(&msgSendBuf, bufId);
	send_msg(RS422);
	return 0;
}

void get_acqRect_from_aim(OSDSTATUS *gConfig_Osd_param)
{
	if(gConfig_Osd_param->ch0_aim_width < 10)
		gConfig_Osd_param->ch0_aim_width = 10;
	if(gConfig_Osd_param->ch1_aim_width < 10)
		gConfig_Osd_param->ch1_aim_width = 10;
	if(gConfig_Osd_param->ch2_aim_width < 10)
		gConfig_Osd_param->ch2_aim_width = 10;
	if(gConfig_Osd_param->ch3_aim_width < 10)
		gConfig_Osd_param->ch3_aim_width = 10;
	if(gConfig_Osd_param->ch4_aim_width < 10)
		gConfig_Osd_param->ch4_aim_width = 10;
	if(gConfig_Osd_param->ch5_aim_width < 10)
		gConfig_Osd_param->ch5_aim_width = 10;
	if(gConfig_Osd_param->ch0_aim_height < 10)
		gConfig_Osd_param->ch0_aim_height = 10;
	if(gConfig_Osd_param->ch1_aim_height < 10)
		gConfig_Osd_param->ch1_aim_height = 10;
	if(gConfig_Osd_param->ch2_aim_height < 10)
		gConfig_Osd_param->ch2_aim_height = 10;
	if(gConfig_Osd_param->ch3_aim_height < 10)
		gConfig_Osd_param->ch3_aim_height = 10;
	if(gConfig_Osd_param->ch4_aim_height < 10)
		gConfig_Osd_param->ch4_aim_height = 10;
	if(gConfig_Osd_param->ch5_aim_height < 10)
		gConfig_Osd_param->ch5_aim_height = 10;
	gConfig_Osd_param->ch0_acqRect_width = gConfig_Osd_param->ch0_aim_width;
	gConfig_Osd_param->ch1_acqRect_width = gConfig_Osd_param->ch1_aim_width;
	gConfig_Osd_param->ch2_acqRect_width = gConfig_Osd_param->ch2_aim_width;
	gConfig_Osd_param->ch3_acqRect_width = gConfig_Osd_param->ch3_aim_width;
	gConfig_Osd_param->ch4_acqRect_width = gConfig_Osd_param->ch4_aim_width;
	gConfig_Osd_param->ch5_acqRect_width = gConfig_Osd_param->ch5_aim_width;
	gConfig_Osd_param->ch0_acqRect_height = gConfig_Osd_param->ch0_aim_height;
	gConfig_Osd_param->ch1_acqRect_height = gConfig_Osd_param->ch1_aim_height;
	gConfig_Osd_param->ch2_acqRect_height = gConfig_Osd_param->ch2_aim_height;
	gConfig_Osd_param->ch3_acqRect_height = gConfig_Osd_param->ch3_aim_height;
	gConfig_Osd_param->ch4_acqRect_height = gConfig_Osd_param->ch4_aim_height;
	gConfig_Osd_param->ch5_acqRect_height = gConfig_Osd_param->ch5_aim_height;
}
void* recv_msg(SENDST *RS422)
{
	unsigned char cmdID = 0;
	unsigned char imgID1 = 0;
	unsigned char imgID2 = 0;
	unsigned char imgID3 = 0;
	unsigned char imgID4 = 0;
	unsigned char imgID5 = 0;	
	unsigned char imgID6 = 0;

	CMD_SENSOR Rsensor;
	CMD_PinP Rpinp;
	CMD_TRK Rtrk;
	CMD_SECTRK Rsectrk;
	CMD_ENH Renh;
	CMD_MTD Rmtd;
	CMD_MMTSELECT Rmmtselect;
	CMD_TRKDOOR Rtrkdoor;
	CMD_SYNC422 Rsync422;
	CMD_POSMOVE Rposmove;
	CMD_POSMOVE Raxismove;
	CMD_ZOOM Rzoom;
	CMD_BoresightPos Rboresightmove;
	CMD_AcqBoxPos Racqpos;
	CMD_ALGOSDRECT Ralgosdrect;
	CMD_IPCRESOLUTION Rresolution;
	LinkagePos posOfLinkage;

	//OSD_param* pOsd = NULL;
	//pOsd = &m_osd;

	if(RS422==NULL)
	{
		return NULL ;
	}

	cmdID	=  RS422->cmd_ID;
	imgID1	=  RS422->param[0];
	imgID2	=  RS422->param[1];
	imgID3	=  RS422->param[2];
	imgID4	=  RS422->param[3];
	imgID5 	=  RS422->param[4];

	CMD_EXT inCtrl, *pMsg = NULL;
	pMsg = &inCtrl;
	memset(pMsg,0,sizeof(CMD_EXT));
	if(startEnable)
		app_ctrl_getSysData(pMsg);

	osdbuffer_t* ppppp  = NULL;	
	printf("cmdID : %d (%02x %02x %02x %02x %02x)\n",cmdID,imgID1,imgID2,imgID3,imgID4,imgID5);
	switch(cmdID)
	{	
		case BoresightPos:
			memcpy(&Rboresightmove, RS422->param, sizeof(Rboresightmove));
			pMsg->opticAxisPosX[pMsg->SensorStat] = Rboresightmove.BoresightPos_x;
			pMsg->opticAxisPosY[pMsg->SensorStat] = Rboresightmove.BoresightPos_y;

			pMsg->AxisPosX[pMsg->SensorStat] = pMsg->opticAxisPosX[pMsg->SensorStat];
			pMsg->AxisPosY[pMsg->SensorStat] = pMsg->opticAxisPosY[pMsg->SensorStat];

			pMsg->AvtPosX[pMsg->SensorStat] = pMsg->AxisPosX[pMsg->SensorStat];
			pMsg->AvtPosY[pMsg->SensorStat] = pMsg->AxisPosY[pMsg->SensorStat];
			
			app_ctrl_setBoresightPos(pMsg);
			break;

		case AcqPos:
			memcpy(&Racqpos, RS422->param, sizeof(Racqpos));
			imgID6 = Racqpos.AcqStat;
			if(2 == imgID6){
				pMsg->AxisPosX[pMsg->SensorStat] = Racqpos.BoresightPos_x;
				pMsg->AxisPosY[pMsg->SensorStat] = Racqpos.BoresightPos_y;
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
			else if(1 == imgID6){
				pMsg->AvtTrkStat = eTrk_mode_acqmove;
				pMsg->AvtPosX[pMsg->SensorStat] = Racqpos.BoresightPos_x;
				pMsg->AvtPosY[pMsg->SensorStat] = Racqpos.BoresightPos_y;
				app_ctrl_setTrkStat(pMsg);
				pMsg->AxisPosX[pMsg->SensorStat] = pMsg->opticAxisPosX[pMsg->SensorStat];
				pMsg->AxisPosY[pMsg->SensorStat] = pMsg->opticAxisPosY[pMsg->SensorStat];
				app_ctrl_setAxisPos(pMsg);
				MSGAPI_msgsend(sectrk);
			}
			else if(0 == imgID6){
				pMsg->AxisPosX[pMsg->SensorStat] = Racqpos.BoresightPos_x;
				pMsg->AxisPosY[pMsg->SensorStat] = Racqpos.BoresightPos_y;
				pMsg->AvtTrkStat = eTrk_mode_acq;
				app_ctrl_setAxisPos(pMsg);
			}
			break;

		case osdbuffer:
			memcpy(&disOsdBuf[imgID1],RS422->param,sizeof(osdbuffer_t));
			setlocale(LC_ALL, "zh_CN.UTF-8");
			memcpy(&disOsdBufbak[imgID1],&disOsdBuf[imgID1],sizeof(osdbuffer_t));
			swprintf(disOsd[imgID1], 33, L"%s", disOsdBuf[imgID1].buf);
			break;
		case read_shm_osdtext:
			{
				osdtext_t *osdtexttmp = ipc_getosdtextstatus_p();
				for(int i = 0; i < 32; i++)
				{
					disOsdBuf[i].osdID = osdtexttmp->osdID[i];
					disOsdBuf[i].color = osdtexttmp->color[i];
					disOsdBuf[i].alpha = osdtexttmp->alpha[i];
					disOsdBuf[i].ctrl = osdtexttmp->ctrl[i];
					disOsdBuf[i].posx = osdtexttmp->posx[i];
					disOsdBuf[i].posy = osdtexttmp->posy[i];
					memcpy((void *)disOsdBuf[i].buf, (void *)osdtexttmp->buf[i], sizeof(disOsdBuf[i].buf));
					setlocale(LC_ALL, "zh_CN.UTF-8");
					memcpy(&disOsdBufbak[i],&disOsdBuf[i],sizeof(osdbuffer_t));
					swprintf(disOsd[i], 33, L"%s", disOsdBuf[i].buf);
				}
			}
			break;

		case Iris:
			IrisAndFocusAndExit = Enable_Iris;
			memcpy(&cmd_triangle, RS422->param, sizeof(cmd_triangle));
			break;

		case focus:
			IrisAndFocusAndExit = Enable_Focus;
			memcpy(&cmd_triangle, RS422->param, sizeof(cmd_triangle));
			break;

		case exit_IrisAndFocus:
			IrisAndFocusAndExit = Disable;
			memcpy(&cmd_triangle, RS422->param, sizeof(cmd_triangle));
			break;

		case read_shm_config:
			//if(!startEnable)
			{		
				OSDSTATUS *osdtmp = ipc_getosdstatus_p();
				UTCTRKSTATUS *utctmp = ipc_getutstatus_p();
				memcpy(&gConfig_Alg_param,utctmp,sizeof(UTCTRKSTATUS));
				memcpy(&gConfig_Osd_param,osdtmp,sizeof(OSDSTATUS));	
				get_acqRect_from_aim(&gConfig_Osd_param);
				MSGDRIV_send(MSGID_EXT_UPDATE_OSD, 0);
				startEnable = 1;
			}
			break;
			
		case read_shm_osd:
			{
				printf("read_shm_osd  \n\n");
				OSDSTATUS *osdtmp = ipc_getosdstatus_p();
				memcpy(&gConfig_Osd_param,osdtmp,sizeof(OSDSTATUS));
				get_acqRect_from_aim(&gConfig_Osd_param);	
				
				MSGDRIV_send(MSGID_EXT_UPDATE_OSD, 0);
			}
			break;

		case read_shm_utctrk:
			{
				UTCTRKSTATUS *utctmp = ipc_getutstatus_p();
				memcpy(&gConfig_Alg_param,utctmp,sizeof(UTCTRKSTATUS));	
				MSGDRIV_send(MSGID_EXT_UPDATE_ALG, 0);	
			}
			break;

		case read_shm_camera:
			break;
		case ipclosttime:
			int losttime;
			memcpy(&losttime,RS422->param,4);
			glosttime = losttime;
			break;

	#if !LINKAGE_FUNC
		case trk:				
			memcpy(&Rtrk,RS422->param,sizeof(Rtrk));
			imgID1 = Rtrk.AvtTrkStat;
			printf("recv TRK : imgID1 : %d\n",imgID1);
			if(imgID1 == 0x1)
				pMsg->AvtTrkStat =eTrk_mode_target;
			else
				pMsg->AvtTrkStat = eTrk_mode_acq;

			if(pMsg->AvtTrkStat == eTrk_mode_acq)
			{
				pMsg->AxisPosX[pMsg->SensorStat] = pMsg->opticAxisPosX[pMsg->SensorStat];
				pMsg->AxisPosY[pMsg->SensorStat] = pMsg->opticAxisPosY[pMsg->SensorStat];
				pMsg->AvtPosX[pMsg->SensorStat]  = pMsg->AxisPosX[pMsg->SensorStat];
				pMsg->AvtPosY[pMsg->SensorStat]  = pMsg->AxisPosY[pMsg->SensorStat];
				
				app_ctrl_setAimPos(pMsg);
				app_ctrl_setAxisPos(pMsg);
			}

			app_ctrl_setTrkStat(pMsg); 
			MSGAPI_msgsend(trk);
			break;
	#endif	
	
		case mmt:
			memcpy(&Rmtd,RS422->param,sizeof(Rmtd));
			imgID1 = Rmtd.ImgMtdStat;	
			printf("recv mmt : imgID1 : %d\n",imgID1);

			if(imgID1 == 0x01)
			{
				pMsg->MmtStat[pMsg->SensorStat] = eImgAlg_Enable;	
			}
			else if(imgID1 == 0x00)
			{
				pMsg->MmtStat[pMsg->SensorStat] = eImgAlg_Disable;
			}
			app_ctrl_setMMT(pMsg);
			MSGAPI_msgsend(mmt);			
			break;

		case mmtselect:
			memcpy(&Rmmtselect,RS422->param,sizeof(Rmmtselect));
			imgID1 = Rmmtselect.ImgMmtSelect;	
			printf("recv mmtselect : imgID1 : %d\n",imgID1);
			imgID1--;
			if(imgID1 >5)
				imgID1 =5;
			
			app_ctrl_setMmtSelect(pMsg,imgID1);	
			pMsg->MmtStat[pMsg->SensorStat] = eImgAlg_Disable;
			app_ctrl_setMMT(pMsg);
			MSGAPI_msgsend(mmt);
			MSGAPI_msgsend(trk);
			break;
			
		case enh:
			memcpy(&Renh,RS422->param,sizeof(Renh));
			imgID1 = Renh.ImgEnhStat;	
			printf("recv enh : imgID1 : %d\n",imgID1);
			if(imgID1 == 1){
				pMsg->ImgEnhStat[pMsg->validChId] = ipc_eImgAlg_Enable;
			}
			else if(imgID1 == 0){
				pMsg->ImgEnhStat[pMsg->validChId] = ipc_eImgAlg_Disable;
			}	
			app_ctrl_setEnhance(pMsg);
			MSGAPI_msgsend(enh);
			break;
#if __MOVE_DETECT__
		case mtd:
			memcpy(&Rmtd,RS422->param,sizeof(Rmtd));
			imgID1 = Rmtd.ImgMtdStat;	
			imgID2 = Rmtd.mtdMode;
			printf("recv mtd : imgID1 : %d  , mode : %d \n",imgID1,imgID2);

			if(imgID1 == 1){
				pMsg->MtdState[pMsg->SensorStat] = ipc_eImgAlg_Enable;
			}
			else if(imgID1 == 0){
				pMsg->MtdState[pMsg->SensorStat] = ipc_eImgAlg_Disable;
			}
			app_ctrl_setMtdStat(pMsg);
			MSGAPI_msgsend(mtd);
			break;

	#if !LINKAGE_FUNC
		case mtdSelect:		
			memcpy(&Rmmtselect,RS422->param,sizeof(Rmmtselect));
			pMsg->MtdSelect[pMsg->SensorStat] = Rmmtselect.ImgMmtSelect;
			app_ctrl_setMtdSelect(pMsg);
			if(ipc_eMMT_Select == Rmmtselect.ImgMmtSelect)
			{
				MSGAPI_msgsend(mtd);
			}
			break;
	#endif
			
#endif
		case sectrk:
			memcpy(&Rsectrk,RS422->param,sizeof(Rsectrk));
			imgID1 = Rsectrk.SecAcqStat;
			if(pMsg->AvtTrkStat != eTrk_mode_acq)
			{
				if(1 == imgID1){
					pMsg->AxisPosX[pMsg->SensorStat] = Rsectrk.ImgPixelX;
					pMsg->AxisPosY[pMsg->SensorStat] = Rsectrk.ImgPixelY;

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
					MSGAPI_msgsend(sectrk);	
				}
				else if(0 == imgID1){
					pMsg->AvtTrkStat = eTrk_mode_sectrk;
					pMsg->AvtPosX[pMsg->SensorStat] = Rsectrk.ImgPixelX;
					pMsg->AvtPosY[pMsg->SensorStat] = Rsectrk.ImgPixelY;				
					app_ctrl_setTrkStat(pMsg);
					pMsg->AxisPosX[pMsg->SensorStat] = pMsg->opticAxisPosX[pMsg->SensorStat];
					pMsg->AxisPosY[pMsg->SensorStat] = pMsg->opticAxisPosY[pMsg->SensorStat];
					app_ctrl_setAxisPos(pMsg);
					MSGAPI_msgsend(sectrk);						
				}			
			}
			break;

		case sensor:
			memcpy(&Rsensor,RS422->param,sizeof(Rsensor));
			printf("recv Rsensor: %d\n",Rsensor.SensorStat);
			pMsg->SensorStat = Rsensor.SensorStat;
			app_ctrl_setSensor(pMsg);
			MSGAPI_msgsend(sensor);
			break;
		
		case pinp:	
			memcpy(&Rpinp,RS422->param,sizeof(Rpinp));
			printf("recv pinp : Rpinp.ImgPicp : %d\n",Rpinp.ImgPicp);
			/*if(Rpinp.ImgPicp == 1)
				pMsg->PicpSensorStat = 0x1;
			else 
				pMsg->PicpSensorStat = 0xff;*/

			pMsg->ImgPicp[pMsg->SensorStat] = Rpinp.ImgPicp;	
			pMsg->PicpSensorStat = Rpinp.PicpSensorStat;
			app_ctrl_setPicp(pMsg);
			break;
					
		case trkdoor:	
			AcqBoxWH aimsize;
			memcpy(&aimsize,RS422->param,sizeof(aimsize));
			pMsg->AimW[pMsg->SensorStat] = aimsize.AimW;
			pMsg->AimH[pMsg->SensorStat]  = aimsize.AimH;
			app_ctrl_setAimSize(pMsg);	

			pMsg->AcqRectW[pMsg->SensorStat] = aimsize.AimW;
			pMsg->AcqRectH[pMsg->SensorStat]  = aimsize.AimH;
			app_ctrl_setAcqRect(pMsg);
			
			MSGAPI_msgsend(trkdoor);
			break;	
			
		case posmove:	
			memcpy(&Rposmove,RS422->param,sizeof(Rposmove));
			printf("recv posmove : Rposmove.AvtMoveX : %d    Rposmove.AvtMoveY :%d\n",Rposmove.AvtMoveX,Rposmove.AvtMoveY);
			pMsg->aimRectMoveStepX = Rposmove.AvtMoveX;
			pMsg->aimRectMoveStepY = Rposmove.AvtMoveY;
			app_ctrl_setAimPos(pMsg);
			break;	

		case elecZoom:
			memcpy(&Rzoom,RS422->param,sizeof(Rzoom));
			printf("recv zoom : Rzoom.ImgZoomStat : %d\n",Rzoom.ImgZoomStat);
			pMsg->ImgZoomStat[pMsg->SensorStat] = Rzoom.ImgZoomStat;
			app_ctrl_setZoom(pMsg);
			break;

		case autocheck:
			break;

		case axismove:
			memcpy(&Raxismove,RS422->param,sizeof(Raxismove));
			imgID1 = Raxismove.AvtMoveX;
			imgID2 = Raxismove.AvtMoveY;		
			printf("recv axismove : Raxismove.AvtMoveX : %d   Raxismove.AvtMoveY : %d \n",Raxismove.AvtMoveX,Raxismove.AvtMoveY);
			if(imgID1 == eTrk_ref_left)
				pMsg->axisMoveStepX = -1;
			else if(imgID1 == eTrk_ref_right)
				pMsg->axisMoveStepX = 1;
			else
				pMsg->axisMoveStepX = 0;
			
			if(imgID2 == eTrk_ref_up)
				pMsg->axisMoveStepY = -1;
			else if(imgID2 == eTrk_ref_down)
				pMsg->axisMoveStepY = 1;
			else
				pMsg->axisMoveStepY= 0;
			
			app_ctrl_setAxisPos(pMsg);
			break;

		case acqBox:
			/*
			AcqBoxWH acqSize;	
			memcpy(&acqSize,RS422->param,sizeof(acqSize));
			pMsg->AcqRectW[pMsg->SensorStat] = acqSize.AimW;
			pMsg->AcqRectH[pMsg->SensorStat]  = acqSize.AimH;
			app_ctrl_setAcqRect(pMsg);
			MSGAPI_msgsend(acqBox);
			*/
			break;
			
		case algosdrect:
			memcpy(&Ralgosdrect,RS422->param,sizeof(Ralgosdrect));
			imgID1 = Ralgosdrect.Imgalgosdrect;
			printf("algosdrect:%d\n",imgID1);
			if(0 == imgID1)
				pMsg->Imgalgosdrect = 0;
			else if(1 == imgID1)
				pMsg->Imgalgosdrect = 1;
			app_ctrl_setalgosdrect(pMsg);
			break;
		case exit_img:
			MSGAPI_msgsend(exit_img);
			ipc_loop = 0;			
			break;

		case ipcwordFont:
			pMsg->osdTextFont = imgID1 ; 
			app_ctrl_setWordFont(pMsg);		
			break;
		case ipcwordSize:
			pMsg->osdTextSize = imgID1 ; 
			app_ctrl_setWordSize(pMsg);		
			break;
		case ipcresolution:
			memcpy(&Rresolution,RS422->param,sizeof(Rresolution));
			for(int i = 0; i <= ipc_eSen_CH3; i++)
			{
					if((0 == Rresolution.resolution[i])||(1 == Rresolution.resolution[i]))
					{
						vcapWH[i][0] = 1920;
						vcapWH[i][1] = 1080;
						vdisWH[i][0] = 1920;
						vdisWH[i][1] = 1080;
						#if LINKAGE_FUNC
						g_sysParam->getSysParam().gun_camera.raw = vdisWH[i][1];
						g_sysParam->getSysParam().gun_camera.col  = vdisWH[i][0];
						#endif
					}
					else if((2 == Rresolution.resolution[i])||(3 == Rresolution.resolution[i]))
					{
						vcapWH[i][0] = 1280;
						vcapWH[i][1] = 720;
						vdisWH[i][0] = 1280;
						vdisWH[i][1] = 720;
					}
			}
			if(0 == Rresolution.resolution[ipc_eSen_CH4])
			{
				vcapWH[ipc_eSen_CH4][0] = 720;
				vcapWH[ipc_eSen_CH4][1] = 576;
				vdisWH[ipc_eSen_CH4][0] = 720;
				vdisWH[ipc_eSen_CH4][1] = 576;
			}
			break;
			
		case querypos:
			#if LINKAGE_FUNC
				memcpy(&posOfLinkage,RS422->param,sizeof(posOfLinkage));
				printf("[%s]:Query IPC Rcv :>> panPos ,tilPos , zoom = (%d ,%d ,%d) \n",__FUNCTION__,posOfLinkage.panPos, posOfLinkage.tilPos, posOfLinkage.zoom);
				app_ctrl_setLinkagePos(posOfLinkage.panPos, posOfLinkage.tilPos, posOfLinkage.zoom);
				
			#endif
			break;
		default:
			break;
	}
}

int send_msg(SENDST *RS422)
{
	if(msgextInCtrl == NULL)
		return -1;
		
	unsigned char cmdID = 0;
	unsigned char imgID1 = 0;
	unsigned char imgID2 = 0;
	unsigned char imgID3 = 0;
	unsigned char imgID4 = 0;
	unsigned char imgID5 = 0;	
	CMD_EXT pIStuts;

	memcpy(&pIStuts,msgextInCtrl,sizeof(CMD_EXT));

	if(RS422==NULL)
	{
		return  -1 ;
	}
	cmdID = RS422->cmd_ID;
	switch (cmdID)
	{
		case trk:
			RS422->param[0] = pIStuts.AvtTrkStat;
			printf("ack trk  :  %d\n",pIStuts.TrkStat);
			break;
			
		case mmt:
			RS422->param[0] = pIStuts.MmtStat[pIStuts.SensorStat];
			//printf("ack mmt  :  %d\n",RS422->param[0]);
			break;
			
		case mmtselect:
			break;
			
		case enh:
			RS422->param[0] = pIStuts.ImgEnhStat[pIStuts.SensorStat];
			printf("ack enh  :  %d\n",RS422->param[0]);			
			break;
			
		case mtd:
			RS422->param[0] = app_ctrl_getMtdStat();
			printf("ack mtd  :  %d\n",RS422->param[0]);			
			break;
			
		case sectrk:
			RS422->param[0] = pIStuts.AvtTrkStat;
			printf("ack sectrk  :  %d\n",RS422->param[0]);						
			break;
			
		case trkdoor:
			break;	
		
		case posmove:
			switch(pIStuts.aimRectMoveStepX)
			{
				case 0:
					RS422->param[0] = 0;
					break;
				case 1:
					RS422->param[0] = 2;
					break;
				case -1:
					RS422->param[0] = 1;
					break;
				default:
					break;		
			}
			switch(pIStuts.aimRectMoveStepY)
			{
				case 0:
					RS422->param[1] = 0;
					break;
				case 1:
					RS422->param[1] = 2;
					break;
				case -1:
					RS422->param[1] = 1;
					break;
				default:
					break;		
			}
			printf("send ++++++++++ AvtMoveXY = (%02x,%02x)  ++++++++++\n",RS422->param[0],RS422->param[1]);				
			break;
		case sensor:
			RS422->param[0] = pIStuts.SensorStat;	
		case exit_img:					
			break;
		default:
			break;
	}
	return 0;
}

static void * ipc_dataRecv(Void * prm)
{
	SENDST test;
	while(ipc_loop)
	{
		ipc_recvmsg(&test,IPC_TOIMG_MSG);
		recv_msg(&test);			
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
	int shm_perm[IPC_MAX];
	shm_perm[IPC_SHA] = shm_rdwr;
	shm_perm[IPC_OSD_SHA] = shm_rdonly;
	shm_perm[IPC_UTCTRK_SHA] = shm_rdonly;	
	shm_perm[IPC_LKOSD_SHA] = shm_rdonly;
	shm_perm[IPC_OSDTEXT_SHA] = shm_rdonly;
	Ipc_init();
	Ipc_create(shm_perm);

	initmessage();
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
}
