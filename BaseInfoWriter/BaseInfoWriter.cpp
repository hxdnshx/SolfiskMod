// BaseInfoWriter.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "..\Dependencies\include\sqlite3.h"
#include <windows.h>
#include <fstream>
#define RECORD_TABLE "DamageInfo"

#pragma comment(lib,"sqlite3")

using namespace std;

static sqlite3 *s_ddb;
static bool s_Enteredview;
static LPCSTR  s_srPath;

void ComboRec_SetPath(LPCSTR path)
{
	s_srPath=path;
}

LPCSTR ComboRec_GetPath()
{
	return s_srPath;
}

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

void ComboRec_Close()
{
	if (s_ddb) {
		sqlite3_close(s_ddb);
		s_ddb = NULL;
	}
}

bool ComboRec_Open(bool create)
{
	sqlite3 *db;
		int rc = sqlite3_open(s_srPath,&db);
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

bool ComboRec_Append(COMBOREC_ITEM *item)
{
	sqlite3_stmt *stmt;
	int rc;

	if(!s_ddb) return false;

	rc = sqlite3_prepare(s_ddb,
		"insert into " RECORD_TABLE "(pid,txt,damage,rate,stun) "
		"values (?,?,?,?,?);",-1,&stmt,NULL);

	if (rc)
	{
		const char* err=sqlite3_errmsg(s_ddb);
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

int _tmain(int argc, _TCHAR* argv[])
{
	char *path="D:\\Program Files\\Alwil Software\\Adobe\\bcdd\\XQL\\tsksrc\\BaseData.db";
	s_ddb=0;
	ComboRec_SetPath(path);
	ComboRec_Open(false);
	ifstream fs;
	fs.open("D:\\Program Files\\Alwil Software\\Adobe\\bcdd\\XQL\\tsksrc\\MikoInfo.txt");
	COMBOREC_ITEM item;
	ComboRec_Enter();
	for(;;)
	{
		fs>>item.pid>>item.txt>>item.damage>>item.rate>>item.stun;
		ComboRec_Append(&item);
		if(fs.eof())
		{
			break;
		}
	}
	ComboRec_Leave(false);
	fs.close();
	ComboRec_Close();
	return 0;
}

