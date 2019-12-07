
#include "app_ctrl.h"
#include "osa.h"
#include "msgDriv.h"
#include "configtable.h"



CMD_EXT *msgextInCtrl;
#define Coll_Save 0 //   1:quit coll is to save  cross  or  0:using save funtion to cross axis
#define FrColl_Change 1 //0:frcoll v1.00 1:frcoll v1.01     //ver1.01 is using 

static int pristatus=0;
LinkagePos_t linkagePos; 



void getMmtTg(unsigned char index,int *x,int *y);
int MmtCoord2mmtTarget(int chid ,unsigned int x,unsigned int y);


#if __MOVE_DETECT__
void getMtdxy(int &x,int &y,int &w,int &h);
void getMtdIdxy(int mtdId,int &x,int &y,int &w,int &h);

#endif


void  app_ctrl_getSysData(CMD_EXT * exthandle)
{
    OSA_assert(exthandle!=NULL);
    if(msgextInCtrl==NULL)
		return ;
    memcpy(exthandle,msgextInCtrl,sizeof(CMD_EXT));
    return ;
}


void app_ctrl_setTrkStat(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;

	if (pInCmd->AvtTrkStat != pIStuts->AvtTrkStat)
	{
		if(pIStuts->AvtTrkStat == eTrk_mode_search && pInCmd->AvtTrkStat == eTrk_mode_target)
		{
			// cancel search
			pIStuts->AvtTrkStat = eTrk_mode_target;
			return ;
		}
		pIStuts->AvtTrkStat = pInCmd->AvtTrkStat;
		if(pIStuts->AvtTrkStat == eTrk_mode_search)
		{
			return ;
		}
		else if(pIStuts->AvtTrkStat==eTrk_mode_sectrk || pIStuts->AvtTrkStat == eTrk_mode_acqmove)
		{
			pIStuts->AvtPosX[pIStuts->SensorStat] = pInCmd->AvtPosX[pIStuts->SensorStat];
			pIStuts->AvtPosY[pIStuts->SensorStat] = pInCmd->AvtPosY[pIStuts->SensorStat];
			pIStuts->AimW[pIStuts->SensorStat] = pInCmd->AimW[pIStuts->SensorStat];
			pIStuts->AimH[pIStuts->SensorStat] = pInCmd->AimH[pIStuts->SensorStat];
		}
		
		if(pInCmd->AvtTrkStat == eTrk_mode_acq)
			pIStuts->TrkStat = 0;
		else if(pInCmd->AvtTrkStat == eTrk_mode_target)
			pIStuts->TrkStat = 1;
		MSGDRIV_send(MSGID_EXT_INPUT_TRACK, 0);
	}
	return ;
}

void app_ctrl_setAimPos(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;

	if (pIStuts->aimRectMoveStepX != pInCmd->aimRectMoveStepX ||pIStuts->aimRectMoveStepY != pInCmd->aimRectMoveStepY)
	{
		pIStuts->aimRectMoveStepX = pInCmd->aimRectMoveStepX;
		pIStuts->aimRectMoveStepY = pInCmd->aimRectMoveStepY;
		MSGDRIV_send(MSGID_EXT_INPUT_AIMPOS, 0);
	}
	else if(pIStuts->AvtPosX[pIStuts->SensorStat]!= pInCmd->AvtPosX[pIStuts->SensorStat] ||
		pIStuts->AvtPosY[pIStuts->SensorStat]!= pInCmd->AvtPosY[pIStuts->SensorStat] )
	{
		pIStuts->AvtPosX[pIStuts->SensorStat] = pInCmd->AvtPosX[pIStuts->SensorStat];
		pIStuts->AvtPosY[pIStuts->SensorStat] = pInCmd->AvtPosY[pIStuts->SensorStat];	
	}
	return ;
}


