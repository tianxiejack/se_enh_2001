
#include "v4l2camera.hpp"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>             /* getopt_long() */
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <osa_buf.h>
#include <osa.h>
#include "arm_neon.h"
#include <cuda.h>
#include <cuda_runtime.h>
#include <malloc.h>

#define BUFFER_NUM_MAX			(CAP_CH_NUM*4)

#define FRAME_WIDTH  1448
#define FRAME_HEIGHT 1156
#define IMAGE_WIDTH  1440
#define IMAGE_HEIGHT 576

int vcapWH[5][2] = {{1920, 1080},{1920, 1080},{1920, 1080},{1920, 1080},{720, 576}};
int vdisWH[5][2] = {{1920, 1080},{1920, 1080},{1920, 1080},{1920, 1080},{720, 576}};

v4l2_camera::v4l2_camera(int devId):io(IO_METHOD_USERPTR),buffers(NULL),force_format(1),m_devFd(-1),n_buffers(0),bRun(false)
{
	sprintf(dev_name, "/dev/video%d",devId);
	switch(devId){
		case video_pal:
			imgformat 	= V4L2_PIX_FMT_UYVY;
			imgwidth  	= FRAME_WIDTH/2;
			imgheight 	= FRAME_HEIGHT;
			imgstride 	= imgwidth*2;
			bufSize 		= imgwidth * imgheight * 2;
			imgtype   	= CV_8UC2;
			memType 	= MEMORY_LOCKED;
			bufferCount 	= 8;
			Id 			= devId;
			break;
		case video_gaoqing0:
		case video_gaoqing:
		case video_gaoqing2:
		case video_gaoqing3:
			imgformat 	= V4L2_PIX_FMT_YUYV;
			imgwidth  	= vcapWH[devId][0];
			imgheight 	= vcapWH[devId][1];
			imgstride 	= imgwidth*2;
			bufSize 		= imgwidth * imgheight * 2;
			imgtype     	= CV_8UC2;
			memType 	= MEMORY_LOCKED;
			bufferCount  = 8;
			Id			= devId;
			break;
		default:
			printf("No such device:%s !!\n", dev_name);
			break;
	}
	if(devId == video_pal)
	{
		bzero(split_buffer_ch,sizeof(split_buffer_ch));
		memset(fieldmask, 0, sizeof(fieldmask));
		memset(iLine, 1, sizeof(iLine));

		m_bufferHndl = (OSA_BufHndl *)malloc(sizeof(OSA_BufHndl));
		OSA_assert(m_bufferHndl != NULL);
		memset(&m_bufferCreate, 0, sizeof(OSA_BufCreate));
		m_bufferCreate.width = vcapWH[video_pal][0];
		m_bufferCreate.height = vcapWH[video_pal][1];
		m_bufferCreate.stride = vcapWH[video_pal][0]*2;
		m_bufferCreate.numBuf = 8;

		for(int k=0; k < m_bufferCreate.numBuf; k++)
		{
			m_bufferCreate.bufVirtAddr[k] = OSA_memAlloc(m_bufferCreate.stride*(m_bufferCreate.height+8) + BUFFER_HEAD_LEN);
			OSA_assert(m_bufferCreate.bufVirtAddr[k] != NULL);
			*BUFFER_ID(m_bufferCreate.bufVirtAddr[k]) = k;
			m_bufferHndl->bufInfo[k].width = m_bufferCreate.width;
			m_bufferHndl->bufInfo[k].height = m_bufferCreate.height;
			//m_bufferHndl->bufInfo[k].strid = m_bufferCreate.stride;
		}
		OSA_bufCreate(m_bufferHndl, &m_bufferCreate);

		alloc_split_buffer();
	}
}

//#define DISABLE_NEON_DEI
#undef DISABLE_NEON_DEI

#define _min2(a, b)	 	((a)<(b))?(a):(b)
#define _max2(a, b) 		((a)>(b))?(a):(b)

