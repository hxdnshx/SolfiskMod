#include "pch.hpp"
#include "ComboRecognize.hpp"
#include "DebugFunc.hpp"
#include "TH135AddrDef.h"

#include <sqlite3.h>

#include <utility>

#include "MinimalMemory.hpp"

#define MINIMAL_USE_PROCESSHEAPSTRING
#include "MinimalPath.hpp"

#define RECORD_TABLE "DamageInfo"

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

