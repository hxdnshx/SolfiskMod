#include "pch.hpp"
#include "TH135AddrDef.h"
#include "Solfisk.hpp"
#include "ScoreLine.hpp"
#include "HttpConn.hpp"
#include <cstdio>

#define MINIMAL_USE_PROCESSHEAPSTRING
#include "MinimalPath.hpp"

#define MINIMAL_USE_PROCESSHEAPARRAY
#include "MinimalArray.hpp"

#define POLL_INTERVAL 25

#define Default_WindowClass    _T("th135")
#define Default_WindowCaption  _T("搶曽怱鉟極 Ver1.33")
#define Default_CoreBase       0x004f99e0


TH135GETPARAM s_paramlist[]={
	//{"act/BattleStatus/global/status/p2/guardcrash_alert",0x8,_T("GuardCrush_P2")},
	//{"act/BattleStatus/global/status/p2/spell_alert",0x8,_T("SpellAlert_P2")},
	//{"act/BattleStatus/global/status/p2/spell_gauge_animation",0x8,_T("spell_gauge_animation_P2")},
	//{"act/BattleStatus/global/status/p2/disble_show_time",0x8,_T("disble_show_time_P2")},
	//{"act/BattleStatus/global/status/p1/guardcrash_alert",0x8,_T("GuardCrush_P1")},
	//{"act/BattleStatus/global/status/p1/spell_alert",0x8,_T("SpellAlert_P1")},
	//{"act/BattleStatus/global/status/p1/spell_gauge_animation",0x8,_T("spell_gauge_animation_P1")},
	//{"act/BattleStatus/global/status/p1/disble_show_time",0x8,_T("disble_show_time_P1")},
	{"actor/player1/p/comboDamage",0x2,_T("ComboDamage_P1")},
	{"actor/player2/p/comboDamage",0x2,_T("ComboDamage_P2")},
	{"act/BattleStatus/global/status/p1/hit",0x2,_T("Hit_P1")},
	{"act/BattleStatus/global/status/p2/hit",0x2,_T("Hit_P2")},
	{"act/BattleStatus/global/status/p1/rate",0x2,_T("Rate_P1")},
	{"act/BattleStatus/global/status/p2/rate",0x2,_T("Rate_P2")},
	{"act/BattleStatus/global/status/p1/stun",0x2,_T("Stun_P1")},
	{"act/BattleStatus/global/status/p2/stun",0x2,_T("Stun_P2")},
	{"act/BattleStatus/global/status/p1/hp_b",0x2,_T("HP_P1")},
	{"act/BattleStatus/global/status/p2/hp_b",0x2,_T("HP_P2")}
};
	const int s_paramcnt=10;

static TH135CHARNAME s_charNames[] = {
	{ _T("博麗霊夢"), _T("霊夢") },
	{ _T("霧雨魔理沙"), _T("魔理沙") },
	{ _T("雲居一輪"), _T("一輪") },
	{ _T("聖白蓮"), _T("白蓮") },
	{ _T("物部布都"), _T("布都") },
	{ _T("豊聡耳神子"), _T("神子") },
	{ _T("河城にとり"), _T("にとり") },
	{ _T("古明地こいし"), _T("こいし") },
	{ _T("二ッ岩マミゾウ"), _T("マミゾウ") },
	{ _T("秦こころ"), _T("こころ") }
};




static TCHAR s_WindowClass[256];
static TCHAR s_WindowCaption[256];

static DWORD s_CoreBase;

static HANDLE s_ThProc;
static HWND   s_ThWnd;
static MODULEINFO s_th135ModuleInfo;

static HWND s_callbackWnd;
static int  s_callbackMsg;

static HANDLE s_userThread;
static HANDLE s_thread;
static DWORD s_threadId;

int s_paramOld[TH135PARAM_MAX+s_paramcnt];
static Minimal::ProcessHeapStringA s_paramStr;

static TH135STATE s_TH135State;