void DeinterlaceYUV_Neon(unsigned char *lpYUVFrame, int ImgWidth, int ImgHeight, int ImgStride)
{
	int	y;
#ifndef DISABLE_NEON_DEI
	int stride8x8 = ImgWidth*2/8;
#pragma omp parallel for
	for(y = 0; y < (ImgHeight-2); y+=2)
	{
		uint8x8_t * __restrict__ pSrc08x8_t, * __restrict__ pSrc28x8_t, * __restrict__ pOdd8x8_t, * __restrict__ pDst8x8_t;
		int i;

		pSrc08x8_t = (uint8x8_t *)(lpYUVFrame + y*ImgStride*2);
		pSrc28x8_t = (uint8x8_t *)(lpYUVFrame + (y+2)*ImgStride*2);
		pOdd8x8_t = (uint8x8_t *)(lpYUVFrame + (y+1)*ImgStride*2);
		pDst8x8_t = (uint8x8_t *)(lpYUVFrame + (y+1)*ImgStride*2);
		for(i=0; i < stride8x8;  i++)
		{
			uint8x8_t a = pSrc08x8_t[i];
			uint8x8_t d = pOdd8x8_t[i];
			uint8x8_t b = pSrc28x8_t[i];
			pDst8x8_t[i]= vsub_u8(vsub_u8(vadd_u8(vadd_u8(a,b), d), vmin_u8( vmin_u8(a,b),d)), vmax_u8( vmax_u8(a,b),d));
		}
	}
#else
#pragma omp parallel for
	for(y = 0; y < (ImgHeight-2); y+=2)
	{
		register unsigned char *pSrc0,  *pSrc2, *pOdd, *pDst;
		int x;

		pSrc0 = lpYUVFrame + y*ImgStride*2;
		pSrc2 = lpYUVFrame + (y+2)*ImgStride*2;
		pOdd = lpYUVFrame + (y+1)*ImgStride*2;
		pDst = lpYUVFrame + (y+1)*ImgStride*2;
		for(x = 0; x < ImgWidth*2; x++)
		{
			register int a = pSrc0[x];
			register int d = pOdd[x];
			register int b = pSrc2[x];

			pDst[x] = a + b + d- _min2(_min2(a, b), d) - _max2(_max2(a, b), d);//medthr(a, b, d);
		}
	}
#endif
}

v4l2_camera::~v4l2_camera()
{
	destroy();
}

unsigned char* v4l2_camera::getEmptyBuffer(int chId)
{
	unsigned char* buffer;
	int status = OSA_SOK;
	int bufId;
	chId = Id*4+chId;

	status = OSA_bufGetEmpty(m_bufferHndl, &bufId, OSA_TIMEOUT_NONE);

	if(status != OSA_SOK)
	{
		//OSA_printf("%s: WARNING buffer queue full !\n", __func__);
		status = OSA_bufGetFull(m_bufferHndl, &bufId, OSA_TIMEOUT_NONE);
	}

	OSA_assert(status == OSA_SOK);
	//OSA_printf("empty bufId = %d(%d - %d) chId = %d", bufId, m_bufferCreate.numBuf, m_bufferHndl->numBuf, chId);

	buffer = BUFFER_DATA(m_bufferHndl->bufInfo[bufId].virtAddr);
	*BUFFER_ID(m_bufferHndl->bufInfo[bufId].virtAddr) = bufId;
	*BUFFER_CHID(m_bufferHndl->bufInfo[bufId].virtAddr) = chId;

	return buffer;
}

int v4l2_camera::putFullBuffer(unsigned char* data)
{
	int status = OSA_SOK;
	int bufId = OSA_BUF_ID_INVALID;
	int chId;

	OSA_assert(data != NULL);

	bufId = *BUFFER_ID_INV(data);
	chId = *BUFFER_CHID_INV(data);
	//OSA_printf("full bufId = %d(%d - %d) chId = %d", bufId, m_bufferCreate.numBuf, m_bufferHndl->numBuf, chId);

	OSA_assert(bufId>OSA_BUF_ID_INVALID && bufId<m_bufferCreate.numBuf);

	m_bufferHndl->bufInfo[bufId].timestamp = (Uint32)OSA_getCurTimeInMsec();
	m_bufferHndl->bufInfo[bufId].width = m_bufferCreate.width;
	m_bufferHndl->bufInfo[bufId].height = m_bufferCreate.height;
	//m_bufferHndl->bufInfo[bufId].strid = m_bufferCreate.stride;

	status = OSA_bufPutFull(m_bufferHndl, bufId);

	return status;
}


