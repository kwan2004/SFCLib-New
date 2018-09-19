#ifndef QUERYBYSFC2_H_
#define QUERYBYSFC2_H_

//#include "stdafx.h"

#include "Point.h"
#include "Rectangle.h"

#include "OutputSchema2.h"
//#include "SFCConversion2.h"
#include "SFCConversion.h"
#include "QueryBySFC.h"

#include <iostream>
#include <vector>
#include <list>
#include <tuple>
#include <queue>
#include <algorithm>
#include <map>

#include <time.h>

#include "tbb/parallel_sort.h"
#include "tbb/concurrent_hash_map.h"
#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"

using namespace tbb;
using namespace std;

//#define RETURN_RANGES 40

//typedef enum
//{
//	Morton,
//	Hilbert,
//} SFCType;
//
//template<typename T>//, int nDims = 2
//class TreeNode
//{
//public:
//	int level;  //which level: i-th level
//	Point<T> minPoint;//(nDims)
//	Point<T> maxPoint;//(nDims)
//
//	int nDims;//= nDims
//
//	void operator=(TreeNode const& other)
//	{
//		level = other.level;
//		minPoint = other.minPoint;
//		maxPoint = other.maxPoint;
//		nDims = other.nDims;
//	}
//	
//	TreeNode():nDims(0), level(0)
//	{}
//	
//	TreeNode(Point<T> minPoint, Point<T> maxPoint, int lvl) //, nDims, nDims
//	{
//		STATIC_ASSERT(minPoint.returnSize() == maxPoint.returnSize());
//
//		nDims = minPoint.returnSize();
//
//		this->minPoint = minPoint;
//		this->maxPoint = maxPoint;
//
//		this->nDims = minPoint.returnSize();
//
//		this->level = lvl;
//	}
//
//	/*
//	return the idx-th childnode
//	one dim, less than middle is 0, bigger than middle is 1
//	0~3 for 2d; upper 2|3----10|11;-----So: YX for 2D, ZYX for 3D, TZYX for 4D
//	            lower 0|1----00|01 ---------put next dimension before current dimension
//	*/
//	TreeNode<T> GetChildNode(int idx)
//	{
//		TreeNode<T> nchild;
//		nchild.minPoint = this->minPoint;
//		nchild.maxPoint = this->maxPoint;
//
//		nchild.nDims = this->minPoint.returnSize();
//
//		for (int i = 0; i < nDims; i++)
//		{
//			if ((idx >> i) & 1)  //the bit on the i-th dimension is 1: bigger
//			{
//				nchild.minPoint[i] = (this->minPoint[i] + this->maxPoint[i]) / 2;
//			}
//			else  //the bit on the i-th dimension is 0: smaller
//			{
//				nchild.maxPoint[i] = (this->minPoint[i] + this->maxPoint[i]) / 2;
//			}
//		}
//
//		nchild.level = this->level + 1;
//
//		return nchild;
//	}
//
//	/*
//	return the relationship between treenode and queryRectangle
//	0: treenode is equal to queryRectangle
//	1: treenode contains queryRectangle
//	2: treenode intersects queryRectangle
//	-1(default): not overlap
//	*/
//	int Spatialrelationship(Rect<T> qrt)
//	{
//		/*
//		equal: 
//		if (nrt.x0 == qrt.x0 && nrt.y0 == qrt.y0 &&
//		nrt.x1 == qrt.x1 && nrt.y1 == qrt.y1)
//		return 0;
//		*/
//		int ncmp = 1;
//		for (int i = 0; i < nDims; i++)
//		{
//			ncmp &= this->minPoint[i] == qrt.minPoint[i] && this->maxPoint[i] == qrt.maxPoint[i];
//		}
//		if (ncmp) return 0;
//
//		/*
//		fully contain:
//		if (nrt.x0 <= qrt.x0 && nrt.y0 <= qrt.y0 &&
//		nrt.x1 >= qrt.x1 && nrt.y1 >= qrt.y1)
//		return 1;
//		*/
//		ncmp = 1;
//		for (int i = 0; i < nDims; i++)
//		{
//			ncmp &= this->minPoint[i] <= qrt.minPoint[i] && this->maxPoint[i] >= qrt.maxPoint[i];
//		}
//		if (ncmp) return 1;
//
//		/*
//		intersect:
//		//http://stackoverflow.com/questions/306316/determine-if-two-rectangles-overlap-each-other
//		RectA.Left < RectB.Right && RectA.Right > RectB.Left && RectA.Top > RectB.Bottom && RectA.Bottom < RectB.Top
//		this can be extended more dimensions
//		//http://stackoverflow.com/questions/5009526/overlapping-cubes
//		if (nrt.x0 < qrt.x1 && nrt.x1 > qrt.x0 &&
//		nrt.y0 < qrt.y1 && nrt.y1 > qrt.y0)
//		return 2;
//		*/
//		ncmp = 1;
//		for (int i = 0; i < nDims; i++)
//		{
//			ncmp &= this->minPoint[i] < qrt.maxPoint[i] && this->maxPoint[i] > qrt.minPoint[i];
//		}
//		if (ncmp) return 2;
//
//		//not overlap
//		return -1;
//	}
//};

