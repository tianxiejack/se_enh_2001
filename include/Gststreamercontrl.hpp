

#ifndef GSTSTREAMERCONTRL_HPP_
#define GSTSTREAMERCONTRL_HPP_
//#include"Gststream.hpp"

class GstreamerContrl;



class GstreamerContrl
{
public:
	static GstreamerContrl* getInstance();

private:
	
	GstreamerContrl();
	~GstreamerContrl();
	static GstreamerContrl* instance;
	

};


#endif /* GSTSTREAMERCONTRL_HPP_ */