int v4l2_camera::alloc_split_buffer(void)
{
	split_buffer = (unsigned char *)malloc(FRAME_WIDTH*FRAME_HEIGHT);//(1920*1080*2); /* No G2D_CACHABLE */
	if(!split_buffer) {
		printf("Fail to allocate physical memory for image buffer!\n");
		return FAILURE_ALLOCBUFFER;
	}
	memset(split_buffer, 0, FRAME_WIDTH*FRAME_HEIGHT);
	for (int i = 0; i < CAP_CHPAL_NUM; i++) {
		split_buffer_ch[i] = getEmptyBuffer(i+Id);

	}
	return 0;
}


void v4l2_camera::parse_line_header2(int channels, unsigned char *p)
{
	int line, lines;
	int max_line_per_ch = IMAGE_HEIGHT/2 + 1;
	int lostLine0 = 0;
	int lostLine1 = 0;
	bool bAll[4] = {false, false, false, false};
	int chFlag = 0;
	int seq[8] = {8, 8, 8, 8, 8, 8, 8, 8};
	int iseq = 0;
	int i;

	int cntLine[4] = {0, 0, 0, 0};
	lines = channels * max_line_per_ch;//total num;

	for(line=0; line<lines; line++)
	{
		uchar* p_src = p + line*(sizeof(LineHeader) + IMAGE_WIDTH + 24);
		int lineId,chId,fieldId;//frameId,
		LineHeader *phead=(LineHeader*)p_src;

		if((p_src[0] & p_src[1] & p_src[2] & p_src[3] & p_src[4] & p_src[5] & p_src[6] & p_src[7] & 0xF0) != 0xA0){
			lostLine0 ++;
			continue;
		}

		fieldId = phead->fieldNo & 0x01;
		lineId  = ((phead->LineNoH&0x0F) << 8)
				| ((phead->LineNoM&0x0F) << 4)
				| (phead->LineNoL & 0x0F);
		chId    = phead->ChId & 0x03;

		if ((chId>3) || (lineId>max_line_per_ch) || (lineId<=0)){
			lostLine1 ++;
			continue;
		}

		cntLine[chId]++;

		OSA_assert(lineId > 0);
		uchar* pDst =  split_buffer_ch[chId] + ((lineId-1)*2 + fieldId)*IMAGE_WIDTH;
		memcpy(pDst, p_src+sizeof(LineHeader), IMAGE_WIDTH);

		if (lineId >= IMAGE_HEIGHT/2 && !bAll[chId])
		{
			bAll[chId] = true;
			chFlag |= (1<<chId);
			fieldmask[chId] |= (1<<fieldId);
			if(fieldmask[chId] == 0x03){
				DeinterlaceYUV_Neon(split_buffer_ch[chId], IMAGE_WIDTH/2, IMAGE_HEIGHT, IMAGE_WIDTH/2);
				putFullBuffer(split_buffer_ch[chId]);
				split_buffer_ch[chId] = getEmptyBuffer(chId);
				fieldmask[chId] = 0;
			}else if(fieldId == 1){
				//OSA_printf("%s: YES !!!!\n", __func__);
			}
			seq[iseq] = chId;
			iseq ++;
		}
	}
	if(chFlag != 0xF || lostLine0 || lostLine1){
		//OSA_printf("%s: chFlag = %x lost = %d %d; %d %d %d %d\n", __func__, chFlag, lostLine0, lostLine1,
		//		cntLine[0], cntLine[1], cntLine[2], cntLine[3]);
	}
}


void v4l2_camera::errno_exit(const char *s)
{
	fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
	exit(EXIT_FAILURE);
}

int v4l2_camera::xioctl(int fh, int request, void *arg)
{
	int ret;

	do {
		ret = ioctl(fh, request, arg);
	} while (-1 == ret && EINTR == errno);

	return ret;
}

