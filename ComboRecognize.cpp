#include "pch.hpp"
#include "ComboRecognize.hpp"
#include "DebugFunc.hpp"
#include "TH135AddrDef.h"

#include <sqlite3.h>

#include <utility>

#include "MinimalMemory.hpp"

#define MINIMAL_USE_PROCESSHEAPSTRING
#include "MinimalPath.hpp"

#define MINIMAL_USE_PROCESSHEAPARRAY
#include "MinimalArray.hpp"

#define RECORD_TABLE "DamageInfo"
#define EPSILONDAMAGE 2
#define EPSILONRATE 1

static sqlite3 *s_ddb;
static bool s_Enteredview;
static Minimal::ProcessHeapString  s_srPath;
static Minimal::ProcessHeapStringA s_srPathU;

void ComboRec_SetPath(LPCTSTR path)
{
	s_srPath=path;
	Minimal::ToUTF8(s_srPathU,path);
}

LPCTSTR ComboRec_GetPath()
{
	return s_srPath;
}

static void ComboRec_UserFunc_AnsiToUTF8(sqlite3_context *context, int argc, sqlite3_value **argv) {
	if (argc >= 1) {
		const char *source = reinterpret_cast<const char *>(sqlite3_value_text(argv[0]));
		Minimal::ProcessHeapStringA utf8ized;
		Minimal::ToUTF8(utf8ized, source);
		sqlite3_result_text(context, utf8ized.GetRaw(), utf8ized.GetSize(), SQLITE_TRANSIENT);
	}
}

static void ComboRec_ConstructFilter(Minimal::ProcessHeapStringA &ret, COMBOREC_FILTER_DESC& filterDesc)
{
	char buff[128];
	ret+=" WHERE";

	int count=0;
	for(int i=1;i<=COMBOREC_FILTER__MAX;i<<=1)
	{
		if (count > 0 && (filterDesc.mask & i)) ret += " AND ";
		switch(filterDesc.mask & i)
		{
		case COMBOREC_FILTER__PID:
			sqlite3_snprintf(_countof(buff), buff, "pid = '%q'", filterDesc.pid);
			ret += buff;
			++count;
			break;
		case COMBOREC_FILTER__DAMAGE:
			sqlite3_snprintf(_countof(buff), buff, "damage >= %ld AND damage <= %ld", filterDesc.damage_l,filterDesc.damage_h);
			ret += buff;
			++count;
			break;
		case COMBOREC_FILTER__RATE:
			sqlite3_snprintf(_countof(buff), buff, "rate >= %ld AND rate <= %ld", filterDesc.rate_l,filterDesc.rate_h);
			ret += buff;
			++count;
			break;
		case COMBOREC_FILTER__STUN:
			sqlite3_snprintf(_countof(buff), buff, "stun >= %ld AND stun <= %ld", filterDesc.stun_l,filterDesc.stun_h);
			ret += buff;
			++count;
			break;
		}
	}
}

int ComboRec_QueryCallback(void *user,int argc,char **argv,char **colName)
{
	if(argc!=4) return 1;

	COMBOREC_ITEM item;
	::lstrcpyA(item.txt,argv[0]);
	item.damage=::StrToIntA(argv[1]);
	item.rate=::StrToIntA(argv[2]);
	item.stun=::StrToIntA(argv[3]);

	std::pair<void(*)(COMBOREC_ITEM*,void*),void*> *cbinfo;
	*(void**)&cbinfo = user;

	cbinfo->first(&item,cbinfo->second);

	return 0;
}

bool ComboRec_QueryRecord(COMBOREC_FILTER_DESC &filterDesc, void(*callback)(COMBOREC_ITEM *,void *), void *user)
{
	if(s_ddb) return false;

	Minimal::ProcessHeapStringT<char> filterStr;
	ComboRec_ConstructFilter(filterStr, filterDesc);

	std::pair<void(*)(COMBOREC_ITEM*,void*),void*> cbinfo
		=std::make_pair(callback,user);
	char *query;
	query = sqlite3_mprintf(
		"SELECT txt,damage,rate,stun"
		"FROM" RECORD_TABLE "AS T %s",
		filterStr.GetRaw()
		);
	int rc=sqlite3_exec(s_ddb,query,ComboRec_QueryCallback,(void*)&cbinfo,NULL);
	sqlite3_free(query);
	if(rc)return false;
	return true;
}

bool ComboRec_Remove(COMBOREC_FILTER_DESC &filterDesc)
{
	if(!s_ddb)return false;

	Minimal::ProcessHeapStringT<char> filterStr;
	ComboRec_ConstructFilter(filterStr, filterDesc);
	char *query = sqlite3_mprintf(
		"DELETE FROM" RECORD_TABLE "AS T %s",
		filterStr.GetRaw());
	int rc = sqlite3_exec(s_ddb, query, NULL, NULL, NULL);
	sqlite3_free(query);
	if (rc) return false;

	return true;
}

bool ComboRec_Append(COMBOREC_ITEM *item)
{
	sqlite3_stmt *stmt;
	int rc;

	if(!s_ddb) return false;

	rc = sqlite3_prepare(s_ddb,
		"insert to" RECORD_TABLE "(pid,txt,damage,rate,stun) "
		"values (?,?,?,?,?);",-1,&stmt,NULL);
	if(rc)
	{
		const char* err=sqlite3_errmsg(s_ddb);
		Minimal::ProcessHeapStringW str;
		Minimal::ToUCS2(str,err);
		str+=_T("\n");
		WriteToLog(str);
		return false;
	}

	sqlite3_bind_int(stmt,1,item->pid);
	sqlite3_bind_text(stmt,2,item->txt,-1,NULL);
	sqlite3_bind_int(stmt,3,item->damage);
	sqlite3_bind_int(stmt,4,item->rate);
	sqlite3_bind_int(stmt,5,item->stun);
	bool ret = (sqlite3_step(stmt)==SQLITE_DONE);

	sqlite3_finalize(stmt);
	
	return ret;
}



