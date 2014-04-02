#include "pch.hpp"
#include "MinimalMemory.hpp"


#ifndef _DEBUG 

extern "C" {

int main();

typedef void (__cdecl *_PVFV)(void);
#pragma data_seg(".CRT$XIA")
static _PVFV __xi_a[] = { NULL };
#pragma data_seg(".CRT$XIZ")
static _PVFV __xi_z[] = { NULL };
#pragma data_seg(".CRT$XCA")
static _PVFV __xc_a[] = { NULL };
#pragma data_seg(".CRT$XCZ")
static _PVFV __xc_z[] = { NULL };
#pragma data_seg()

#pragma comment(linker, "/MERGE:.CRT=.rdata")
#pragma comment(linker, "/MERGE:.rdata=.text")

static _PVFV *__xx_a = NULL;
static size_t __xx_c = 0;
static _PVFV *__xx_z = NULL;

int __cdecl atexit(_PVFV func)
{
	if (!__xx_a) {
		__xx_a = (_PVFV *)Minimal::malloc(sizeof _PVFV);
		__xx_z = __xx_a + 1;
		__xx_c = 1;
	} else {
		__xx_a = (_PVFV *)Minimal::realloc(__xx_a, sizeof _PVFV * (++__xx_c));
		__xx_z = __xx_a + __xx_c;
	}
	*(__xx_z - 1) = func;
	return 0;
}

void EntryPoint()
{
	Minimal::Memory_Initialize();

	for (_PVFV* p = __xi_a; p < __xi_z; ++p) if (*p) (*p)();
	for (_PVFV* p = __xc_a; p < __xc_z; ++p) if (*p) (*p)();

	int result = main();

	for (size_t i = 0; i < __xx_c; ++i)
		__xx_a[i]();

	ExitProcess(result);
}

}
#endif