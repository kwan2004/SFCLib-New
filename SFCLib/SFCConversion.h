//#pragma once
#ifndef SFCCONERSION_H_
#define SFCCONERSION_H_

#include <array>            // std::array
#include "Point.h"


//template< int nDims> //,  int  mBits
class SFCConversion
{
private:
	std::vector<unsigned long> g_mask;//nDims
	int nDims; //,  
	int mBits;

	unsigned long calc_P3(int i, Point<long> H);//
	unsigned long calc_P2(unsigned long S);
	unsigned long calc_J(unsigned long P);
	unsigned long calc_T(unsigned long P);
	unsigned long calc_tS_tT_l(unsigned long xJ, unsigned long val);
	unsigned long calc_tS_tT_r(unsigned long xJ, unsigned long val);

public:
	//Point<long, nDims> ptCoord; //n*m
	//Point<long,  mBits> ptBits; //m*n

	SFCConversion(int dims, int bits): g_mask(dims, 0), nDims(dims), mBits(bits)
	{
		for (int i = 0; i < nDims; i++)
		{
			g_mask[i] = ((unsigned long)1) << (nDims - i - 1);
		}
	}

	sfc_bigint MortonEncode(Point<long> ptCoord);// from n*m coords to m*n bitsequence //, nDims
	Point<long> MortonDecode(sfc_bigint idx);// from m*n bitsequence to n*m coords //, nDims
	 
	sfc_bigint HilbertEncode(Point<long> ptCoord); // from n*m coords to m*n bitsequence
	Point<long> HilbertDecode(sfc_bigint idx); // from m*n bitsequence to n*m coords

private:
	sfc_bigint BitSequence2Value(Point<long> ptBits)
	{
		//if (mBits * nDims >= 64) return 0;
		sfc_bigint  result = 0;
		for (int i = 0; i < mBits; i++)
		{
			if (ptBits[i])
			{
				sfc_bigint a = (((sfc_bigint)ptBits[i]) << (mBits - i - 1)*nDims);
				result |= a;
			}
				
		}
		return result;
	}

	Point<long> Value2BitSequence(sfc_bigint value)
	{
		/*Point<long, mBits> ptOutput;
		if (mBits * nDims >= 64) return ptOutput;

		long long mask = ((long long)1 << nDims - 1);
		for (int i = 0; i < mBits; i++)
		{
		ptOutput[mBits - i - 1] = (value >> (i*nDims)) & mask;
		}

		return ptOutput;*/

		Point<long> ptOutput(mBits);
		//if (mBits * nDims >= 64) return ptOutput;

		long mask = (((long)1 << nDims) - 1);
		for (int i = 0; i < mBits; i++)
		{
			ptOutput[mBits - i - 1] = (long)((value >> (i*nDims)) & mask);
		}

		return ptOutput;
	}
};

//template< int nDims, int  mBits>
sfc_bigint SFCConversion::MortonEncode(Point<long> ptCoord)// from n*m to m*n
{
	Point<long> ptBits(mBits);

	for (int i = 0; i < mBits; i++)//m
	{
		ptBits[i] = 0;
		long mask = ((unsigned long)1) << (mBits - i - 1); //move to the ith bit

		for (int j = 0; j < nDims; j++) //get one bit from each nDims
		{
			if (ptCoord[j] & mask) // both 1
				ptBits[i] |= (long)1 << (nDims - j - 1);// push this bit to dim position(xyz...) nDims -----(nDims - j)
		}//
	}//m group

	sfc_bigint idx = BitSequence2Value(ptBits);
	return idx;
}


//template< int nDims, int  mBits>
//Point<long, nDims> SFCConversion<nDims, mBits>::MortonDecode(sfc_bigint idx)
Point<long> SFCConversion::MortonDecode(sfc_bigint idx)
{
	Point<long> ptCoord(nDims); //n*m
	Point<long> ptBits = Value2BitSequence(idx); //m*n, mBits

	for (int i = 0; i < nDims; i++)//m n-bits
	{
		ptCoord[i] = 0;
		long mask = ((unsigned long)1) << (nDims - i - 1);

		for (int j = 0; j < mBits; j++)
		{
			if (ptBits[j] & mask) //both 1 
				ptCoord[i] |= (long)1 << (mBits - j - 1); //get the i-th bit from  j-th bits
		}//
	}//n nDims

	return ptCoord;
}


/*===========================================================*/
/* calc_P ---Get each key part from the input key*/
/*===========================================================*/
//template< int nDims, int  mBits>
//unsigned long SFCConversion<nDims, mBits>::calc_P3(int i, Point<long, mBits> H)
unsigned long SFCConversion::calc_P3(int i, Point<long> H) //, mBits
{
	//int mBits = H.returnSize();
	unsigned long P = H[(mBits * nDims - i) / nDims - 1];

	return P;
}

/*===========================================================*/
/* calc_P2 */
/*===========================================================*/
////template< int nDims, int  mBits>
//unsigned long SFCConversion::calc_P2(unsigned long S)
unsigned long SFCConversion::calc_P2(unsigned long S)
{
	int i;
	unsigned long P;
	P = S & g_mask[0];
	for (i = 1; i < nDims; i++)
		if (S & g_mask[i] ^ (P >> 1) & g_mask[i])
			P |= g_mask[i];
	return P;
}

//template< int nDims, int  mBits>
//unsigned long SFCConversion<nDims, mBits>::calc_J(unsigned long P)
unsigned long SFCConversion::calc_J(unsigned long P)
{
	int i;
	unsigned long J;
	J = nDims;
	for (i = 1; i < nDims; i++)
		if ((P >> i & 1) == (P & 1))
			continue;
		else
			break;
	if (i != nDims)
		J -= i;
	return J;
}

