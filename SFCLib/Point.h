//#pragma once
#ifndef POINT_H_
#define POINT_H_

#include <array>            // std::array
#include "typedef.h"

using namespace std;

#define STATIC_ASSERT( e ) static_assert( e, "!(" #e ")" )

template< typename T>//, int nDims=2
class Point
{
private:
	std::array< T, DIM_MAX > elements_; //2nDims at most 40 dimensions
	//vector<T> elements_;
	
	int bitLength=64; //length means the input m or the output n

public:
	int nDims;
	//typedef T ValueType;

	Point():nDims(0)
	{

	}

	int returnSize() const
	{
		//return this->elements_.size();
		return nDims;
	}

	Point (std::size_t dims) 
	{
		nDims = dims;
		for (int i = 0; i < nDims; ++i)
		{
			elements_[i] = 0;
		}
	}

	T& operator[](int const i)
	{
		return elements_[i];
	}

	T const& operator[](int const i) const
	{
		return elements_[i];
	}

	void operator+=(Point const& other)
	{
		nDims = other.returnSize();

		for (int i = 0; i < nDims; ++i)
		{
			elements_[i] += other.elements_[i];
		}
	}
	

	void operator=(Point const& other)
	{
		nDims = other.returnSize();
		
		for (int i = 0; i < nDims; ++i)
		{
			elements_[i] = other.elements_[i];
		}
	}

	void operator-=(Point const& other)
	{
		nDims = other.returnSize();

		for (int i = 0; i < nDims; ++i)
		{
			elements_[i] -= other.elements_[i];
		}
	}

	friend Point operator+(Point const& a, Point const& b)
	{
		Point ret(a);

		ret += b;
		return ret;
	}

	friend Point operator-(Point const&a, Point const& b)
	{
		Point ret(a);

		ret -= b;
		return ret;
	}

	

	//set the input  m or the output n 
	void getBitLength(int bitLength)
	{
		this->bitLength = bitLength;
	}

	int returnBitLength()
	{
		return this->bitLength;
	}

	/*Point() : elements_() {}

	Point(int x, int y)
	{
		STATIC_ASSERT(nDims == 2);
		elements_[0] = x;
		elements_[1] = y;
	}

	Point(int x, int y, int z)
	{
		STATIC_ASSERT(nDims == 3);
		elements_[0] = x;
		elements_[1] = y;
		elements_[2] = z;
	}*/


	Point(T *coordinates, int dims)
	{
		nDims = dims;
		for (int i = 0; i < this->nDims; i++)
		{
			elements_[i] = *(coordinates+i);
		}
	}

	/*Point(const Point & points)
	{
		points->returnSize();
		for (int i = 0; i < nDims; i++)
		{
			elements_[i] = points[i];
		}
	}*/

	T getElements(int i)
	{ 
		return elements_[i];
	}

	T* getAllElements()
	{
		return elements_.data();
	}

	
};

//typedef Point< int > Point2D(2);
//typedef Point< int > Point3D(3);

#endif //POINT_H_