static void TH135LoadProfile()
{
#define LoadAddress(N) { TCHAR addrStr[16]; \
	s_##N = profile.ReadString(_T("Address"), _T(#N), _T(""), addrStr, sizeof addrStr); \
	if (!::StrToIntEx(addrStr, STIF_SUPPORT_HEX, reinterpret_cast<int *>(&s_##N))) s_##N = Default_##N;  }
#define LoadChar(N) profile.ReadString(_T("TH135"), _T(#N), Default_##N, s_##N, sizeof s_##N)

	Minimal::ProcessHeapPath profPath = g_appPath;
	CProfileIO profile(profPath /= _T("TH135Addr.ini"));
	LoadChar(WindowClass);
	LoadChar(WindowCaption);

	LoadAddress(CoreBase);
}

// strtok 偺傛偆側 iterator class
class StringSplitter {
private:
	Minimal::ProcessHeapStringA buff;
	LPCSTR begin;
	LPCSTR end;
	char sep;

public:
	StringSplitter(LPCSTR src, char separator):
		begin(src), end(src), sep(separator) {}

public:
	LPCSTR Next() {
		if (begin == NULL) return NULL;
		end = ::StrChrA(begin, sep);
		if (end == NULL) {
			buff = begin;
			begin = end;
		} else {
			buff = Minimal::ProcessHeapStringA(begin, end).GetRaw();
			begin = end + 1;
		}
		return buff.GetRaw();
	}
};



static bool EnumRTChild(LPCSTR path,int level)
{
	DWORD_PTR items;
	DWORD_PTR itemVal;
	DWORD_PTR titemVal;
	DWORD_PTR itemVal2;
	DWORD itemType;
	DWORD itemNum;
	DWORD itemLen;
	DWORD itemIndex;
	DWORD readSize;
	DWORD titemType;

	BOOL ret;
	DWORD j;
	int curlevel=-1;

	// CoreBase 偐傜 table 傪撉傒崬傓
	ret = ::ReadProcessMemory(s_ThProc, (LPVOID)((DWORD_PTR)s_th135ModuleInfo.lpBaseOfDll + s_CoreBase), &itemVal, sizeof itemVal, &readSize);
	if (!ret) return false;
	ret = ::ReadProcessMemory(s_ThProc, (LPVOID)((DWORD_PTR)itemVal + 0x34), &itemVal, sizeof itemVal, &readSize);
	if (!ret) return false;
	//此处需要Corebase正确


	itemType = 0x20;

	StringSplitter tokenizer(path, '/');
	LPCSTR pathToken;
	while (pathToken = tokenizer.Next()) {
		++curlevel;
		switch(itemType & 0xFFFFF) {
		case 0x20:	// TABLE
			// Table 偺忣曬傪撉傒崬傓
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x20), &items, sizeof items, &readSize);
			if (!ret) return false;
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x24), &itemNum, sizeof itemNum, &readSize);
			if (!ret) return false;
			// Table items 傪扵嶕
			for (j = 0; j < itemNum; ++j) {
				// Table item key 傪撉傒崬傓
				ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + j * 0x14 + 0x08), &itemType, sizeof itemType, &readSize);
				if (!ret) return false;
				ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + j * 0x14 + 0x0c), &itemVal, sizeof itemVal, &readSize);
				if (!ret) return false;
				//  Table item key 偑暥帤楍偺偲偒偩偗庴偗晅偗傞
				if ((itemType & 0xFFFFF) == 0x10) {	
					//  Table item key 傪暥帤楍偲偟偰撉傒崬傓
					ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x14), &itemLen, sizeof itemLen, &readSize);
					if (!ret) return false;
					Minimal::ProcessHeapArrayT<char> key(itemLen + 1);
					ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x1C), key.GetRaw(), itemLen, &readSize);
					if (!ret) return false;
					key[itemLen] = 0;
					// Table item key 偺抣偑 path 偺僩乕僋儞偲堦抳偟偨傜 table item value 傪撉傒崬傓
					ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + j * 0x14 + 0x00), &titemType, sizeof titemType, &readSize);
					if (!ret) return false;
					ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + j * 0x14 + 0x04), &titemVal, sizeof titemVal, &readSize);
					if (!ret) return false;
					if(((titemType & 0xFFFFF)==0x20 || (titemType & 0xFFFFF)==0x8000) && level==curlevel)//Table 则继续向下遍历
					{
						Minimal::ProcessHeapArrayT<char> newpath(strlen(path)+itemLen+2);
						strcpy_s(newpath.GetRaw(),strlen(path)+itemLen+2,path);
						if(level!=0)
						{
							
						}
						strcat_s(newpath.GetRaw(),strlen(path)+itemLen+2,key.GetRaw());
						strcat_s(newpath.GetRaw(),strlen(path)+itemLen+2,"/");
						EnumRTChild(newpath.GetRaw(),level+1);
					}
					if(level==curlevel)
					{
						int bsize=strlen(path)+itemLen+30;
						Minimal::ProcessHeapArrayT<char> newpath(bsize);
						sprintf_s(newpath.GetRaw(),bsize,"%s %s 0x%x %x %x \n",path,key.GetRaw(),(DWORD_PTR)(items + j * 0x14 + 0x04),(titemType & 0xFFFFF),titemVal);
						OutputDebugStringA(newpath.GetRaw());
					}
					if (::lstrcmpA(key.GetRaw(), pathToken) == 0) {
						ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + j * 0x14 + 0x00), &itemType, sizeof itemType, &readSize);
						if (!ret) return false;
						ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + j * 0x14 + 0x04), &itemVal, sizeof itemVal, &readSize);
						if (!ret) return false;
						
						break;
					}
				}
			}
			if (j == itemNum) return false;
			break;
		case 0x40:	// ARRAY
			// Array item 偺愭摢傾僪儗僗偲 item 悢傪撉傒崬傓
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x18), &items, sizeof items, &readSize);
			if (!ret) return false;
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x1C), &itemNum, sizeof itemNum, &readSize);
			if (!ret) return false;
			// Path 偺僩乕僋儞傪 Array item 偺 index 偲尒側偟偰斖埻僠僃僢僋
			itemIndex = StrToIntA(pathToken);
			if(itemNum <= itemIndex) return false;
			// Array item 傪撉傒崬傓
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + itemIndex * 0x08 + 0x00), &itemType, sizeof itemType, &readSize);
			if (!ret) return false;
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + itemIndex * 0x08 + 0x04), &itemVal, sizeof itemVal, &readSize);
			if (!ret) return false;
			break;
		case 0x8000:	// INSTANCE
			// Instance members 偺忣曬傪撉傒崬傓
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x1C), &items, sizeof items, &readSize);
			if (!ret) return false;
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + 0x18), &items, sizeof items, &readSize);
			if (!ret) return false;
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + 0x24), &itemNum, sizeof itemNum, &readSize);
			if (!ret) return false;
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + 0x20), &items, sizeof items, &readSize);
			if (!ret) return false;
			// Instance members 傪扵嶕
			for (j = 0; j < itemNum; ++j) {
				// Instance member metadata value 傪撉傒崬傓
				//int tmp1,tmp2;
				ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + j * 0x14 + 0x00), &itemType, sizeof itemType, &readSize);
				if (!ret) return false;
				ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + j * 0x14 + 0x04), &itemVal2, sizeof itemVal, &readSize);
				if (!ret) return false;
				// Instance member metadata value 偑桳岠側偲偒偩偗庴偗晅偗傞
				if ((itemType & 0xFFFFF) == 0x02) {
					// Instance member metadata value 偐傜 instance member type 偲 instance member index 傪拪弌
					DWORD memberType  = itemVal2 & 0xFF000000;
					DWORD memberIndex = itemVal2 & 0x00FFFFFF;
					// Instance member type 偑桳岠側偲偒偩偗庴偗晅偗傞
					if (memberType == 0x02000000) {
						// Instance member metadata key 傪撉傒崬傓
						ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + j * 0x14 + 0x08), &itemType, sizeof itemType, &readSize);
						if (!ret) return false;
						ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + j * 0x14 + 0x0c), &itemVal2, sizeof itemVal, &readSize);
						if (!ret) return false;
						// Instance member metadata key 偑暥帤楍偺偲偒偩偗庴偗晅偗傞
						if ((itemType & 0xFFFFF) == 0x10) {
							//  Instance member metadata key 傪暥帤楍偲偟偰撉傒崬傓
							ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal2 + 0x14), &itemLen, sizeof itemLen, &readSize);
							if (!ret) return false;
							Minimal::ProcessHeapArrayT<char> key(itemLen + 1);
							ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal2 + 0x1C), key.GetRaw(), itemLen, &readSize);
							if (!ret) return false;
							key[itemLen] = 0;
							ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x2C + memberIndex * 0x08 + 0x00), &titemType, sizeof titemType, &readSize);
							ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x2C + memberIndex * 0x08 + 0x04), &titemVal, sizeof titemVal, &readSize);
							if(((titemType & 0xFFFFF)==0x20 || (titemType & 0xFFFFF)==0x8000) && level==curlevel)//Table 则继续向下遍历
							{
								Minimal::ProcessHeapArrayT<char> newpath(strlen(path)+itemLen+2);
								strcpy_s(newpath.GetRaw(),strlen(path)+itemLen+2,path);
								if(level!=0)
								{
							
								}
								strcat_s(newpath.GetRaw(),strlen(path)+itemLen+2,key.GetRaw());
								strcat_s(newpath.GetRaw(),strlen(path)+itemLen+2,"/");
								EnumRTChild(newpath.GetRaw(),level+1);
							}
							if(level==curlevel){
								int bsize=strlen(path)+itemLen+30;
								Minimal::ProcessHeapArrayT<char> newpath(bsize);
								sprintf_s(newpath.GetRaw(),bsize,"%s %s 0x%x %x  %x\n",path,key.GetRaw(),(DWORD_PTR)(itemVal + 0x2C + memberIndex * 0x08 + 0x04),(titemType & 0xFFFFF),titemVal);
								OutputDebugStringA(newpath.GetRaw());
							}
							// Instance member metadata key 偺抣偑 path 偺僩乕僋儞偲堦抳偟偨傜 instance member value 傪撉傒崬傓
							if (::lstrcmpA(key.GetRaw(), pathToken) == 0) {
								ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x2C + memberIndex * 0x08 + 0x00), &itemType, sizeof itemType, &readSize);
								if (!ret) return false;
								ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x2C + memberIndex * 0x08 + 0x04), &itemVal, sizeof itemVal, &readSize);
								if (!ret) return false;
								break;
							}
						}
					}
				}
			}
			if (j == itemNum) return false;
			break;
		}
	}

	return true;
}

