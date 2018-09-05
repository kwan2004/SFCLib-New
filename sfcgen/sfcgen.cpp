// sfcgen.cpp : Defines the entry point for the console application.
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

int main(int argc, char* argv[])
{

	//if (argc == 1) return 0;
	//if (argc % 2 != 1) return 0; //attribute pair plus exe_name	

	int ndims = 3;
	int mbits = 13;

	//-p 0 -s 1 -e 2 -t ct.txt -l 10 -i ahn2.txt -o ee.txt 
	int nparallel = 0;

	int nsfc_type = 0;
	int nencode_type = 0;

	bool bisonlysfc = false;

	int nitem_num = 5000;

	bool bislod = false;
	int lod_levels = 0;

	char szinput[256] = { 0 };//1.xyz
	char szoutput[256] = { 0 };
	char sztransfile[256] = { 0 };

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-n") == 0)//dimension number
		{
			i++;
			ndims = atoi(argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-m") == 0)//sfc level number
		{
			i++;
			mbits = atoi(argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-p") == 0)//if parallel: 0 sequential, 1 max parallel
		{
			i++;
			nparallel = atoi(argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-i") == 0)//input file path
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

		if (strcmp(argv[i], "-onlysfc") == 0)//output onlye sfc code
		{
			i++;
			bisonlysfc = true; //(bool)atoi(argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-l") == 0)//if generate the lod value and the lod levels
		{
			i++;
			bislod = true;
			lod_levels = atoi(argv[i]);
			continue;
		}
		if (strcmp(argv[i], "-c") == 0)//if points number per chunk during parallel
		{
			i++;
			nitem_num = atoi(argv[i]);
			continue;
		}
	}

	///////////////////////////////////////////////////
	///get the coordinates transfomration file--one more for lod value
	double delta[DIM_MAX + 1] = { 0 }; // 526000, 4333000, 300 //dims at most 20 dimension
	double  scale[DIM_MAX + 1] = { 1 }; //100, 100, 1000

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
			char ele[64];

			//////translation
			memset(buf, 0, 1024);
			fgets(buf, 1024, input_file);

			j = 0;
			lastpos = buf;
			pch = strchr(buf, ',');
			while (pch != NULL)
			{
				memset(ele, 0, 64);
				strncpy(ele, lastpos, pch - lastpos);
				//printf("found at %d\n", pch - str + 1);
				delta[j] = atof(ele);
				j++;

				//if (j > ndims) break;

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
				memset(ele, 0, 64);
				strncpy(ele, lastpos, pch - lastpos);
				//printf("found at %d\n", pch - str + 1);
				scale[j] = atof(ele);
				j++;

				//if (j > ndims) break;

				lastpos = pch + 1;
				pch = strchr(lastpos, ',');
			}
			scale[j] = atof(lastpos); //final part

			fclose(input_file);
		}//end if input_file
	}//end if strlen

	/////////////////////////////////
	////pipeline
	if (nparallel == 0)
	{
		if (strlen(szoutput) != 0) printf("serial run   "); //if not stdout ,print sth
		tbb::task_scheduler_init init_serial(1);

		if (bislod)//lod value, one more dimension
			//run_pipeline<ndims + 1, mbits>(1, szinput, szoutput, nitem_num, nsfc_type, nencode_type, delta, scale, bisonlysfc, bislod, lod_levels);
			run_pipeline(ndims + 1, mbits,1, szinput, szoutput, nitem_num, nsfc_type, nencode_type, delta, scale, bisonlysfc, bislod, lod_levels);
		else
			//run_pipeline<ndims, mbits>(1, szinput, szoutput, nitem_num, nsfc_type, nencode_type, delta, scale, bisonlysfc, bislod, lod_levels);
			run_pipeline(ndims, mbits,1, szinput, szoutput, nitem_num, nsfc_type, nencode_type, delta, scale, bisonlysfc, bislod, lod_levels);

	}

	if (nparallel == 1)
	{
		if (strlen(szoutput) != 0)  printf("parallel run "); //if not stdout ,print sth
		tbb::task_scheduler_init init_parallel(tbb::task_scheduler_init::automatic);

		if (bislod)//lod value, one more dimension
			//run_pipeline<ndims + 1, mbits>(init_parallel.default_num_threads(), szinput, szoutput, nitem_num, nsfc_type, \
			nencode_type, delta, scale, bisonlysfc, bislod, lod_levels);
			run_pipeline(ndims + 1, mbits,init_parallel.default_num_threads(), szinput, szoutput, nitem_num, nsfc_type, \
				nencode_type, delta, scale, bisonlysfc, bislod, lod_levels);
		else
			//run_pipeline<ndims, mbits>(init_parallel.default_num_threads(), szinput, szoutput, nitem_num, nsfc_type, \
			nencode_type, delta, scale, bisonlysfc, bislod, lod_levels);
			run_pipeline(ndims, mbits,init_parallel.default_num_threads(), szinput, szoutput, nitem_num, nsfc_type, \
				nencode_type, delta, scale, bisonlysfc, bislod, lod_levels);
	}
	//system("pause");
	return 0;
}

