
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.hpp>
#include <opencv2/opencv.hpp>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "osa.h"
#include "gst_capture.h"

#define clip(rslt, a, b)  (rslt>b)?(b):((rslt<a)?a:rslt)

#define ENT_CHN_MAX		(1)
typedef struct _enctran_enc_param
{
	int fps;
	int bitrate;
	int minQP;
	int maxQP;
	int minQI;
	int maxQI;
	int minQB;
	int maxQB;
}ENCTRAN_encPrm;

typedef struct _enctran_init_param
{
	bool bRtp;
	char destIpaddr[32];
	int destPort;
	bool defaultEnable[ENT_CHN_MAX];	// no used
	CAPTURE_SRC srcType[ENT_CHN_MAX];
	ENCTRAN_encPrm encPrm[ENT_CHN_MAX];
	cv::Size imgSize[ENT_CHN_MAX];
	int inputFPS[ENT_CHN_MAX];	// no used
	int nChannels;
}ENCTRAN_InitPrm;

//////////////////////////////////////////////////////
static ENCTRAN_InitPrm m_initPrm = {0};
static GstCapture_data m_gstCapture_data[ENT_CHN_MAX];
static RecordHandle * m_record_handle[ENT_CHN_MAX];
static char strFormat[16] = "I420";//"YUY2"//"GRAY8"

//////////////////////////////////////////////////////
static int createEncoder(int chId, ENCTRAN_InitPrm *pEncInit)
{
	if(m_record_handle[chId] != NULL)
		gstCaptureUninit(m_record_handle[chId]);

	memset(&m_gstCapture_data[chId], 0, sizeof(GstCapture_data));
	m_gstCapture_data[chId].width = pEncInit->imgSize[chId].width;
	m_gstCapture_data[chId].height = pEncInit->imgSize[chId].height;
	m_gstCapture_data[chId].framerate = pEncInit->encPrm[chId].fps;
	m_gstCapture_data[chId].bitrate = pEncInit->encPrm[chId].bitrate;
	m_gstCapture_data[chId].ip_port = pEncInit->destPort;
	m_gstCapture_data[chId].filp_method = FLIP_METHOD_VERTICAL_FLIP;
	m_gstCapture_data[chId].capture_src = pEncInit->srcType[chId];
	//m_gstCapture_data[chId].format = "YUY2";
	m_gstCapture_data[chId].format = strFormat;//"I420";
	if(pEncInit->bRtp)
		m_gstCapture_data[chId].ip_addr = &pEncInit->destIpaddr[0];
	else
		m_gstCapture_data[chId].ip_addr = NULL;
	//m_gstCapture_data[chId].sd_cb=;
	for(int i=0;i<ENC_QP_PARAMS_COUNT;i++)
		m_gstCapture_data[chId].Q_PIB[i]=-1;
	//m_gstCapture_data[chId].notify = m_semScheduler[chId];

	m_record_handle[chId] = gstCaptureInit(m_gstCapture_data[chId]);

	return (m_record_handle[chId] == NULL) ? -1 : 0;
}

static int deleteEncoder(int chId)
{
	return gstCaptureUninit(m_record_handle[chId]);
}

int GstEncTransCreate(bool bRtp, int rmIpaddr, int rmPort)
{
	int ret = 0, chId = 0;
	ENCTRAN_InitPrm *pEncInit = &m_initPrm;
	char stripaddr[32];
	struct in_addr in;

	memset(m_record_handle, 0, sizeof(m_record_handle));
	memset(pEncInit, 0, sizeof(ENCTRAN_InitPrm));
	pEncInit->nChannels = 1;
	if(bRtp && (rmIpaddr != 0))
	{
		pEncInit->destPort = clip(rmPort, 11000, 12000);
		in.s_addr = htonl(rmIpaddr);
		strcpy(stripaddr, inet_ntoa(in));
		strcpy(pEncInit->destIpaddr, stripaddr);
		pEncInit->bRtp = true;
	}
	for(chId=0; chId<pEncInit->nChannels; chId++)
	{
		pEncInit->imgSize[chId] = cv::Size(1920, 1080);	// set max value because crgst molloc need
		pEncInit->encPrm[chId].fps = 25;
		pEncInit->encPrm[chId].bitrate = 5600000;
		pEncInit->srcType[chId] = XIMAGESRC;
		createEncoder(chId, pEncInit);
	}

	return ret;
}

int GstEncTransDestroy()
{
	int ret = 0, chId = 0;
	ENCTRAN_InitPrm *pEncInit = &m_initPrm;

	for(chId=0; chId<pEncInit->nChannels; chId++)
	{
		deleteEncoder(chId);
		m_record_handle[chId] = NULL;
	}

	return ret;
}