template<typename T>//, int nDims = 2, int mBits = 4
class QueryBySFC_S
{
public:
	int nDims;
	int mBits;

	map<sfc_bigint, long long>* m_hp;
	int m_hp_lvls;
	SFCType m_sfc_type;

private:
	//void query_approximate(TreeNode<T> nd, Rect<T> queryrect, vector<TreeNode<T>>& resultTNode);
	void query_approximate2(TreeNode<T> nd, Rect<T> queryrect, vector<TreeNode<T>>& resultTNode, int nranges, int ktimes);
	bool check_treenode_isnull(TreeNode<T> nd);

public:
	vector<sfc_bigint>  RangeQueryByBruteforce_LNG(Rect<T> queryRect, SFCType sfc_type);
	vector<sfc_bigint>  RangeQueryByRecursive_LNG(Rect<T> queryrect, SFCType sfc_type, int nranges, int ktimes);

	vector<sfc_bigint>  RangeQueryByRecursive_LNG_P(Rect<T> queryrect, SFCType sfc_type, int nranges, int ktimes);

	//vector<string>  RangeQueryByBruteforce_STR(Rect<T, nDims> queryRect, SFCType sfc_type, StringType encode_type);
	//vector<string>  RangeQueryByRecursive_STR(Rect<T, nDims> queryrect, SFCType sfc_type, StringType encode_type, int nranges, int ktimes);

