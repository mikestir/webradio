/*
 * WebRadio web-based Software Defined Radio
 *
 * Copyright (C) 2013 Mike Stirling
 *
 * This file is part of WebRadio (http://www.mike-stirling.com/webradio)
 *
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SPECTRUMSINK_H_
#define SPECTRUMSINK_H_

#include <vector>
#include <string>
#include <fftw3.h>
#include <pthread.h>

#include "ioblock.h"

#define DEFAULT_FFT_SIZE		512

using namespace std;

/* NOTE: Using single-precision version of FFTW since e.g. the Pi doesn't
 * have a double-precision FPU.  The incoming interleaved I/Q samples are
 * copied directly to the FFTW buffer, so if using double-precision is
 * desired then the entire app needs to be updated to pass around doubles
 * instead of floats! */

class SpectrumSink : public IOBlock
{
public:
	SpectrumSink(const string &name = "<undefined>");
	virtual ~SpectrumSink();

	unsigned int fftSize() const { return _fftSize; }
	void setFftSize(unsigned int size);

	void getSpectrum(float *magnitudes); // FIXME: Better for clients to register for a callback?
private:
	bool init();
	void deinit();
	bool process(const DspData &in, DspData &out);

	fftwf_complex *inbuf;
	fftwf_complex *outbuf;
	fftwf_plan p;
	unsigned int inoffset;
	
	pthread_mutex_t mutex;
	
	unsigned int _fftSize;
	vector<float> window;
};

#endif /* SPECTRUMSINK_H_ */