int v4l2_camera::read_frame(unsigned char **data)
{
	struct v4l2_buffer buf;
	int i=0;

	switch (io) {
//	case IO_METHOD_READ:
//		if (-1 == read(fd, buffers[0].start, buffers[0].length)) {
//			switch (errno) {
//			case EAGAIN:
//				return 0;
//			case EIO:
//				/* Could ignore EIO, see spec. */
//				/* fall through */
//			default:
//				errno_exit("read");
//			}
//		}
//		process_image(buffers[0].start, buffers[0].length);
//		break;
	case IO_METHOD_MMAP:
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;

		if (-1 == xioctl(m_devFd, VIDIOC_DQBUF, &buf)) {
			switch (errno) {
			case EAGAIN:
				return -1;
			case EIO:
				/* Could ignore EIO, see spec. */
				/* fall through */
			default:
				errno_exit("VIDIOC_DQBUF");
			}
		}
//		printf("fun:[%s]  line:[%d] buf.index=%d n_buffers=%d\n",__func__,__LINE__, buf.index, n_buffers);
		assert(buf.index < n_buffers);
		//process_image(buffers[buf.index].start, buf.bytesused);

		if (-1 == xioctl(m_devFd, VIDIOC_QBUF, &buf))
			errno_exit("VIDIOC_QBUF");
		break;
	case IO_METHOD_USERPTR:
		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;

		if (-1 == xioctl(m_devFd, VIDIOC_DQBUF, &buf)) {
			switch (errno) {
			case EAGAIN:
				return 0;
			case EIO:
//				/* Could ignore EIO, see spec. */
//				/* fall through */
			default:
				errno_exit("VIDIOC_DQBUF");
			}
		}

		for (i=0; i<n_buffers; ++i)
			if (buf.m.userptr == (unsigned long)buffers[i].start
					&& buf.length == buffers[i].length)
				break;

		assert(i < n_buffers);

//		process_image((void *)buf.m.userptr, buf.bytesused);
//
//		if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
//			errno_exit("VIDIOC_QBUF");
//		break;
	default:
		break;
	}

	return 0;
}

void v4l2_camera::stop_capturing(void)
{
	enum v4l2_buf_type type;
	switch (io) {
//	case IO_METHOD_READ:
//		/* Nothing to do. */
//		break;
	case IO_METHOD_MMAP:
//	case IO_METHOD_USERPTR:
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == xioctl(m_devFd, VIDIOC_STREAMOFF, &type))
			errno_exit("VIDIOC_STREAMOFF");
		break;
	}
}

void v4l2_camera::start_capturing(void)
{
	unsigned int i;
	enum v4l2_buf_type type;

	switch (io) {
//	case IO_METHOD_READ:
//		/* Nothing to do. */
//		break;
	case IO_METHOD_MMAP:
		for (i = 0; i < n_buffers; ++i) {
			struct v4l2_buffer buf;

			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;

			if (-1 == xioctl(m_devFd, VIDIOC_QBUF, &buf))
				errno_exit("VIDIOC_QBUF");
		}
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == xioctl(m_devFd, VIDIOC_STREAMON, &type))
			errno_exit("VIDIOC_STREAMON");
		break;
	case IO_METHOD_USERPTR:
		for (i = 0; i < n_buffers; ++i) {
			struct v4l2_buffer buf;

			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_USERPTR;
			buf.index = i;
			buf.m.userptr = (unsigned long)buffers[i].start;
			buf.length = buffers[i].length;

			if (-1 == xioctl(m_devFd, VIDIOC_QBUF, &buf)){
				OSA_printf("%s: i= %d, p=%p, len=%d, 111111...\n", __func__, i, buffers[i].start, buffers[i].length);
				errno_exit("VIDIOC_QBUF");
			}
		}
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == xioctl(m_devFd, VIDIOC_STREAMON, &type)){
			OSA_printf("%s: 222222...\n", __func__);
			errno_exit("VIDIOC_STREAMON");
		}
		break;
	default:
		break;
	}
}

void v4l2_camera::start_capturing_2(void)
{
	unsigned int i;
	enum v4l2_buf_type type;

	for (i = 0; i < n_buffers; ++i) {
		struct v4l2_buffer buf;

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;
		buf.index = i;
		buf.m.userptr = (unsigned long)buffers[i].start;
		buf.length = buffers[i].length;

		//printf("******************i = %d\n",i);

		xioctl(m_devFd, VIDIOC_QBUF,&buf);

	}


}