bool ComboRec_Open(bool create)
{
	sqlite3 *db;
	if(PathFileExists(s_srPath))
	{
		int rc = sqlite3_open(s_srPathU,&db);
		if(rc)
		{
			sqlite3_close(db);
			return false;
		}
		char *errmsg;
		rc = sqlite3_exec(db,
			"PRAGMA temp_store = MEMORY;"
			"PRAGMA cache_size = 1048576;"
			, NULL, NULL, &errmsg);
		if (rc) {
			sqlite3_close(db);
			return false;
		}

	}
	else
	{
		int rc = sqlite3_open(s_srPathU, &db);
		if (rc) {
			sqlite3_close(db);
			return false;
		}
		char *errmsg;
		rc = sqlite3_exec(db, 
			"CREATE TABLE " RECORD_TABLE "(\n"
			"pid	INTEGER NOT NULL,\n"
			"txt     TEXT,\n"
			"damage       INTEGER NOT NULL,\n"
			"rate      INTEGER NOT NULL,\n"
			"stun       INTEGER NOT NULL\n"
			")", NULL, NULL, &errmsg);
		if (rc) {
			sqlite3_close(db);
			::DeleteFile(s_srPath);
			return false;
		}
	}
	if (s_ddb) {
		sqlite3_close(s_ddb);
		s_ddb = NULL;
	}
	s_ddb=db;
	return true;
}

void ComboRec_Enter()
{
	if (!s_ddb) return;
	sqlite3_exec(s_ddb, "BEGIN", NULL, NULL, NULL);
}

void ComboRec_Leave(bool failed)
{
	if (!s_ddb) return;
	sqlite3_exec(s_ddb, failed ? "ROLLBACK" : "COMMIT", NULL, NULL, NULL);
}

void ComboRec_Close()
{
	if (s_ddb) {
		sqlite3_close(s_ddb);
		s_ddb = NULL;
	}
}

inline bool equal(double a,double b,int EPSILON)
{
	return a>b-EPSILON && a<b+EPSILON;
}
inline bool equal(int a,int b,int EPSILON)
{
	return a>b-EPSILON && a<b+EPSILON;
}

void ComboRec_AnalysisCallBack(COMBOREC_ITEM *src,void *user)
{
	Minimal::MinimalArrayT<COMBOREC_ITEM> *list=reinterpret_cast<Minimal::MinimalArrayT<COMBOREC_ITEM>*>(user);
	list->Push(*src);
}

void ComboRec_Analysis(const COMBOREC_ITEM &src,Minimal::MinimalStringT<wchar_t> &ret,int hit,int prevlife,int prevrate)
{
	//Search
	COMBOREC_FILTER_DESC filter;
	filter.mask|=COMBOREC_FILTER__DAMAGE;
	filter.damage_l=0;
	filter.damage_h=src.damage+EPSILONDAMAGE;
	if((!src.israteMin) || src.rate>0)
	{
		filter.mask|=COMBOREC_FILTER__RATE;
		filter.rate_l=src.rate-EPSILONRATE;
		filter.rate_h=(src.israteMin?120:(src.rate+EPSILONRATE));
	}
	if((!src.isstunMax) || src.stun>0)
	{
		filter.mask|=COMBOREC_FILTER__STUN;
		filter.stun_l=src.stun-EPSILONRATE;
		filter.stun_h=(src.isstunMax?120:(src.stun+EPSILONRATE));
	}
	filter.pid=src.pid;
	filter.mask|=COMBOREC_FILTER__PID;

	//Calc
	Minimal::MinimalArrayT<COMBOREC_ITEM> list(&Minimal::g_allocator);

	ComboRec_QueryRecord(filter,ComboRec_AnalysisCallBack,&list);

	int i;
	bool flag=false;
	int cnt=list.GetSize();
	int *stat=(int*)Minimal::g_allocator.Allocate(sizeof(int) * hit);
	int curlife,currate;
	int curdam;
	int alldam;
	int outcnt=0;


	ZeroMemory(stat,sizeof(int)*hit);
	for(;;)
	{
		//½øÎ»Ä£Äâ
		for(i=hit;i>=0;i--)
		{
			if(stat[i]>=cnt)
			{
				if(i==0)
				{
					flag=true;
					break;
				}
				else
				{
					++(stat[i-1]);
					stat[i]=0;
				}
			}
		}

		if(flag)
		{
			break;
		}

		curlife=prevlife;
		currate=prevrate;
		alldam=0;
		for(i=0;i<hit;++i)
		{
			curdam=(list[stat[i]].damage)*currate/100;
			curdam=GetLifeReduceDamage(curlife,curdam);
			alldam+=curdam;
			curlife-=curdam;
			currate-=list[stat[i]].rate;
			currate=currate>=10?currate:10;
		}
		if(equal(alldam,src.damage,EPSILONDAMAGE))
		{
			if(outcnt>0)
			{
				ret+=_T(" || ");
			}
			if(hit>1)
			{
				ret+=_T("(");
			}
			for(i=0;i<hit;i++)
			{
				if(i!=0)
				{
					ret+=_T(" ");
				}
				ret+=MinimalA2T(list[stat[i]].txt);
			}
			if(hit>1)
			{
				ret+=_T(")");
			}
		}
		++(stat[hit-1]);
	}
}