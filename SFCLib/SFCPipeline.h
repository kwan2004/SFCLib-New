#ifndef SFCPIPELINE_H_
#define SFCPIPELINE_H_

#include "tbb/pipeline.h"
#include "tbb/tick_count.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/tbb_allocator.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
//#include "common/utility/utility.h"

#include <iostream>
#include <fstream>

#include "Point.h"
#include "CoordTransform.h"

//#include "SFCConversion2.h"
#include "SFCConversion.h"
#include "OutputSchema2.h"

#include "RandomLOD.h"

static double	g_step1_time = 0.0f; //input
static double	g_step2_time = 0.0f; //encoding
static double	g_step3_time = 0.0f; //output

//template<int nDims>
class InputItem
{
public:
	Point<double>* pPtsArray; //, nDims
	int _actual_size;
	int _alloc_size;
	int nDims;
	
	InputItem(int dims)
	{
		nDims = dims;
		_alloc_size = 0;
		_actual_size = 0;

		pPtsArray = NULL;
	}
};

//template<int nDims>
class OutputItem
{
public:
	Point<double>* pPtsArray;

	sfc_bigint* out_value;
	char* out_string;

	int _pt_alloc_size;
	int _actual_size;
	int _encode_mode;
	int _str_len;

	int nDims;

	OutputItem(int dims)
	{
		nDims = dims;

		_actual_size = 0;
		_encode_mode = 0;

		_str_len = 0;

		pPtsArray = NULL;
		out_value = NULL;
		out_string = NULL;
	}
};

//template<int nDims>
class InputFilter : public tbb::filter
{
private:
	FILE* input_file;
	int _size;

	bool bis_lod;
	RandomLOD*  p_rnd_gen;

	int nDims;

	//int ncount[20];

public:
	InputFilter(FILE* input_file_, int size, int dims) :
		filter(serial_in_order),
		input_file(input_file_),
		_size(size),
		bis_lod(false),
		p_rnd_gen(NULL),
		nDims(dims)
	{
		///memset(ncount, 0, sizeof(int) * 20);
	}

	InputFilter(FILE* input_file_, int size,  int levels, int levelmax, int dims) :
		filter(serial_in_order),
		input_file(input_file_),
		_size(size),
		bis_lod(true),
		nDims(dims)
	{
		//p_rnd_gen = new RandomLOD<nDims-1>(levels, levelmax);
		p_rnd_gen = new RandomLOD(levels, levelmax, dims-1);

		//memset(ncount, 0, sizeof(int) * 20);
	}

	~InputFilter()
	{
		if (p_rnd_gen == NULL)
		{
			delete p_rnd_gen;
		}

		/*for (int i = 0; i < 20; i++)
		{
			cout << i << ":" << ncount[i] << endl;
		}*/
	}


	/*override*/ void* operator()(void*)
	{
		//tbb::tick_count t0;
		//t0 = tbb::tick_count::now();

		// Read raw points coornidates
		InputItem* pItem = (InputItem*)tbb::tbb_allocator<InputItem>().allocate(1);
		pItem->pPtsArray = (Point<double>*)tbb::tbb_allocator<Point<double>>().allocate(_size);
		pItem->_alloc_size = _size;
		pItem->nDims = nDims;

		char buf[1024];
		char * pch, * lastpos;
		char ele[64];

		int i, j ;
		i = 0;
		while (1) //always true
		{
			if (i == _size) break; //full, maximum _size;

			j = 0;
			memset(buf, 0, 1024);
			fgets(buf,1024, input_file);

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

			if (strlen(lastpos) != 0 && strcmp(lastpos, "\n") != 0)//final part
			{
				pItem->pPtsArray[i][j] = atof(lastpos);
				j++;
			}

			if (bis_lod)
			{
				pItem->pPtsArray[i][j] =  p_rnd_gen->RLOD_Gen();

				//int a = pItem->pPtsArray[i][j];
				//ncount[a]++;
			}

			i++;
		}

		pItem->_actual_size = i;

		if (i == 0)
		{
			tbb::tbb_allocator<Point<double>>().deallocate((Point<double>*)pItem->pPtsArray, pItem->_alloc_size);
			tbb::tbb_allocator<InputItem>().deallocate((InputItem*)pItem, 1);
			
			return NULL; //read nothing here, terminate
		}

		//tbb::tick_count t1 = tbb::tick_count::now();
		//g_step1_time += (t1 - t0).seconds();

		return pItem;
	}
};

///////////////////////////////////////////////////
//new whole transfomartion filter
//template<int nDims>
class NewSFCGenFilter : public tbb::filter
{
public:
	NewSFCGenFilter(int sfctype, int conv_type, double* delta, double* scale, int dims, int mBits) :
		tbb::filter(parallel),
		_sfctype(sfctype),
		_conv_type(conv_type),
		nDims(dims),
		mBits(mBits)
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

