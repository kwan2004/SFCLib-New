#ifndef SFCDPIPELINE_H_
#define SFCDPIPELINE_H_

#include "tbb/pipeline.h"
#include "tbb/tick_count.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/tbb_allocator.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>

#include <iostream>
#include <fstream>

#include "Point.h"
#include "CoordTransform.h"

//#include "SFCConversion2.h"
#include "SFCConversion.h"
#include "OutputSchema2.h"

static double	g_step4_time = 0.0f; //input
static double	g_step5_time = 0.0f; //decoding
static double	g_step6_time = 0.0f; //output

//template<int nDimsR>//this dim is for other attributes
class InputItem
{
public:
	sfc_bigint* pKeys;
	Point<double>* pPtsArray; //, nDimsR
	int _actual_size;
	int _alloc_size;
	int nDimsR;

	InputItem(int dimsr)
	{
		_alloc_size = 0;
		_actual_size = 0;

		pKeys = NULL;
		pPtsArray = NULL;

		nDimsR = dimsr;
	}
};

//template<int nDims, int nDimsR>//this first dim is for decoding dims, the second dims are for other attributes
class OutputItem
{
public:
	Point<double>* pPtsArray; //, nDimsR
	Point<double>* pPtsArrayD; //, nDimsfor decode

	int _pt_alloc_size;
	int _actual_size;
	int _encode_mode;

	int nDims;
	int nDimsR;

	OutputItem(int dims, int dimsr)
	{
		_actual_size = 0;
		_encode_mode = 0;

		pPtsArray = NULL;
		pPtsArrayD = NULL;

		nDims = dims;
		nDimsR = dimsr;
	}
};

//template<int nDimsR>//this dim is for other attributes
class InputFilterD : public tbb::filter
{
private:
	FILE* input_file;
	int _size;

	int nDimsR;

public:
	InputFilterD(FILE* input_file_, int size, int dimsr) :
		filter(serial_in_order),
		input_file(input_file_),
		_size(size),
		nDimsR(dimsr)
	{
		///memset(ncount, 0, sizeof(int) * 20);
	}

	~InputFilterD()
	{

	}


	/*override*/ void* operator()(void*)
	{
		//tbb::tick_count t0;
		//t0 = tbb::tick_count::now();

		// Read raw points coornidates
		InputItem* pItem = (InputItem*)tbb::tbb_allocator<InputItem>().allocate(1);
		
		pItem->pKeys = (sfc_bigint*)tbb::tbb_allocator<sfc_bigint>().allocate(_size);

		if (nDimsR != 0)
			pItem->pPtsArray = (Point<double>*)tbb::tbb_allocator<Point<double>>().allocate(_size);//nDimsR

		pItem->_alloc_size = _size;

		char buf[1024];
		char * pch, *lastpos;
		char ele[64];

		int i, j;
		i = 0;

		if (nDimsR == 0) //pure keys
		{
			while (1) //always true
			{
				if (i == _size) break; //full, maximum _size;

				memset(buf, 0, 1024);
				fgets(buf, 1024, input_file);

				if (strlen(buf) == 0) break; // no more data

				memset(ele, 0, 64);
				strncpy(ele, buf, strlen(buf) - 1); //remove \n

				pItem->pKeys[i] = sfc_bigint(ele);

				i++;
			}//end while
		}
		else //with other attributes
		{
			while (1) //always true
			{
				if (i == _size) break; //full, maximum _size;

				j = 0;
				memset(buf, 0, 1024);
				fgets(buf, 1024, input_file);

				if (strlen(buf) == 0) break; // no more data

				lastpos = buf;
				pch = strchr(buf, ',');
				while (pch != NULL)
				{
					memset(ele, 0, 64);
					strncpy(ele, lastpos, pch - lastpos);
					//printf("found at %d\n", pch - str + 1);
					if (strlen(ele) != 0)
					{
						pItem->pPtsArray[i][j] = atof(ele);
						j++;
					}

					lastpos = pch + 1;
					pch = strchr(lastpos, ',');
				}

				if (strlen(lastpos) != 0 && strcmp(lastpos, "\n") != 0)//final part for key
				{
					memset(ele, 0, 64);
					strncpy(ele, lastpos, strlen(lastpos)-1);
					pItem->pKeys[i] = sfc_bigint(ele);//buflastpos
				}

				i++;
			}//end while
		}//end if check pure keys		

		pItem->_actual_size = i;

		if (i == 0) //no data are read here
		{
			if (nDimsR != 0)
				tbb::tbb_allocator<Point<double>>().deallocate((Point<double>*)pItem->pPtsArray, pItem->_alloc_size);//, nDimsR
			tbb::tbb_allocator<sfc_bigint>().deallocate((sfc_bigint*)pItem->pKeys, pItem->_alloc_size);
			
			tbb::tbb_allocator<InputItem>().deallocate((InputItem*)pItem, 1);//nDimsR


			return NULL; //read nothing here, terminate
		}

		//tbb::tick_count t1 = tbb::tick_count::now();
		//g_step1_time += (t1 - t0).seconds();
		return pItem;
	}
};

