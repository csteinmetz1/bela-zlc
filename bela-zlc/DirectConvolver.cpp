/***** DirectConvolver.cpp *****/

#include "DirectConvolver.h"

// Constructor taking the path of a file to load
DirectConvolver::DirectConvolver(std::vector<float> &h, int k, std::vector<float> &x, std::vector<float> &y)
{
	setup(h, k, x, y);
}

// Load an audio file from the given filename. Returns true on success.
bool DirectConvolver::setup(std::vector<float> &h, int k, std::vector<float> &x, std::vector<float> &y)
{
	// store public member values
	k_ = k;
	x_ = &x;
	y_ = &y;
	outPointer_ = k_;

	// load the impulse response block
	for (int n = 0; n < h.size(); n++)
	{
		h_.push_back(h[n]);
	}

	return true;
}

// Apply the filter h to an input sample in the time domain
void DirectConvolver::process(unsigned int inPointer)
{
	float out = 0;

	// iterate over the coefficients of the filter h
	for (int i = 0; i < h_.size(); i++)
	{
		int circularBufferIndex = (inPointer - i + x_->size()) % x_->size();
		out += x_->at(circularBufferIndex) * h_[i];
	}

	// write output sample to the output circular buffer
	int circularBufferIndex = (outPointer_ + y_->size()) % y_->size();
	y_->at(circularBufferIndex) += out;

	// update the write pointer one sample ahead
	outPointer_ = (outPointer_ + 1) % y_->size();
}