	QueryBySFC_S(int dims, int bits, map<sfc_bigint, long long>* hp, int hp_lvls, SFCType type) :nDims(dims), mBits(bits), m_hp(hp), m_hp_lvls(hp_lvls), m_sfc_type(type)
	{	}


};
//
/////depth-first traversal in the 2^n-ary tree
//template<typename T>//, int nDims, int mBits
////void QueryBySFC<T, nDims, mBits>::query_approximate(TreeNode<T> nd, Rect<T> queryrect, vector<TreeNode<T, nDims>>& resultTNode)
//void QueryBySFC_S<T>::query_approximate(TreeNode<T> nd, Rect<T> queryrect, vector<TreeNode<T>>& resultTNode)
//{
//
//	/*
//	divide current tree node
//	*/
//	int nary_num = 1 << nDims;  //max count: 2^nDims
//	vector<TreeNode<T>> nchild(nary_num);
//	/*
//	find the currentnode exactly contains queryrectangle; and its child node intersects queryrectangle
//	*/
//	TreeNode<T> currentNode = nd;
//	int res = 1;
//	do
//	{
//		for (int i = 0; i < nary_num; i++)
//		{
//			nchild[i] = currentNode.GetChildNode(i);
//			if (nchild[i].Spatialrelationship(queryrect) == 0)  //equal: stop
//			{
//				resultTNode.push_back(nchild[i]);
//				return ;
//			}
//			else if (nchild[i].Spatialrelationship(queryrect) == 2)  //intersect: divide queryrectangle
//			{
//				res = 0;
//				break;
//			}
//			else  if (nchild[i].Spatialrelationship(queryrect) == 1)//contain: divide the tree node
//			{
//				currentNode = nchild[i];
//				break;
//			}
//		}
//	} while (res);
//
//	
//	/*
//	divide the input query rectangle into even parts, e.g. 2 or 4 parts
//	0~3 for 2d; upper 2|3----10|11;----- YX for 2D, ZYX for 3D, TZYX for 4D--each dim one bit
//	            lower 0|1----00|01 ------one dim: less = 0; greater = 1
//	*/
//	
//	vector<Rect<T>> qrtcut(nary_num);  //2^nDims parts
//	vector<int> qrtpos(nary_num);  //the qrtcut corresponds to treenode
//	for (int i = 0; i < nary_num; i++)
//	{
//		qrtpos[i] = 0;
//	}
//	vector<int> mid(nDims);  //middle cut line--dim number
//	for (int i = 0; i < nDims; i++)
//	{
//		mid[i] = (currentNode.minPoint[i] + currentNode.maxPoint[i]) / 2;
//	}
//
//	int ncount = 1;
//	qrtcut[0] = queryrect;
//	
//	Point<T> pttmp(nDims);  //temporary point or corner
//	for (int i = 0; i < nDims; i++)  //dimension iteration
//	{
//		int newadd = 0;
//		for (int j = 0; j < ncount; j++)
//		{
//			if (qrtcut[j].minPoint[i] < mid[i] && qrtcut[j].maxPoint[i] > mid[i])
//			{
//				Rect<T> rtnew = qrtcut[j];
//				pttmp = rtnew.minPoint;
//				pttmp[i] = mid[i];
//				rtnew.SetMinPoint(pttmp);
//
//				pttmp = qrtcut[j].maxPoint;
//				pttmp[i] = mid[i];
//				qrtcut[j].SetMaxPoint(pttmp);
//
//				qrtpos[ncount + newadd] = (1 << i) + qrtpos[j];
//				qrtcut[ncount + newadd] = rtnew;
//
//				newadd++;
//			}
//
//			if (qrtcut[j].minPoint[i] >= mid[i])  //all bigger than the middle line
//			{
//				qrtpos[j] |= 1 << i;  //just update its position---put 1 on the dimension bit
//			}
//		}  //end for rect count
//
//		ncount += newadd;  //update all rectangle count
//	}  //end for dimension
//	
//	for (int i = 0; i < ncount; i++)   //final rect number 
//	{
//		TreeNode<T> cNode = currentNode.GetChildNode(qrtpos[i]);
//		int rec = cNode.Spatialrelationship(qrtcut[i]);
//		if (rec == 0)
//		{
//			resultTNode.push_back(cNode);  //equal
//		}
//		else if (rec == -1)
//		{
//		}
//		else
//		{
//			query_approximate(cNode, qrtcut[i], resultTNode);  //recursive query
//		}		
//	}
//}

template<typename T>//check tree node is null in the histogram pyramid
bool QueryBySFC_S<T>::check_treenode_isnull(TreeNode<T> nd)
{
	if(nd.level >= m_hp_lvls) return false; //more deeper, no histogram in this level,so assume this node has data
	
	SFCConversion sfc(nDims, nd.level);
	sfc_bigint val;

	Point<T> pttemp = nd.minPoint;
	for (int i = 0; i < nDims; ++i) ///scale the point coordinates
	{
		pttemp[i] = pttemp[i]>> (mBits - nd.level);
	}

	if (m_sfc_type == Hilbert) //encoding minPoint
	{
		val = sfc.HilbertEncode(pttemp);
	}
	if (m_sfc_type == Morton)
	{
		val = sfc.MortonEncode(pttemp);
	}

	//cout << "check:" << nd.level<< ","<<val << endl;

	if (m_hp[nd.level].find(val) == m_hp[nd.level].end())
	{
		// not found--null node
		return true;
	}
	else
	{
		// found- not null
		return false;
	}

	return true;
}