void app_ctrl_setMmtSelect(CMD_EXT * pIStuts,unsigned char index)
{	
	int curx,cury;
	getMmtTg(index, &curx, &cury);

	CMD_EXT tmp = {0};
	tmp.AvtTrkStat = eTrk_mode_sectrk;
	tmp.AvtPosX[pIStuts->SensorStat] = curx;
	tmp.AvtPosY[pIStuts->SensorStat] = cury;

	tmp.AimW[pIStuts->SensorStat]  = 32;
	tmp.AimH[pIStuts->SensorStat]  = 32;
	app_ctrl_setTrkStat(&tmp);//track

	//pIStuts->AxisPosX[pIStuts->SensorStat] = pIStuts->opticAxisPosX[pIStuts->SensorStat];
	//pIStuts->AxisPosY[pIStuts->SensorStat] = pIStuts->opticAxisPosY[pIStuts->SensorStat];
	//app_ctrl_setAxisPos(pIStuts);
	return ;
}

void app_ctrl_setMtdSelfCorrd(CMD_EXT * pIStuts,unsigned int x,unsigned int y)
{	
	CMD_EXT pMsg;
	memset(&pMsg,0,sizeof(CMD_EXT));

	pMsg.AvtTrkStat =eTrk_mode_sectrk;
	pMsg.AvtPosX[pIStuts->SensorStat]  = vdisWH[pIStuts->SensorStat][0]/2;
	pMsg.AvtPosY[pIStuts->SensorStat]  = vdisWH[pIStuts->SensorStat][1]/2;
	pMsg.AimW[pIStuts->SensorStat]  = (int)(pIStuts->AcqRectW[pIStuts->SensorStat]*1.5);
	pMsg.AimH[pIStuts->SensorStat]  = (int)(pIStuts->AcqRectH[pIStuts->SensorStat]*1.5);
	app_ctrl_setTrkStat(&pMsg);//track
	return ;
}


void app_ctrl_setMtdCorrd(CMD_EXT * pIStuts,unsigned int x,unsigned int y)
{	
	int curx,cury;
	int index = MtdCoord2mtdTarget(pIStuts->SensorStat, x, y);
	if(index == -1)
		return ;

	app_ctrl_trkMtdId(index);

	return ;
}



void app_ctrl_setMmtCorrd(CMD_EXT * pIStuts,unsigned int x,unsigned int y)
{	
	int curx,cury;

	int index = MmtCoord2mmtTarget(pIStuts->SensorStat, x, y);
	if(index == -1)
		return ;
	
	app_ctrl_setMmtSelect(pIStuts,index);
	
	return ;
}


void app_ctrl_setEnhance(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;
	
	if(pInCmd->ImgEnhStat[pInCmd->SensorStat] != pIStuts->ImgEnhStat[pInCmd->SensorStat])
	{
		pIStuts->ImgEnhStat[pInCmd->SensorStat] = pInCmd->ImgEnhStat[pInCmd->SensorStat];
		if(pIStuts->ImgEnhStat[pInCmd->SensorStat]==0)
		{
			pIStuts->ImgEnhStat[pInCmd->SensorStat^1]=0;
		}
		MSGDRIV_send(MSGID_EXT_INPUT_ENENHAN, 0);
	}
	return ;
}


void app_ctrl_setAxisPos(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;
	unsigned char mask = 0;
	if(pInCmd->axisMoveStepX != 0  || pInCmd->axisMoveStepY != 0)
	{	
		pIStuts->axisMoveStepX = pInCmd->axisMoveStepX;
		pIStuts->axisMoveStepY = pInCmd->axisMoveStepY;
		mask++;
	}

	if(pIStuts->AxisPosX[pIStuts->SensorStat] != pInCmd->AxisPosX[pIStuts->SensorStat] || pIStuts->AxisPosY[pIStuts->SensorStat]!= pInCmd->AxisPosY[pIStuts->SensorStat])
	{
		pIStuts->AxisPosX[pIStuts->SensorStat] = pInCmd->AxisPosX[pIStuts->SensorStat];
		pIStuts->AxisPosY[pIStuts->SensorStat] = pInCmd->AxisPosY[pIStuts->SensorStat];
		mask++;
	}
	if(mask)
		MSGDRIV_send(MSGID_EXT_INPUT_AXISPOS, 0);	
	
	return ;
}



