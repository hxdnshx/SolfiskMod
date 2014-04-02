#pragma once

#include "TH135AddrDef.h"

enum SCORELINE_FILTER_MASK
{
	SCORELINE_FILTER__TIMESTAMP_BEGIN = 1,
	SCORELINE_FILTER__TIMESTAMP_END = SCORELINE_FILTER__TIMESTAMP_BEGIN << 1,
	SCORELINE_FILTER__P1NAME = SCORELINE_FILTER__TIMESTAMP_END << 1,
	SCORELINE_FILTER__P1NAMELIKE = SCORELINE_FILTER__P1NAME << 1,
	SCORELINE_FILTER__P1ID = SCORELINE_FILTER__P1NAMELIKE << 1,
	SCORELINE_FILTER__P1WIN = SCORELINE_FILTER__P1ID << 1,
	SCORELINE_FILTER__P2NAME = SCORELINE_FILTER__P1WIN << 1,
	SCORELINE_FILTER__P2NAMELIKE = SCORELINE_FILTER__P2NAME << 1,
	SCORELINE_FILTER__P2ID = SCORELINE_FILTER__P2NAMELIKE << 1,
	SCORELINE_FILTER__P2WIN = SCORELINE_FILTER__P2ID << 1,
	SCORELINE_FILTER__MAX  = SCORELINE_FILTER__P2WIN,
	SCORELINE_FILTER__SQLMASK = (SCORELINE_FILTER__MAX << 1) - 1,
	SCORELINE_FILTER__LIMIT = SCORELINE_FILTER__MAX << 1,
};

struct SCORELINE_FILTER_DESC
{
	int mask;
	time_t t_begin;
	time_t t_end;
	char p1name[MAX_PATH];
	int p1id;
	int p1win;
	char p2name[MAX_PATH];
	int p2id;
	int p2win;
	int limit;

	struct SCORELINE_FILTER_DESC & operator <<=(const struct SCORELINE_FILTER_DESC &src)
	{
		this->mask |= src.mask;

		if (src.mask & SCORELINE_FILTER__TIMESTAMP_BEGIN) {
			this->t_begin = src.t_begin;
		}
		if (src.mask & SCORELINE_FILTER__TIMESTAMP_END) {
			this->t_end = src.t_end;
		}
		if (src.mask & (SCORELINE_FILTER__P1NAME | SCORELINE_FILTER__P1NAMELIKE)) {
			memcpy(this->p1name, src.p1name, sizeof(this->p1name));
		}
		if (src.mask & SCORELINE_FILTER__P1ID) {
			this->p1id = src.p1id;
		}
		if (src.mask & SCORELINE_FILTER__P1WIN) {
			this->p1win = src.p1win;
		}
		if (src.mask & (SCORELINE_FILTER__P2NAME | SCORELINE_FILTER__P2NAMELIKE)) {
			memcpy(this->p2name, src.p2name, sizeof(this->p2name));
		}
		if (src.mask & SCORELINE_FILTER__P2ID) {
			this->p2id = src.p2id;
		}
		if (src.mask & SCORELINE_FILTER__P2WIN) {
			this->p2win = src.p2win;
		}
		if (src.mask & SCORELINE_FILTER__LIMIT) {
			this->limit = src.limit;
		}
		return *this;
	}
};

struct SCORELINE_ITEM
{
	time_t timestamp;
	char p1name[MAX_PATH];
	int p1id;
	int p1win;
	char p2name[MAX_PATH];
	int p2id;
	int p2win;
};

enum COMBOINFO_FILTER_MASK
{
	COMBOINFO_FILTER__TIMESTAMPSTART=1,
	COMBOINFO_FILTER__TIMESTAMPEND=COMBOINFO_FILTER__TIMESTAMPSTART<<1,
	COMBOINFO_FILTER__BATTLE=COMBOINFO_FILTER__TIMESTAMPEND<<1,
	COMBOINFO_FILTER__PNAME=COMBOINFO_FILTER__BATTLE<<1,
	COMBOINFO_FILTER__PID=COMBOINFO_FILTER__PNAME<<1,
	COMBOINFO_FILTER__FIN=COMBOINFO_FILTER__PID<<1,
	COMBOINFO_FILTER__MAX=COMBOINFO_FILTER__FIN<<1
};

struct COMBOINFO_FILTER_DESC
{
	int mask;
	time_t timestamps;
	time_t timestampe;
	INT64 battle;
	char pname[MAX_PATH];
	int pid;
	int fin;
	COMBOINFO_FILTER_DESC& operator <<=(const COMBOINFO_FILTER_DESC& src)
	{
		mask|=src.mask;
		if(src.mask & COMBOINFO_FILTER__TIMESTAMPSTART)
		{
			timestamps=src.timestamps;
		}
		if(src.mask & COMBOINFO_FILTER__TIMESTAMPEND)
		{
			timestampe=src.timestampe;
		}
		if(src.mask & COMBOINFO_FILTER__BATTLE)
		{
			battle=src.battle;
		}
		if(src.mask & COMBOINFO_FILTER__PNAME)
		{
			memcpy(pname,src.pname,sizeof(src.pname));
		}
		if(src.mask & COMBOINFO_FILTER__PID)
		{
			pid=src.pid;
		}
		if(src.mask & COMBOINFO_FILTER__FIN)
		{
			fin=src.fin;
		}
	}
};

struct COMBOINFO_ITEM
{
	time_t timestamp;
	INT64 battle;
	char pname[MAX_PATH];
	int pid;
	int damage;
	int stun;
	int rate;
	int hit;
	int currenthp;
	int fin;
	COMBOINFO_ITEM()
	{
		timestamp=-1;
		battle=-1;
		pid=-1;
		damage=-1;
		stun=-1;
		pname[0]='\0';
		hit=-1;
		rate=-1;
		currenthp=-1;
		fin=0;
	}
	COMBOINFO_ITEM(const COMBOINFO_ITEM &src)
	{
		timestamp=src.timestamp;
		battle=src.battle;
		pid=src.pid;
		damage=src.damage;
		stun=src.stun;
		strcpy_s(pname,MAX_PATH*sizeof(char),src.pname);
		hit=src.hit;
		currenthp=src.currenthp;
		rate=src.rate;
		fin=src.fin;
	}
};

bool ScoreLine_Open(bool create);
void ScoreLine_Close();

void ScoreLine_Enter();
void ScoreLine_Leave(bool failed);

bool ComboInfo_QueryRecord(COMBOINFO_FILTER_DESC &filterdesc,void(*callback)(COMBOINFO_ITEM*,void*),void* user);
bool ScoreLine_QueryTrackRecord(SCORELINE_FILTER_DESC &filterdesc);
bool ScoreLine_QueryProfileRank(SCORELINE_FILTER_DESC &filterdesc, void(*callback)(SCORELINE_ITEM *, void *), void *user);
bool ScoreLine_QueryTrackRecordLog(SCORELINE_FILTER_DESC &filterdesc, void(*callback)(SCORELINE_ITEM *, void *), void *user);
bool ScoreLine_QueryTrackRecordTop(SCORELINE_FILTER_DESC &filterDesc, SCORELINE_ITEM &ret);

DWORD ScoreLine_Read(int p1, int p2, int idx);

bool ScoreLine_Append(SCORELINE_ITEM *item);
bool ComboInfo_Append(COMBOINFO_ITEM *item);
bool ScoreLine_Remove(time_t timestamp);

void ScoreLine_SetPath(LPCTSTR path);
LPCTSTR ScoreLine_GetPath();