///breadth-first traversal in the 2^n-ary tree
template<typename T>//, int nDims, int mBits
//void QueryBySFC<T, nDims, mBits>::query_approximate2(TreeNode<T, nDims> nd, Rect<T, nDims> queryrect, vector<TreeNode<T, nDims>>& resultTNode, int nranges, int ktimes)
void QueryBySFC_S<T>::query_approximate2(TreeNode<T> nd, Rect<T> queryrect, vector<TreeNode<T>>& resultTNode, int nranges, int ktimes)
{
	int nary_num = 1 << nDims;  //max count: 2^nDims

	typedef tuple<TreeNode<T>, Rect<T>> NRTuple;
	queue<NRTuple> query_queue;

	TreeNode<T> nchild;//=nd
	int res, last_level;
	///////////////////////////////////////////
	//queue the root node
	query_queue.push(NRTuple(nd, queryrect));
	last_level = 0;

	for (; !query_queue.empty(); query_queue.pop())
	{
		NRTuple currenttuple = query_queue.front();

		TreeNode<T> currentNode = std::get<0>(currenttuple);
		Rect<T> qrt = std::get<1>(currenttuple);

		//cout << currentNode.level << endl;
		//////////////////////////////////////////////////////
		//check the level and numbers of results
		if ((nranges != 0) && (last_level != currentNode.level) && (resultTNode.size() + query_queue.size() > ktimes * nranges)) //we are in the new level and full
		{
			///move all the left nodes in the queue to the resuts node vector
			for (; !query_queue.empty(); query_queue.pop())
			{
				TreeNode<T> nd = std::get<0>(query_queue.front());

				if(check_treenode_isnull(nd) == false) //if has data ,queue it
					resultTNode.push_back(nd); //std::get<0>(query_queue.front())
			}

			break; //now
		}

		/////////////////////////////////////////////////////////////////////
		////get all children nodes till equal or intersect, if contain, continue to get children nodes
		do
		{
			for (int i = 0; i < nary_num; i++)
			{
				nchild = currentNode.GetChildNode(i);

				///////////////////////////////////////////////////
				///go down ,go down till the  query rect and the child are on the same scale : equal or intersect
				if (nchild.Spatialrelationship(qrt) == 0)  //equal: stop
				{
					////////////////////////////
					//check data in this node before put it in results-20180913
					if (check_treenode_isnull(nchild) == false)  //check if null before put it in results vector
						resultTNode.push_back(nchild);

					res = 1;
					break; //break for and while ---to continue queue iteration
				}
				else if (nchild.Spatialrelationship(qrt) == 2)  //intersect: divide queryrectangle
				{
					res = 2;
					break;  //break for and while ---divide queryrectangle
				}
				else  if (nchild.Spatialrelationship(qrt) == 1)//contain: go down to the next level untill equal or intersect
				{
					res = 0;
					currentNode = nchild;
					break; //break for but to continue while
				}
			}//end for nary children
		} while (!res);

		if (res == 1) continue; //equal----here break to continue for (queue iteration)
		//			
		//	//divide the input query rectangle into even parts, e.g. 2 or 4 parts
		//	//0~3 for 2d; upper 2|3----10|11;----- YX for 2D, ZYX for 3D, TZYX for 4D--each dim one bit
		//	//0~3 for 2d; lower 0|1----00|01 ------one dim: less = 0; greater = 1		
		vector<Rect<T>> qrtcut(nary_num);  //2^nDims parts
		vector<int> qrtpos(nary_num);  //the qrtcut corresponds to treenode

		for (int i = 0; i < nary_num; i++)
		{
			qrtpos[i] = 0;
		}

		vector<int> mid(nDims);  //middle cut line--dim number
		for (int i = 0; i < nDims; i++)
		{
			mid[i] = (currentNode.minPoint[i] + currentNode.maxPoint[i]) / 2;
		}

		int ncount = 1;
		qrtcut[0] = qrt;

		Point<T> pttmp(nDims);  //temporary point or corner
		for (int i = 0; i < nDims; i++)  //dimension iteration
		{
			int newadd = 0;
			for (int j = 0; j < ncount; j++)
			{
				if (qrtcut[j].minPoint[i] < mid[i] && qrtcut[j].maxPoint[i] > mid[i])
				{
					Rect<T> rtnew = qrtcut[j];
					pttmp = rtnew.minPoint;
					pttmp[i] = mid[i];
					rtnew.SetMinPoint(pttmp);

					pttmp = qrtcut[j].maxPoint;
					pttmp[i] = mid[i];
					qrtcut[j].SetMaxPoint(pttmp);

					qrtpos[ncount + newadd] = (1 << i) + qrtpos[j];
					qrtcut[ncount + newadd] = rtnew;

					newadd++;
				}

				if (qrtcut[j].minPoint[i] >= mid[i])  //all bigger than the middle line
				{
					qrtpos[j] |= 1 << i;  //just update its position---put 1 on the dimension bit
				}
			}//end for rect count

			ncount += newadd;  //update all rectangle count
		}//end for dimension

		for (int i = 0; i < ncount; i++)   //final rect number 
		{
			TreeNode<T> cNode = currentNode.GetChildNode(qrtpos[i]);

			////////////////////////////////////////////////////////////

			int rec = cNode.Spatialrelationship(qrtcut[i]);
			if (rec == 0)
			{
				if (check_treenode_isnull(cNode) == false)  //not null before put it in results vector
					resultTNode.push_back(cNode);  //equal
			}
			else if (rec == -1)
			{
			}
			else
			{
				//query_approximate(cNode, qrtcut[i], resultTNode);  //recursive query
				query_queue.push(NRTuple(cNode, qrtcut[i]));
			}
		}//end for rect division check

	}///end for queue iteration
}


