/*
 * osd_text.cpp
 *
 *  Created on: 2018年5月23日
 *      Author: alex
 */

#include <stdio.h>
#include <stdlib.h>
//#include <GL/glut.h>
#include "freetype.hpp"

FreeTypeFont*	_font_hd_big_st;
FreeTypeFont*	_font_hd_small_st;
FreeTypeFont*	_font_pal_big_st;
FreeTypeFont*	_font_pal_small_st;

FreeTypeFont*	_font_hd_big_ht;
FreeTypeFont*	_font_hd_small_ht;
FreeTypeFont*	_font_pal_big_ht;
FreeTypeFont*	_font_pal_small_ht;

void OSDCreatText()
{
	_font_hd_big_st   =   new FreeTypeFont();
	_font_hd_big_st->create("simsun.ttc",40,512,512);
	
	_font_hd_small_st   =   new FreeTypeFont();
	_font_hd_small_st->create("simsun.ttc",25,512,512);

	_font_pal_big_st   =   new FreeTypeFont();
	_font_pal_big_st->create("simsun.ttc",16,512,512);
	
	_font_pal_small_st   =   new FreeTypeFont();
	_font_pal_small_st->create("simsun.ttc",12,512,512);
	
}

void OSDdrawText(int x,int y,wchar_t* text,char font,char fontsize,int win_width,int win_height)
{
	if(win_width == 1920){
		if(fontsize == 0x05){
			_font_hd_big_st->begin(win_width,win_height);
			_font_hd_big_st->drawText(x,y,0,Rgba(255,255,255,255),text,0,0,0);
			_font_hd_big_st->end();
		}else{
			_font_hd_small_st->begin(win_width,win_height);
			_font_hd_small_st->drawText(x,y,0,Rgba(255,255,255,255),text,0,0,0);
			_font_hd_small_st->end();
		}
	}else{
		if(fontsize == 0x05){
			_font_pal_big_st->begin(win_width,win_height);
			_font_pal_big_st->drawText(x,y,0,Rgba(255,255,255,255),text,0,0,0);
			_font_pal_big_st->end();
		}else{
			_font_pal_small_st->begin(win_width,win_height);
			_font_pal_small_st->drawText(x,y,0,Rgba(255,255,255,255),text,0,0,0);
			_font_pal_small_st->end();
		}
	}
	return ;
}
