
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.hpp>
#include <glut.h>
#include "process51.hpp"
#include "Ipc.hpp"
#include "msgDriv.h"
#include <sys/time.h>

using namespace std;
using namespace cv;

bool startEnable = false;

int main(int argc, char **argv)
{
	struct timeval tv;

	MSGDRIV_create();
#ifdef __IPC__
	Ipc_pthread_start();
#endif
	CProcess proc;
	while(false == startEnable)
	{
		tv.tv_sec = 0;
		tv.tv_usec = 50000;
		select( 0, NULL, NULL, NULL, &tv );
	};
	proc.creat();
	proc.init();
	proc.run();
	glutMainLoop();
	proc.destroy();
#ifdef __IPC__
	Ipc_pthread_stop();
#endif
    	return 0;
}



//__IPC__
//__MOVE_DETECT__
//__TRACK__

//---------------------------
//__MMT__
//__BLOCK__

