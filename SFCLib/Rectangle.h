#ifndef RECTANGLE_H_
#define RECTANGLE_H_

#include"Point.h"
#include<vector>

using std::vector;

#define STATIC_ASSERT( e ) static_assert( e, "!(" #e ")" )

template< typename T>//, int nDims = 2
class Rect
{
public:
	Point<T> minPoint;//, nDims
	Point<T> maxPoint;//, nDims

	int  nDims;
public:

	Rect(Point<T> minPoint, Point<T> maxPoint) //, nDims, nDims
	{
		//STATIC_ASSERT(minPoint.returnSize() == maxPoint.returnSize());

		this->minPoint = minPoint;
		this->maxPoint = maxPoint;

		this->nDims = minPoint.returnSize();
	}

	Rect() :nDims(0)
	{

	}

	int GetDimensions()
	{
		return this->nDims;
	}

	Point<T> GetMinPoint() //, nDims
	{
		return this->minPoint;
	}

	Point<T> GetMaxPoint() //, nDims
	{
		return this->maxPoint;
	}

	void SetMinPoint(Point<T> minpt) //, nDims
	{
		this->minPoint = minpt;
		this->nDims = minpt.returnSize();
	}

	void SetMaxPoint(Point<T> maxpt)
	{
		this->maxPoint = maxpt;
		this->nDims = maxpt.returnSize();
	}

	void SetPoints(Point<T> minPoint, Point<T> maxPoint)
	{
		STATIC_ASSERT(minPoint.returnSize() == maxPoint.returnSize());

		this->minPoint = minPoint;
		this->maxPoint = maxPoint;

		this->nDims = minPoint.returnSize();
	}

	long GetDimWidth(int idx)
	{
		return this->maxPoint[idx] - this->minPoint[idx];
	}

	//void DFS(vector<T> minVector, vector<T> maxVector,
	//	int i, vector<T > tmp, vector<vector<T >>& result)
	//{
	//	if (i >= minVector.size() || i >= maxVector.size()) {
	//		if (!tmp.empty())
	//			result.push_back(tmp);
	//		return;
	//	}

	//	tmp.push_back(minVector[i]);
	//	DFS(minVector, maxVector, i + 1, tmp, result);
	//	tmp.pop_back();

	//	tmp.push_back(maxVector[i]);
	//	DFS(minVector, maxVector, i + 1, tmp, result);
	//	tmp.pop_back();
	//}

	/*generate all the points of rectangle;
	for example ,from minPoint and the maxPoint,
	the 2d Rectangle will generate 4 points while the 3d Rectangle will
	generate 8 points.
	*/
	//vector<Point<T, nDims>>
	//	GetAllCornerPts(Point<T, nDims> minPoint, Point<T, nDims> maxPoint)
	//{
	//	vector<T>minVector;
	//	vector<T>maxVector;
	//	for (int i = 0; i < nDims; i++)
	//	{
	//		minVector.push_back(minPoint.getElements(i));
	//		maxVector.push_back(maxPoint.getElements(i));
	//	}
	//	vector<vector<T>> result;
	//	vector<T> tmp;
	//	DFS(minVector, maxVector, 0, vector<T>(), result);
	//	vector<Point<T, nDims>> Rec_Vector;
	//	Point<T, nDims> points;
	//	for (int i = 0; i < result.size(); ++i) {
	//		for (int j = 0; j < result[i].size(); ++j) {
	//			points[j] = result[i][j];
	//		}
	//		Rec_Vector.push_back(points);
	//	}
	//	return Rec_Vector;
	//}
};


#endif