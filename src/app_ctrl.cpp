
#include"app_ctrl.h"
#include"osa.h"
#include "msgDriv.h"
#include "configable.h"

CMD_EXT *msgextInCtrl;
#define Coll_Save 0 //   1:quit coll is to save  cross  or  0:using save funtion to cross axis
#define FrColl_Change 1 //0:frcoll v1.00 1:frcoll v1.01     //ver1.01 is using 

static int pristatus=0;
void getMmtTg(unsigned char index,int *x,int *y);


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
		pIStuts->AvtTrkStat = pInCmd->AvtTrkStat;
		if(pIStuts->AvtTrkStat == eTrk_mode_search)
		{
			return ;
		}	
		else if(pIStuts->AvtTrkStat==eTrk_mode_sectrk || pIStuts->AvtTrkStat == eTrk_mode_acqmove)
		{
			pIStuts->AvtPosX[pIStuts->SensorStat] = pInCmd->AvtPosX[pIStuts->SensorStat];
			pIStuts->AvtPosY[pIStuts->SensorStat] = pInCmd->AvtPosY[pIStuts->SensorStat];
		}
		
		if(pInCmd->AvtTrkStat == eTrk_mode_acq)
			pIStuts->TrkStat = 0;
		else if(pInCmd->AvtTrkStat == eTrk_mode_target)
			pIStuts->TrkStat = 1;
		MSGDRIV_send(MSGID_EXT_INPUT_TRACK, 0);
	}
	return ;
}



void app_ctrl_setSysmode(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;
	if(pInCmd->SysMode != pIStuts->SysMode)
		pIStuts->SysMode = pInCmd->SysMode;
	return ;
}

unsigned char app_ctrl_getSysmode()
{
	if(msgextInCtrl==NULL)
		return 255;
	return  msgextInCtrl->SysMode;
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
	
	pIStuts->AvtTrkStat = eTrk_mode_sectrk;
	pIStuts->AvtPosX[pIStuts->SensorStat] = curx;
	pIStuts->AvtPosY[pIStuts->SensorStat] = cury;
	app_ctrl_setTrkStat(pIStuts);

	pIStuts->AxisPosX[pIStuts->SensorStat] = pIStuts->opticAxisPosX[pIStuts->SensorStat];
	pIStuts->AxisPosY[pIStuts->SensorStat] = pIStuts->opticAxisPosY[pIStuts->SensorStat];
	app_ctrl_setAxisPos(pIStuts);
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


	printf("pIStuts->opticAxisPosX[%d] = %d \n",pIStuts->SensorStat,pIStuts->opticAxisPosX[pIStuts->SensorStat]);
	printf("pIStuts->opticAxisPosY[%d] = %d \n",pIStuts->SensorStat,pIStuts->opticAxisPosY[pIStuts->SensorStat]);

	printf("pIStuts->AxisPosX[%d] = %d \n",pIStuts->SensorStat,pIStuts->AxisPosX[pIStuts->SensorStat]);
	printf("pIStuts->AvtPosX[%d] = %d \n",pIStuts->SensorStat,pIStuts->AvtPosX[pIStuts->SensorStat]);

	printf("address  pIStuts->AvtPosX = %x \n",pIStuts->AvtPosX);

	return ;
}



void app_ctrl_setMtdStat(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;

	if(pIStuts->MtdState[pIStuts->SensorStat] != pInCmd->MtdState[pIStuts->SensorStat])
	{
		pIStuts->MtdState[pIStuts->SensorStat] = pInCmd->MtdState[pIStuts->SensorStat];
		MSGDRIV_send(MSGID_EXT_MVDETECT, 0);
	}
	return ;
}


unsigned char app_ctrl_getMtdStat()
{
	if(msgextInCtrl==NULL)
		return 0xff;
	CMD_EXT *pIStuts = msgextInCtrl;

	return pIStuts->MtdState[pIStuts->validChId];
}


void app_ctrl_setMMT(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;

	if(pInCmd->MMTTempStat != pIStuts->MMTTempStat)
		pIStuts->MMTTempStat = pInCmd->MMTTempStat;

	if(pInCmd->AvtTrkStat != eTrk_mode_target)
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
	bool enable = 0;
	if (pIStuts->AimH[pIStuts->validChId] != pInCmd->AimH[pInCmd->validChId])
	{
		pIStuts->AimH[pIStuts->validChId] = pInCmd->AimH[pInCmd->validChId];
		enable=1;
	}
	if (pIStuts->AimW[pIStuts->validChId] != pInCmd->AimW[pInCmd->validChId])
	{
		pIStuts->AimW[pIStuts->validChId] = pInCmd->AimW[pInCmd->validChId];
		enable=1;
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


void app_ctrl_setDispGrade(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;

    if (pIStuts->DispGrp[0] != pInCmd->DispGrp[0] 
                        || pIStuts->DispGrp[1] != pInCmd->DispGrp[1])
    {
        pIStuts->DispGrp[0] = pInCmd->DispGrp[0];
        pIStuts->DispGrp[1] = pInCmd->DispGrp[1];
    }
	return ;
}


void app_ctrl_setDispColor(CMD_EXT * pInCmd )
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;
	  
	if(pInCmd->DispColor[0] !=0x07)
	{
		if (pIStuts->DispColor[0] != pInCmd->DispColor[0] 
	                    || pIStuts->DispColor[1] != pInCmd->DispColor[1])
		{
		    pIStuts->DispColor[0] = pInCmd->DispColor[0];
		    pIStuts->DispColor[1] = pInCmd->DispColor[1]; 
		}
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
	if(pIStuts->AcqRectW[pIStuts->validChId] != pInCmd->AcqRectW[pInCmd->validChId])
	{
		pIStuts->AcqRectW[pIStuts->validChId] = pInCmd->AcqRectW[pInCmd->validChId];
	}
	if(pIStuts->AcqRectH[pIStuts->validChId] != pInCmd->AcqRectH[pInCmd->validChId])
	{
		pIStuts->AcqRectH[pIStuts->validChId] = pInCmd->AcqRectH[pInCmd->validChId];
	}
}

void app_ctrl_setalgosdrect(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;
	if(pIStuts->Imgalgosdrect != pInCmd->Imgalgosdrect)
	{
		pIStuts->Imgalgosdrect = pInCmd->Imgalgosdrect;
		MSGDRIV_send(MSGID_EXT_INPUT_ALGOSDRECT, 0);
	}

}

void app_ctrl_setWordFont(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;
	if(pIStuts->osdTextFont != pInCmd->osdTextFont)
	{
		pIStuts->osdTextFont = pInCmd->osdTextFont;
	}
}

void app_ctrl_setWordSize(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;
	if(pIStuts->osdTextSize != pInCmd->osdTextSize)
	{
		pIStuts->osdTextSize = pInCmd->osdTextSize;
	}
}