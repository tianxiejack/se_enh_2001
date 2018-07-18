
#include"app_ctrl.h"
#include"osa.h"
#include "msgDriv.h"
#include"dx.h"
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
		else if(pIStuts->AvtTrkStat==eTrk_mode_sectrk)
		{
			pIStuts->AvtPosX[0] = pInCmd->AvtPosX[0];
			pIStuts->AvtPosY[0] = pInCmd->AvtPosY[0];
		}
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
	else if(pIStuts->AvtPosX[0]!= pInCmd->AvtPosX[0] ||pIStuts->AvtPosY[0]!= pInCmd->AvtPosY[0] )
	{
		pIStuts->AvtPosX[0] = pInCmd->AvtPosX[0];
		pIStuts->AvtPosY[0] = pInCmd->AvtPosY[0];		
	}
	return ;
}


void app_ctrl_setMmtSelect(CMD_EXT * pIStuts,unsigned char index)
{	
	int curx,cury;
	getMmtTg(index, &curx, &cury);
	
	pIStuts->AvtTrkStat = eTrk_mode_sectrk;
	pIStuts->AvtPosX[0] = curx;
	pIStuts->AvtPosY[0] = cury;
	app_ctrl_setTrkStat(pIStuts);

	pIStuts->AxisPosX[0] = pIStuts->opticAxisPosX[0];
	pIStuts->AxisPosY[0] = pIStuts->opticAxisPosY[0];
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
	if(pInCmd->axisMoveStepX != 0  || pInCmd->axisMoveStepY !=0)
	{	
		pIStuts->axisMoveStepX = pInCmd->axisMoveStepX;
		pIStuts->axisMoveStepY = pInCmd->axisMoveStepY;
		mask++;
	}

	if(pIStuts->AxisPosX[0] != pInCmd->AxisPosX[0] || pIStuts->AxisPosY[0]!= pInCmd->AxisPosY[0])
	{
		pIStuts->AxisPosX[0] = pInCmd->AxisPosX[0];
		pIStuts->AxisPosY[0] = pInCmd->AxisPosY[0];
		mask++;
	}
	if(mask)
		MSGDRIV_send(MSGID_EXT_INPUT_AXISPOS, 0);	
	
	return ;
}


void app_ctrl_setMtdStat(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;

	if(pIStuts->MtdState[pIStuts->validChId] != pInCmd->MtdState[pIStuts->validChId])
	{
		pIStuts->MtdState[pIStuts->validChId] = pInCmd->MtdState[pIStuts->validChId];
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
		if (pIStuts->MmtStat[0] != pInCmd->MmtStat[0])
		{     
			pIStuts->MmtStat[0] = pInCmd->MmtStat[0];
			{
			    MSGDRIV_send(MSGID_EXT_INPUT_ENMTD, 0);
			}
		}
	}
	return ;
}



//********************************************

void app_ctrl_Sensorchange(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;
	pIStuts->unitAimX=pIStuts->opticAxisPosX[pIStuts->SensorStat];
	pIStuts->unitAimY=pIStuts->opticAxisPosY[pIStuts->SensorStat];
	//pIStuts->unitAimX_new[pIStuts->SensorStat]=pIStuts->opticAxisPosX[pIStuts->SensorStat];
	//pIStuts->unitAimY_new[pIStuts->SensorStat]=pIStuts->opticAxisPosY[pIStuts->SensorStat];
}

void app_ctrl_setReset(CMD_EXT * pInCmd)
{

	if(msgextInCtrl==NULL)
		return ;
       CMD_EXT *pIStuts = msgextInCtrl;
	if(pIStuts->AvtTrkStat != eTrk_mode_acq)
		{
			pIStuts->AvtTrkStat = eTrk_mode_acq;
			 MSGDRIV_send(MSGID_EXT_INPUT_TRACK, 0);
		}
	if(pIStuts->MmtStat[pIStuts->SensorStat] != eImgAlg_Disable)
		{
			pIStuts->MmtStat[pIStuts->SensorStat] = eImgAlg_Disable;
			  MSGDRIV_send(MSGID_EXT_INPUT_ENMTD, 0);
		}
	if(pIStuts->TvCollimation==1)
		{
			pIStuts->TvCollimation=2;
			MSGDRIV_send(MSGID_EXT_INPUT_ENBDT, 0);
		}

	if(pIStuts->ImgEnhStat[pIStuts->SensorStat] == 0x01)
		{
			pIStuts->ImgEnhStat[pIStuts->SensorStat] = 0x00;
			MSGDRIV_send(MSGID_EXT_INPUT_ENENHAN, 0);
		}
	if(pIStuts->FrCollimation==0x01)
		{
			pIStuts->FrCollimation=0x02;
			MSGDRIV_send(MSGID_EXT_INPUT_ENFREZZ, 0);
			
		}


}

