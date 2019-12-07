#ifndef ENCTRANS_HPP_
#define ENCTRANS_HPP_

#define ENCTRANS_ON
//#undef ENCTRANS_ON

#define ENT_CHN_MAX		(2)

int EncTrans_create(int nChannels, cv::Size imgSize[]);
int EncTrans_destroy(void);

////////////////////////////////////
int EncTrans_screen_rtpout(bool bRtp, int rmIpaddr, int rmPort);
int EncTrans_appcap_rtpout(int chId, bool bRtp, int rmIpaddr, int rmPort);
int EncTrans_appcap_frame(int chId, char *pbuffer, int datasize);

#endif /* GSTENCTRANS_HPP_ */

