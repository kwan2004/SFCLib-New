#ifndef SFCCONERSION2_H_
#define SFCCONERSION2_H_

#include "Point.h"

#define adjust_rotation(rotation,nDims,bits)                            \
do {                                                                    \
      /* rotation = (rotation + 1 + ffs(bits)) % nDims; */              \
      bits &= bits & nd1Ones;                                          \
	  	  	  	        while (bits)                                                      \
        bits >>= 1, ++rotation;                                         \
      if ( ++rotation >= nDims )                                        \
        rotation -= nDims;                                              \
} while (0)

#define ones(T,k) ((((T)2) << (k-1)) - 1)

#define rdbit(w,k) (((w) >> (k)) & 1)

#define rotateRight(arg, nRots, nDims)                                  \
((((arg) >> (nRots)) | ((arg) << ((nDims)-(nRots)))) & ones(bitmask_t,nDims))

#define rotateLeft(arg, nRots, nDims)                                   \
((((arg) << (nRots)) | ((arg) >> ((nDims)-(nRots)))) & ones(bitmask_t,nDims))


//template< int nDims, int  mBits>
class SFCConversion2
{
private:


#define DLOGB_BIT_TRANSPOSE
	static 	bitmask_t
		bitTranspose(unsigned n_Dims, unsigned m_Bits, bitmask_t inCoords)
#if defined(DLOGB_BIT_TRANSPOSE)
	{
		unsigned const nDims1 = n_Dims - 1;
		unsigned inB = m_Bits;
		unsigned utB;
		bitmask_t inFieldEnds = 1;
		bitmask_t inMask = ones(bitmask_t, inB);
		bitmask_t coords = 0;

		while ((utB = inB / 2))
		{
			unsigned const shiftAmt = nDims1 * utB;
			bitmask_t const utFieldEnds =
				inFieldEnds | (inFieldEnds << (shiftAmt + utB));
			bitmask_t const utMask =
				(utFieldEnds << utB) - utFieldEnds;
			bitmask_t utCoords = 0;
			unsigned d;
			if (inB & 1)
			{
				bitmask_t const inFieldStarts = inFieldEnds << (inB - 1);
				unsigned oddShift = 2 * shiftAmt;
				for (d = 0; d < n_Dims; ++d)
				{
					bitmask_t in = inCoords & inMask;
					inCoords >>= inB;
					coords |= (in & inFieldStarts) << oddShift++;
					in &= ~inFieldStarts;
					in = (in | (in << shiftAmt)) & utMask;
					utCoords |= in << (d*utB);
				}
			}
			else
			{
				for (d = 0; d < n_Dims; ++d)
				{
					bitmask_t in = inCoords & inMask;
					inCoords >>= inB;
					in = (in | (in << shiftAmt)) & utMask;
					utCoords |= in << (d*utB);
				}
			}
			inCoords = utCoords;
			inB = utB;
			inFieldEnds = utFieldEnds;
			inMask = utMask;
		}
		coords |= inCoords;
		return coords;
	}
#else
	{
		bitmask_t coords = 0;
		unsigned d;
		for (d = 0; d < nDims; ++d)
		{
			unsigned b;
			bitmask_t in = inCoords & ones(bitmask_t, mBits);
			bitmask_t out = 0;
			inCoords >>= mBits;
			for (b = mBits; b--;)
			{
				out <<= nDims;
				out |= rdbit(in, b);
			}
			coords |= out << d;
		}
		return coords;
	}
#endif


	 void hilbert_i2c(bitmask_t index, bitmask_t coord[])
	{
		if (nDims > 1)
		{
			bitmask_t coords;
			halfmask_t const nbOnes = ones(halfmask_t, mBits);
			unsigned d;

			if (mBits > 1)
			{
				unsigned const nDimsBits = nDims*mBits;
				halfmask_t const ndOnes = ones(halfmask_t, nDims);
				halfmask_t const nd1Ones = ndOnes >> 1; 
				unsigned b = nDimsBits;
				unsigned rotation = 0;
				halfmask_t flipBit = 0;
				bitmask_t const nthbits = ones(bitmask_t, nDimsBits) / ndOnes;
				index ^= (index ^ nthbits) >> 1;
				coords = 0;
				do
				{
					halfmask_t bits = halfmask_t((index >> (b -= nDims)) & ndOnes);
					coords <<= nDims;
					coords |= rotateLeft(bits, rotation, nDims) ^ flipBit;
					flipBit = (halfmask_t)1 << rotation;
					adjust_rotation(rotation, nDims, bits);
				} while (b);
				for (b = nDims; b < nDimsBits; b *= 2)
					coords ^= coords >> b;
				coords = bitTranspose(mBits, nDims, coords);
			}
			else
				coords = index ^ (index >> 1);

			for (d = 0; d < nDims; ++d)
			{
				coord[d] = coords & nbOnes;
				coords >>= mBits;
			}
		}
		else
			coord[0] = index;
	}