// CoreBase class instance 偵偁傞 SQVM instance 偺 Root Table 偐傜 巜掕偝傟偨僷僗偺 item value 傪 fetch
static bool FindRTChild(LPCSTR path, DWORD &retType, DWORD &retVal)
{
	DWORD_PTR items;
	DWORD_PTR itemVal;
	DWORD_PTR itemVal2;
	DWORD itemType;
	DWORD itemNum;
	DWORD itemLen;
	DWORD itemIndex;
	DWORD readSize;
	DWORD titemType;
	BOOL ret;
	DWORD j;

	// CoreBase 偐傜 table 傪撉傒崬傓
	ret = ::ReadProcessMemory(s_ThProc, (LPVOID)((DWORD_PTR)s_th135ModuleInfo.lpBaseOfDll + s_CoreBase), &itemVal, sizeof itemVal, &readSize);
	if (!ret) return false;
	ret = ::ReadProcessMemory(s_ThProc, (LPVOID)((DWORD_PTR)itemVal + 0x34), &itemVal, sizeof itemVal, &readSize);
	if (!ret) return false;
	//此处需要Corebase正确


	itemType = 0x20;

	StringSplitter tokenizer(path, '/');
	LPCSTR pathToken;
	while (pathToken = tokenizer.Next()) {
		switch(itemType & 0xFFFFF) {
		case 0x20:	// TABLE
			// Table 偺忣曬傪撉傒崬傓
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x20), &items, sizeof items, &readSize);
			if (!ret) return false;
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x24), &itemNum, sizeof itemNum, &readSize);
			if (!ret) return false;
			// Table items 傪扵嶕
			for (j = 0; j < itemNum; ++j) {
				// Table item key 傪撉傒崬傓
				ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + j * 0x14 + 0x08), &itemType, sizeof itemType, &readSize);
				if (!ret) return false;
				ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + j * 0x14 + 0x0c), &itemVal, sizeof itemVal, &readSize);
				if (!ret) return false;
				//  Table item key 偑暥帤楍偺偲偒偩偗庴偗晅偗傞
				if ((itemType & 0xFFFFF) == 0x10) {	
					//  Table item key 傪暥帤楍偲偟偰撉傒崬傓
					ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x14), &itemLen, sizeof itemLen, &readSize);
					if (!ret) return false;
					Minimal::ProcessHeapArrayT<char> key(itemLen + 1);
					ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x1C), key.GetRaw(), itemLen, &readSize);
					if (!ret) return false;
					key[itemLen] = 0;
					// Table item key 偺抣偑 path 偺僩乕僋儞偲堦抳偟偨傜 table item value 傪撉傒崬傓
					ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + j * 0x14 + 0x00), &titemType, sizeof titemType, &readSize);

					if (::lstrcmpA(key.GetRaw(), pathToken) == 0) {
						ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + j * 0x14 + 0x00), &itemType, sizeof itemType, &readSize);
						if (!ret) return false;
						ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + j * 0x14 + 0x04), &itemVal, sizeof itemVal, &readSize);
						if (!ret) return false;
						break;
					}
				}
			}
			if (j == itemNum) return false;
			break;
		case 0x40:	// ARRAY
			// Array item 偺愭摢傾僪儗僗偲 item 悢傪撉傒崬傓
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x18), &items, sizeof items, &readSize);
			if (!ret) return false;
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x1C), &itemNum, sizeof itemNum, &readSize);
			if (!ret) return false;
			// Path 偺僩乕僋儞傪 Array item 偺 index 偲尒側偟偰斖埻僠僃僢僋
			itemIndex = StrToIntA(pathToken);
			if(itemNum <= itemIndex) return false;
			// Array item 傪撉傒崬傓
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + itemIndex * 0x08 + 0x00), &itemType, sizeof itemType, &readSize);
			if (!ret) return false;
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + itemIndex * 0x08 + 0x04), &itemVal, sizeof itemVal, &readSize);
			if (!ret) return false;
			break;
		case 0x8000:	// INSTANCE
			// Instance members 偺忣曬傪撉傒崬傓
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x1C), &items, sizeof items, &readSize);
			if (!ret) return false;
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + 0x18), &items, sizeof items, &readSize);
			if (!ret) return false;
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + 0x24), &itemNum, sizeof itemNum, &readSize);
			if (!ret) return false;
			ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + 0x20), &items, sizeof items, &readSize);
			if (!ret) return false;
			// Instance members 傪扵嶕
			for (j = 0; j < itemNum; ++j) {
				// Instance member metadata value 傪撉傒崬傓
				ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + j * 0x14 + 0x00), &itemType, sizeof itemType, &readSize);
				if (!ret) return false;
				ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + j * 0x14 + 0x04), &itemVal2, sizeof itemVal, &readSize);
				if (!ret) return false;
				// Instance member metadata value 偑桳岠側偲偒偩偗庴偗晅偗傞
				if ((itemType & 0xFFFFF) == 0x02) {
					// Instance member metadata value 偐傜 instance member type 偲 instance member index 傪拪弌
					DWORD memberType  = itemVal2 & 0xFF000000;
					DWORD memberIndex = itemVal2 & 0x00FFFFFF;
					// Instance member type 偑桳岠側偲偒偩偗庴偗晅偗傞
					if (memberType == 0x02000000) {
						// Instance member metadata key 傪撉傒崬傓
						ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + j * 0x14 + 0x08), &itemType, sizeof itemType, &readSize);
						if (!ret) return false;
						ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(items + j * 0x14 + 0x0c), &itemVal2, sizeof itemVal, &readSize);
						if (!ret) return false;
						// Instance member metadata key 偑暥帤楍偺偲偒偩偗庴偗晅偗傞
						if ((itemType & 0xFFFFF) == 0x10) {
							//  Instance member metadata key 傪暥帤楍偲偟偰撉傒崬傓
							ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal2 + 0x14), &itemLen, sizeof itemLen, &readSize);
							if (!ret) return false;
							Minimal::ProcessHeapArrayT<char> key(itemLen + 1);
							ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal2 + 0x1C), key.GetRaw(), itemLen, &readSize);
							if (!ret) return false;
							key[itemLen] = 0;
							// Instance member metadata key 偺抣偑 path 偺僩乕僋儞偲堦抳偟偨傜 instance member value 傪撉傒崬傓
							if (::lstrcmpA(key.GetRaw(), pathToken) == 0) {
								ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x2C + memberIndex * 0x08 + 0x00), &itemType, sizeof itemType, &readSize);
								if (!ret) return false;
								ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(itemVal + 0x2C + memberIndex * 0x08 + 0x04), &itemVal, sizeof itemVal, &readSize);
								if (!ret) return false;
								break;
							}
						}
					}
				}
			}
			if (j == itemNum) return false;
			break;
		}
	}

	retType = itemType;
	retVal = itemVal;
	return true;
}

