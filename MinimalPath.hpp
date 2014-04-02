﻿#pragma once

#include "MinimalString.hpp"

namespace Minimal
{

template <class StringType>
class MinimalPathT
{
public:
	MinimalPathT() :
	  m_str() {}

	MinimalPathT(typename StringType::ConstStringType str) :
	  m_str(str) {}

public:
	MinimalPathT<StringType> & operator =(typename StringType::ConstStringType str)
	{
		m_str = str;
		return *this;
	}

	MinimalPathT<StringType> & operator /=(typename StringType::ConstStringType leaf)
	{
		size_t strLen = MinimalString::MinimalStringLength(leaf);
		m_str.Grow(m_str.GetSize() + strLen + 2);
		::PathAppend(m_str, leaf);
		m_str.Repair();
		return *this;
	}

	operator typename StringType::ConstStringType ()
	{
		return m_str;
	}

private:
	StringType m_str;
};

#ifdef MINIMAL_USE_PROCESSHEAPSTRING
typedef Minimal::MinimalPathT< Minimal::ProcessHeapStringA > ProcessHeapPathA;
typedef Minimal::MinimalPathT< Minimal::ProcessHeapStringW > ProcessHeapPathW;
#ifdef UNICODE
#define ProcessHeapPath ProcessHeapPathW
#else
#define ProcessHeapPath ProcessHeapPathA
#endif

#endif

}