///////////////////////////////////////////////////
//new whole transfomartion filter
//template<int nDims, int mBits, int nDimsR>//this first dim is for decoding dims, the second dims are for other attributes
class DecodeFilter : public tbb::filter
{
public:
	int nDims;
	int mBits;
	int nDimsR;

	DecodeFilter(int sfctype, int conv_type, double* delta, double* scale, int dims, int bits, int dimsr) :
		tbb::filter(parallel),
		_sfctype(sfctype),
		_conv_type(conv_type),
		nDims(dims),
		mBits(bits),
		nDimsR(dimsr)
	{
			if (delta != NULL && scale != NULL)
			{
				_delta = delta;
				_scale = scale;
			}
	}

	/*override*/void* operator()(void* item)
	{
		//tbb::tick_count t0, t1;
		//t0 = tbb::tick_count::now();

		InputItem*  pin_item = static_cast<InputItem*>(item);//<nDimsR><nDimsR>
		Point<double>*  input = pin_item->pPtsArray;//, nDimsR

		//////////////////////////////<nDims, nDimsR>

		OutputItem* pout_item = (OutputItem*)tbb::tbb_allocator<OutputItem>().allocate(1);
		pout_item->pPtsArray = pin_item->pPtsArray;
		pout_item->_actual_size = pin_item->_actual_size;
		pout_item->_pt_alloc_size = pin_item->_alloc_size;
		pout_item->_encode_mode = _conv_type;

		pout_item->pPtsArrayD = (Point<double>*)tbb::tbb_allocator<Point<double>>().allocate(pin_item->_actual_size);//nDims
		//////////////////////////////////////////////////////
		CoordTransform<double, long> cotrans(nDims);

		SFCConversion sfcgen(nDims, mBits);
		//OutputSchema2<nDims - ex_dim, mBits> outtrans;

		if (_delta != NULL && _scale != NULL)
		{
			cotrans.SetTransform(_delta, _scale);
		}

		Point<long> ptSFC(nDims);
		sfc_bigint val;

		for (int i = 0; i < pin_item->_actual_size; i++)
		{
			if (_sfctype == 0) //morton
			{
				pout_item->pPtsArrayD[i] = cotrans.InverseTransform(sfcgen.MortonDecode(pin_item->pKeys[i])); //input[i]
				//ptBits = sfcgen.ptBits;
			}

			if (_sfctype == 1) //hilbert
			{
				pout_item->pPtsArrayD[i] = cotrans.InverseTransform(sfcgen.HilbertDecode(pin_item->pKeys[i])); //input[i]
				//ptBits = sfcgen.ptBits;
			}
		}

		////////////////
		tbb::tbb_allocator<sfc_bigint>().deallocate((sfc_bigint*)pin_item->pKeys, pout_item->_pt_alloc_size);
		tbb::tbb_allocator<InputItem>().deallocate((InputItem *)pin_item, 1); //nDimsRonly release the inputitem

		//t1 = tbb::tick_count::now();
		//g_step2_time += (t1 - t0).seconds();

		return pout_item;
	}

private:
	double* _delta;
	double* _scale;

	int _sfctype;
	int _conv_type;
};