// Squirrel item value 偐傜 string 傪 fetch
static bool RTChildToString(DWORD_PTR childVal, Minimal::ProcessHeapStringA &out)
{
	DWORD strLen;
	DWORD readSize;
	BOOL ret;

	ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(childVal + 0x14), &strLen, sizeof strLen, &readSize);
	if (!ret) return false;
	Minimal::ProcessHeapArrayT<char> buff(strLen + 1);
	ret = ::ReadProcessMemory(s_ThProc, (LPVOID)(childVal + 0x1C), buff.GetRaw(), strLen, &readSize);
	if (!ret) return false;
	buff[strLen] = 0;
	out = buff.GetRaw();
	return true;
}

static void APIENTRY InterruptAPCCallback(ULONG_PTR)
{
}

static void TH135Callback(short Msg, short param1, int param2)
{
	if (s_callbackWnd) {
		::PostMessage(s_callbackWnd, s_callbackMsg, MAKELONG(Msg, param1), param2);
	}
}

static TH135STATE TH135StateFindWindow()
{
	s_ThWnd = ::FindWindow(s_WindowClass, s_WindowCaption);
	if (s_ThWnd != NULL) {
		DWORD dwThId;
		::GetWindowThreadProcessId(s_ThWnd, &dwThId);
		s_ThProc = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwThId);

		DWORD enumBufNeeded;
		::EnumProcessModules(s_ThProc, NULL, 0, &enumBufNeeded);
		Minimal::ProcessHeapArrayT<HMODULE> modules(enumBufNeeded);
		::EnumProcessModules(s_ThProc, modules.GetRaw(), enumBufNeeded, &enumBufNeeded);

		::GetModuleInformation(s_ThProc, modules[0], &s_th135ModuleInfo, sizeof s_th135ModuleInfo);

		TH135Callback(TH135MSG_STATECHANGE, TH135STATE_WAITFORNETBATTLE, 0);
		::ZeroMemory(s_paramOld, sizeof s_paramOld);
		return TH135STATE_WAITFORNETBATTLE;
	} else {
		return TH135STATE_NOTFOUND;
	}
}

