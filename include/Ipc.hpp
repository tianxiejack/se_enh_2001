
#ifndef IPC_HPP_
#define IPC_HPP_

//#include "ipc_custom_head.hpp"

void Ipc_pthread_start(void);
void Ipc_pthread_stop(void);

void cfg_ctrl_acqReset(void * pIn);	// reset all channel acqrect by configtable

int cfg_set_trkMode(unsigned int bTrack, unsigned int bScene);
int cfg_set_trkFeedback(unsigned int trackstatus, float trackposx, float trackposy);
int cfg_set_mtdFeedback(unsigned int bMtd, unsigned int bMtdDetect);

void cfg_dbg_setDefault(float * configTab);
void cfg_dbg_getDefault(float * configTab, unsigned char *configUser);
void cfg_dbg_setCmd(int cmd, int prm);

#endif /* IPC_HPP_ */