		InputItem *  pin_item = static_cast<InputItem *>(item);
		Point<double>*  input = pin_item->pPtsArray;
		
		//////////////////////////////
		int base;
		int nstrlen;
		int ntotalbits = mBits * nDims;
		if (_conv_type == 1)
		{
			base = 5;
			nstrlen = (ntotalbits % base) ? (ntotalbits / base + 2) : (ntotalbits / base + 1); //the last is for \0
		}
		if (_conv_type == 2)
		{
			base = 6;
			nstrlen = (ntotalbits % base) ? (ntotalbits / base + 2) : (ntotalbits / base + 1); //the last is for \0
		}
		
		OutputItem* pout_item = (OutputItem*)tbb::tbb_allocator<OutputItem>().allocate(1);
		pout_item->pPtsArray = pin_item->pPtsArray;
		pout_item->_actual_size = pin_item->_actual_size;
		pout_item->_pt_alloc_size = pin_item->_alloc_size;
		pout_item->_encode_mode = _conv_type;
		if (_conv_type == 0)
			pout_item->out_value = (sfc_bigint*)tbb::tbb_allocator<sfc_bigint>().allocate(pin_item->_actual_size);
		else
		{ 
			pout_item->out_string = (char*)tbb::tbb_allocator<char>().allocate(pin_item->_actual_size * nstrlen);
			pout_item->_str_len = nstrlen;

			memset(pout_item->out_string, 0, pin_item->_actual_size * nstrlen); ///initialize to zero
		}

		//////////////////////////////////////////////////////
		const int ex_dim = 0; //exclude defined columns

		CoordTransform<double, long> cotrans(nDims - ex_dim);

		SFCConversion sfcgen(nDims - ex_dim, mBits);//<nDims - ex_dim, mBits>
		OutputSchema2 outtrans(nDims - ex_dim, mBits);//<nDims - ex_dim, mBits>

		if (_delta != NULL && _scale != NULL)
		{
			cotrans.SetTransform(_delta, _scale);
		}

		Point<long > ptSFC(nDims - ex_dim); //
		sfc_bigint val;

		for (int i = 0; i < pin_item->_actual_size; i++)
		{
			Point<double> pt_small(nDims - ex_dim);//exclude the defined columns
			for (int q = 0; q < nDims - ex_dim; q++)
				pt_small[q] = input[i][q];

			if (_sfctype == 0) //morton
			{
				ptSFC = cotrans.Transform(pt_small); //input[i]
				val = sfcgen.MortonEncode(ptSFC);//, nDims - ex_dim, mBits
				//ptBits = sfcgen.ptBits;
			}

			if (_sfctype == 1) //hilbert
			{
				ptSFC = cotrans.Transform(pt_small); //input[i]
				val = sfcgen.HilbertEncode(ptSFC);//, nDims - ex_dim, mBits
				//ptBits = sfcgen.ptBits;
			}

			if (_conv_type == 0)
			{
				pout_item->out_value[i] = val;//outtrans.BitSequence2Value(ptBits);
			}

			//if (_conv_type == 1)
			//{
			//	strcpy(pout_item->out_string + i* nstrlen, outtrans.Value2String(val, Base32).c_str());
			//	//pout_item->out_string[i] = "a";//outtrans.BitSequence2String(ptBits, Base32);
			//}

			//if (_conv_type == 2)
			//{
			//	strcpy(pout_item->out_string + i* nstrlen, outtrans.Value2String(val, Base64).c_str());
			//	//pout_item->out_string[i] = "a";//outtrans.BitSequence2String(ptBits, Base64);
			//}
		}

		////////////////
		tbb::tbb_allocator<InputItem>().deallocate((InputItem*)pin_item, 1); //only release the inputitem

		//t1 = tbb::tick_count::now();
		//g_step2_time += (t1 - t0).seconds();

		return pout_item;
	}

private:
	double* _delta;
	double* _scale;

	int mBits;
	int nDims;

	int _sfctype;
	int _conv_type;
};

//////////////////////////////////////////////////////////
//! Filter that writes each buffer to a file.
//template<int nDims>
class OutputFilter : public tbb::filter
{
private:
	//FILE* output_file;
	ostream& output_file;

	bool bis_onlysfc;
	int nDims;

	//bool bis_lod;
	//RandomLOD<nDims>*  p_rnd_gen;

public:
	OutputFilter(ostream& output, bool is_onlysfc, int dims) : //FILE* output_file
		tbb::filter(serial_in_order),
		output_file(output),
		bis_onlysfc(is_onlysfc),
		nDims(dims)
	{
	}


	~OutputFilter()
	{
	}

