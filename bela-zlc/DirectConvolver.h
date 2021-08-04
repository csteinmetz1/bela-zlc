/***** DirectConvolver.h *****/
/*
 ____  _____ _        _    
| __ )| ____| |      / \   
|  _ \|  _| | |     / _ \  
| |_) | |___| |___ / ___ \ 
|____/|_____|_____/_/   \_\

http://bela.io

*/

// This class encapsulates a direct convolution.

#pragma once

#include <Bela.h>
#include <vector>
#include <string>
#include <memory>

class DirectConvolver
{
public:
	// Constructors: the one with arguments automatically calls setup()
	DirectConvolver() {}
	DirectConvolver(std::vector<float> &h, int k, std::vector<float> &x, std::vector<float> &y);

	// Create a direct convolution object. Returns true on success.
	bool setup(std::vector<float> &h, int k, std::vector<float> &x, std::vector<float> &y);

	void process(unsigned int inPointer);

	// Destructor
	~DirectConvolver() {}

private:
	int k_;					  // block (sample) offset within the complete filter
	std::vector<float> *x_;	  // pointer to the input circular buffer
	std::vector<float> *y_;	  // pointer to the output circular buffer
	std::vector<float> h_;	  // internal copy of the filter coefficients
	unsigned int outPointer_; // tracking position to write in the output buffer
};
