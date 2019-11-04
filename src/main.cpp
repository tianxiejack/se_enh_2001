
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.hpp>
#include <glut.h>
#include <sys/time.h>

#include "process51.hpp"
#include "Ipc.hpp"
#include "msgDriv.h"
#include "hahaha.hpp"

using namespace std;
using namespace cv;

extern int startEnable, ipcDbgOn;

#include <pthread.h>
#include <sched.h>
#include <stdio.h>
static pthread_t start_thread(void *(*__start_routine) (void *), void * __arg)
{
	pthread_t ret ;
	pthread_attr_t attr;
	struct sched_param param;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	param.sched_priority = 50;
	pthread_attr_setschedparam(&attr, &param);
	int id = pthread_create(&ret, &attr, __start_routine,__arg);
	if(id != 0)
	{
		printf("error pthread_create failed\n");
	}
	pthread_attr_destroy(&attr);
//	pthread_mutex_init(&m_Mutex, NULL);
	return ret;
}

CProcess proc;

static void keyboard_event111(unsigned char key, int x, int y)
{
	#if 1
	cv::Size winSize(80, 60);
	static bool mmtdEnable = false;
	static bool trkEnable = false;
	static bool motionDetect = false;
	static bool blobEnable = false;

	char strMenus[2048] ={
			"\n"
			"----------------------------------------\n"
			"|---Main Menu -------------------------|\n"
			"----------------------------------------\n"
			" [c] Enable/Disable Track               \n"
			" [d] Enable/Disable MMTD                \n"
			" [esc][q]Quit                           \n"
			"--> "
	};

	switch(key)
	{
	case 'c':
		proc.OnKeyDwn(key);
		break;
	case 'd':
		proc.OnKeyDwn(key);
		break;
	case 'q':
	case 27:
		;
		break;
	default:
		printf("%s",strMenus);
		break;
	}
	#endif
}



static void *thrdhndl_keyevent(void *context)
{
	for(;;){
		int key = getchar();//getc(stdin);
		if( (key == 'q' || key == 'Q' || key == 27)){
			break;
		}
		//OSA_printf("key = %d\n\r", key);
		keyboard_event111(key, 0, 0);
	}
	if(*(bool*)context)
		;
	OSA_printf("%s %d: leave.", __func__, __LINE__);
	//exit(0);
	return NULL;
}



int main(int argc, char **argv)
{
	struct timeval tv;
	int testMode=0;

	if(argc > 1)
	{
		if(strcmp(argv[1], "test") == 0)
		{
			testMode = 1;
			ipcDbgOn = 1;
			printf("====== self test mode enter ======\n");
		}
	}

#if defined(__linux__)
	setenv ("DISPLAY", ":0", 0);
	printf("\n setenv DISPLAY=%s\n",getenv("DISPLAY"));
#endif

	MSGDRIV_create();
#ifdef __IPC__
	Ipc_pthread_start();
#endif

	if(testMode)
	{
		//proc.SaveTestConfig();
		proc.ReadTestConfig();
	}
	while(false == startEnable)
	{
		tv.tv_sec = 0;
		tv.tv_usec = 50000;
		select( 0, NULL, NULL, NULL, &tv );
	};
	proc.loadIPCParam();
	proc.creat();
	proc.init();
	proc.run();
	if(testMode)
		glutKeyboardFunc(keyboard_event);

	bool a = true;
	start_thread(thrdhndl_keyevent, &a);
	
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

