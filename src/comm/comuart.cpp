/*
 * 803uart.cpp
 *
 *  Created on: 2019年7月31日
 *      Author: alex
 */

#include "comuart.h"
#include <math.h>
#include <opencv2/core/core.hpp>

using namespace cv;

static C803COM* gThis;

C803COM::C803COM(pFuncCallvback pfunc):pCom1(NULL),pCom2(NULL),
					existRecvThread(false),m_cmdlength(9),m_trktime(3)
{
	memset(m_senddata,0,sizeof(m_senddata));
	memset(m_recvdata,0,sizeof(m_recvdata));
	OSA_mutexCreate(&m_com1mutex);
	m_rcvBuf.clear();
	m_pFunc = pfunc;
	gThis = this;
}

C803COM::~C803COM()
{
	if(pCom1 != NULL)
	{
		pCom1->cclose();
		delete pCom1;
	}

	if(pCom2 != NULL)
	{
		pCom1->cclose();
		delete pCom1;
	}
	
	OSA_mutexDelete(&m_com1mutex);
}


void C803COM::createPort()
{
	pCom1 = PortFactory::createProduct(1);
	pCom2 = PortFactory::createProduct(3);

	if(pCom1 != NULL)
		com1fd = pCom1->copen();
	if(pCom2 != NULL)
		com2fd = pCom2->copen();

	return;
}

void C803COM::sendtrkerr(int chid,int status,float errx,float erry,int rendercount)
{}

void C803COM::packageSendbaseData()
{}

void C803COM::calcCheckNum()
{
	int sum = 0;
	for(int i=3;i<=9;i++)
		sum += m_senddata[i-1];

	m_senddata[9] = sum&(0xff);
	return;	
}

void* C803COM::runUpExtcmd(void *)
{
	gThis->getExtcmd();
	return NULL;
}

void C803COM::getExtcmd()
{
	int sizercv;
	while(existRecvThread == false)
	{
		sizercv = pCom2->crecv(com2fd, m_recvdata,sizeof(m_recvdata));
		findValidData(m_recvdata,sizercv);
	}
}

void C803COM::findValidData(unsigned char *tmpRcvBuff, int sizeRcv)
{
	unsigned int uartdata_pos = 0;
	unsigned char frame_head[]={0xAA, 0x50};
	
	static struct data_buf
	{
		unsigned int len;
		unsigned int pos;
		unsigned char reading;
		unsigned char buf[1024];
	}swap_data = {0, 0, 0,{0}};

	uartdata_pos = 0;
	if(sizeRcv>0)
	{
		printf("recv data is :    ");
		for(int j=0;j<sizeRcv;j++)
		{
			printf("%02x ",tmpRcvBuff[j]);
		}
		printf("\n");

		while (uartdata_pos< sizeRcv)
		{
	        		if((0 == swap_data.reading) || (2 == swap_data.reading))
	       		{
	            			if(frame_head[swap_data.len] == tmpRcvBuff[uartdata_pos])
	            			{
	                			swap_data.buf[swap_data.pos++] =  tmpRcvBuff[uartdata_pos++];
	                			swap_data.len++;
	                			swap_data.reading = 2;
	                			if(swap_data.len == sizeof(frame_head)/sizeof(char))
	                    				swap_data.reading = 1;
	            			}
		           		 else
		            		{
		                		uartdata_pos++;
		                		if(2 == swap_data.reading)
		                    		memset(&swap_data, 0, sizeof(struct data_buf));
		            		}
			}
		        	else if(1 == swap_data.reading)
			{
				swap_data.buf[swap_data.pos++] = tmpRcvBuff[uartdata_pos++];
				swap_data.len++;
				if(swap_data.len>=4)
				{
					if(swap_data.len==(((swap_data.buf[2]<<8)|(swap_data.buf[3]))+5))
					{

						for(int i=0;i<swap_data.len;i++)
						{
							rcvBufQue.push_back(swap_data.buf[i]);
						}
						parsingComEvent();
						memset(&swap_data, 0, sizeof(struct data_buf));
					}
				}
			}
		}
	}
}


int C803COM::parsingComEvent()
{
	int ret =  -1;
	int cmdLength= (rcvBufQue.at(2)<<8 | rcvBufQue.at(3))+5;
	int block, field;
	float value;
	unsigned char tempbuf[4];
	unsigned char contentBuff[128]={0};
	
	if(cmdLength<6)
	{
        	printf("Warning::Invaild frame\r\n");
        	rcvBufQue.erase(rcvBufQue.begin(),rcvBufQue.begin()+cmdLength);
        	return ret;
    	}
    	unsigned char checkSum = recvcheck_sum(cmdLength);

    	if(checkSum== rcvBufQue.at(cmdLength-1))
    	{	
    		switch(rcvBufQue.at(4))
    		{
            		case 0x01:
				if(0 == rcvBufQue.at(5)) 	//tv
					m_pFunc(0);
				else if(1 == rcvBufQue.at(5))	//pal
					m_pFunc(1);
				break;

			 case 0x02:
				if(0 == rcvBufQue.at(5)) //enbale enh
					m_pFunc(2);
				else if(1 == rcvBufQue.at(5))//disable enh
					m_pFunc(3);
				break;
				
    		default:
       			printf("INFO: Unknow  Control Command, please check!!!\r\n ");
        			ret =0;
        			break;
   		 }
    	}
    	else
		printf("%s,%d, checksum error:,cal is %02x,recv is %02x\n",__FILE__,__LINE__,checkSum,rcvBufQue.at(cmdLength-1));
		
	rcvBufQue.erase(rcvBufQue.begin(),rcvBufQue.begin()+cmdLength);
	return 1;

}


unsigned char C803COM::recvcheck_sum(int len_t)
{
    unsigned char cksum = 0;
    for(int n=4; n<len_t-1; n++)
    {
        cksum ^= rcvBufQue.at(n);
    }
    return  cksum;
}


unsigned int C803COM::recvcheck_sum()
{
	unsigned int sum = 0;
	for(int i=2;i<=6;i++)
		sum += m_rcvBuf[i];
	return sum;
}


void C803COM::parsing()
{
}


int C803COM::readtrktime()
{}


void C803COM::saveTrktime()
{}

void C803COM::calcCheckNumMtdprm()
{
	int sum = 0;
	for(int i=3;i<=23;i++)
		sum += m_sendMtdPrm[i-1];

	m_sendMtdPrm[43] = sum&(0xff);

	return;	
}