void app_ctrl_freezeReset(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
	    return ;
       CMD_EXT *pIStuts = msgextInCtrl;
	pIStuts->FreezeresetCount = 25;//default force trk 1second

	if((pristatus&0x03)!=(pIStuts->unitFaultStat&0x03))
		{
			pristatus=pIStuts->unitFaultStat;
			//MSGAPI_AckSnd( AckPeroidChek);
		}
	//printf("*************************\n");
}


void app_ctrl_setSensor(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;

	CMD_EXT *pIStuts = msgextInCtrl;
	if (pIStuts->SensorStat != pInCmd->SensorStat)
	{
		pIStuts->changeSensorFlag = 1;
		pIStuts->SensorStat = pInCmd->SensorStat;
		app_ctrl_Sensorchange(pInCmd);
		MSGDRIV_send(MSGID_EXT_INPUT_SENSOR, 0);
	}
	return ;
}



void app_ctrl_setForceCoast(CMD_EXT * pInCmd)
{
    if(msgextInCtrl==NULL)
		return ;
     CMD_EXT *pIStuts = msgextInCtrl;


     
    /** < set avt21 config */
    if ((pIStuts->AvtTrkStat != eTrk_mode_target)
                    && (pIStuts->AvtTrkStat != eTrk_mode_mtd))
        return ;

     pIStuts->AvtTrkCoast = 1;
     pIStuts->TrkCoastCount = (1000 /40) *2;//default force trk 2 second
     
     MSGDRIV_send(MSGID_EXT_INPUT_COAST, 0);
   
   return ;
}


