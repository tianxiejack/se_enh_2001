#include"Gststreamercontrl.hpp"
#include "stdio.h"


GstreamerContrl* GstreamerContrl::instance=NULL;

GstreamerContrl::GstreamerContrl()
{

}

GstreamerContrl::~GstreamerContrl()
{

}

GstreamerContrl* GstreamerContrl::getInstance()
{
	if(instance==NULL)
		{
			instance=new GstreamerContrl();
			printf("************%s***********************\n",__func__);
		}
	return instance;

}
