#include "pch.hpp"

#define MINIMAL_USE_PROCESSHEAPSTRING
#include "MinimalString.hpp"

int HttpRequestPost(Minimal::MinimalStringT<wchar_t> &host,Minimal::MinimalStringT<wchar_t> &addr,Minimal::MinimalStringT<char> &data, Minimal::MinimalStringT<char> &receive);
int HttpRequestGet(Minimal::MinimalStringT<wchar_t> &host,Minimal::MinimalStringT<wchar_t> &addr, Minimal::MinimalStringT<char> &receive)
