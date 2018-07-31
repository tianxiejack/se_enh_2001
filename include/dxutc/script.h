#ifndef _SCRIPT_H
#define _SCRIPT_H

typedef struct _Enum_Def{
	char text[48];
	int value;
}ENUMDEF;
typedef struct _Enum_Tab{
	int tablenum;
	ENUMDEF def[16];
	int idef;
}ENUMTAB;
//FIELD <blkid> <fldno> <lab> <scr> <x> <y> <format> [<limits>] <init> [<opfs>] [ENUMS <tabno>]
typedef struct _Script_Field{
	int mode;// FIELD or WRITE_ONLY
	bool bCheckBox;
	int blkid;	int fldno; char lab[128]; int scr; int x; int y; int dataFmt;
	int imin; int imax; float fmin; float fmax; int iInit; float fInit; char opfs[8]; int enumtabno;
	ENUMTAB *enumtab;
}SCRIPT_FIELD;

typedef struct _DataBlk{
	bool bValid;
	int  number;
	char label[48];
	int x; int y;
	char text[48];
	SCRIPT_FIELD field[16];
	int iValidField;
}DATABLK;

typedef struct _App_Script{
	DATABLK dataBlk[256];
	int nDataBlk;
}APP_SCRIPT;

typedef struct _Script_Object{
	APP_SCRIPT m_script;
	int m_nEnumTab;
	ENUMTAB m_enumTab[128];
	int m_iCurDataBlk;
}SCRIPT_OBJ;

int script_create(char *scriptfile, SCRIPT_OBJ** ppObj);
void   script_destroy(SCRIPT_OBJ *pObj);

#endif
