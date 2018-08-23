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
FreeTypeFont*	_font_hd_mid_st;
FreeTypeFont*	_font_hd_small_st;

FreeTypeFont*	_font_hd_big_ht;
FreeTypeFont*	_font_hd_mid_ht;
FreeTypeFont*	_font_hd_small_ht;

void OSDCreatText()
{
	_font_hd_big_st   =   new FreeTypeFont();
	_font_hd_big_st->create("simsun.ttc",40,512,512);

	_font_hd_mid_st   =   new FreeTypeFont();
	_font_hd_mid_st->create("simsun.ttc",32,512,512);	
	
	_font_hd_small_st   =   new FreeTypeFont();
	_font_hd_small_st->create("simsun.ttc",25,512,512);
	
	_font_hd_big_ht   =   new FreeTypeFont();
	_font_hd_big_ht->create("SIMLI.TTF",40,512,512);

	_font_hd_mid_ht   =   new FreeTypeFont();
	_font_hd_mid_ht->create("SIMLI.TTF",32,512,512);
	
	_font_hd_small_ht   =   new FreeTypeFont();
	_font_hd_small_ht->create("SIMLI.TTF",25,512,512);
	
}

void OSDdrawText(int x,int y,wchar_t* text,char font,char fontsize,int win_width,int win_height)
{
	FreeTypeFont* pTmp  = NULL;

	if(font == 0x02){
		if(fontsize == 0x03)
			pTmp = _font_hd_big_ht;
		else if(fontsize == 0x02)
			pTmp = _font_hd_mid_ht;
		else
			pTmp = _font_hd_small_ht;
	}else{
		if(fontsize == 0x03)
			pTmp = _font_hd_big_st;
		else if(fontsize == 0x02)
			pTmp = _font_hd_mid_st;
		else
			pTmp = _font_hd_small_st;
	}
	
	pTmp->begin(win_width,win_height);
	pTmp->drawText(x,y,0,Rgba(255,255,255,255),text,0,0,0);
	pTmp->end();
	return ;
}
