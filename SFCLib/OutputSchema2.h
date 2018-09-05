//#pragma once
#ifndef OUTPUTSCHEMA2_H_
#define OUTPUTSCHEMA2_H_

#include "Point.h"
#include <iostream>
#include <bitset>
using namespace std;


//template<int nDims, int  mBits>
class OutputSchema2
{
public:
	int nDims;
	int mBits;

	OutputSchema2(int dims, int bits) :nDims(dims), mBits(bits)
	{	}

	
	string Value2String(sfc_bigint val, StringType str_type)
	{
		int base = 0; 
		const char* char_table = NULL;
		
		if (str_type == Base32) 
		{ 
			base = 5; 
			char_table = BASE32_TABLE_E2; 
		}
		if (str_type == Base64) 
		{ 
			base = 6; 
			char_table = BASE64_TABLE_E2; 
		}

		int ntotalbits = mBits * nDims;

		//allocate the string space -----one more space for residual
		int nstrlen = (ntotalbits % base) ? (ntotalbits / base + 1) : (ntotalbits / base);
		char* szstr = new char[nstrlen + 1]; //last char for zero
		memset(szstr, 0, nstrlen + 1);
	
		unsigned int mask = ((unsigned int)1 << base) - 1;
		int idx = 0;
		for (int i = 0; i < nstrlen; i++)
		{
			idx = int(( val >> i*base)) & mask;
			szstr[nstrlen - i - 1] = char_table[idx];
		}

		return string(szstr);
	}

	sfc_bigint String2Value(string szCode, StringType str_type)
	{
		sfc_bigint idx = 0;

		////////////////////
		int base = 0;
		const unsigned char* char_table = NULL;

		if (str_type == Base32)
		{
			base = 5;
			//char_table = BASE32_TABLE_E2;
		}
		if (str_type == Base64)
		{
			base = 6;
			char_table = BASE64_TABLE_D2;
		}

		///////////////
		//unsigned int mask = ((unsigned int)1 << base) - 1;
		int nstrlen = szCode.length();
		const char* p = szCode.c_str();

		for (int i = 0; i < nstrlen; i++)
		{
			sfc_bigint c = (sfc_bigint)(char_table[*(p + i)]);
			if (c != 0)
				idx |= c << ((nstrlen - 1 - i)*base);
		}
		

		return idx;
	}
};

#endif