void app_ctrl_setBoresightPos(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;

	if(pIStuts->opticAxisPosX[pIStuts->SensorStat] != pInCmd->opticAxisPosX[pIStuts->SensorStat] 
		|| pIStuts->opticAxisPosY[pIStuts->SensorStat]!= pInCmd->opticAxisPosY[pIStuts->SensorStat])
	{
		pIStuts->opticAxisPosX[pIStuts->SensorStat] = pInCmd->opticAxisPosX[pIStuts->SensorStat];
		pIStuts->opticAxisPosY[pIStuts->SensorStat] = pInCmd->opticAxisPosY[pIStuts->SensorStat];
	}

	if(pIStuts->AxisPosX[pIStuts->SensorStat] != pInCmd->AxisPosX[pIStuts->SensorStat] 
		|| pIStuts->AxisPosY[pIStuts->SensorStat]!= pInCmd->AxisPosY[pIStuts->SensorStat])
	{
		pIStuts->AxisPosX[pIStuts->SensorStat] = pInCmd->AxisPosX[pIStuts->SensorStat];
		pIStuts->AxisPosY[pIStuts->SensorStat] = pInCmd->AxisPosY[pIStuts->SensorStat];
	}

	if(pIStuts->AvtPosX[pIStuts->SensorStat] != pInCmd->AvtPosX[pIStuts->SensorStat] 
		|| pIStuts->AvtPosY[pIStuts->SensorStat]!= pInCmd->AvtPosY[pIStuts->SensorStat])
	{
		pIStuts->AvtPosX[pIStuts->SensorStat] = pInCmd->AvtPosX[pIStuts->SensorStat];
		pIStuts->AvtPosY[pIStuts->SensorStat] = pInCmd->AvtPosY[pIStuts->SensorStat];
	}


	//printf("pIStuts->opticAxisPosX[%d] = %d \n",pIStuts->SensorStat,pIStuts->opticAxisPosX[pIStuts->SensorStat]);
	//printf("pIStuts->opticAxisPosY[%d] = %d \n",pIStuts->SensorStat,pIStuts->opticAxisPosY[pIStuts->SensorStat]);

	//printf("pIStuts->AxisPosX[%d] = %d \n",pIStuts->SensorStat,pIStuts->AxisPosX[pIStuts->SensorStat]);
	//printf("pIStuts->AvtPosX[%d] = %d \n",pIStuts->SensorStat,pIStuts->AvtPosX[pIStuts->SensorStat]);

	//printf("address  pIStuts->AvtPosX = %x \n",pIStuts->AvtPosX);

	return ;
}


#if __MOVE_DETECT__
void app_ctrl_setMtdStat(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;
	
	if( pInCmd->AvtTrkStat == eTrk_mode_acq  && !pIStuts->MmtStat[pIStuts->SensorStat] && !pInCmd->ImgEnhStat[pIStuts->SensorStat])
	{
		if((pIStuts->MtdState[pIStuts->SensorStat] != pInCmd->MtdState[pIStuts->SensorStat]))
		{
			pIStuts->MtdState[pIStuts->SensorStat] = pInCmd->MtdState[pIStuts->SensorStat];
			MSGDRIV_send(MSGID_EXT_MVDETECT, 0);
		}
	}
	return ;
}


void app_ctrl_trkMtdId(int mtdId)
{
	CMD_EXT pMsg;
	memset(&pMsg,0,sizeof(CMD_EXT));
	
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;

	int curx,cury,curw,curh;
	getMtdIdxy(mtdId,curx,cury,curw,curh);
	if( -1 == curw || -1 == curh )	
	{
		//do nothing
		printf(" mtd target get failed do nothing \n");
	}
	else
	{
		pMsg.AvtTrkStat =eTrk_mode_sectrk;
		pMsg.AvtPosX[pIStuts->SensorStat]  = curx;
		pMsg.AvtPosY[pIStuts->SensorStat]  = cury;
		pMsg.AimW[pIStuts->SensorStat]  = curw;
		pMsg.AimH[pIStuts->SensorStat]  = curh;
		app_ctrl_setTrkStat(&pMsg);//track
	}	
	return ;	
}


