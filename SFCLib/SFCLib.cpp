// SFCLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Point.h"
#include "Rectangle.h"

#include "SFCConversion.h"
//#include "OutputSchema.h"
#include "QueryBySFC2.h"

#include "SFCPipeline.h"

#include "SFCConversion2.h"
#include "OutputSchema2.h"

#include "RandomLOD.h"

//#include "tbb/task_scheduler_init.h"

#include <iostream>
#include <fstream>
using namespace std;

void print_bits(unsigned int x)
{
	int i;
	for (i = 8 * sizeof(x) - 1; i >= 0; i--) // 8 * sizeof(x) - 1
	{
		(x & (1 << i)) ? putchar('1') : putchar('0');
	}
	//printf("\n");
}

void print_ranges(char * str, vector<sfc_bigint>& ranges)
{
	sfc_bigint ntotal_len = 0;
	if (str == NULL) return;

	cout << str << endl;
	for (int i = 0; i < ranges.size(); i = i + 2)
	{
		//printf("\n");

		//printf("%lld---%lld\n", ranges[i], ranges[i + 1]);
		cout << ranges[i] << "----" << ranges[i + 1] <<endl;

		ntotal_len += (ranges[i + 1] - ranges[i] + 1);		
	}

	cout << "total ranges len:  " << ntotal_len << endl;
}

void print_ranges_str(char * str, vector<string>& ranges)
{
	if (str == NULL) return;

	printf("%s \n", str);
	for (int i = 0; i < ranges.size(); i = i + 2)
	{
		//printf("\n");

		printf("%s---%s\n", ranges[i].c_str(), ranges[i + 1].c_str());

	}
}