template< typename T>//, int nDims, int mBits
//vector<sfc_bigint>  QueryBySFC<T, nDims, mBits>::RangeQueryByRecursive_LNG(Rect<T, nDims> queryrect, SFCType sfc_type, int nranges, int ktimes)
vector<sfc_bigint>  QueryBySFC_S<T>::RangeQueryByRecursive_LNG(Rect<T> queryrect, SFCType sfc_type, int nranges, int ktimes)
{
	vector<TreeNode<T>> resultTNode;  //tree nodes correspond to queryRectangle
	TreeNode<T> root;  //root node
	root.level = 0;
	root.nDims = nDims;

	root.minPoint.nDims = root.maxPoint.nDims = nDims;
	for (int i = 0; i < nDims; i++)
	{
		root.minPoint[i] = 0;
		root.maxPoint[i] = 1 << mBits;

		queryrect.maxPoint[i] += 1;
	}

	int res = root.Spatialrelationship(queryrect);
	if (res == 0)  //equal
	{
		resultTNode.push_back(root);
	}
	if (res == 1)  //contain
	{
		query_approximate2(root, queryrect, resultTNode, nranges, ktimes);
	}
	//cout << resultTNode.size() << endl;

	int ncorners = 1 << nDims; //corner points number
	//vector<Point<T>> nodePoints(ncorners);
	//vector<sfc_bigint> node_vals(ncorners);

	map<sfc_bigint, sfc_bigint, less<sfc_bigint>> map_range;
	map<sfc_bigint, sfc_bigint, less<sfc_bigint>>::iterator itr;

	sfc_bigint val, r_start, r_end;
	//sfc_bigint k1, k2;

	for (int i = 0; i < resultTNode.size(); i++)
	{
		SFCConversion sfc(nDims, mBits);

		long long node_width = 1 << nDims * (mBits - resultTNode[i].level);//2^(n*(m-l))

		if (sfc_type == Hilbert) //encoding minPoint
		{
			val = sfc.HilbertEncode(resultTNode[i].minPoint);
		}
		if (sfc_type == Morton)
		{
			val = sfc.MortonEncode(resultTNode[i].minPoint);
		}

		r_start = val - val % node_width;
		r_end = r_start + node_width - 1;// cout << r_start << "," << r_end << endl;
		map_range[r_start] = r_end;

		//if (resultTNode[i].level == mBits) //leaf node--just one point
		//{		

		//	if (sfc_type == Hilbert)
		//	{ 
		//		val = sfc.HilbertEncode(resultTNode[i].minPoint);
		//		map_range[val] = val;
		//	}
		//	if (sfc_type == Morton)
		//	{
		//		val = sfc.MortonEncode(resultTNode[i].minPoint);
		//		map_range[val] = val;
		//	}

		//	continue;
		//}

		//nodePoints[0] = resultTNode[i].minPoint;		
		//for (int j = 0; j < nDims; j++)
		//{
		//	int nnow = 1 << j;
		//	for (int k = 0; k < nnow; k++) //1-->2;2-->4, --->2^Dims
		//	{
		//		Point<T, nDims> newPoint = nodePoints[k];
		//		newPoint[j] = resultTNode[i].maxPoint[j] - 1;  //get the cordinate from maxpoint in this dimension
		//		
		//		nodePoints[nnow+k] = newPoint;
		//	}
		//}

		//for (int j = 0; j < ncorners; j++)
		//{ 
		//	if (sfc_type == Hilbert) node_vals[j] = sfc.HilbertEncode(nodePoints[j]);
		//	if (sfc_type == Morton) node_vals[j] = sfc.MortonEncode(nodePoints[j]);
		//}

		//std::sort(node_vals.begin(), node_vals.end());
		//map_range[node_vals[0]] = node_vals[ncorners-1];
	}

	/////////////////////////////////////////////////
	///merg continuous range--->if nranges=0, gap=1; if nranges !=0 ,find the Nth big gap
	///find the suitable distance dmin
	sfc_bigint dmin = 1;//for full ranges
	int nsize = map_range.size();
	if (nranges != 0) //not full ranges---control by nranges N
	{
		vector<sfc_bigint> vec_dist(nsize - 1);

		itr = map_range.begin();
		sfc_bigint last = itr->second;
		for (itr++; itr != map_range.end(); itr++)
		{
			vec_dist.push_back((itr->first - last));

			//cout << itr->first - last << endl;

			last = itr->second;
		}

		tbb::parallel_sort(vec_dist.begin(), vec_dist.end(), std::greater<sfc_bigint>());

		//for (int q = 0; q<nsize - 1;q++)
		//	cout << vec_dist[q]  << endl;

		dmin = vec_dist[nranges - 1];

		//cout << "min gap:" << dmin << endl;
	}

	//////merge
	sfc_bigint k1, k2;
	vector<sfc_bigint> rangevec;

	itr = map_range.begin();
	k1 = itr->first; //k1---k2 current range
	k2 = itr->second;

	//int ncc = 0;
	while (1)
	{
		itr++; //get next range
		//cout << k1  << ',' <<k2 << endl;
		if (itr == map_range.end())
		{
			rangevec.push_back(k1);
			rangevec.push_back(k2);

			break;
		}

		if ((itr->first - k2) <= dmin) // if the next range is continuous to k2 //itr->first == k2 + 1
		{
			//ncc++;
			k2 = itr->second; //enlarge current range
		}
		else //if the next range is not continuous to k2---sotre current range and start another search
		{
			rangevec.push_back(k1);
			rangevec.push_back(k2);

			k1 = itr->first;
			k2 = itr->second;
		}//end if
	}//end while	

	//cout << rangevec.size();

	return rangevec;
}