static TH135STATE TH135StateWaitForNetBattle()
{
	DWORD ret = ::WaitForSingleObject(s_ThProc, 0);
	if (ret != WAIT_TIMEOUT) {
		::CloseHandle(s_ThProc);
		s_ThProc = NULL;
		TH135Callback(TH135MSG_STATECHANGE, TH135STATE_NOTFOUND, 0);
		return TH135STATE_NOTFOUND;
	}

	

	DWORD childType, childVal;

	//::EnumRTChild("actor/player1/tempAtkData/",3);
	//COMBOINFO_ITEM it;
	//ComboInfo_Append(&it);




	if ((::FindRTChild("game", childType, childVal) && (childType & 0x20) != 0) &&
		(::FindRTChild("network_inst", childType, childVal) && (childType & 0x8000) != 0) &&
		(::FindRTChild("network_is_watch", childType, childVal) && (childType & 0x8) != 0 && childVal == 0)) {
		return TH135STATE_NETBATTLE;
	} else {
#ifndef _DEBUG
		return TH135STATE_WAITFORNETBATTLE;
#else
		return TH135STATE_NETBATTLE;
#endif
	}
}

static TH135STATE TH135StateNetBattle()
{
	
	DWORD ret = ::WaitForSingleObject(s_ThProc, 0);
	if (ret != WAIT_TIMEOUT) {
		::CloseHandle(s_ThProc);
		s_ThProc = NULL;
		TH135Callback(TH135MSG_STATECHANGE, TH135STATE_NOTFOUND, 0);
		return TH135STATE_NOTFOUND;
	}
	

	DWORD childType, childVal;
#ifdef _DEBUG
	if ((::FindRTChild("game", childType, childVal) && (childType & 0x20) != 0) //&&
		//(::FindRTChild("network_inst", childType, childVal) && (childType & 0x8000) != 0) &&
		//(::FindRTChild("network_is_watch", childType, childVal) && (childType & 0x8) != 0 && childVal == 0)
		) {
#else
	if ((::FindRTChild("game", childType, childVal) && (childType & 0x20) != 0) &&
		(::FindRTChild("network_inst", childType, childVal) && (childType & 0x8000) != 0) &&
		(::FindRTChild("network_is_watch", childType, childVal) && (childType & 0x8) != 0 && childVal == 0)
		) {
#endif
		int param;
		bool flag=false;
		for (int i = 0; i < TH135PARAM_MAX+s_paramcnt; ++i) {
			if ((param = TH135AddrGetParam(i)) != -1) {
				if (param != s_paramOld[i]){
					if(IsInParamList(i)){
						flag=true;
					}
					else
					{
						TH135Callback(TH135MSG_PARAMCHANGE, i, param);
					}
				}
				s_paramOld[i] = param;
			} else s_paramOld[i] = 0;
		}
		if(flag)
		{
			TH135Callback(TH135MSG_PARAMCHANGE, TH135PARAM_LISTCHANGED, -1);
		}
		return TH135STATE_NETBATTLE;
	} else {
		return TH135STATE_WAITFORNETBATTLE;
	}
}

static DWORD WINAPI TH135AddrWorkThread(LPVOID)
{
	MSG msg;

	s_TH135State = TH135STATE_NOTFOUND;
	while(!::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) || msg.message != WM_QUIT) {
		switch(s_TH135State) {
		case TH135STATE_NOTFOUND:            s_TH135State = TH135StateFindWindow(); break;
		case TH135STATE_WAITFORNETBATTLE:    s_TH135State = TH135StateWaitForNetBattle(); break;
		case TH135STATE_NETBATTLE:           s_TH135State = TH135StateNetBattle(); break;
		};
		::SleepEx(POLL_INTERVAL, TRUE);
	}
	::ExitThread(0);
	return 0;
}