	bitmask_t hilbert_c2i(bitmask_t const coord[])
	{
		if (nDims > 1)
		{
			unsigned const nDimsBits = nDims*mBits;
			bitmask_t index;
			unsigned d;
			bitmask_t coords = 0;
			for (d = nDims; d--;)
			{
				coords <<= mBits;
				coords |= coord[d];
			}

			if (mBits > 1)
			{
				halfmask_t const ndOnes = ones(halfmask_t, nDims);
				halfmask_t const nd1Ones = ndOnes >> 1; 
				unsigned b = nDimsBits;
				unsigned rotation = 0;
				halfmask_t flipBit = 0;
				bitmask_t const nthbits = ones(bitmask_t, nDimsBits) / ndOnes;
				coords = bitTranspose(nDims, mBits, coords);
				coords ^= coords >> nDims;
				index = 0;
				do
				{
					halfmask_t bits = halfmask_t((coords >> (b -= nDims)) & ndOnes);
					bits = halfmask_t(rotateRight(flipBit ^ bits, rotation, nDims));
					index <<= nDims;
					index |= bits;
					flipBit = (halfmask_t)1 << rotation;
					adjust_rotation(rotation, nDims, bits);
				} while (b);
				index ^= nthbits >> 1;
			} // if (mBits > 1)
			else
				index = coords;

			for (d = 1; d < nDimsBits; d *= 2)
				index ^= index >> d;

			return index;

		}//if (nDims > 1)
		else
			return coord[0];
	}
public:

	int nDims;
	int mBits;
	SFCConversion2(int dims, int bits):nDims(dims), mBits(bits)
	{}

	sfc_bigint HilbertEncode(Point<long> ptCoord)
	{	
		bitmask_t pt[10]; //nDims<<10
		for (int i = 0; i < nDims; i++) pt[i] = ptCoord[i];

		return hilbert_c2i(pt);
	}
	Point<long> HilbertDecode(sfc_bigint idx)
	{
		bitmask_t pt[10]; //nDims<<10
		hilbert_i2c(idx, pt);

		Point<long> newpt(nDims);
		for (int i = 0; i < nDims; i++) newpt[i] = long(pt[i]);
		return newpt;
	}

	sfc_bigint MortonEncode(Point<long> ptCoord)
	{
		bitmask_t pt[10]; //nDims<<10
		for (int i = 0; i < nDims; i++) pt[i] = ptCoord[i];

		//val |= ((x & ((uint64_t)1 << i)) << 3 * i) | ((y & ((uint64_t)1 << i)) << (3 * i + 1)) | ((z & ((uint64_t)1 << i)) << (3* i + 2));
		bitmask_t val = 0;
		for (unsigned int i = 0; i < mBits; i++) 
		{
			for (unsigned int j = 0; j < nDims; j++)
			{
				val |= (pt[j] & ((bitmask_t)1 << i)) << (nDims * i + j);
			}			
		}

		return val;
	}
	
	Point<long> MortonDecode(sfc_bigint idx)
	{
		bitmask_t pt[10]; //nDims<<10
		//hilbert_i2c(idx, pt);
		unsigned int mask = ((unsigned int)1 << nDims) - 1;
		for (int i = 0; i < mBits; i++)
		{
			int val = int((idx >> i*nDims)) & mask;

			for (int j = 0; j < nDims; j++)
			{
				pt[j] |= ((val >> j) & 1) << i;
			}
		}

		Point<long> newpt(nDims);
		for (int i = 0; i < nDims; i++) newpt[i] = long(pt[i]);
		return newpt;
	}
};

#endif