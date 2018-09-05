#include <iostream>
#include <fstream>

#include "SFCConversion.h"
#include "typedef.h"

using namespace std;

int main(int argc, char *argv[])
{
	const int ndims = 4;
	const int mbits = 30;

	int nsfc_type = 0;
	int nencode_type = 0;

	char szinput[256] = { 0 };//1.xyz
	char szoutput[256] = { 0 };
	char sztransfile[256] = { 0 };

	for (int i = 1; i < argc; i++)
	{
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
	double delta[ndims + 1] = { 0 }; // 526000, 4333000, 300
	long  scale[ndims + 1] = { 1 }; //100, 100, 1000

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
				scale[j] = atoi(ele);
				j++;

				lastpos = pch + 1;
				pch = strchr(lastpos, ',');
			}
			scale[j] = atoi(lastpos); //final part

			fclose(input_file);
		}//end if input_file
	}//end if strlen

	///////////////////////////////////////
	FILE* input_file = NULL;
	if (szinput != NULL && strlen(szinput) != 0)
	{
		input_file = fopen(szinput, "r");
		if (!input_file)
		{
			return 0;
		}
	}
	else
	{
		input_file = stdin;
	}

	std::ostream* out_s;
	std::ofstream of;
	if (szoutput != NULL && strlen(szoutput) != 0)
	{
		of.open(szoutput);
		out_s = &of;
	}
	else
	{
		out_s = &cout;
	}

	////////////////////////////////////
	//read
	SFCConversion<ndims, mbits> sfctest;

	Point<long, ndims> inPt;
	Point<double, ndims> outPt;

	sfc_bigint val;

	char buf[1024];
	char buf2[1024];
	while (1) //always true
	{
		memset(buf, 0, 1024);
		fgets(buf, 1024, input_file);

		if (strlen(buf) == 0) break; // no more data

		memset(buf2, 0, 1024);
		char* pos = strchr(buf, '\n');
		if (pos != NULL)
			strncpy(buf2, buf, pos - buf);//remove newline char
		else
			strcpy(buf2, buf);

		val = sfc_bigint(buf2);//buf

		if (nsfc_type == 0)
			inPt = sfctest.MortonDecode(val);
		else
			inPt = sfctest.HilbertDecode(val);

		for (int i = 0; i < ndims; i++)
		{
			//outPt[i] = lround((inPt[i] - _delta[i])*_scale[i]);//encoding
			outPt[i] = ((double)inPt[i]) / scale[i] + delta[i]; //decoding

			(*out_s) << setprecision(9) << outPt[i];

			if (i != ndims - 1) (*out_s) << ",";
		}
		(*out_s) << endl;
	}

	fclose(input_file);
	of.close();

	return 0;
}