void app_ctrl_setMtdSelect(CMD_EXT * pInCmd)
{
	CMD_EXT pMsg;
	memset(&pMsg,0,sizeof(CMD_EXT));
	
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;
	
	if(2 == pInCmd->MtdSelect[pInCmd->SensorStat] || 1 == pInCmd->MtdSelect[pInCmd->SensorStat])
	{
		if(eImgAlg_Enable == pIStuts->MtdState[pIStuts->SensorStat])
		{
			pIStuts->MtdSelect[pIStuts->SensorStat] = pInCmd->MtdSelect[pIStuts->SensorStat];
			MSGDRIV_send(MSGID_EXT_MVDETECTSELECT, 0);
		}
	}
	else if(3 == pInCmd->MtdSelect[pInCmd->SensorStat])
	{
		int curx,cury,curw,curh;
		getMtdxy(curx, cury, curw, curh);
		if( -1 == curw || -1 == curh )	
		{
			//do nothing
			printf(" mtd target get failed do nothing \n");
		}
		else
		{
			pMsg.AvtTrkStat =eTrk_mode_sectrk;
			pMsg.AvtPosX[pIStuts->SensorStat]  = curx;
			pMsg.AvtPosY[pIStuts->SensorStat]  = cury;
			pMsg.AimW[pIStuts->SensorStat]  = curw;
			pMsg.AimH[pIStuts->SensorStat]  = curh;
			app_ctrl_setTrkStat(&pMsg);//track

			//pMsg.MtdState[pMsg.SensorStat] = eImgAlg_Disable;
			//app_ctrl_setMtdStat(&pMsg);//close
		}	
	}
	return ;
}
#endif

void app_ctrl_setMMT(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;

	if(pInCmd->MMTTempStat != pIStuts->MMTTempStat)
		pIStuts->MMTTempStat = pInCmd->MMTTempStat;

	if(pInCmd->AvtTrkStat != eTrk_mode_target && !pInCmd->MtdState[pIStuts->SensorStat] && !pInCmd->ImgEnhStat[pIStuts->SensorStat])
	{
		if (pIStuts->MmtStat[pIStuts->SensorStat] != pInCmd->MmtStat[pIStuts->SensorStat])
		{     
			pIStuts->MmtStat[pIStuts->SensorStat] = pInCmd->MmtStat[pIStuts->SensorStat];
			{
			    MSGDRIV_send(MSGID_EXT_INPUT_ENMTD, 0);
			}
		}
	}
	return ;
}

void app_ctrl_Sensorchange(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;
	pIStuts->AxisPosX[pIStuts->SensorStat]=pIStuts->opticAxisPosX[pIStuts->SensorStat];
	pIStuts->AxisPosY[pIStuts->SensorStat]=pIStuts->opticAxisPosY[pIStuts->SensorStat];
}

void app_ctrl_setReset(CMD_EXT * pInCmd)
{

	if(msgextInCtrl==NULL)
		return ;
    CMD_EXT *pIStuts = msgextInCtrl;
	if(pIStuts->AvtTrkStat != eTrk_mode_acq){
		pIStuts->AvtTrkStat = eTrk_mode_acq;
		 MSGDRIV_send(MSGID_EXT_INPUT_TRACK, 0);
	}
	if(pIStuts->MmtStat[pIStuts->SensorStat] != eImgAlg_Disable){
		pIStuts->MmtStat[pIStuts->SensorStat] = eImgAlg_Disable;
		MSGDRIV_send(MSGID_EXT_INPUT_ENMTD, 0);
	}
	if(pIStuts->ImgEnhStat[pIStuts->SensorStat] == 0x01){
		pIStuts->ImgEnhStat[pIStuts->SensorStat] = 0x00;
		MSGDRIV_send(MSGID_EXT_INPUT_ENENHAN, 0);
	}
}