//////////////////////////////////////////////////////////
//! Filter that writes each buffer to a file.
//template<>//this first dim is for decoding dims, the second dims are for other attributes
class OutputFilterD : public tbb::filter
{
private:
	//FILE* output_file;
	ostream& output_file;

public:

	int nDims;
	int nDimsR;

	OutputFilterD(ostream& output, int dims, int dimsr) : //FILE* output_file
		tbb::filter(serial_in_order), //parallel
		output_file(output),
		nDims(dims),
		nDimsR(dimsr)
	{
	}


	~OutputFilterD()
	{
	}

	/*override*/void* operator()(void* item)
	{
		tbb::tick_count t0, t1;
		t0 = tbb::tick_count::now();

		OutputItem*  pout_item = static_cast<OutputItem*>(item); //nDims, nDimsR

		///////////////////////
		for (int i = 0; i < pout_item->_actual_size; i++)
		{
			for (int j = 0; j < nDimsR; j++)
			{
				output_file << setprecision(9) << pout_item->pPtsArray[i][j];
				output_file << ",";
			}
			for (int j = 0; j < nDims-1; j++)
			{
				output_file << setprecision(9) << pout_item->pPtsArrayD[i][j];
				output_file << ",";
			}			
			output_file << setprecision(9) << pout_item->pPtsArrayD[i][nDims - 1] << endl;

		}///end for


		///////////////////////
		if (nDimsR != 0)
			tbb::tbb_allocator<Point<double>>().deallocate((Point<double>*)pout_item->pPtsArray, pout_item->_pt_alloc_size);//, nDimsR
		tbb::tbb_allocator<Point<double>>().deallocate((Point<double>*)pout_item->pPtsArrayD, pout_item->_pt_alloc_size); //, nDims

		tbb::tbb_allocator<OutputItem>().deallocate((OutputItem*)pout_item, 1); //<nDims, nDimsR>

		//t1 = tbb::tick_count::now();
		//g_step3_time += (t1 - t0).seconds();

		return NULL;
	}
};

template<int nDims, int mBits, int nDimsR>//this first dim is for decoding dims, the second dims are for other attributes
int run_decode_pipeline(int nthreads, char* InputFileName, char* OutputFileName, \
	int item_num, int sfc_type, int conv_type, double* delta, double* scale)
{
	FILE* input_file = NULL;
	if (InputFileName != NULL && strlen(InputFileName) != 0)
	{
		input_file = fopen(InputFileName, "r");
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
	if (OutputFileName != NULL && strlen(OutputFileName) != 0)
	{
		of.open(OutputFileName);
		out_s = &of;
	}
	else
	{
		out_s = &cout;
	}

	// Create the pipeline
	tbb::pipeline pipeline;

	// Create file-reading writing stage and add it to the pipeline
	InputFilterD* input_filter = NULL;
	input_filter = new InputFilterD(input_file, item_num,nDimsR); //
	pipeline.add_filter(*input_filter);

	DecodeFilter nsfcgen_filter(sfc_type, conv_type, delta, scale,nDims, mBits, nDimsR);
	//nsfcgen_filter.SetTransform(delta, scale);
	pipeline.add_filter(nsfcgen_filter);

	// Create file-writing stage and add it to the pipeline
	OutputFilterD output_filter(*out_s, nDims, nDimsR);//, nlodlevels, 20 //output_file
	pipeline.add_filter(output_filter);

	// Run the pipeline
	tbb::tick_count t0 = tbb::tick_count::now();
	// Need more than one token in flight per thread to keep all threads 
	// busy; 2-4 works
	pipeline.run(nthreads* 4);// 

	tbb::tick_count t1 = tbb::tick_count::now();

	//fclose(output_file);
	fclose(input_file);
	of.close();

	if (strlen(OutputFileName) != 0)
	{
		//printf("step1 INPUT  time:   %g\n", g_step1_time);
		//printf("step2 SFC_EN  time:   %g\n", g_step2_time);
		//printf("step3 OUTPUT    time:   %g\n", g_step3_time);
		printf("thread: %d ; time = %g\n", nthreads, (t1 - t0).seconds());
	}

	delete input_filter;

	return 1;
}

#endif