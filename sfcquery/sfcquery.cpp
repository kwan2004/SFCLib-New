// sfcquery.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include "Point.h"
#include "Rectangle.h"

#include "SFCConversion.h"
//#include "OutputSchema.h"
#include "QueryBySFC.h"

#include "SFCPipeline.h"

#include "SFCConversion2.h"
#include "OutputSchema2.h"

#include "RandomLOD.h"

//#include "tbb/task_scheduler_init.h"

#include <iostream>
#include <fstream>
using namespace std;


void print_ranges(char * str, vector<sfc_bigint>& ranges)
{
	sfc_bigint ntotal_len = 0;
	if (str == NULL) return;

	cout << str << endl;
	for (int i = 0; i < ranges.size(); i = i + 2)
	{
		//printf("\n");

		//printf("%lld---%lld\n", ranges[i], ranges[i + 1]);
		//cout << ranges[i] << "----" << ranges[i + 1] <<endl;

		ntotal_len += (ranges[i + 1] - ranges[i] + 1);
	}

	cout << "total ranges len:  " << ntotal_len << endl;
}



int main(int argc, char* argv[])
{
	//here the SFCQuery tool
	const int ndims = 4;
	const int mbits = 20;

	///-i 347068810/347068850/-73.96/-73.91/40.5/41/-73.99/-73.90/40.5/41 -s 1 -e 0 -t ./cttaxi.txt -n 2000 -k 4 -o range.sql

	///85999.42,446266.47,-1.65,9,651295384353375995169439
	//-i 85999.0/85999.5/446266/446266.5/-2.0/-1.5/8/9 -s 1 -e 0 -t ct.txt -n 1000 -o qq3.sql

	//85999.1,446250.23,-1.69,9,651295397912973650169147
	//-i 85999.0/85999.5/446250/446250.4/-2.0/-1.5/8/9 -s 1 -e 0 -t ct.txt -n 0 -o qq5.sql
	//-i 85545.3000/85695.3000/446465.6500/446615.6500/-99999999.0000/-99999999.0000/-99999999.0000/-99999999.0000 -s 1 -e 0 -t ct.txt -n 0 -o qq5.sql
	//-i 85545.3000/85695.3000/446465.6500/446615.6500/-2.0000/-1.5000/8.0000/9.0000 -s 1 -e 0 -t ..\SFCLib\ct.txt -n 5000 -o qq5.sql -v -p 1
	int nsfc_type = 0;
	int nencode_type = 0;

	bool bstat = false; //control statistics

	//bool bislod = false;
	//int lod_levels = 0;
	int nparallel = 1;
	int nranges = 0; //if nranges =0; means search to the bottom level
	int ktimes = ndims; // the ktimes* nranges is used to control tree traversal depth

	char szinput[1024] = { 0 };//1.xyz
	char szoutput[256] = { 0 };
	char sztransfile[256] = { 0 };

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-i") == 0)//input filter coordinates
		{
			i++;
			strcpy(szinput, argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-o") == 0)//output file path
		{
			i++;
			strcpy(szoutput, argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-s") == 0)//sfc conversion type: 0 morthon, 1 hilbert
		{
			i++;
			nsfc_type = atoi(argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-e") == 0)//output encoding type: 0 number 1 base32 2 base64
		{
			i++;
			nencode_type = atoi(argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-t") == 0)//coordinates transformation file, two lines: translation and scale, comma separated
		{
			i++;
			strcpy(sztransfile, argv[i]);
			continue;
		}
		if (strcmp(argv[i], "-n") == 0)//number of return ranges
		{
			i++;
			nranges = atoi(argv[i]);
			continue;
		}
		if (strcmp(argv[i], "-k") == 0)//k times of returned ranes for gap merge
		{
			i++;
			ktimes = atoi(argv[i]);
			continue;
		}
		if (strcmp(argv[i], "-v") == 0)//k times of returned ranes for gap merge
		{
			//i++;
			bstat = true;
			continue;
		}
		if (strcmp(argv[i], "-p") == 0)//if parallel: 0 sequential, 1 max parallel
		{
			i++;
			nparallel = atoi(argv[i]);
			continue;
		}
	}

	///////////////////////////////////////////////////
	///get the coordinates transfomration file--one more for lod value
	double delta[ndims + 1] = { 0 }; // 526000, 4333000, 300
	double  scale[ndims + 1] = { 1 }; //100, 100, 1000

	for (int i = 1; i < ndims + 1; i++)
	{
		delta[i] = 0;
		scale[i] = 1;
	}

	if (strlen(sztransfile) != 0)
	{
		FILE* input_file = NULL;
		input_file = fopen(sztransfile, "r");
		if (input_file)
		{
			int j;
			char buf[1024];
			char * pch, *lastpos;
			char ele[128];

			//////translation
			memset(buf, 0, 1024);
			fgets(buf, 1024, input_file);

			j = 0;
			lastpos = buf;
			pch = strchr(buf, ',');
			while (pch != NULL)
			{
				memset(ele, 0, 128);
				strncpy(ele, lastpos, pch - lastpos);
				//printf("found at %d\n", pch - str + 1);
				delta[j] = atof(ele);
				j++;

				lastpos = pch + 1;
				pch = strchr(lastpos, ',');
			}
			delta[j] = atof(lastpos); //final part

			//////scale
			memset(buf, 0, 1024);
			fgets(buf, 1024, input_file);

			j = 0;
			lastpos = buf;
			pch = strchr(buf, ',');
			while (pch != NULL)
			{
				memset(ele, 0, 128);
				strncpy(ele, lastpos, pch - lastpos);
				//printf("found at %d\n", pch - str + 1);
				scale[j] = atof(ele);
				j++;

				lastpos = pch + 1;
				pch = strchr(lastpos, ',');
			}
			scale[j] = atof(lastpos); //final part

			fclose(input_file);
		}//end if input_file
	}//end if strlen

	CoordTransform<double, long> cotrans(ndims);
	cotrans.SetTransform(delta, scale);
	////////////////////////////////////////////////
	//get the input filter
	double pt1[ndims] = { 0.0f };
	double pt2[ndims] = { 0.0f };

	unsigned int dim_valid[ndims] = { 0 };

	memset(pt1, 0, sizeof(double)*ndims);
	memset(pt2, 0, sizeof(double)*ndims);
	memset(dim_valid, 0, sizeof(unsigned int)*ndims);

	char * pch, *lastpos;
	char ele[128];

	lastpos = szinput;
	for (int i = 0; i < ndims; i++)
	{
		///////min
		memset(ele, 0, 128);

		pch = strchr(lastpos, '//');
		strncpy(ele, lastpos, pch - lastpos);

		if (strcmp(ele, "-99999999.0000") != 0)//if "-99999999.0000", not set
		{
			pt1[i] = atof(ele);
			dim_valid[i] = 1;
		}
		else
		{
			pt1[i] = 0; ///this min value is not set,just assign 0
		}

		lastpos = pch + 1;
		///////max
		if (i != ndims - 1)
		{
			memset(ele, 0, 128);

			pch = strchr(lastpos, '//');
			strncpy(ele, lastpos, pch - lastpos);

			if (strcmp(ele, "-99999999.0000") != 0) //if "-99999999.0000", not set 
			{
				pt2[i] = atof(ele);
				dim_valid[i] = 1;
			}
			else
			{
				pt2[i] = 0; ///this max value is not set,just assign 2^mbits -11 <  < mbits - 1
			}

			lastpos = pch + 1;
		}
		else
		{
			if (strcmp(lastpos, "-99999999.0000") != 0) //if "-99999999.0000", not set
			{
				pt2[i] = atof(lastpos);
				dim_valid[i] = 1;
			}
			else
			{
				pt2[i] = 0; ///this max value is not set,just assign 2^mbits -1 1 << mbits - 1
			}
		}

	}
	///////////////////////////////////////////////
	//point transfomration
	Point<double> MinPt1(pt1, ndims); //, ndims
	Point<double> MaxPt1(pt2, ndims);//ndims

	Point<long> MinPt2 = cotrans.Transform(MinPt1); //, ndims
	Point<long> MaxPt2 = cotrans.Transform(MaxPt1);//, ndims

	///to check if any dim is not set
	for (int i = 0; i < ndims; i++)
	{
		if (dim_valid[i] == 0)// this dim is not set
		{
			MinPt2[i] = 0;
			MaxPt2[i] = 1 << mbits - 1;
		}
	}

	/////////////////////////////////////////////////////
	////query
	Rect<long> rec(MinPt2, MaxPt2); //, ndims
	QueryBySFC<long> querytest(ndims, mbits); //

	std::ostream* out_s;
	ofstream range_file;
	if (strlen(szoutput) != 0)
	{
		range_file.open(szoutput);
		out_s = &range_file;
	}
	else
	{
		out_s = &cout;
	}

	tbb::task_scheduler_init init(tbb::task_scheduler_init::default_num_threads());

	tbb::tick_count t0 = tbb::tick_count::now();

	if (nencode_type == 0) //number
	{
		vector<sfc_bigint> vec_res2;

		if (nparallel == 0)
			vec_res2 = querytest.RangeQueryByRecursive_LNG(rec, (SFCType)nsfc_type, nranges, ktimes);
		else
			vec_res2 = querytest.RangeQueryByRecursive_LNG_P(rec, (SFCType)nsfc_type, nranges, ktimes);

		if (bstat == false) //direct output
		{
			for (int i = 0; i < vec_res2.size(); i = i + 2)
			{
				(*out_s) << vec_res2[i] << "," << vec_res2[i + 1] << endl;
			}
		}
		else
		{
			sfc_bigint tot = 1;
			for (int i = 0; i < ndims; i++)
			{
				tot *= (MaxPt2[i] - MinPt2[i]);
			}
			print_ranges("hilbert recursive", vec_res2);
			cout << "parallel:  " << nparallel << endl;
			cout << "total input len:  " << tot << endl;
			cout << "total ranges:  " << vec_res2.size() / 2 << endl;
		}

	}
	else //string BASE32 BASE64
	{
		//vector<string> vec_res5 = querytest.RangeQueryByBruteforce_STR(rec, (SFCType)nsfc_type, (StringType)(nencode_type - 1));
		//print_ranges_str("hilbert 2d brute force", vec_res5);

		//vector<string> vec_res6 = querytest.RangeQueryByRecursive_STR(rec, (SFCType)nsfc_type, (StringType)(nencode_type - 1), nranges, ktimes);
		//print_ranges_str("hilbert 2d recursive", vec_res6);

		//for (int i = 0; i < vec_res6.size(); i = i + 2)
		//{
		//	//fprintf(output_file, "%s,%s\n", vec_res6[i].c_str(), vec_res6[i + 1].c_str());
		//	(*out_s) << vec_res6[i] << "," << vec_res6[i + 1] << endl;
		//}
	}

	//if (output_file != NULL) fclose(output_file);
	range_file.close();

	tbb::tick_count t1 = tbb::tick_count::now();

	if (bstat)
		cout << "ranges time = " << (t1 - t0).seconds() << endl;
	//system("pause");
	return 0;
}