void app_ctrl_setSensor(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;

	CMD_EXT *pIStuts = msgextInCtrl;
	if (pIStuts->SensorStat != pInCmd->SensorStat)
	{
		pIStuts->changeSensorFlag = 1;
		pIStuts->SensorStatpri = pIStuts->SensorStat;
		pIStuts->SensorStat = pInCmd->SensorStat;
		//app_ctrl_Sensorchange(pInCmd);
		MSGDRIV_send(MSGID_EXT_INPUT_SENSOR, 0);
	}
	return ;
}


void app_ctrl_setZoom(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;
	   
	if(pIStuts->ImgZoomStat[0] != pInCmd->ImgZoomStat[0])
	{
		pIStuts->ImgZoomStat[0] = pInCmd->ImgZoomStat[0];
		MSGDRIV_send(MSGID_EXT_INPUT_ENZOOM, 0);
	}
	return ;
}


void app_ctrl_setAimSize(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;
	int i, enable = 0;

	//for(i=0; i<CAMERACHMAX; i++)	// sync all aimsize

	i = pIStuts->SensorStat;
	{
		if (pIStuts->AimH[i] != pInCmd->AimH[i])
		{
			pIStuts->AimH[i] = pInCmd->AimH[i];
			if(i == pInCmd->SensorStat)
				enable=1;
		}
		if (pIStuts->AimW[i] != pInCmd->AimW[i])
		{
			pIStuts->AimW[i] = pInCmd->AimW[i];
			if(i == pInCmd->SensorStat)
				enable=1;
		}
	}

	if(enable)
		MSGDRIV_send(MSGID_EXT_INPUT_AIMSIZE, 0);
	return ;
}


void app_ctrl_setSerTrk(CMD_EXT * pInCmd )
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;

	if((pInCmd->AvtPosX[pIStuts->SensorStat] != pIStuts->AvtPosX[pIStuts->SensorStat])
		|| (pInCmd->AvtPosY[pIStuts->SensorStat] != pIStuts->AvtPosY[pIStuts->SensorStat]))
	{
		pIStuts->AvtPosX[pIStuts->SensorStat] = pInCmd->AvtPosX[pIStuts->SensorStat];
		pIStuts->AvtPosY[pIStuts->SensorStat] = pInCmd->AvtPosY[pIStuts->SensorStat];
	}	
	return ;	
}

void app_ctrl_setPicp(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;
	bool enable = false;
	if(pIStuts->PicpSensorStat != pInCmd->PicpSensorStat)
	{
		pIStuts->PicpSensorStat = pInCmd->PicpSensorStat;
		enable = true;
	}
	if(pIStuts->ImgPicp[pIStuts->SensorStat] != pInCmd->ImgPicp[pIStuts->SensorStat])
	{
		pIStuts->ImgPicp[pIStuts->SensorStat] = pInCmd->ImgPicp[pIStuts->SensorStat];
		if(pIStuts->ImgPicp[pIStuts->SensorStat]==0)
			pIStuts->PicpSensorStat = 255;
		
		enable = true;
	}
	if(enable)
		MSGDRIV_send(MSGID_EXT_INPUT_PICPCROP, 0);
	return ;
}


void app_ctrl_setAcqRect(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;
	int i;
	for(i=0; i<CAMERACHMAX; i++)	// sync all aimsize
	{
		if (pIStuts->AcqRectW[i] != pInCmd->AcqRectW[i])
		{
			pIStuts->AcqRectW[i] = pInCmd->AcqRectW[i];
		}
		if (pIStuts->AcqRectH[i] != pInCmd->AcqRectH[i])
		{
			pIStuts->AcqRectH[i] = pInCmd->AcqRectH[i];
		}
	}
	return ;
}

void app_ctrl_setSceneTrk(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;
	
	if (pInCmd->SceneAvtTrkStat != pIStuts->SceneAvtTrkStat)
	{
		pIStuts->SceneAvtTrkStat = pInCmd->SceneAvtTrkStat;
		if(pIStuts->SceneAvtTrkStat == eTrk_mode_search)
		{
			return ;
		}	
		
		MSGDRIV_send(MSGID_EXT_INPUT_SCENETRK, 0);
	}

	return ;
}
