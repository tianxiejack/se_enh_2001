
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include "MultiChVideo.hpp"
#include "v4l2camera.hpp"
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

MultiChVideo::MultiChVideo():m_usrFunc(NULL),m_user(NULL)
{
	memset(m_thrCap, 0, sizeof(m_thrCap));
	memset(VCap, 0, MAX_CHAN*sizeof(void*));
}

MultiChVideo::~MultiChVideo()
{
	destroy();
}
static void pabort(const char *s)
{
	perror(s);
	abort();
}
static volatile int front = 1;
static int fd = 0;
static int frameflag = 0;

int MultiChVideo::creat()
{
	for(int i=0; i<MAX_CHAN; i++){	
		VCap[i] = new v4l2_camera(i);
		VCap[i]->creat();
	}
	return 0;
}

int MultiChVideo::destroy()
{
	for(int i=0; i<MAX_CHAN; i++){
		if(VCap[i] != NULL)
		{
			stop();
			VCap[i]->stop();
			delete VCap[i];
			VCap[i] = NULL;
		}
	}
	return 0;
}

int MultiChVideo::run()
{
	int iRet = 0;

	for(int i=0; i<MAX_CHAN; i++){	
		VCap[i]->run();
		m_thrCxt[i].pUser = this;
		m_thrCxt[i].chId = i;
		iRet = OSA_thrCreate(&m_thrCap[i], capThreadFunc, 0, 0, &m_thrCxt[i]);
		if(i == video_pal)
		{
			//for(int j=0; j<4; j++){
			int j = 0;
				m_palthrCxt[j].pUser = this;
				m_palthrCxt[j].chId = j;
				iRet = OSA_thrCreate(&m_palthrCap[j], cappal4ThreadFunc, 0, 0, &m_palthrCxt[j]);
			//}
		}
	}

	return iRet;
}

int MultiChVideo::stop()
{
	for(int i=0; i<MAX_CHAN; i++){
		VCap[i]->stop();
		OSA_thrDelete(&m_thrCap[i]);
	}

	return 0;
}

int MultiChVideo::run(int chId)
{
	int iRet;
	if(chId<0 || chId>=MAX_CHAN)
		return -1;

	VCap[chId]->run();
	m_thrCxt[chId].pUser = this;
	m_thrCxt[chId].chId = chId;
	iRet = OSA_thrCreate(&m_thrCap[chId], capThreadFunc, 0, 0, &m_thrCxt[chId]);

	return iRet;
}

int MultiChVideo::stop(int chId)
{
	if(chId<0 || chId>=MAX_CHAN)
		return -1;

	OSA_thrDelete(&m_thrCap[chId]);
	VCap[chId]->stop();

	return 0;
}

void MultiChVideo::process()
{
	int chId;
	fd_set fds;
	struct timeval tv;
	int ret;

	Mat frame;

	FD_ZERO(&fds);

	for(chId=0; chId<MAX_CHAN; chId++){
		if(VCap[chId]->m_devFd != -1 &&VCap[chId]->bRun )
			FD_SET(VCap[chId]->m_devFd, &fds);
		//OSA_printf("MultiChVideo::process: FD_SET ch%d -- fd %d", chId, VCap[chId]->m_devFd);
	}

	tv.tv_sec = 2;
	tv.tv_usec = 0;

	ret = select(FD_SETSIZE, &fds, NULL, NULL, &tv);
	if(-1 == ret)
	{
		if (EINTR == errno)
			return;
	}else if(0 == ret)
	{
		return;
	}

	for(chId=0; chId<MAX_CHAN; chId++){
		if(VCap[chId]->m_devFd != -1 && FD_ISSET(VCap[chId]->m_devFd, &fds)){
			struct v4l2_buffer buf;
			memset(&buf, 0, sizeof(buf));
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory =V4L2_MEMORY_USERPTR;//V4L2_MEMORY_MMAP;//

			if (-1 == v4l2_camera::xioctl(VCap[chId]->m_devFd, VIDIOC_DQBUF, &buf))
			{
				fprintf(stderr, "cap ch%d DQBUF Error!\n", chId);
			}
			else
			{
				
				if(m_usrFunc != NULL){
					frame = cv::Mat(VCap[chId]->imgheight, VCap[chId]->imgwidth, VCap[chId]->imgtype,
							VCap[chId]->buffers[buf.index].start);

					m_usrFunc(m_user, chId, 0, frame);
				}
				
				if (-1 == v4l2_camera::xioctl(VCap[chId]->m_devFd, VIDIOC_QBUF, &buf)){
					fprintf(stderr, "VIDIOC_QBUF error %d, %s\n", errno, strerror(errno));
					exit(EXIT_FAILURE);
				}
			}
		}
	}

}