int main(int argc, char* argv[])
{	
	//if (argc == 1) return 0;
	//if (argc % 2 != 1) return 0; //attribute pair plus exe_name	

#ifdef SFC_GEN_TEST
	////////5D taxi tests
	//347068803,-73.9890213,40.7215385,-73.9889603,40.7216988,16793554232693735
	Point<double> pt7(5);
	pt7[0] = 347068803;//1010
	pt7[1] = -73.9890213;//1011
	pt7[2] = 40.7215385; //0011
	pt7[3] = -73.9889603;//1101
	pt7[4] = 40.7216988;

	double delta7[5] = { 347068000, -81, 34, -81, 34 }; 
	double  scale7[5] = { 1, 10000, 10000, 10000, 10000 }; //100, 100, 1000

	CoordTransform<double, long> cotrans7( 5);
	cotrans7.SetTransform(delta7, scale7);

	Point<long> MinPt7 = cotrans7.Transform(pt7);

	for (int i = 0; i < 5; i++)
	{
		cout << MinPt7[i] << endl;
	}


	SFCConversion sfctest7(5, 30);

	sfc_bigint p7 = sfctest7.HilbertEncode(MinPt7);
	cout << p7 << endl;

	Point<long> MinPt8 = sfctest7.HilbertDecode(p7);

	for (int i = 0; i < 5; i++)
	{
		cout << MinPt8[i] << endl;
	}

	//////////////////////////////////////////////
	////////3D taxi tests
	//347068803,-73.9890213,40.7215385,-73.9889603,40.7216988,16793554232693735
	Point<double> pt8(3);
	pt8[0] = 347068800;//1010
	pt8[1] = -73.9903717;//1011
	pt8[2] = 40.7346954; //0011

	double delta8[3] = { 347068000, -75, 40 };
	double  scale8[3] = { 0.00333, 1500, 1500 }; //100, 100, 1000

	CoordTransform<double, long> cotrans8(3);
	cotrans8.SetTransform(delta8, scale8);

	Point<long> MinPt9 = cotrans8.Transform(pt8);

	for (int i = 0; i < 3; i++)
	{
		cout << MinPt9[i] << endl;
	}
	
	SFCConversion sfctest8(3, 18);
	sfc_bigint p8 = sfctest8.HilbertEncode(MinPt9);
	cout << p8 << endl;

	//SFCConversion<3, 10> sfctest9;
	Point<long> MinPt10 = sfctest8.HilbertDecode(p8);//

	for (int i = 0; i < 3; i++)
	{
		cout << MinPt10[i] << endl;
	}

	Point<double> pt10 = cotrans8.InverseTransform(MinPt10);//

	for (int i = 0; i < 3; i++)
	{
		cout << pt10[i] << endl;
	}

	tbb::tick_count t6 = tbb::tick_count::now();

	for (int i = 0; i < 1000000; i++)
	{
		MinPt10 = sfctest8.HilbertDecode(p8);
	}
	tbb::tick_count t7 = tbb::tick_count::now();
	cout << "hilbert decode time = " << (t7 - t6).seconds() << endl;

	/////////////////////////////////////////////////////////
	///here just for unit tests of SFC code calculation and query
	///3D case
	Point<long> pt3d(3); //SFC coordinates n=2

	SFCConversion2 sfc3d(3, 3);
	SFCConversion sfc3d_old(3, 3);
	//OutputSchema2<2, 4> trans;

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			for (int k = 0; k < 8; k++)
			{
				pt3d[0] = i;//i
				pt3d[1] = j;//j
				pt3d[2] = k;//k

				sfc_bigint outval = sfc3d.HilbertEncode(pt3d);
				pt3d = sfc3d.HilbertDecode(outval);
				cout << i << ", " << j << ", " << k << "---M-->" << outval << "----->" << pt3d[0] << ", " << pt3d[1] << ", " <<  pt3d[2]<<endl; //
			}
		}
	}
	cout << "-----------------------------------" << endl;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			for (int k = 0; k < 8; k++)
			{
				pt3d[0] = i;//i
				pt3d[1] = j;//j
				pt3d[2] = k;//k

				sfc_bigint outval_old = sfc3d_old.HilbertEncode(pt3d);
				pt3d = sfc3d_old.HilbertDecode(outval_old);
				cout << i << ", " << j << ", " << k << "---L-->" << outval_old << "----->" << pt3d[0] << ", " << pt3d[1] << ", " << pt3d[2] << endl;
			}
		}
	}

	pt3d = sfc3d_old.HilbertDecode(511);
	long long  aaaaa = (long long)sfc3d_old.HilbertEncode(pt3d);

	/////////////////////////////////////////////////////
	///true 4D data 85999.4,446266,-1.65,9,651295384353375995169439
	Point<double> pt1(4);
	pt1[0] = 85999.42;//1010
	pt1[1] = 446266.47;//1011
	pt1[2] = -1.65; //0011
	pt1[3] = 9;//1101

	Point<double> pt2(4);
	pt2[0] = 85098.38;//1010
	pt2[1] = 446440.06;//1011
	pt2[2] = 18.34; //0011
	pt2[3] = 9;//1101

	double delta[4] = { 80000.00, 437500.00, -20.0, 0.0 }; // 526000, 4333000, 300
	double  scale[4] = { 100, 100, 100, 1 }; //100, 100, 1000

	CoordTransform<double, long> cotrans(4);
	cotrans.SetTransform(delta, scale);

	Point<long> MinPt2 = cotrans.Transform(pt1);
	Point<long> MaxPt2 = cotrans.Transform(pt2);

	SFCConversion sfctest(4, 30);
	OutputSchema2 transtest(4, 30);

	sfc_bigint p1 = sfctest.HilbertEncode(MinPt2);
	cout << p1 << endl;
	cout << transtest.Value2String(p1, Base64).c_str() << endl;

	Point<long> pt0 = sfctest.HilbertDecode(p1);

	sfc_bigint p2 = sfctest.HilbertEncode(MaxPt2);
	string stra = transtest.Value2String(p2, Base64).c_str();
	cout << p2 << endl;
	cout << stra << "  "<< p2.str()<< endl;

	string res= stra;//85098.38 446440.06 38.34 9

	SFCConversion sfctest2(4, 30);
	OutputSchema2 transtest2(4, 30);

	sfc_bigint p3 = transtest2.String2Value(res, Base64);
	cout << p3 << endl;
	if (p2 == p3)
	{
		int a = 0;
	}
	Point<long> Pt3 = sfctest2.HilbertDecode(p3);

	long long aq = 2349505055;
	string aa = transtest.Value2String(aq, Base64);
	cout << aa << endl;
	sfc_bigint bq = transtest2.String2Value(aa, Base64);
	cout << bq << endl;

	///////////////////////////////////////
	////2D case 16*16, i.e n=2, m=4
	Point<long> ptCoord(2); //SFC coordinates n=2

	SFCConversion2 sfc(2, 4);
	OutputSchema2  trans(2, 4);

	SFCConversion sfc_old(2, 1);
	//OutputSchema2<2, 4> trans;

	int a, b;
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			ptCoord[0] = i;//i
			ptCoord[1] = j;//j

			//sfc_bigint outval = sfc.HilbertEncode(ptCoord);

			ptCoord[0] = i;//i
			ptCoord[1] = j;//j

			sfc_bigint outval_old = sfc_old.HilbertEncode(ptCoord);

			/*cout << i << ", " << j << "---M-->" << outval <<  "  ,  " << trans.Value2String(outval, Base64); //<< endl
			
			Point<long, 2> pt2d = sfc.HilbertDecode(outval);

			a = pt2d[0];
			b = pt2d[1];
			cout  << "--->" << a << " , " << b << endl;*/

			
			cout << i << ", " << j << "---L-->" << outval_old << "  ,  " << trans.Value2String(outval_old, Base64); //<< endl

			Point<long>  pt2d = sfc_old.HilbertDecode(outval_old);
			a = pt2d[0];
			b = pt2d[1];
			cout << "--->" << a << " , " << b << endl;
		}
	}

	SFCConversion sfc_a(4, 6);
	SFCConversion2  sfc_b(4, 6);
	Point<long> pta = sfc_a.HilbertDecode(476);
	Point<long> ptb = sfc_b.HilbertDecode(476);

	//////////////////////////////////
	/////Butz's sample, n=5; m=4;
	Point<long> pt3(5);
	pt3[0] = 10;//1010
	pt3[1] = 11;//1011
	pt3[2] = 3; //0011
	pt3[3] = 13;//1101
	pt3[4] = 5; //0101

	SFCConversion sfc2_old(5, 30);
	sfc_bigint val_old = sfc2_old.HilbertEncode(pt3);
	//Point<long, 6> pt4; //SFC bit sequence m=3

	pt3[0] = 5;//1010
	pt3[1] = 13;//1011
	pt3[2] = 3; //0011
	pt3[3] = 11;//1101
	pt3[4] = 10; //0101

	SFCConversion2 sfc2(5, 30);
	sfc_bigint val = sfc2.HilbertEncode(pt3);	

	cout << "butz--" << val << "," << val_old << endl;
	//print_bits(val);

	Point<long> pt4 = sfc2_old.HilbertDecode(val_old);
	Point<long> pt5 = sfc2.HilbertDecode(val);

	///////////////////////////////////////////////////////////////
	///performance comparison between JKLawder and Doug Moore-- both are based on Butz bits operation
	///Moore: 272s ; lawders: 14s
	tbb::tick_count t0 = tbb::tick_count::now();

	for (int i = 0; i < 1000000; i++)
	{
		sfc2_old.HilbertEncode(pt3);
	}
	tbb::tick_count t1 = tbb::tick_count::now();
	cout << "Laweder's hilbert time = " <<(t1 - t0).seconds()<<endl;

	//////////
	t0 = tbb::tick_count::now();

	for (int i = 0; i < 1000000; i++)
	{
		sfc2_old.MortonEncode(pt3);
	}
	t1 = tbb::tick_count::now();
	cout << "Laweder's morton time = " << (t1 - t0).seconds() << endl;

	///////////////
	t0 = tbb::tick_count::now();

	for (int i = 0; i < 1000000; i++)
	{
		sfc2.HilbertEncode(pt3);
	}
	t1 = tbb::tick_count::now();
	cout << "Moore's time = " << (t1 - t0).seconds() << endl;