	/*override*/void* operator()(void* item)
	{
		//tbb::tick_count t0, t1;
		//t0 = tbb::tick_count::now();

		OutputItem*  pout_item = static_cast<OutputItem*>(item);

		if (bis_onlysfc) //only output sfc code
		{
			for (int i = 0; i < pout_item->_actual_size; i++)
			{
				if (pout_item->_encode_mode == 0) ///value type
					//fprintf(output_file, "%lu", pout_item->out_value[i]);
					output_file << pout_item->out_value[i];
				else
				{
					//fprintf(output_file, "%s", pout_item->out_string + i * pout_item->_str_len); //pout_item->out_string[i].c_str()
					output_file << pout_item->out_string + i * pout_item->_str_len;
				}

				//fprintf(output_file, "\n");
				output_file << endl;
			}//end for
		}
		else
		{
			///////////////////////
			for (int i = 0; i < pout_item->_actual_size; i++)
			{
				for (int j = 0; j < nDims; j++)
				{
					//fwrite(input[i], sizeof(long), 1, my_output_file);
					//fprintf(output_file, "%.6f", pout_item->pPtsArray[i][j]);
					//fprintf(output_file, ",");
					output_file << setprecision(9) << pout_item->pPtsArray[i][j];
					output_file << ",";
				}

				// one field for SFC code
				if (pout_item->_encode_mode == 0) ///value type
					//fprintf(output_file, "%lu", pout_item->out_value[i]);
					output_file << pout_item->out_value[i];
				else
				{
					//fprintf(output_file, "%s", pout_item->out_string + i * pout_item->_str_len); //pout_item->out_string[i].c_str()
					output_file << pout_item->out_string + i * pout_item->_str_len;
				}

				////one field for lod value
				//if (bis_lod)
				//{
				//	fprintf(output_file, ",%d", p_rnd_gen->RLOD_Gen());
				//}

				//fprintf(output_file, "\n");
				output_file << endl;
			}///end for
		}///end if

		///////////////////////
		tbb::tbb_allocator<Point<double>>().deallocate((Point<double>*)pout_item->pPtsArray, pout_item->_pt_alloc_size);
				
		if (pout_item->_encode_mode == 0)
			tbb::tbb_allocator<long>().deallocate((long*)pout_item->out_value, pout_item->_actual_size);
		else
			tbb::tbb_allocator<char>().deallocate((char*)pout_item->out_string, pout_item->_actual_size * pout_item->_str_len);

		tbb::tbb_allocator<OutputItem>().deallocate((OutputItem*)pout_item, 1);

		//t1 = tbb::tick_count::now();
		//g_step3_time += (t1 - t0).seconds();

		return NULL;
	}
};

//template<int nDims, int mBits>
int run_pipeline(int nDims, int mBits, int nthreads, char* InputFileName, char* OutputFileName, \
	int item_num, int sfc_type, int conv_type, double* delta, double* scale, bool onlysfc, bool bgenlod, int nlodlevels)
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
	
	//FILE* output_file = NULL;
	//if (OutputFileName != NULL && strlen(OutputFileName) != 0)
	//{
	//	output_file = fopen(OutputFileName, "w");
	//	if (!output_file)
	//	{
	//		return 0;
	//	}
	//}
	//else
	//{
	//	output_file = stdout;
	//}
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
	InputFilter* input_filter=NULL;
	if (bgenlod) //should generate lod value for each point
	{		
		input_filter = new InputFilter(input_file, item_num, nlodlevels, 20, nDims);
	}
	else
	{		
		input_filter = new InputFilter(input_file, item_num, nDims);
	}
	pipeline.add_filter(*input_filter);
	

	// Create squaring stage and add it to the pipeline
	//CoordTransFilter<> coordtrans_filter;
	//pipeline.add_filter(coordtrans_filter);

	//SFCGenFilter<> sfcgen_filter;
	//pipeline.add_filter(sfcgen_filter);

	//BitsConvFilter<> bitsconv_filter;
	//pipeline.add_filter(bitsconv_filter);
	NewSFCGenFilter nsfcgen_filter(sfc_type, conv_type, delta, scale, nDims, mBits);
	//nsfcgen_filter.SetTransform(delta, scale);
	pipeline.add_filter(nsfcgen_filter);

	// Create file-writing stage and add it to the pipeline
	OutputFilter output_filter(*out_s, onlysfc, nDims);//, nlodlevels, 20 //output_file
	pipeline.add_filter(output_filter);

	// Run the pipeline
	tbb::tick_count t0 = tbb::tick_count::now();
	// Need more than one token in flight per thread to keep all threads 
	// busy; 2-4 works
	pipeline.run(nthreads);// * 4

	tbb::tick_count t1 = tbb::tick_count::now();

	//fclose(output_file);
	fclose(input_file);
	of.close();


	if (strlen(OutputFileName) != 0)
	{
		//printf("step1 INPUT  time:   %g\n", g_step1_time);
		//printf("step2 SFC_EN  time:   %g\n", g_step2_time);
		//printf("step3 OUTPUT    time:   %g\n", g_step3_time);
		printf("time = %g\n", (t1 - t0).seconds());
	}

	delete input_filter;
	
	return 1;
}

#endif