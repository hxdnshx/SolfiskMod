#pragma once

#include "MinimalString.hpp"

void WriteToLog(Minimal::MinimalStringT<wchar_t> str);
void WriteToLog(LPCVOID ptr,int size);
void WriteToLog(LPCWSTR ptr);

void Initial_LogSys();

void Finalize_LogSys();