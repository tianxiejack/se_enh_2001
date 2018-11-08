#include"osd_cv.h"


 CvScalar  GetcvColour(int colour)
{

	switch(colour)
		{
		case 0:
			return cvScalar(0,0,0,0);
			break;
		case 1:
			return cvScalar(0,0,0,255);
			break;
		case 2:
			return cvScalar(255,255,255,255);
			break;
		case 3:
			return cvScalar(0,0,255,255);
			break;
		case 4:
			return cvScalar(0,255,255,255);
			break;
		case 5:
			return cvScalar(255,0,0,255);
			break;
		case 6:
			return cvScalar(0,255,0,255);
			break;
		default:
			return cvScalar(255,255,255,255);
			break;
		}


}


void drawdashlinepri(Mat frame,int startx,int starty,int endx,int endy,int linelength,int dashlength,int colour1)
{
	  int i=0;
	  int flagx=1;
	  int flagy=1;
	  double totallengthy=0.0;
	  double  totallengthx=0.0;
	  int totallength=dashlength+linelength;  
	 // float len=sqrt((endy-starty)*(endy-starty)+(endx-startx)*(endx-startx));
	 int len=abs(endy-starty)+abs(endx-startx);
  	  double nCount=len/totallength;// 
  	  CvScalar colour=GetcvColour(colour1);
  	  totallengthx=abs(endx-startx)*1.0/nCount;
  	  totallengthy=abs(endy-starty)*1.0/nCount;
  	  
  	  Point start,end;//start and end point of each dash  
  	  if(startx>endx)
  	  	{
			flagx=-1;
  	  	}
	   if(starty>endy)
  	  	{
			flagy=-1;
  	  	}
	if(startx==endx)
		{

		
			for (int i=0;i<nCount;i=i+2)
		   	 {  
		      		end.x=startx;
		      		start.x=startx; 
		      		start.y=cvRound(starty+i*totallengthy*flagy);
		      		end.y=cvRound(starty+(i+1)*totallengthy*flagy);//draw left dash line
				 line(frame, start, end, colour, 1, 8, 0 ); 
		  	  }  
			return ;
		}
	if(starty==endy)
		{
			for (int i=0;i<nCount;i=i+2)
		   	 {  
		      		start.x=cvRound(startx+i*totallengthx*flagx);
		      		end.x=cvRound(startx+(i+1)*totallengthx*flagx); 
		      		start.y=starty;
				end.y=starty; 
				 line(frame, start, end, colour, 1, 8, 0 ); 
		  	  }  
			return ;


		}
	  
	for (int i=0;i<nCount;i=i+2)
   	 {  
      		end.x=cvRound(startx+(i+1)*totallengthx*flagx);//draw top dash line  
      		start.x=cvRound(startx+i*totallengthx*flagx);  
      		start.y=cvRound(starty+i*totallengthy*flagy);  
		end.y=cvRound(starty+(i+1)*totallengthy*flagy);//draw left dash line  
  
		 line(frame, start, end, colour, 1, 8, 0 ); 
  	  }  
}


void DrawcvDashcross(Mat frame,Line_Param_fb *lineparm,int linelength,int dashlength)
{
	int centx, centy, width;
	if(lineparm==NULL)
		{
			return;
		}
	centx=lineparm->x;
	centy=lineparm->y;
	width=lineparm->width;
	int startx=centx-width/2;
	int starty=centy;
	int endx=centx+width/2;
	int endy=centy;
	drawdashlinepri(frame,startx,starty,endx,endy,linelength,dashlength,lineparm->frcolor);

	startx=centx;
	starty=centy-width/2;
	endx=centx;
	endy=centy+width/2;
	drawdashlinepri(frame,startx,starty,endx,endy,linelength,dashlength,lineparm->frcolor);
	

}