void v4l2_camera::uninit_device(void)
{
	unsigned int i;

	switch (io) {
	case IO_METHOD_READ:
		free(buffers[0].start);
		break;
	case IO_METHOD_MMAP:
		for (i = 0; i < n_buffers; ++i)
			if (-1 == munmap(buffers[i].start, buffers[i].length))
				errno_exit("munmap");
		break;
	case IO_METHOD_USERPTR:
		for (i = 0; i < n_buffers; ++i){
			if(memType == MEMORY_NORMAL){
				free(buffers[i].start);
			}else{
				cudaFreeHost(buffers[i].start);
			}
		}
		break;
	default:
		break;
	}

	free(buffers);
}

void v4l2_camera::init_read(unsigned int buffer_size)
{
	buffers = (struct buffer *)calloc(1, sizeof(*buffers));

	if (!buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	buffers[0].length = buffer_size;
	buffers[0].start = (struct buffer *) malloc(buffer_size);

	if (!buffers[0].start) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}
}

void v4l2_camera::init_mmap(void)
{
	struct v4l2_requestbuffers req;

	CLEAR(req);

	req.count = 6;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(m_devFd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s does not support memory mapping\n", dev_name);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2) {
		fprintf(stderr, "Insufficient buffer memory on %s\n", dev_name);
		exit(EXIT_FAILURE);
	}

	OSA_printf("%s qbuf cnt = %d\n", dev_name, req.count);

	buffers = (struct buffer *)calloc(req.count, sizeof(*buffers));

	if (!buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		struct v4l2_buffer buf;

		CLEAR(buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;

		if (-1 == xioctl(m_devFd, VIDIOC_QUERYBUF, &buf))
			errno_exit("VIDIOC_QUERYBUF");

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start  =	mmap(NULL /* start anywhere */,
										buf.length,
										PROT_READ | PROT_WRITE /* required */,
										MAP_SHARED /* recommended */,
										m_devFd, buf.m.offset);
		memset(buffers[n_buffers].start,0x80,buf.length);

		if (MAP_FAILED == buffers[n_buffers].start)
			errno_exit("mmap");
	}
}

void v4l2_camera::init_userp(unsigned int buffer_size)
{
	struct v4l2_requestbuffers req;
	cudaError_t ret = cudaSuccess;
	unsigned int page_size;
	page_size = getpagesize();
	buffer_size = (buffer_size + page_size-1)&~(page_size-1);
	
	CLEAR(req);

	req.count  = bufferCount;//6;//MAX_CHAN;
	req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;

	if (-1 == xioctl(m_devFd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s does not support "
					"user pointer i/o\n", dev_name);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}
	if (req.count < 2) {
			fprintf(stderr, "Insufficient buffer memory on %s\n", dev_name);
			exit(EXIT_FAILURE);
		}
	OSA_printf("%s qbuf cnt = %d\n", dev_name, req.count);
	buffers = (struct buffer *)calloc(req.count, sizeof(*buffers));

	if (!buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		buffers[n_buffers].length = buffer_size;
		if(memType == MEMORY_NORMAL)
			//buffers[n_buffers].start = malloc(buffer_size);
			buffers[n_buffers].start = memalign(page_size,buffer_size);
		else // MEMORY_LOCKED
			ret = cudaHostAlloc(&buffers[n_buffers].start, buffer_size, cudaHostAllocDefault);
		assert(ret == cudaSuccess);
		//cudaFreeHost();

		if (!buffers[n_buffers].start) {
			fprintf(stderr, "Out of memory\n");
			exit(EXIT_FAILURE);
		}
	}
}

int v4l2_camera::init_device(void)
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

	if (-1 == xioctl(m_devFd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s is no V4L2 device\n", dev_name);
			return FAILURE_DEVICEINIT;
		} else {
			errno_exit("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "%s is no video capture device\n", dev_name);
		return FAILURE_DEVICEINIT;
	}

	switch (io) {
	case IO_METHOD_READ:
		if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
			fprintf(stderr, "%s does not support read i/o\n", dev_name);
			return FAILURE_DEVICEINIT;
		}
		break;
	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
			fprintf(stderr, "%s does not support streaming i/o\n", dev_name);
			return FAILURE_DEVICEINIT;
		}
		break;
	}

	/* Select video input, video standard and tune here. */

	CLEAR(cropcap);

	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == xioctl(m_devFd, VIDIOC_CROPCAP, &cropcap)) {
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; /* reset to default */

		if (-1 == xioctl(m_devFd, VIDIOC_S_CROP, &crop)) {
		}
	} else {
		/* Errors ignored. */
	}

	CLEAR(fmt);

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (force_format) {
		fprintf(stderr, "Set uyvy\r\n");
		fmt.fmt.pix.width       = imgwidth; //replace
		fmt.fmt.pix.height      = imgheight; //replace
		fmt.fmt.pix.pixelformat = imgformat;// V4L2_PIX_FMT_UYVY;
		fmt.fmt.pix.field       = V4L2_FIELD_ANY;
		//fmt.fmt.pix.code=0;
		OSA_printf("******width =%d height=%d\n",fmt.fmt.pix.width,fmt.fmt.pix.height);

		if (-1 == xioctl(m_devFd, VIDIOC_S_FMT, &fmt))
		{
			 errno_exit("VIDIOC_S_FMT");
		}

		/* Note VIDIOC_S_FMT may change width and height. */
	} else {
		/* Preserve original settings as set by v4l2-ctl for example */
		if (-1 == xioctl(m_devFd, VIDIOC_G_FMT, &fmt))
			errno_exit("VIDIOC_G_FMT");
	}

	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	switch (io) {
	case IO_METHOD_READ:
		init_read(fmt.fmt.pix.sizeimage);
		break;
	case IO_METHOD_MMAP:
		init_mmap();
		break;
	case IO_METHOD_USERPTR:
		init_userp(fmt.fmt.pix.sizeimage);
		break;
	}
	return EXIT_SUCCESS;
}

