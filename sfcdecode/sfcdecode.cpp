// sfcdecode.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <fstream>

#include "SFCConversion.h"
#include "typedef.h"

#include "SFCDePipe.h"

using namespace std;

int main(int argc, char *argv[])
{
	int ndims = 3;//dims for decoding
	int mbits = 13;

	int ndimsR = 0; //dims for other attributes

	////////////////////////////
	int nparallel = 0;
	int nitem_num = 5000;

	int nsfc_type = 0;
	int nencode_type = 0;

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

		if (strcmp(argv[i], "-r") == 0)//dims for other attributes
		{
			i++;
			ndimsR = atoi(argv[i]);
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
	}

	///////////////////////////////////////////////////
	///get the coordinates transfomration file--one more for lod value
	double delta[DIM_MAX + 1] = { 0 }; // 526000, 4333000, 300 //ndims
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

		//run_decode_pipeline<ndims, mbits, ndimsR>(1, szinput, szoutput, nitem_num, nsfc_type, nencode_type, delta, scale);
		run_decode_pipeline(ndims, mbits, ndimsR, 1, szinput, szoutput, nitem_num, nsfc_type, nencode_type, delta, scale);

	}

	if (nparallel == 1)
	{
		if (strlen(szoutput) != 0)  printf("parallel run "); //if not stdout ,print sth
		tbb::task_scheduler_init init_parallel(tbb::task_scheduler_init::automatic);

		//run_decode_pipeline<ndims, mbits, ndimsR>(init_parallel.default_num_threads(), szinput, szoutput, nitem_num, nsfc_type, \
			nencode_type, delta, scale);
		run_decode_pipeline(ndims, mbits, ndimsR, init_parallel.default_num_threads(), szinput, szoutput, nitem_num, nsfc_type, \
			nencode_type, delta, scale);
	}
	
	//system("pause");
	return 0;

}