void MultiChVideo::process(int chId)
{
	fd_set fds;
	struct timeval tv;
	int ret;
	Mat frame;
	enum v4l2_buf_type type;

	FD_ZERO(&fds);

	FD_SET(VCap[chId]->m_devFd, &fds);

	tv.tv_sec = 1;
	tv.tv_usec = 0;//250000

	ret = select(VCap[chId]->m_devFd+1, &fds, NULL, NULL, &tv);

	if(-1 == ret)
	{
		if (EINTR == errno)
			return;
	}else if(0 == ret)
	{
		return;
	}

	if(VCap[chId]->m_devFd != -1 && FD_ISSET(VCap[chId]->m_devFd, &fds))
	{
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;//V4L2_MEMORY_MMAP

		if (-1 == v4l2_camera::xioctl(VCap[chId]->m_devFd, VIDIOC_DQBUF, &buf))
		{
			fprintf(stderr, "cap ch%d DQBUF Error!\n", chId);
		}
		else
		{
			if(m_usrFunc != NULL){
				if((chId == video_gaoqing0)||(chId == video_gaoqing)||(chId == video_gaoqing2)||(chId == video_gaoqing3))
				{
					frame = cv::Mat(VCap[chId]->imgheight, VCap[chId]->imgwidth, VCap[chId]->imgtype,
							VCap[chId]->buffers[buf.index].start);
					m_usrFunc(m_user, chId, 0, frame);
				}
				else if(chId == video_pal)
				{
					 	VCap[chId]->parse_line_header2(4,(unsigned char *)VCap[chId]->buffers[buf.index].start);
				}
			}
			if (-1 == v4l2_camera::xioctl(VCap[chId]->m_devFd, VIDIOC_QBUF, &buf)){
				fprintf(stderr, "VIDIOC_QBUF error %d, %s\n", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		
	}
}

void MultiChVideo::pal4process(int chid)
{

	int status;
	unsigned char *bufdata  = NULL;
	int bufId,CHID;
	int width, height;
	Mat frame;
	int nLost = 0;
	int nProcess = 0;
	int chId=video_pal;
	Uint32 fullCnt;

	for(;;)
	{
		nProcess = 0;
		{	
			status = OSA_bufGetFull(VCap[chId]->m_bufferHndl, &bufId, OSA_TIMEOUT_FOREVER);

			if(status == 0){
				bufdata	= BUFFER_DATA(VCap[chId]->m_bufferHndl->bufInfo[bufId].virtAddr);
				CHID = *BUFFER_CHID(VCap[chId]->m_bufferHndl->bufInfo[bufId].virtAddr);
				width	= VCap[chId]->m_bufferHndl->bufInfo[bufId].width;
				height	= VCap[chId]->m_bufferHndl->bufInfo[bufId].height;
				frame = cv::Mat(576, 720, VCap[chId]->imgtype,
						bufdata);
				CHID %= (4*video_pal);
				m_usrFunc(m_user, video_pal, CHID, frame);
				OSA_bufPutEmpty(VCap[chId]->m_bufferHndl, bufId);
				nProcess ++;
			}
		}
	}
}