//typedef concurrent_hash_map<sfc_bigint, sfc_bigint> range_table;
//
//template< typename T>//, int nDims, int mBits>
//struct node2range
//{
//	//range_table&  table;
//	sfc_bigint* vec_minmax = NULL;
//	vector<TreeNode<T>>& vec_nodes; //, nDims
//	SFCType sfc_type;
//	int nDims;
//	int mBits;
//
//	node2range(sfc_bigint*  vec_minmax_, vector<TreeNode<T>>& nodes_, SFCType type_, int dims_, int bits_) : \
//		vec_minmax(vec_minmax_), vec_nodes(nodes_), sfc_type(type_), nDims(dims_), mBits(bits_){}
//
//	void operator( )(const blocked_range<size_t> range)const
//	{
//		SFCConversion sfc(nDims, mBits);//<>
//		sfc_bigint val;
//
//		int ncorners = 1 << nDims; //corner points number
//		vector<Point<T>> nodePoints(ncorners);
//		vector<sfc_bigint> node_vals(ncorners);
//
//		for (size_t i = range.begin(); i != range.end(); ++i)
//		{
//			long long node_width = 1 << nDims* (mBits - vec_nodes[i].level);//2^(n*(m-l))
//
//			if (sfc_type == Hilbert) //encoding minPoint
//			{
//				val = sfc.HilbertEncode(vec_nodes[i].minPoint);
//			}
//			if (sfc_type == Morton)
//			{
//				val = sfc.MortonEncode(vec_nodes[i].minPoint);
//			}
//
//			vec_minmax[2 * i] = val - val % node_width;
//			vec_minmax[2 * i + 1] = val - val % node_width + node_width - 1;
//			//range_table::accessor a;
//
//			//if (vec_nodes[i].level == mBits) //leaf node--just one point
//			//{
//			//	if (sfc_type == Hilbert)
//			//	{
//			//		val = sfc.HilbertEncode(vec_nodes[i].minPoint);
//			//		
//			//		//map_range[val] = val;
//			//		//table.insert(a, val);
//			//		//a->second = val;
//			//		vec_minmax[2 * i] = val;
//			//		vec_minmax[2 * i + 1] = val;
//			//	}
//			//	continue;
//			//}
//
//			//nodePoints[0] = vec_nodes[i].minPoint;
//			//for (int j = 0; j < nDims; j++)
//			//{
//			//	int nnow = 1 << j;
//			//	for (int k = 0; k < nnow; k++) //1-->2;2-->4, --->2^Dims
//			//	{
//			//		Point<T, nDims> newPoint = nodePoints[k];
//			//		newPoint[j] = vec_nodes[i].maxPoint[j] - 1;  //get the cordinate from maxpoint in this dimension
//
//			//		nodePoints[nnow + k] = newPoint;
//			//	}
//			//}
//
//			//for (int j = 0; j < ncorners; j++)
//			//{
//			//	if (sfc_type == Hilbert) node_vals[j] = sfc.HilbertEncode(nodePoints[j]);
//			//	if (sfc_type == Morton) node_vals[j] = sfc.MortonEncode(nodePoints[j]);
//			//}
//
//			//std::sort(node_vals.begin(), node_vals.end());
//			//
//			////map_range[node_vals[0]] = node_vals[ncorners - 1];
//			////table.insert(a, node_vals[0]);
//			////a->second = node_vals[ncorners - 1];
//			//vec_minmax[2 * i] = node_vals[0];
//			//vec_minmax[2 * i + 1] = node_vals[ncorners - 1];
//		}//end for
//	}//end functioner
//};


