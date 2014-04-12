#pragma once

#include "TH135AddrDef.h"

enum COMBOREC_FILTER_MASK
{
	COMBOREC_FILTER__PID=1,
	COMBOREC_FILTER__DAMAGE=COMBOREC_FILTER__PID<<1,
	COMBOREC_FILTER__RATE=COMBOREC_FILTER__DAMAGE<<1,
	COMBOREC_FILTER__STUN=COMBOREC_FILTER__RATE<<1,
	COMBOREC_FILTER__MAX=COMBOREC_FILTER__STUN
};

struct COMBOREC_FILTER_DESC
{
	int mask;
	int pid;
	int damage_l,damage_h;
	int rate_l,rate_h;
	int stun_l,stun_h;

	struct COMBOREC_FILTER_DESC & operator <<=(const struct COMBOREC_FILTER_DESC &src)
	{
		mask|=src.mask;
		if(src.mask & COMBOREC_FILTER__PID)
		{
			pid=src.pid;
		}
		if(src.mask & COMBOREC_FILTER__DAMAGE)
		{
			damage_l=src.damage_l;
			damage_h=src.damage_h;
		}
		if(src.mask & COMBOREC_FILTER__RATE)
		{
			rate_l=src.rate_l;
			rate_h=src.rate_h;
		}
		if(src.mask & COMBOREC_FILTER__STUN)
		{
			stun_l=src.stun_l;
			stun_h=src.stun_h;
		}
		return *this;
	}
};

struct COMBOREC_ITEM
{
	int pid;
	char txt[MAX_PATH];
	int damage;
	int rate;
	int stun;
	bool isstunMax;
	bool israteMin;
};

void ComboRec_SetPath(LPCTSTR path);
LPCTSTR ComboRec_GetPath();
bool ComboRec_QueryRecord(COMBOREC_FILTER_DESC &filterDesc, void(*callback)(COMBOREC_ITEM *,void *), void *user);
bool ComboRec_Remove(COMBOREC_FILTER_DESC &filterDesc);
bool ComboRec_Append(COMBOREC_ITEM *item);
bool ComboRec_Open(bool create);
void ComboRec_Enter();
void ComboRec_Leave(bool failed);
void ComboRec_Close();
void ComboRec_Analysis(const COMBOREC_ITEM &src,Minimal::MinimalStringT<wchar_t> &ret,int hit,int prevlife,int prevrate,int prevstun);