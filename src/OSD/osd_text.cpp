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

	_font_hd_big_ht   =   new FreeTypeFont();
	_font_hd_big_ht->create("FZLTCXHJW.TTF",46,512,512);
	
	_font_hd_small_ht   =   new FreeTypeFont();
	_font_hd_small_ht->create("FZLTCXHJW.TTF",32,512,512);
	
	_font_pal_big_ht   =   new FreeTypeFont();
	_font_pal_big_ht->create("FZLTCXHJW.TTF",22,512,512);
	
	_font_pal_small_ht   =   new FreeTypeFont();
	_font_pal_small_ht->create("FZLTCXHJW.TTF",18,512,512);

}

void OSDdrawText(int x,int y,wchar_t* text,char font,char fontsize,int win_width,int win_height)
{
	FreeTypeFont* pTmp  = NULL;
	if(win_width == 1920){
		if(font == 0x02){
			if(fontsize == 0x06)
				pTmp = _font_hd_small_ht;
			else
				pTmp = _font_hd_big_ht;
		}else{
			if(fontsize == 0x06)
				pTmp = _font_hd_small_st;
			else
				pTmp = _font_hd_big_st;
		}
	}else{
		if(font == 0x02){
			if(fontsize == 0x06)
				pTmp = _font_pal_small_ht;
			else
				pTmp = _font_pal_big_ht;
		}else{
			if(fontsize == 0x05)
				pTmp = _font_pal_small_st;
			else
				pTmp = _font_pal_big_st;
		}
	}
	pTmp->begin(win_width,win_height);
	pTmp->drawText(x,y,0,Rgba(255,255,255,255),text,0,0,0);
	pTmp->end();
	return ;
}