template< typename T>//, int nDims, int mBits
//vector<sfc_bigint>  QueryBySFC<T, nDims, mBits>::RangeQueryByRecursive_LNG_P(Rect<T, nDims> queryrect, SFCType sfc_type, int nranges, int ktimes)
vector<sfc_bigint>  QueryBySFC_S<T>::RangeQueryByRecursive_LNG_P(Rect<T> queryrect, SFCType sfc_type, int nranges, int ktimes)
{
	vector<TreeNode<T>> resultTNode;  //tree nodes correspond to queryRectangle
	TreeNode<T> root;  //root node
	root.level = 0;
	root.nDims = nDims;
	root.minPoint.nDims = root.maxPoint.nDims = nDims;

	for (int i = 0; i < nDims; i++)
	{
		root.minPoint[i] = 0;
		root.maxPoint[i] = 1 << mBits;
		queryrect.maxPoint[i] += 1;
	}

	int res = root.Spatialrelationship(queryrect);
	if (res == 0)  //equal
	{
		resultTNode.push_back(root);
	}
	if (res == 1)  //contain
	{
		query_approximate2(root, queryrect, resultTNode, nranges, ktimes);
	}
	//cout << resultTNode.size() << endl;

	map<sfc_bigint, sfc_bigint, less<sfc_bigint>> map_range;
	map<sfc_bigint, sfc_bigint, less<sfc_bigint>>::iterator itr;

	size_t node_size = resultTNode.size();
	sfc_bigint* pvec_minmax = new sfc_bigint[node_size * 2];
	parallel_for(blocked_range<size_t>(0, node_size, 200), node2range<T>(pvec_minmax, resultTNode, sfc_type, nDims, mBits));

	//sort
	for (size_t i = 0; i < node_size; ++i)
	{
		map_range[pvec_minmax[2 * i]] = pvec_minmax[2 * i + 1];
	}

	/////////////////////////////////////////////////
	///merg continuous range--->if nranges=0, gap=1; if nranges !=0 ,find the Nth big gap
	///find the suitable distance dmin
	sfc_bigint dmin = 1;//for full ranges
	int nsize = map_range.size();
	if (nranges != 0) //not full ranges---control by nranges N
	{
		vector<sfc_bigint> vec_dist(nsize - 1);

		itr = map_range.begin();
		sfc_bigint last = itr->second;
		for (itr++; itr != map_range.end(); itr++)
		{
			vec_dist.push_back((itr->first - last));

			//cout << itr->first - last << endl;

			last = itr->second;
		}

		tbb::parallel_sort(vec_dist.begin(), vec_dist.end(), std::greater<sfc_bigint>());

		//for (int q = 0; q<nsize - 1;q++)
		//	cout << vec_dist[q]  << endl;

		dmin = vec_dist[nranges - 1];

		//cout << "min gap:" << dmin << endl;
	}

	//////merge
	sfc_bigint k1, k2;
	vector<sfc_bigint> rangevec;

	itr = map_range.begin();
	k1 = itr->first; //k1---k2 current range
	k2 = itr->second;

	//int ncc = 0;
	while (1)
	{
		itr++; //get next range
		//cout << k1  << ',' <<k2 << endl;
		if (itr == map_range.end())
		{
			rangevec.push_back(k1);
			rangevec.push_back(k2);

			break;
		}

		if ((itr->first - k2) <= dmin) // if the next range is continuous to k2 //itr->first == k2 + 1
		{
			//ncc++;
			k2 = itr->second; //enlarge current range
		}
		else //if the next range is not continuous to k2---sotre current range and start another search
		{
			rangevec.push_back(k1);
			rangevec.push_back(k2);

			k1 = itr->first;
			k2 = itr->second;
		}//end if
	}//end while	

	//cout << rangevec.size();

	return rangevec;
}

