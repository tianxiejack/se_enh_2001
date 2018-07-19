#include "Ipcctl.h"
#include <string.h>
#include "osa_thr.h"
#include "osa_buf.h"
#include "app_status.h"
#include "app_ctrl.h"
#include "configable.h"
#include "osd_text.hpp"
#include "msgDriv.h"


#define DATAIN_TSK_PRI              (2)
#define DATAIN_TSK_STACK_SIZE       (0)
#define SDK_MEM_MALLOC(size)                                            OSA_memAlloc(size)

extern  bool startEnable;
extern OSDSTATUS gConfig_Osd_param ;
extern UTCTRKSTATUS gConfig_Alg_param;
int ipc_loop = 1;
extern void inputtmp(unsigned char cmdid);

OSA_BufCreate msgSendBufCreate;
OSA_BufHndl msgSendBuf;

OSA_ThrHndl thrHandleDataIn_recv;
OSA_ThrHndl thrHandleDataIn_send;

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

void* recv_msg(SENDST *RS422)
{
	unsigned char cmdID = 0;
	unsigned char imgID1 = 0;
	unsigned char imgID2 = 0;
	unsigned char imgID3 = 0;
	unsigned char imgID4 = 0;
	unsigned char imgID5 = 0;	

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
	
	printf("cmdID : %d (%02x %02x %02x %02x %02x)\n",cmdID,imgID1,imgID2,imgID3,imgID4,imgID5);
	switch(cmdID)
	{	
	
		case read_shm_config:
			if(!startEnable){		
				OSDSTATUS *osdtmp = ipc_getosdstatus_p();
				UTCTRKSTATUS *utctmp = ipc_getutstatus_p();
				memcpy(&gConfig_Alg_param,utctmp,sizeof(UTCTRKSTATUS));
				memcpy(&gConfig_Osd_param,osdtmp,sizeof(OSDSTATUS));
				startEnable = 1;
			}
			break;
			
		case read_shm_osd:
			{
				printf("read_shm_osd  \n\n");
				OSDSTATUS *osdtmp = ipc_getosdstatus_p();
				memcpy(&gConfig_Osd_param,osdtmp,sizeof(OSDSTATUS));
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
			
		case trk:	
			memcpy(&Rtrk,RS422->param,sizeof(Rtrk));
			imgID1 = Rtrk.AvtTrkStat;
			printf("recv TRK : imgID1 : %d\n",imgID1);
			if(imgID1 == 0x1)
				pMsg->AvtTrkStat =eTrk_mode_target;
			else
				pMsg->AvtTrkStat = eTrk_mode_acq;
			//printf("avt status = %d \n",pMsg->AvtTrkStat);
			if(pMsg->AvtTrkStat == eTrk_mode_acq)
			{
				pMsg->AxisPosX[pMsg->SensorStat] = pMsg->opticAxisPosX[pMsg->SensorStat];
				pMsg->AxisPosY[pMsg->SensorStat] = pMsg->opticAxisPosY[pMsg->SensorStat];
				pMsg->AvtPosX[pMsg->SensorStat]  = pMsg->AxisPosX[pMsg->SensorStat];
				pMsg->AvtPosY[pMsg->SensorStat]  = pMsg->AxisPosY[pMsg->SensorStat];
				//printf("sensor = %d , avtpos x,y(%d  %d ) \n",pMsg->SensorStat,
				//	pMsg->AvtPosX[pMsg->SensorStat],pMsg->opticAxisPosY[pMsg->SensorStat]);
				
				app_ctrl_setAimPos(pMsg);
				app_ctrl_setAxisPos(pMsg);
			}
			
			app_ctrl_setTrkStat(pMsg); 
			MSGAPI_msgsend(trk);
			break;
			
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

		case mtd:
			memcpy(&Rmtd,RS422->param,sizeof(Rmtd));
			imgID1 = Rmtd.ImgMtdStat;	
			printf("recv mtd : imgID1 : %d\n",imgID1);

			if(imgID1 == 1){
				pMsg->MtdState[pMsg->validChId] = ipc_eImgAlg_Enable;
			}
			else if(imgID1 == 0){
				pMsg->MtdState[pMsg->validChId] = ipc_eImgAlg_Disable;
			}	
			app_ctrl_setMtdStat(pMsg);
			MSGAPI_msgsend(mtd);
			break;

		case sectrk:
			memcpy(&Rsectrk,RS422->param,sizeof(Rsectrk));
			imgID1 = Rsectrk.SecAcqStat;
			if(pMsg->AvtTrkStat != eTrk_mode_acq)
			{
				if(1 == imgID1){
					pMsg->AxisPosX[pMsg->SensorStat] = Rsectrk.ImgPixelX;
					pMsg->AxisPosY[pMsg->SensorStat] = Rsectrk.ImgPixelY;

					int width = 0,height = 0;
					if(pMsg->SensorStat == video_pal){
						width  = VIDEO_IMAGE_WIDTH_0;
						height = VIDEO_IMAGE_HEIGHT_0;
					}else{
						width  = VIDEO_IMAGE_WIDTH_1;
						height = VIDEO_IMAGE_HEIGHT_1;
					}
					if(pMsg->AxisPosX[pMsg->SensorStat] + pMsg->crossAxisWidth/2 > width)
						pMsg->AxisPosX[pMsg->SensorStat] = width - pMsg->crossAxisWidth/2;
					if(pMsg->AxisPosY[pMsg->SensorStat] + pMsg->crossAxisHeight/2 > height)
						pMsg->AxisPosY[pMsg->SensorStat] = height - pMsg->crossAxisHeight/2;

					if(pMsg->AxisPosX[pMsg->SensorStat] <  pMsg->crossAxisWidth/2)
						pMsg->AxisPosX[pMsg->SensorStat] =  pMsg->crossAxisWidth/2;
					if(pMsg->AxisPosY[pMsg->SensorStat]  <  pMsg->crossAxisHeight/2)
						pMsg->AxisPosY[pMsg->SensorStat] =  pMsg->crossAxisHeight/2;
	
					pMsg->AvtTrkStat =eTrk_mode_search;
					app_ctrl_setTrkStat(pMsg);
					app_ctrl_setAxisPos(pMsg);
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
			AcqBoxWH acqSize;	
			memcpy(&acqSize,RS422->param,sizeof(acqSize));
			pMsg->AcqRectW[pMsg->SensorStat] = acqSize.AimW;
			pMsg->AcqRectH[pMsg->SensorStat]  = acqSize.AimH;
			app_ctrl_setAcqRect(pMsg);
			MSGAPI_msgsend(acqBox);
			break;
			
		case exit_img:
			MSGAPI_msgsend(exit_img);
			ipc_loop = 0;			
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
			printf("ack trk  :  %d\n",RS422->param[0]);
			break;
			
		case mmt:
			RS422->param[0] = pIStuts.MmtStat[pIStuts.SensorStat];
			printf("ack mmt  :  %d\n",RS422->param[0]);
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
