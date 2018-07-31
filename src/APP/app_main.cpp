#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


#include "osa.h"
#include "dx.h"
#include "dx_config.h"
#include "app_user.h"
#include "dx_main.h"

char gAPP_mainMenu[] =
    {
        "\r\n ========="
        "\r\n [APP MAIN] Main Menu"
        "\r\n ========="
        "\r\n 1: MCFW VCAP +VDIS "
        "\r\n 2: MCFW VCAP +VENC+ VDIS+RTSP  - SD/HD"
        "\r\n 3: MCFW IMAGE PROCESS BOARD VIDEO START"
        "\r\n"
        "\r\n e: Exit"
        "\r\n"
        "\r\n Enter Choice: "
    };
char gAPP_runMenu[] =
    {
        "\r\n ============="
        "\r\n [APP MAIN] Run-Time Menu"
        "\r\n ============="
        "\r\n"
        "\r\n i: Print detailed system information"
        "\r\n s: Core Status: Active/In-active"
        "\r\n c: Configure setup"
        "\r\n b: Print link buffers statistics"
        "\r\n e: Stop App"
        "\r\n"
        "\r\n Enter Choice: "
    };


static Int32 usercase_run( int type )
{
    int status;
    Bool    done = FALSE;
    char    ch;
    //unsigned char brigthness;
   

    status = Dx_postMsg( DX_MSGID_START, &type, sizeof( type ) );
    if ( status < 0 )
    {
        OSA_printf( " WARNING: app usercase start failed !!!\n" );
        return status;
    }

   // app_createObj();

    while ( !done )
    {
       ch = APP_getChar( );
	OSA_waitMsecs(1000000);
    }

    status = Dx_postMsg( DX_MSGID_STOP, NULL, 0 );

    return 0;
}


static void* dxmain(void *pPrm)
{
    Bool ui_loop = TRUE;
    int onece=0;
    char ch = '3';
    OSA_printf("%s:  enter %d   \n", __func__, __LINE__);
    //Dx_run();
    Dx_initparam();//init dx param
    OSA_semSignal((OSA_SemHndl *)pPrm);
    OSA_printf("%s:%d\n", __func__, __LINE__);

    while (ui_loop)
    {
        printf(gAPP_mainMenu);

        switch (ch)
        {
        case '1':
            usercase_run(MCFW_VCAP_VDIS_CASE);
            break;
        case '2':
            usercase_run(MCFW_VCAP_VENC_CASE);
            break;
        case '3':
            usercase_run(MCFW_IMAGE_PROCESS_BOARD_CASE);
            break;
        }

        if (ch == 'e')
        {
            ui_loop = FALSE;
            break;
        }

        ch = APP_getChar();
    }
    Dx_stop();
    //return;
}


  OSA_MutexHndl muxLock;
  OSA_SemHndl tskGraphicSemmain;
  OSA_ThrHndl tskGraphicHndlmain;
int App_dxmain( )
{
	int status=0;
	initgdxd_info();
	
       status = OSA_semCreate(&tskGraphicSemmain, 1, 0);
	
       OSA_assert(status == OSA_SOK);

       status = OSA_thrCreate(
                 &tskGraphicHndlmain,
                 dxmain,
                 OSA_THR_PRI_DEFAULT,
                 0,
             (void *)&tskGraphicSemmain
             );
  OSA_assert(status == OSA_SOK);

  OSA_semWait(&tskGraphicSemmain,OSA_TIMEOUT_FOREVER);

}
#endif