template< typename T>//, int nDims, int mBits
//vector<sfc_bigint>  QueryBySFC<T, nDims, mBits>::RangeQueryByBruteforce_LNG(Rect<T, nDims> queryRect, SFCType sfc_type)
vector<sfc_bigint>  QueryBySFC_S<T>::RangeQueryByBruteforce_LNG(Rect<T> queryRect, SFCType sfc_type)
{
	Point<T> minPoint = queryRect.GetMinPoint();
	Point<T> maxPoint = queryRect.GetMaxPoint();

	long *difference = new long[nDims];
	long long *para = new long long[nDims + 1];

	para[0] = 1;
	for (int i = 0; i < nDims; i++)
	{
		difference[i] = maxPoint[i] - minPoint[i] + 1;// for brute force , needs to add 1
		para[i + 1] = para[i] * difference[i]; //the coordinates are in the cell center
	}

	vector<vector<T>> queryVector;
	for (int i = 0; i < nDims; i++)
	{
		vector<T> tempVector;
		//int difference = maxPoint[i] - minPoint[i];
		T temp = minPoint[i];
		for (long j = 0; j <= difference[i]; j++)
		{
			tempVector.push_back(temp + j);
		}
		queryVector.push_back(tempVector);
	}

	Point<T> point(nDims); //

	long long tmp = para[nDims] - 1;
	SFCConversion sfc(nDims, mBits);

	sfc_bigint val = 0;
	long long  size = tmp + 1;
	sfc_bigint* result = new sfc_bigint[size];

	for (long long count = tmp; count >= 0; count--)
	{
		long long offset = count;
		for (int j = nDims - 1; j >= 0; j--)
		{
			long long div = para[j];
			int n = offset / div;
			offset = offset % div;
			point[j] = queryVector[j][n];
		}

		if (sfc_type == Hilbert)  val = sfc.HilbertEncode(point);
		if (sfc_type == Morton)  val = sfc.MortonEncode(point);

		result[count] = val;

		//cout << val << ",";
	}

	delete[]para;
	delete[]difference;

	//std::sort(result, result+size);
	tbb::parallel_sort(result, result + size, std::less<sfc_bigint>());

	vector<sfc_bigint> rangevec;
	int nstart = 0;
	int i = 0; //current pos
	while (1)
	{
		i++;
		if (i == size)//over the last one
		{
			rangevec.push_back(result[nstart]);
			rangevec.push_back(result[i - 1]);

			break;
		}

		//cout << result[i] << "," << result[i + 1] << endl;
		if (result[i] != (result[i - 1] + 1)) //discontinuous
		{
			rangevec.push_back(result[nstart]);
			rangevec.push_back(result[i - 1]);

			nstart = i;
		}
	}

	delete[]result;
	return rangevec;
}

///////////////////////////////////////
//template< typename T, int nDims, int mBits>
//vector<string>  QueryBySFC<T, nDims, mBits>::RangeQueryByRecursive_STR(Rect<T, nDims> queryrect, SFCType sfc_type, StringType encode_type, int nranges, int ktimes)
//{
//	vector<string> rangevec;
//	OutputSchema2<nDims, mBits> trans;
//	
//	vector<sfc_bigint> vec_res = RangeQueryByRecursive_LNG(queryrect, sfc_type, nranges, ktimes);
//
//	vector<sfc_bigint>::iterator itr;
//	for (itr = vec_res.begin(); itr != vec_res.end(); itr++)
//	{
//		rangevec.push_back(trans.Value2String(*itr, encode_type));
//	}
//
//	return rangevec;
//
//}
//
//template< typename T, int nDims, int mBits>
//vector<string>  QueryBySFC<T, nDims, mBits>::RangeQueryByBruteforce_STR(Rect<T, nDims> queryRect, SFCType sfc_type, StringType encode_type)
//{
//	vector<string> rangevec;
//	OutputSchema2<nDims, mBits> trans;
//
//	vector<sfc_bigint> vec_res = RangeQueryByBruteforce_LNG(queryRect, sfc_type);
//
//	vector<sfc_bigint>::iterator itr;
//	for (itr = vec_res.begin(); itr != vec_res.end(); itr++)
//	{
//		rangevec.push_back(trans.Value2String(*itr, encode_type));
//	}
//
//	return rangevec;
//}


#endif