void DrawcvLine(Mat frame,Osd_cvPoint *start,Osd_cvPoint *end,int frcolor,int linew)
{
	Point pt1,pt2,center;
	 CvScalar colour=GetcvColour(frcolor);
	pt1.x=start->x;
	pt1.y=start->y;
	pt2.x=end->x;
	pt2.y=end->y;
	line(frame, pt1, pt2, colour, linew, 8, 0 );	
}
void Drawcvcross(Mat frame,Line_Param_fb *lineparm)
{
	return ;
	int centx, centy, width;
	int crossw=width;
	Point pt1,pt2,center;
	if(lineparm==NULL)
	{
		return ;
	}
	Osd_cvPoint start;
	Osd_cvPoint end;
	start.x=lineparm->x-lineparm->width/2;
	start.y=lineparm->y;
	end.x=lineparm->x+lineparm->width/2;
	end.y=lineparm->y;
	DrawcvLine(frame,&start,&end,lineparm->frcolor,1);

	start.x=lineparm->x;
	start.y=lineparm->y-lineparm->width/2;
	end.x=lineparm->x;
	end.y=lineparm->y+lineparm->width/2;
	DrawcvLine(frame,&start,&end,lineparm->frcolor,1);

}
void Drawcvcrossaim(Mat frame,Line_Param_fb *lineparm)
{

	int centx, centy, width;
	Point pt1,pt2,center;
	if(lineparm==NULL)
		return ;

	int sep=10;
	Osd_cvPoint start;
	Osd_cvPoint end;

	// right
	start.x=lineparm->x+sep;
	start.y=lineparm->y;
	end.x=lineparm->x+lineparm->width/2;
	end.y=lineparm->y;
	DrawcvLine(frame,&start,&end,lineparm->frcolor,1);
	
	start.x=lineparm->x+sep;
	start.y=lineparm->y+1;
	end.x=lineparm->x+lineparm->width/2;
	end.y=lineparm->y+1;
	DrawcvLine(frame,&start,&end,lineparm->frcolor,1);

	// left
	start.x=lineparm->x-lineparm->width/2;
	start.y=lineparm->y;
	end.x=lineparm->x-sep;
	end.y=lineparm->y;
	DrawcvLine(frame,&start,&end,lineparm->frcolor,1);
	
	start.x=lineparm->x-lineparm->width/2;
	start.y=lineparm->y+1;
	end.x=lineparm->x-sep;
	end.y=lineparm->y+1;
	DrawcvLine(frame,&start,&end,lineparm->frcolor,1);

	//up
	start.x=lineparm->x;
	start.y=lineparm->y-lineparm->height/2;
	end.x=lineparm->x;
	end.y=lineparm->y-sep;
	DrawcvLine(frame,&start,&end,lineparm->frcolor,1);
	
	start.x=lineparm->x+1;
	start.y=lineparm->y-lineparm->height/2;
	end.x=lineparm->x+1;
	end.y=lineparm->y-sep;
	DrawcvLine(frame,&start,&end,lineparm->frcolor,1);

	//down
	start.x=lineparm->x;
	start.y=lineparm->y+sep;
	end.x=lineparm->x;
	end.y=lineparm->y+lineparm->height/2;
	DrawcvLine(frame,&start,&end,lineparm->frcolor,1);
	
	start.x=lineparm->x+1;
	start.y=lineparm->y+sep;
	end.x=lineparm->x+1;
	end.y=lineparm->y+lineparm->height/2;
	DrawcvLine(frame,&start,&end,lineparm->frcolor,1);

	//point
	start.x=lineparm->x-1;
	start.y=lineparm->y;
	end.x=lineparm->x+1;
	end.y=lineparm->y;
	DrawcvLine(frame,&start,&end,lineparm->frcolor,1);

	start.x=lineparm->x;
	start.y=lineparm->y-1;
	end.x=lineparm->x;
	end.y=lineparm->y+1;
	DrawcvLine(frame,&start,&end,lineparm->frcolor,1);
	
}


void drawcvrect(Mat frame,int x,int y,int width,int height,int frcolor)
{
	int thickness = 1;
	if(frcolor == 6 || frcolor == 0)
		thickness = 2;
	CvScalar colour=GetcvColour(frcolor);
	rectangle( frame,Point( x,y ),Point( x+width, y+height),colour, thickness, 8);
}

