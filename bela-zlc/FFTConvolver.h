/*
 ____  _____ _        _    
| __ )| ____| |      / \   
|  _ \|  _| | |     / _ \  
| |_) | |___| |___ / ___ \ 
|____/|_____|_____/_/   \_\

http://bela.io

*/

// This class encapsulates a block based FFT convolution.

#pragma once

#include <libraries/Fft/Fft.h>
#include <Bela.h>
#include <vector>
#include <string>
#include <memory>

class FFTConvolver {
public:
	// Constructors: the one with arguments automatically calls setup()
	FFTConvolver() {}
	FFTConvolver(int fftSize, std::vector<float>& h, int k, std::vector<float>& x, std::vector<float>& y);
	
	// Load an audio file from the given filename. Returns true on success.
	bool setup(int fftSize, std::vector<float>& h, int k, std::vector<float>& x, std::vector<float>& y);
	
	// check if the convolver has been queued
	bool isQueued(void);
	
	// retrieve the FFT size
	int getFftSize(void);
	
	// Queue a block by passing the starting location in the input buffer
	void queue(unsigned int inPointer, bool bypass);
	
	// Process a block
	void process();
	
	// Destructor
	~FFTConvolver() {}
	
private:
	
	// FFT objects
	std::shared_ptr<Fft> fftBuffer, fftH, fftX;
	
	bool queued_;			// whether the filter block samples are ready
	int fftSize_;			// size of the fft with h = fftSize/2
	int k_;			 		// block (sample) offset within the complete filter
	bool bypass_;			// do not process, but update the write pointer
	
	std::vector<float>* x_;		// pointer to input circular buffer
	unsigned int inPointer_;	// read position within the input circular buffer
	std::vector<float>* y_;		// pointer to the output circular buffer
	unsigned int outPointer_;	// write position within the output circular buffer
	
};