/*===========================================================*/
/* calc_T */
/*===========================================================*/
//template< int nDims, int  mBits>
//unsigned long SFCConversion<nDims, mBits>::calc_T(unsigned long P)
unsigned long SFCConversion::calc_T(unsigned long P)
{
	if (P < 3)
		return 0;
	if (P % 2)
		return (P - 1) ^ (P - 1) / 2;
	return (P - 2) ^ (P - 2) / 2;
}
/*===========================================================*/
/* calc_tS_tT ---right cirular shift xj*/
/*===========================================================*/
//template< int nDims, int  mBits>
//unsigned long SFCConversion<nDims, mBits>::calc_tS_tT_r(unsigned long xJ, unsigned long val)
unsigned long SFCConversion::calc_tS_tT_r(unsigned long xJ, unsigned long val)
{
	unsigned long retval, temp1, temp2;
	retval = val;
	if (xJ % nDims != 0)
	{
		temp1 = val >> (xJ % nDims);
		temp2 = val << (nDims - xJ % nDims);
		retval = temp1 | temp2;
		retval &= ((unsigned long)1 << nDims) - 1;
	}
	return retval;
}

/*===========================================================*/
/* calc_tS_tT---left circular shift xj */
/*===========================================================*/
//template< int nDims, int  mBits>
//unsigned long SFCConversion<nDims, mBits>::calc_tS_tT_l(unsigned long xJ, unsigned long val)
unsigned long SFCConversion::calc_tS_tT_l(unsigned long xJ, unsigned long val)
{
	unsigned long retval, temp1, temp2;
	retval = val;
	if (xJ % nDims != 0)
	{
		temp1 = val << (xJ % nDims);
		temp2 = val >> (nDims - xJ % nDims);
		retval = temp1 | temp2;
		retval &= ((unsigned long)1 << nDims) - 1;
	}
	return retval;
}


//template< int nDims, int  mBits>
//sfc_bigint SFCConversion<nDims, mBits>::HilbertEncode(Point<long, nDims> ptCoord) // from n*m to m*n
sfc_bigint SFCConversion::HilbertEncode(Point<long> ptCoord) // from n*m to m*n
{
	Point<long> ptBits(mBits);

	unsigned long mask = ((unsigned long)1) << ( mBits - 1), element,
		A, W = 0, S, tS, T, tT, J, P = 0, xJ;
	//Point h = { 0 };
	int i = mBits * nDims - nDims, j;

	A = 0;
	for (j = 0; j < nDims; j++)
	{
		if (ptCoord[j] & mask)
			A |= g_mask[j];
	}
	S = tS = A;
	P = calc_P2(S);

	element = (mBits * nDims - i) / nDims - 1;
	ptBits[element] = P;

	J = calc_J(P);
	xJ = J - 1;
	T = calc_T(P);
	tT = T;
	for (i -= nDims, mask >>= 1; i >= 0; i -= nDims, mask >>= 1)
	{
		A = 0;
		for (j = 0; j < nDims; j++)
		{
			if (ptCoord[j] & mask)
				A |= g_mask[j];
		}
		W ^= tT;
		tS = A ^ W;
		S = calc_tS_tT_l(xJ, tS);
		P = calc_P2(S);
	
		element = (mBits * nDims - i) / nDims - 1;
		ptBits[element] = P;
		
		if (i > 0)
		{
			T = calc_T(P);
			tT = calc_tS_tT_r(xJ, T);
			J = calc_J(P);
			xJ += J - 1;
		}
	}

	sfc_bigint idx = BitSequence2Value(ptBits);
	return idx;
}

//template< int nDims, int  mBits>
//Point<long, nDims> SFCConversion<nDims, mBits>::HilbertDecode(sfc_bigint idx)
Point<long> SFCConversion::HilbertDecode(sfc_bigint idx)
{
	Point<long> ptCoord(nDims); //n*m
	Point<long> ptBits = Value2BitSequence(idx); //m*n //, mBits

	unsigned long mask = ((unsigned long)1) << (mBits - 1),
		A, W = 0, S, tS, T, tT, J, P = 0, xJ;
	//Point pt = { 0 };
	int i = mBits * nDims - nDims, j;

	P = calc_P3(i, ptBits); //get key part from the hilbert key

	J = calc_J(P);
	xJ = J - 1;
	A = S = tS = P ^ P / 2;
	T = calc_T(P);
	tT = T;
	/*--- distrib bits to coords ---*/
	for (j = nDims - 1; A > 0; A >>= 1, j--)
	{
		if (A & 1)
			ptCoord[j] |= mask;
	}
	/*pt.hcode[(nDims*mBits -i)/mBits -1] = A;*/

	for (i -= nDims, mask >>= 1; i >= 0; i -= nDims, mask >>= 1)
	{
		P = calc_P3(i, ptBits); //get key part from the hilbert key

		S = P ^ P / 2;
		tS = calc_tS_tT_r(xJ, S);
		W ^= tT;
		A = W ^ tS;
		/*--- distrib bits to coords ---*/
		for (j = nDims - 1; A > 0; A >>= 1, j--)
		{
			if (A & 1)
				ptCoord[j] |= mask;
		}
		/*pt.hcode[(nDims*mBits - i) / mBits - 1] = A;*/

		if (i >= 0)
		{
			T = calc_T(P);
			tT = calc_tS_tT_r(xJ, T);
			J = calc_J(P);
			xJ += J - 1;
		}
	}

	return ptCoord;
}


#endif