void app_ctrl_setTrkBomen(CMD_EXT * pInCmd)
{
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



void app_ctrl_setFRColl(CMD_EXT * pInCmd)
{
}

/*******************************************************************************
  Function:    void app_ctrl_setTvColl()
  Description: this function is used to
  Calls:       null
  Called By:
  Input:
  Output:      null
  Return:      null
  Others:
*******************************************************************************/
void app_ctrl_setTvColl(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;

	if(pInCmd->TvCollimation != pIStuts->TvCollimation)
	{
		pIStuts->TvCollimation = pInCmd->TvCollimation ;
		MSGDRIV_send(MSGID_EXT_INPUT_ENBDT, 0);
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

void app_ctrl_setFovCtrl(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;
	float fDiffMask = 0.0;

	if(pIStuts->FovCtrl != pInCmd->FovCtrl)
	{
		pIStuts->FovCtrl = pInCmd->FovCtrl;
		MSGDRIV_send(MSGID_EXT_INPUT_FOVCMD, 0);
	}
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



void app_ctrl_poweron(CMD_EXT * pInCmd )
{
	 if(msgextInCtrl==NULL)
		return ;
     	CMD_EXT *pIStuts = msgextInCtrl;

	return ;	
}



void app_ctrl_setFovselect(CMD_EXT * pInCmd)
{
	 if(msgextInCtrl==NULL)
		return ;
     	CMD_EXT *pIStuts = msgextInCtrl;
        if(pInCmd->FovStat != pIStuts->FovStat)
            pIStuts->FovStat = pInCmd->FovStat;
	MSGDRIV_send(MSGID_EXT_INPUT_FOVSELECT, 0);

        return ;
}



void app_ctrl_setSaveCfg(CMD_EXT * pInCmd)
{
     if(msgextInCtrl==NULL)
		return ;
     	CMD_EXT *pIStuts = msgextInCtrl;
	
    //if (pIStuts->AvtCfgSave != pInCmd->AvtCfgSave)
    {
        pIStuts->AvtCfgSave = pInCmd->AvtCfgSave;
	//OSA_printf("AvtCfgSave = %d,FovStat = %d\n",pIStuts->AvtCfgSave,pIStuts->FovStat);
        MSGDRIV_send(MSGID_EXT_INPUT_CFGSAVE, 0);
    }

    /***for **reply*****/
    //MSGAPI_AckSnd( ACK);

  return ;
}


void app_ctrl_SaveCollXY()
{ 
//no used
       if(msgextInCtrl==NULL)
		return ;
     	CMD_EXT *pIStuts = msgextInCtrl;

    if(pIStuts->TvCollimation == 0x01)
    {
        pIStuts->AvtCfgSave = eSave_Enable;
        //printf("save cfg\n");
        MSGDRIV_send(MSGID_EXT_INPUT_AXISPOS, 0);
        MSGDRIV_send(MSGID_EXT_INPUT_CFGSAVE, 0);
        
    }
    else if(pIStuts->FrCollimation == 0x01)
    {
#if (!FrColl_Change)
#endif      
        pIStuts->AvtCfgSave = eSave_Enable;

       // MSGDRIV_send(MSGID_EXT_INPUT_AXISPOS, 0);
        MSGDRIV_send(MSGID_EXT_INPUT_CFGSAVE, 0);
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
        CFGID_CONFIG_SET(CFGID_OC25_TV_OSD_LEVEL,   pIStuts->DispGrp[0] );
        CFGID_CONFIG_SET(CFGID_OC25_FR_OSD_LEVEL,   pIStuts->DispGrp[1]);
    
      //  MSGDRIV_send(MSGID_EXT_INPUT_DISPGRADE, 0);
    }
	
       /***for **reply*****/
   	//MSGAPI_AckSnd( ACK);
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
	    CFGID_CONFIG_SET(CFGID_RTS_TV_SEN_COLOR,   pIStuts->DispColor[0]);

        CFGID_CONFIG_SET(CFGID_RTS_FR_SEN_COLOR,   pIStuts->DispColor[1]);
	  //  if(pIStuts->DispColor[0] < 7)
	    //MSGDRIV_send(MSGID_EXT_INPUT_DISPCOLOR, 0);
	}
}
     /***for **reply*****/
     //MSGAPI_AckSnd( ACK);
   return ;
}


void app_ctrl_setTvdispaly(CMD_EXT * pInCmd)
{
	 if(msgextInCtrl==NULL)
		return ;
     	CMD_EXT *pIStuts = msgextInCtrl;

	//fov change need  
	/*
		TV:2400~960=Big,960~330=Mid,330~260=Sml,110=superSml,55=Zoom
		FR:4000~1000=Big,1000~330=Mid,330~120=Sml,120~90=SuperSml,60=Zoom
	*/
	if(pIStuts->SensorStat==0)
	{

		//OSA_printf("TVfov*************the unitFovAngle=%f  \n",pIStuts->unitFovAngle[pInCmd->SensorStat]);
		if(pIStuts->unitFovAngle[pInCmd->SensorStat]>960&&pIStuts->unitFovAngle[pInCmd->SensorStat]<=2400)
		{
			//940 big fov position <- y positon
			int tvunit=(2400-960)/30;
			int tvpos=940-(2400-pIStuts->unitFovAngle[pInCmd->SensorStat])/tvunit;
			if(tvpos>940)
				tvpos=940;
			
			 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_LEFT_X(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1), 150); 
				 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_TOP_Y(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1 ), tvpos);

		}
		else if(pIStuts->unitFovAngle[pInCmd->SensorStat]>330&&pIStuts->unitFovAngle[pInCmd->SensorStat]<=960)
		{
			//910 big fov position<- yposition
			int tvunit=(960-330)/30;
			int tvpos=910-(960-pIStuts->unitFovAngle[pInCmd->SensorStat])/tvunit;
			if(tvpos>910)
				tvpos=910;
			 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_LEFT_X(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1), 150); 
			 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_TOP_Y(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1 ), tvpos);


		}
		else if(pIStuts->unitFovAngle[pInCmd->SensorStat]>260&&pIStuts->unitFovAngle[pInCmd->SensorStat]<=330)
		{
			 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_LEFT_X(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1), 150); 
			 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_TOP_Y(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1 ), 880);
		}
		else if(pIStuts->unitFovAngle[pInCmd->SensorStat]==110){
				 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_LEFT_X(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1), 150); 
				 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_TOP_Y(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1 ), 850);
		}
		else if(pIStuts->unitFovAngle[pInCmd->SensorStat]==55){
				 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_LEFT_X(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1), 150); 
				 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_TOP_Y(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1 ), 820);
		}
		else if(pIStuts->unitFovAngle[pInCmd->SensorStat]>2400)
		{

			 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_LEFT_X(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1), 150); 
			 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_TOP_Y(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1 ), 940);
		}		
	}
	else
	{
		//OSA_printf("FRfov*************the unitFovAngle=%f  \n",pIStuts->unitFovAngle[pInCmd->SensorStat]);
		if(pIStuts->unitFovAngle[pInCmd->SensorStat]>1000&&pIStuts->unitFovAngle[pInCmd->SensorStat]<=4000)
		{
			//940 big fov position <- y positon
			int frunit=(4000-1000)/30;
			int frpos=940-(4000-pIStuts->unitFovAngle[pInCmd->SensorStat])/frunit;
			if(frpos>940)
				frpos=940;
			
			 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_LEFT_X(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1), 150); 
				 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_TOP_Y(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1 ), frpos);

		}
		else if(pIStuts->unitFovAngle[pInCmd->SensorStat]>330&&pIStuts->unitFovAngle[pInCmd->SensorStat]<=1000)
		{
			//910 big fov position<- yposition
			int frunit=(1000-330)/30;
			int frpos=910-(1000-pIStuts->unitFovAngle[pInCmd->SensorStat])/frunit;
			if(frpos>910)
				frpos=910;
			 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_LEFT_X(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1), 150); 
			 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_TOP_Y(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1 ), frpos);


		}
		else if(pIStuts->unitFovAngle[pInCmd->SensorStat]>120&&pIStuts->unitFovAngle[pInCmd->SensorStat]<=330)
		{
			//880 big fov position<- yposition
			int frunit=(330-120)/30;
			int frpos=880-(330-pIStuts->unitFovAngle[pInCmd->SensorStat])/frunit;
			if(frpos>880)
				frpos=880;
			 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_LEFT_X(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1), 150); 
			 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_TOP_Y(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1 ), frpos);


		}
		else if(pIStuts->unitFovAngle[pInCmd->SensorStat]>90&&pIStuts->unitFovAngle[pInCmd->SensorStat]<=120)
		{
			 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_LEFT_X(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1), 150); 
			 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_TOP_Y(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1 ), 850);


		}
		else if(pIStuts->unitFovAngle[pInCmd->SensorStat]==60){
			CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_LEFT_X(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1), 150); 
			CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_TOP_Y(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1 ), 820);

		}
		else if(pIStuts->unitFovAngle[pInCmd->SensorStat]>4000)
		{

			 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_LEFT_X(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1), 150); 
			 CFGID_CONFIG_SET(CFGID_OSD_TEXT_2_TOP_Y(BLOCK_BASE_OSD_TEXT_FOVCHOOSE_1 ), 940);
		}
	}

}


