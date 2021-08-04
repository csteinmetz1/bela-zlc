/***** ZLConvolver.h *****/
/*
 ____  _____ _        _    
| __ )| ____| |      / \   
|  _ \|  _| | |     / _ \  
| |_) | |___| |___ / ___ \ 
|____/|_____|_____/_/   \_\

http://bela.io

*/

// This class encapsulates a zero-latency convolution.

#pragma once

#include <Bela.h>
#include <vector>
#include <string>
#include <memory>

#include "MonoFilePlayer.h"
#include "FFTConvolver.h"
#include "DirectConvolver.h"

class ZLConvolver {
public:
	// Constructors: the one with arguments automatically calls setup()
	ZLConvolver() {}
	ZLConvolver(int blockSize, int audioSampleRate, std::string impulseFilename, bool random, int kernelSize);
	
	// Create a zero-latency convolver. Returns true on success.
	bool setup(int blockSize, int audioSampleRate, std::string impulseFilename, bool random, int kernelSize);
	
	// After passing pointer to convolver, launch the convolver
	static void convolverLauncher(void * convolverPtr)
	{
		if (((FFTConvolver *) convolverPtr)->isQueued()){
			((FFTConvolver *) convolverPtr)->process();
		}
	}
	
	// Generate a random float between low and high
	static float randFloat(float low, float high)
	{
		float r = low + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(high-low)));
		return r;
	}
	
	float process(float in, float wet, float dry, bool nl, int maxBlocks, float sparsity);
	
private:
	
	bool random_;		// randomly generate the filter (not implemented)
	
	// FFT
	int N_; 									// base FFT size
	int blocks_;								// number of blocks for impulse (number of fftConvolvers)
	int basePriority_ = 90;						// base thread priority
	std::vector<FFTConvolver> fftConvolvers_;	// array of fftConvolvers
	DirectConvolver directConvolver_;			// single direct convolution for zero latency
	std::vector<int> convolverBufferSamples_;	// array of number of samples since last call for each convolver
	std::vector<AuxiliaryTask> convolverThreads_;// each convolver gets its own thread
	std::vector<int> convolverPriority_;		// array of priority values for each convolver thread

	// Input and Output circular buffers
	const int bufferSize_ = 131072;
	std::vector<float> inputBuffer_;
	int inputBufferPointer_ = 0;
	std::vector<float> outputBuffer_;
	int outputBufferReadPointer_ = 0;

};