int TH135AddrInit(HWND callbackWnd, int callbackMsg)
{
	TH135LoadProfile();
	s_callbackWnd = callbackWnd;
	s_callbackMsg = callbackMsg;
	s_thread = ::CreateThread(NULL, 0, TH135AddrWorkThread, NULL, 0, &s_threadId);
	s_userThread = ::GetCurrentThread();
	return s_thread != NULL;
}

int TH135AddrFinish()
{
	if (s_thread) {
		::PostThreadMessage(s_threadId, WM_QUIT, 0, 0);
		::QueueUserAPC(InterruptAPCCallback, s_thread, 0);
		::WaitForSingleObject(s_thread, INFINITE);
		::CloseHandle(s_thread);
	}
	return 0;
}

TH135STATE TH135AddrGetState()
{
	return s_TH135State;
}

DWORD_PTR TH135AddrGetParam(int param)
{
	DWORD childType, childVal, readSize;

	switch(param) {
	case TH135PARAM_BATTLESTATE:
		if (::FindRTChild("game/battleState", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_ISNETCLIENT:
		if (::FindRTChild("network_is_client", childType, childVal) && (childType & 0xFFFFF) == 0x08) { 
			return childVal;
		}
		break;
	case TH135PARAM_P1CHAR:
		if (::FindRTChild("load_data/player/0", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_P2CHAR:
		if (::FindRTChild("load_data/player/1", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_P1WIN:
		if (::FindRTChild("act/BattleStatus/global/status/p1/win", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_P2WIN:
		if (::FindRTChild("act/BattleStatus/global/status/p2/win", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_P1HPB:
		if(::FindRTChild("act/BattleStatus/global/status/p1/hp_b", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_P1HPF:
		if(::FindRTChild("act/BattleStatus/global/status/p1/hp_f", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_P2HPB:
		if(::FindRTChild("act/BattleStatus/global/status/p2/hp_b", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_P2HPF:
		if(::FindRTChild("act/BattleStatus/global/status/p2/hp_f", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_P1GOOD:
		if(::FindRTChild("act/BattleStatus/global/status/p1/good", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_P2GOOD:
		if(::FindRTChild("act/BattleStatus/global/status/p2/good", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_P1HATE:
		if(::FindRTChild("act/BattleStatus/global/status/p1/hate", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_P2HATE:
		if(::FindRTChild("act/BattleStatus/global/status/p2/hate", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_P1HIT:
		if(::FindRTChild("act/BattleStatus/global/status/p1/hit", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_P2HIT:
		if(::FindRTChild("act/BattleStatus/global/status/p2/hit", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_P1RATE:
		if(::FindRTChild("act/BattleStatus/global/status/p1/rate", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_P2RATE:
		if(::FindRTChild("act/BattleStatus/global/status/p2/rate", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_P2DAM:
		if(::FindRTChild("act/BattleStatus/global/status/p2/damage", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_P1DAM:
		if(::FindRTChild("act/BattleStatus/global/status/p1/damage", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_P1STUN:
		if(::FindRTChild("act/BattleStatus/global/status/p1/stun", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_P2STUN:
		if(::FindRTChild("act/BattleStatus/global/status/p2/stun", childType, childVal) && (childType & 0xFFFFF) == 0x02) { 
			return childVal;
		}
		break;
	case TH135PARAM_TOADDR:
		if ((::FindRTChild("mb_client", childType, childVal) && (childType & 0xFFFFF) == 0x8000) || (::FindRTChild("mb_server", childType, childVal) && (childType & 0xFFFFF) == 0x8000)) {
			if (::ReadProcessMemory(s_ThProc, (LPCVOID)(childVal + 0x20), &childVal, sizeof childVal, &readSize) && readSize == 4) {
				if (::ReadProcessMemory(s_ThProc, (LPCVOID)(childVal + 0x10), &childVal, sizeof childVal, &readSize) && readSize == 4) {
					if (::ReadProcessMemory(s_ThProc, (LPCVOID)(childVal + 0x23C), &childVal, sizeof childVal, &readSize) && readSize == 4) {
						return childVal;
					}
				}
			}
		}
		break;
	case TH135PARAM_P1NAME:
		if (::FindRTChild("load_data/profile/0/name", childType, childVal) && (childType & 0xFFFFF) == 0x10 && RTChildToString(childVal, s_paramStr)) { 
			return (DWORD_PTR)s_paramStr.GetRaw();
		}
		break;
	case TH135PARAM_P2NAME:
		if (::FindRTChild("load_data/profile/1/name", childType, childVal) && (childType & 0xFFFFF) == 0x10 && RTChildToString(childVal, s_paramStr)) {
			return (DWORD_PTR)s_paramStr.GetRaw();
		}
		break;
	default:
		if(IsInParamList(param))
		{
			int rparam=P2PL(param);
			if(::FindRTChild(s_paramlist[rparam].target, childType, childVal) && (childType & 0xFFFFF) == s_paramlist[rparam].vtype)
			{
				if(s_paramlist[rparam].vtype==0x10){
					RTChildToString(childVal, s_paramStr);
					return (DWORD_PTR)s_paramStr.GetRaw();
				}
				return childVal;
			}
		}
		break;
	}
	return -1;
}

const TH135CHARNAME * const TH135AddrGetCharName(int index)
{
	if (TH135AddrGetCharCount() <= index) return NULL;
	return &s_charNames[index];
}

int TH135AddrGetCharCount()
{
	return _countof(s_charNames);
}
