/*
 * hahaha.hpp
 *
 *  Created on: 2019年3月6日
 *      Author: alex
 */

#ifndef HAHAHA_HPP_
#define HAHAHA_HPP_

#include "ipc_custom_head.h"
extern void cfg_dbg_setCmd(int cmd, int prm);

void testhahahaha()
{

}

static void keyboard_event(unsigned char key, int x, int y)
{
	static int sensid = 0, bPos = 0, scenestat = 0, trkstat = 0, mtdstat = 0;
	static int acqstat = 0, secstat = 0, mtdsw = 0, movestat = 0, movecnt = 0;
	static int workmd = 0, irisdir = 0, focusdir = 0;
	char strMenus[1024] = {
			"\n"
			"----------------------------------------\n"
			"|---Main Menu -------------------------|\n"
			"----------------------------------------\n"
			" [0-9 a-c] test ipc cmd   \n"
			"--> "
	};

	switch(key)
	{
	case '0':
		sensid = (sensid+1)%5;
		cfg_dbg_setCmd(sensor, sensid);
		break;
	case '1':
		bPos = (bPos+1)%2;
		cfg_dbg_setCmd(BoresightPos, bPos);		
		break;
	case '2':
		acqstat = (acqstat+1)%3;
		if(acqstat == 1)
			cfg_dbg_setCmd(AcqPos, 2);
		else if(acqstat == 2)
			cfg_dbg_setCmd(AcqPos, 0);
		else
		{
			cfg_dbg_setCmd(AcqPos, 1);
			trkstat = 1;
		}
		break;
	case '3':
		scenestat = (scenestat+1)%2;
		cfg_dbg_setCmd(sceneTrk, scenestat);
		break;
	case '4':
		trkstat = (trkstat+1)%2;
		cfg_dbg_setCmd(trk, trkstat);
		break;
	case '5':
		mtdstat = (mtdstat+1)%2;
		cfg_dbg_setCmd(mtd, mtdstat);
		break;
	case '6':
		mtdsw = (mtdsw+1)%2;
		if(mtdsw)
		{
			//cfg_dbg_setCmd(mtdSelect, 1);
			cfg_dbg_setCmd(mtdSelect, 3);
			trkstat = 1;
		}
		else
		{
			//cfg_dbg_setCmd(mtdSelect, 2);
			cfg_dbg_setCmd(mtdSelect, 3);
			trkstat = 1;
		}
		break;
	case '7':
		secstat = (secstat+1)%3;
		if(secstat == 1)
		{
			if(trkstat == 0)
			{
				trkstat = 1;
				cfg_dbg_setCmd(trk, trkstat);
			}
			cfg_dbg_setCmd(sectrk, 1);
		}
		else if(secstat == 2)
		{
			if(trkstat == 0)
			{
				trkstat = 1;
				cfg_dbg_setCmd(trk, trkstat);
			}
			cfg_dbg_setCmd(sectrk, 3);
		}
		else
		{
			if(trkstat == 0)
			{
				trkstat = 1;
				cfg_dbg_setCmd(trk, trkstat);
			}
			cfg_dbg_setCmd(sectrk, 1);
			cfg_dbg_setCmd(sectrk, 2);
		}
		break;
	case '8':
		movestat = (movestat+1)%2;
		cfg_dbg_setCmd(posmove, movestat);
		break;

	case '9':
		workmd = (workmd+1)%8;
		cfg_dbg_setCmd(workmode, workmd);
		break;
	case 'a':
		irisdir = (irisdir+1)%3;
		cfg_dbg_setCmd(Iris, irisdir);
		break;
	case 'b':
		focusdir = (focusdir)%3;
		cfg_dbg_setCmd(focus, focusdir);
		break;
	case 'c':
		cfg_dbg_setCmd(exit_IrisAndFocus, 0);
		break;
	case 'q':
	case 27:
		//glutLeaveMainLoop();
		break;
	default:
		printf("%s",strMenus);
		break;
	}
}

#endif /* HAHAHA_HPP_ */