#endif

#ifdef SFC_QUERY_TEST
	///////////////////////////////////
	////2D sample--range query
	long Point1[2] = { 3, 2}; //3, 2 //8, 4
	long Point2[2] = { 4, 14 };  //5, 5//12, 9
	Point<long> MinPoint(Point1, 2);
	Point<long> MaxPoint(Point2, 2);
	Rect<long> rec(MinPoint, MaxPoint);
	QueryBySFC<long> querytest(2, 4);

	vector<sfc_bigint> vec_res = querytest.RangeQueryByBruteforce_LNG(rec, Hilbert);
	print_ranges("hilbert 2d brute force", vec_res);

	vector<sfc_bigint> vec_res2 = querytest.RangeQueryByRecursive_LNG(rec, Hilbert, 0, 1);
	print_ranges("hilbert 2d recursive", vec_res2);

	/*vector<string> vec_res5 = querytest.RangeQueryByBruteforce_STR(rec, Hilbert, Base64);
	print_ranges_str("hilbert 2d brute force", vec_res5);

	vector<string> vec_res6 = querytest.RangeQueryByRecursive_STR(rec, Hilbert, Base64, 0);
	print_ranges_str("hilbert 2d recursive", vec_res6);*/

	///////////////////////////////////
	////3D sample----range query
	long Point31[4] = { 6, 2, 15, 6 };//
	long Point32[4] = { 35, 38, 17, 28 };//

	//long Point31[3] = { 8, 8, 16 };//,6
	//long Point32[3] = { 15, 15, 23 };//, 28 

	SFCConversion sfc_3d(4, 18);

	Point<long> MinPoint3(Point31,4);
	Point<long> MaxPoint3(Point32,4);

	Rect<long> rec3(MinPoint3, MaxPoint3);

	QueryBySFC<long> querytest3( 4, 8);
	vector<sfc_bigint> vec_res3 = querytest3.RangeQueryByBruteforce_LNG(rec3, Hilbert);
	print_ranges("Hilbert 3d brute force", vec_res3);

	vector<sfc_bigint> vec_res4 = querytest3.RangeQueryByRecursive_LNG(rec3, Hilbert, 0, 1);
	print_ranges("Hilbert 3d recursive", vec_res4);
	

	/*vector<string> vec_res7 = querytest3.RangeQueryByBruteforce_STR(rec3, Hilbert, Base64);
	print_ranges_str("hilbert 3d brute force", vec_res7);

	vector<string> vec_res8 = querytest3.RangeQueryByRecursive_STR(rec3, Hilbert, Base64, 0);
	print_ranges_str("hilbert 3d recursive", vec_res8);*/

	//SFCConversion<3, 9> sfc3D;
	//OutputSchema<3, 9> trans3D;
	//sfc3D.ptCoord = Point31;
	//sfc3D.MortonEncode();
	//string str3D = trans3D.BitSequence2String(sfc3D.ptBits, Base64);
	//Point<long, 9> mm = trans3D.String2BitSequence(str3D, Base64);
	////Point<long, 10> pttt = trans3D.String2BitSequence(str3D, Base64);

	//long Point333[3] = { 4, 2, 5 };
	//SFCConversion<3, 4> sfc3D3;
	//OutputSchema<3, 4> trans3D3;
	//sfc3D3.ptCoord = Point333;
	//sfc3D3.MortonEncode();
	//string str3D3 = trans3D3.BitSequence2String(sfc3D3.ptBits, Base32);
	//Point<long, 4> mmm = trans3D3.String2BitSequence(str3D3, Base32);
	
#endif


#ifdef RANDOM_LOD
	// random lod tests here
	RandomLOD<3> rnd_gen(10,20);

	int ncount[14] = { 0 };

	int nlevel=0;
	for (int i = 0; i < 10000000; i++)
	{
		nlevel = rnd_gen.RLOD_Gen();

		ncount[nlevel]++;
	}

	for (int i = 0; i < 14; i++)
	{
		cout << i << ":" << ncount[i] << endl;
	}

	cout << "out: " << rnd_gen.ntest << endl;
#endif
	//system("pause");
 	return 0;
}

