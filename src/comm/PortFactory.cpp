#include "PortFactory.hpp"
#include "CUartProc.hpp"
#include "CNetProc.hpp"
#include <iostream>
#include "configtable.h"
#include <stdio.h>
#include <opencv2/core/core.hpp>

using namespace cv;

PortFactory::PortFactory()
{

}
  
PortFactory::~PortFactory()
{
 
}

CPortInterface* PortFactory::createProduct(int type)
{
	CPortInterface* temp = NULL;
	uartparams_t param = {9600, 0, 8, 'N', 1};
    	switch(type)
    	{
		case 1:
			temp = new CUartProc("/dev/ttyTHS1", 19200, 0, 8, 'N', 1);
	            	break;
	        case 2:
			temp = new CNetProc(10000);
		   	break;
	        case 3:
			temp = new CUartProc("/dev/ttyTHS2", 19200, 0, 8, 'N', 1);
			break;
	        default:
	            	break;
	}
    	return temp;
}