void v4l2_camera::close_device(void)
{
	if (-1 == close(m_devFd))
		errno_exit("close");

	m_devFd = -1;
}

int v4l2_camera::open_device(void)
{
	struct stat st;

	if (-1 == stat(dev_name, &st)) {
		fprintf(stderr, "Cannot identify '%s': %d, %s\n", dev_name, errno, strerror(errno));
		return FAILURE_DEVICEOPEN;
	}

	if (!S_ISCHR(st.st_mode)) {
		fprintf(stderr, "%s is no device\n", dev_name);
		return FAILURE_DEVICEOPEN;
	}

	m_devFd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
	if (-1 == m_devFd) {
		fprintf(stderr, "Cannot open '%s': %d, %s\n", dev_name, errno, strerror(errno));
		return FAILURE_DEVICEOPEN;
	}
	return EXIT_SUCCESS;
}

bool v4l2_camera::creat()
{
	int ret;

	ret = open_device();
	if(ret < 0)
		return false;

	ret = init_device();
	if(ret < 0)
		return false;

	return true;
}

void v4l2_camera::destroy()
{
	//int stoptime = OSA_getCurTimeInMsec();
	stop();
	//printf("***************stoptime = %d\n",OSA_getCurTimeInMsec()-stoptime);
	//int uninit_devicetime = OSA_getCurTimeInMsec();
	uninit_device();
	//printf("***************uninit_devicetime = %d\n",OSA_getCurTimeInMsec()-uninit_devicetime);
	//int close_devicetime = OSA_getCurTimeInMsec();
	close_device();
	//printf("***************close_devicetime = %d\n",OSA_getCurTimeInMsec()-close_devicetime);

}

void v4l2_camera::run()
{
	start_capturing();
	bRun = true;
}

void v4l2_camera::stop()
{
	stop_capturing();
	bRun = false;
}


/*
void v4l2_camera::mainloop(void)
{
	fd_set fds;
	struct timeval tv;
	int ret;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	/ * Timeout. * /
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	ret = select(fd + 1, &fds, NULL, NULL, &tv);

	if(-1 == ret)
	{
		if (EINTR == errno)
			return;

		errno_exit("select");
	}else if(0 == ret)
	{
		fprintf(stderr, "select timeout\n");
		exit(EXIT_FAILURE);
	}

	if(-1 == read_frame())  / * EAGAIN - continue select loop. * /
		return;

}*/

int v4l2_camera::init_Captureparm(int devid,int width,int height)
{
	if(width==0||height==0)
		return -1;
	imgwidth=width;
	imgheight=height;
}




