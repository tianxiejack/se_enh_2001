
#ifndef APP_CTRL_H_
#define APP_CTRL_H_

#include "app_status.h"
extern CMD_EXT *msgextInCtrl;

void app_ctrl_setReset(CMD_EXT * pInCmd);
void app_ctrl_setSensor(CMD_EXT * pInCmd);
void app_ctrl_setTrkStat(CMD_EXT * pInCmd);
void app_ctrl_setMMT(CMD_EXT * pInCmd);
void app_ctrl_setAimPos(CMD_EXT * pInCmd);
void app_ctrl_setZoom(CMD_EXT * pInCmd);
void app_ctrl_setAimSize(CMD_EXT * pInCmd);
void app_ctrl_setSerTrk(CMD_EXT * pInCmd );
void app_ctrl_setAxisPos(CMD_EXT * pInCmd);
void app_ctrl_setEnhance(CMD_EXT * pInCmd);
void app_ctrl_setPicp(CMD_EXT * pInCmd);

void  app_ctrl_getSysData(CMD_EXT * exthandle);
void app_ctrl_setMmtSelect(CMD_EXT * pIStuts,unsigned char index);
void app_ctrl_setMtdStat(CMD_EXT * pInCmd);
void app_ctrl_setMtdSelect(CMD_EXT * pInCmd);
void app_ctrl_setAcqRect(CMD_EXT * pInCmd);
void app_ctrl_setBoresightPos(CMD_EXT * pInCmd);
void app_ctrl_setSceneTrk(CMD_EXT *sceneFlag);

void app_ctrl_trkMtdId(int mtdId);

void app_ctrl_setMmtCorrd(CMD_EXT * pIStuts,unsigned int x,unsigned int y);
void app_ctrl_setMtdCorrd(CMD_EXT * pIStuts,unsigned int x,unsigned int y);
int MtdCoord2mtdTarget(int chid ,unsigned int x,unsigned int y);
void app_ctrl_setMtdSelfCorrd(CMD_EXT * pIStuts,unsigned int x,unsigned int y);
void app_ctrl_trkMtd(int index);

#endif /* APP_CTRL_H_ */
