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

#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

#include <fftw3.h>

#include "debug.h"
#include "lowpass.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* FIXME: Make runtime variable */
#define FIR_LENGTH 	64

LowPass::LowPass(const string &name) : DspBlock(name, "LowPass"),
	_firLength(FIR_LENGTH), spec(NULL), impulse(NULL),
	_passband(0),
	_reqDecimation(0),
	_reqOutputRate(DEFAULT_SAMPLE_RATE)
{

}

LowPass::~LowPass()
{

}

void LowPass::setPassband(unsigned int hz)
{
	_passband = hz;

	if (isRunning())
		recalculate();
}

void LowPass::setDecimation(unsigned int n)
{
	if (isRunning())
		return;

	_reqDecimation = n;
	_reqOutputRate = 0;
}

void LowPass::setOutputSampleRate(unsigned int hz)
{
	if (isRunning())
		return;

	_reqOutputRate = hz;
	_reqDecimation = 0;
}

bool LowPass::init()
{
	/* Calculate required decimation ratio based on whether the caller
	 * asked for a specific value, or for a specific output rate.
	 * DspBlock will enforce an integer decimation ratio. */
	if (_reqOutputRate > 0) {
		_outputSampleRate = _reqOutputRate;
	} else if (_reqDecimation > 0) {
		_outputSampleRate = inputSampleRate() / _reqDecimation;
	} else {
		LOG_ERROR("Must specify either decimation or output rate\n");
		return false;
	}
	_outputChannels = inputChannels();

	/* Set up FFTW for coefficient calculation */
	/* FIXME: Can probably use the r2c_1d plan for better performance */
	spec = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * _firLength);
	impulse = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * _firLength);
	p = fftwf_plan_dft_1d(_firLength, spec, impulse, FFTW_BACKWARD, FFTW_ESTIMATE);

	/* Pre-calculate window */
	/* FIXME: Using a rectangle is not really ideal! */
	window.resize(_firLength);
	for (unsigned int n = 0; n < _firLength; n++) {
		//window[n] = 1.0 / (float)_firLength; // also applies IFFT output scale factor
		// Hamming window (temp)
		window[n] = 0.54 - 0.46 * cosf(2 * M_PI * (float)n / (float)(_firLength - 1));
		window[n] /= (float)_firLength;
	}

	/* Generate initial coefficients */
	recalculate();

	return true;
}

void LowPass::deinit()
{
	fftwf_destroy_plan(p);
	fftwf_cleanup();
	fftwf_free(spec);
	fftwf_free(impulse);

	/* Release vector memory */
	vector<float>().swap(window);
	vector<float>().swap(coeff);
	vector<float>().swap(block);
}

bool LowPass::process(const vector<sample_t> &inBuffer, vector<sample_t> &outBuffer)
{
	unsigned int historySize = inputChannels() * (_firLength - 1);

	/* Push new block into processing buffer, retaining _firLength - 1 frames
	 * from the previous block.  This copy step simplifies the inner loop by
	 * ensuring current and previous input samples are contiguous. */
	if (block.size() != inBuffer.size() + historySize)
		block.resize(inBuffer.size() + historySize);
	vector<sample_t>::iterator it = block.begin();
	it = copy(block.end() - historySize, block.end(), it);
	copy(inBuffer.begin(), inBuffer.end(), it);

	/* Clear output buffer */
	fill(outBuffer.begin(), outBuffer.end(), 0.0);

	float *in = (float*)block.data();
	float *out = (float*)outBuffer.data();
	unsigned int nframes = outBuffer.size() / outputChannels();
	unsigned int instep = inputChannels() * decimation();
	while (nframes--) {
		float *inptr = in;
		for (vector<float>::reverse_iterator it = coeff.rbegin();
				it != coeff.rend(); ++it)
			for (unsigned int c = 0; c < inputChannels(); ++c)
				out[c] += (*it) * (*inptr++);
		out += outputChannels();
		in += instep;
	}

	return true;
}

void LowPass::recalculate()
{
	/* Determine cutoff bin for desired pass band */
	unsigned int maxbin = _firLength * _passband / inputSampleRate() / 2;

	/* Filter spec is pure-real so -ve frequency components are the
	 * same as +ve (conjugate, but no imaginary part).
	 */
	unsigned int mask = _firLength - 1;
	for (unsigned int n = 0; n < _firLength / 2 + 1; n++) {
		spec[n][0] = spec[(_firLength - n) & mask][0] =
				(n < maxbin) ? 1.0 : 0.0;
		spec[n][1] = spec[(_firLength - n) & mask][1] =
				0.0;
	}

	fftwf_execute(p);

	coeff.resize(_firLength);
	for (unsigned int n = 0; n < _firLength; n++) {
		unsigned int bin = (n + _firLength / 2) & (_firLength - 1);

		/* Response is real, so discard imaginary component,
		 * re-order into FIR coefficient vector, apply window */
		coeff[n] = impulse[bin][0] * window[n];
	}

#if 0
	/* Dump impulse response */
	for (vector<float>::iterator it = coeff.begin(); it != coeff.end(); ++it)
		LOG_DEBUG("%f\n", *it);

#endif
}