void app_ctrl_setTvFovCtrl(CMD_EXT * pInCmd)
{
    if(msgextInCtrl==NULL)
		return ;
     	CMD_EXT *pIStuts = msgextInCtrl;
    float fDiffMask = 0.0;

    fDiffMask = pIStuts->unitFovAngle[pInCmd->SensorStat] - pInCmd->unitFovAngle[pInCmd->SensorStat];
   //if (fDiffMask > 0.0 || fDiffMask < 0.0)
    {
	
       pIStuts->unitFovAngle[pInCmd->SensorStat] = pInCmd->unitFovAngle[pInCmd->SensorStat];
       app_ctrl_setTvdispaly(pInCmd);
       MSGDRIV_send(MSGID_EXT_INPUT_FOVSTAT, 0);
	//printf("the unitFovAngle=%f\n",pIStuts->unitFovAngle[pInCmd->SensorStat]);
     }
//printf("the unitFovAngle=%f  fDiffMask=%f\n",pIStuts->unitFovAngle[pInCmd->SensorStat],fDiffMask);
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


void app_ctrl_setverti(CMD_EXT * pInCmd)
{
   return ;
}


void app_ctrl_detectvideo()
{
	
	 MSGDRIV_send(MSGID_EXT_INPUT_VIDEOEN, 0);
	return ;

}

void app_ctrl_ack()
{
	//MSGAPI_AckSnd( AckFrColl);
}

void app_err_feedbak()
{
	//MSGAPI_AckSnd( AckTrkErr);
}

void app_ctrl_setAcqRect(CMD_EXT * pInCmd)
{
	if(msgextInCtrl==NULL)
		return ;
	CMD_EXT *pIStuts = msgextInCtrl;
	if (pIStuts->AcqRectW[pIStuts->validChId] != pInCmd->AcqRectW[pInCmd->validChId])
	{
		pIStuts->AcqRectW[pIStuts->validChId] = pInCmd->AcqRectW[pInCmd->validChId];
	}
	if (pIStuts->AcqRectH[pIStuts->validChId] != pInCmd->AcqRectH[pInCmd->validChId])
	{
		pIStuts->AcqRectH[pIStuts->validChId] = pInCmd->AcqRectH[pInCmd->validChId];
	}
}
