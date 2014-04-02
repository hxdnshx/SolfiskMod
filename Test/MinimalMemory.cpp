#include "pch.hpp"
#include "MinimalMemory.hpp"

namespace Minimal {

static HANDLE s_processHeap;

void Memory_Initialize()
{
	s_processHeap = GetProcessHeap();
}

void * malloc(size_t N) {
	return ::HeapAlloc(s_processHeap, 0, N);
}

void * realloc(void *memBlock, size_t N)
{
	return ::HeapReAlloc(s_processHeap, 0, memBlock, N);
}

void free(void *p)
{
	::HeapFree(s_processHeap, 0, p);
}

__declspec(naked)
void * __cdecl memset(void *_dst, int _val, size_t _size)
{
	__asm {
		push edi
		mov edx, [esp+16]
		mov eax, [esp+12]
		mov edi, [esp+8]
		mov ecx, edx
		shr ecx, 2
		and edx, 3
		push edi
		rep stosd
		mov ecx, edx
		rep stosb
		pop eax
		pop edi
		ret
	}
}

#if 0
errno_t  __cdecl memcpy_s(__out_bcount_part_opt(_DstSize, _MaxCount) void * _Dst, __in rsize_t _DstSize, __in_bcount_opt(_MaxCount) const void * _Src, __in rsize_t _MaxCount)
{
	if (_Dst == NULL || _Src == NULL) return EINVAL;
	if (_DstSize < _MaxCount) return ERANGE;
	memcpy(_Dst, _Src, _MaxCount);
	return 0;
}

errno_t __cdecl memmove_s(__out_bcount_part_opt(_DstSize,_MaxCount) void * _Dst, __in rsize_t _DstSize, __in_bcount_opt(_MaxCount) const void * _Src, __in rsize_t _MaxCount)
{
	if (_Dst == NULL || _Src == NULL) return EINVAL;
	if (_DstSize < _MaxCount) return ERANGE;
	if (_Src < _Dst) {
		for (rsize_t i = _DstSize; i-- > 0;)
			((char *)_Dst)[i] = ((char *)_Src)[i];
	} else if (_Src > _Dst) {
		memcpy(_Dst, _Src, _MaxCount);
	}
	return 0;
}

errno_t __cdecl wcscpy_s(wchar_t *strDestination, size_t numberOfElements, const wchar_t *strSource)
{
	if (strDestination == NULL || strSource == NULL) return EINVAL;
	rsize_t srcLen = ::lstrlenW(strSource);
	if (srcLen + 1 > numberOfElements) return ERANGE;
	::StrCpyW(strDestination, strSource);
	return 0;
}

int __cdecl _resetstkoflw ( void )
{
	ExitProcess(-1);
}

#endif

}

#ifndef _DEBUG

void * operator new(size_t N) {
	return Minimal::malloc(N);
}

void * operator new[](size_t N) {
	return Minimal::malloc(N);
}

void* operator new(std::size_t, void* p) throw() {
	return p;
}

void operator delete(void *p) {
	Minimal::free(p);
}

void operator delete[](void *p) {
	Minimal::free(p);
}

#endif