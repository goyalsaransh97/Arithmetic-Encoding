// g++ main.cpp encode.cpp decode.cpp compress.cpp  -I boost/include -lboost_iostreams
#include "encode.h"
#include <boost/iostreams/device/mapped_file.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
using namespace std;
vector<char> vec;


Encode::Encode(void)
{
	buffer = 0;
	bits_in_buf = 0;

	low = 0;
	high = MAX_VALUE;
	opposite_bits = 0;
}

Encode::~Encode(void)
{
}

void Encode::encode(char *infile, char *outfile)
{
	boost::iostreams::mapped_file_source file;
	long numberOfBytes = 143666240;
	file.open(infile, numberOfBytes,0);
	if(file.is_open()) {
		unsigned char * data = (unsigned char *)file.data(); 	
		for(long i = 0; i < numberOfBytes; i++)
		{
			int ch = (int) data[i];
			int symbol;
			symbol = char_to_index[ch];
			encode_symbol(symbol);
			update_tables(symbol);
		}
		encode_symbol(EOF_SYMBOL);
		end_encoding();
	}
	else {
		cout << "Error: Can`t open file" << endl;
		return;
	}
	file.close();

	FILE* out1 = fopen(outfile, "wb");
	fwrite(&vec[0], 1, vec.size(), out1);
	fclose(out1);
}

void Encode::encode_symbol(int symbol)
{
	int range;

	range = high - low;
	high = low + (range * cum_freq [symbol - 1]) / cum_freq [0];
	low = low + (range * cum_freq [symbol]) / cum_freq [0];
	for (;;)
	{
		if (high < HALF)
			output_bits(0);
		else if (low >= HALF)
		{
			output_bits(1);
			low -= HALF;
			high -= HALF;
		}
		else if (low >= FIRST_QTR && high < THIRD_QTR)
		{
			opposite_bits++;
			low -= FIRST_QTR;
			high -= FIRST_QTR;
		}
		else
			break;
		low = 2 * low;
		high = 2 * high;
	}
}

void Encode::end_encoding(void)
{
  opposite_bits++;
  if (low < FIRST_QTR)
    output_bits(0);
  else
    output_bits(1);

  vec.push_back(buffer >> bits_in_buf);
}

void Encode::output_bits(int bit)
{
	write_bit(bit);
	while (opposite_bits > 0)
	{
		write_bit(!bit);
		opposite_bits--;
	}
}

void Encode::write_bit(int bit)
{
	buffer >>= 1;
	if (bit)
		buffer |= 0x80;
	bits_in_buf++;
	if (bits_in_buf == 8)
	{
		vec.push_back(buffer);
		bits_in_buf = 0;
	}
}