/********************************************************************************************************
   Copyright (C), 2011-2012, ChamRun Tech. Co., Ltd.
   File name:	dxTimer.c
   Author:	    xavier       Version:  1.0.0      Date: 2013-4-17
   Description:  timer module in this file.Here we use select to complete timer heart.
    our time tick is 1 ms, which is setted when create.
    as we quite sure that how many clocks will used in this software,
    and no timer will added dynamically, no resource mux_lock used.
   Version:      1.0.0
   Function List:
    1.  int ZETIMER_create(int count)
      This function creates zeTimer module Object. Here we set the timer tick 1ms.

    2.	TIMER_destroy()
     This function destroy zeTimer timer Object.

    3.	timer_id  TIMER_add(int interval, int tim_stat, timer_expiry *cb, void *arg);

     This function add a new timer to the timer list.

    4.  int TIMER_reset(timer_id tid,int interval).

    5.  int TIMER_stop(timer_id tid)

    6.	static void sig_func(int signo)
     This is the timer callback function.

   History:
      <author>  <time>   <version >   <desc>
      xavier    13/04/17     1.0     build this moudle
********************************************************************************************************/

typedef struct _dx_timer {
	int		interval;
	int		curTamp;

	void ( *fnxCall )( void );
	volatile int	flag;

}DX_TIMER;

static DX_TIMER		cTimer;

static void *sig_func( void * p );


/********************************************************************************************************
   Function:		TIMER_create(...)
   Description:  This function creates zeTimer moudle Object. Actually it creates a timer list.
      Here we define tick granularity is 1 ms. So we set TIMER_START = TIMER_TICK = 1ms,
      and the timer will keep on running until TIMER_destroy is called.
   Calls:        \setitimer() as a time tick
      \LIST_INIT() init timer list
   Called By:   \user init interface
   Input:        int count , timer number, should be  < MAX_TIMER_NUM
   Output:          null
   Return:          @return 0 if sucess or -1
   Others:
 **********************************************************************************************************/
int dxTimer_create( int interval, void ( *fnxCall )( void ) )
{
}

/********************************************************************************************************
   Function:	 TIMER_destroy(...)
   Description:  This function free zeTimer moudle Object. Actually it frees a timer list.
   Calls:        \pthread_cancel()
                 \pthread_join()
   Called By:    \user init interface
   Input:        int count , timer number, should be  < MAX_TIMER_NUM
   Output:       null
   Return:       0 means ok, the other means fail.
   Others:
 **********************************************************************************************************/
void dxTimer_destroy( )
{

}

/**************************************************************************************
   Function:	 void sig_func(int signo)
   Description:  This function is a Tick Bookkeeping.call the callback function 
                 if time is out for each timer.
   Calls:
   Called By:   \System
   Input:       @null
   Output:      @null
   Return:      @null
   Others:
 **************************************************************************************/
static void *sig_func( void * p )
{
	//return NULL;
}
/************************************** The End Of File **************************************/
