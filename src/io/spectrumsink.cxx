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
#include <algorithm>

#include <fftw3.h>
#include <math.h>

#include "debug.h"
#include "spectrumsink.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* NOTE: Accepts either real or interleaved IQ samples depending on the
 * number of channels specified */
SpectrumSink::SpectrumSink(const string &name) :
			IOBlock(DspData::Float, DspData::None, name, "SpectrumSink"),
			inbuf(NULL), outbuf(NULL),
			_fftSize(DEFAULT_FFT_SIZE)
{
	pthread_mutex_init(&mutex, NULL);
}

SpectrumSink::~SpectrumSink()
{
	pthread_mutex_destroy(&mutex);
}

void SpectrumSink::setFftSize(unsigned int size)
{
	if (isRunning())
		return;

	if (size & (size - 1)) {
		LOG_ERROR("size must be a power of 2\n");
		return;
	}
	_fftSize = size;
}

bool SpectrumSink::init()
{
	// FIXME: Might there be a case for real inputs?
	if (inputChannels() != 2) {
		LOG_ERROR("Expected IQ input\n");
		return false;
	}

	/* FIXME: Handle real->complex for spectrum of real signals */
	inbuf = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * _fftSize);
	outbuf = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * _fftSize);
	p = fftwf_plan_dft_1d(_fftSize, inbuf, outbuf, FFTW_FORWARD, FFTW_ESTIMATE);

	/* Calculate Hamming window */
	window.resize(_fftSize);
	for (unsigned int n = 0; n < _fftSize; n++) {
		window[n] = 0.54 - 0.46 * cosf(2 * M_PI * (float)n / (float)(_fftSize - 1));
	}

	return true;
}

void SpectrumSink::deinit()
{
	fftwf_destroy_plan(p);
	fftwf_cleanup();
	fftwf_free(inbuf);
	fftwf_free(outbuf);
	inbuf = outbuf = NULL;
}

bool SpectrumSink::process(const DspData &in, DspData &out)
{
	const float *inptr = (const float*)in.data();
	
	// FIXME: Support for real (1 channel) inputs?

	if (in.size() / 2 < _fftSize) {
		LOG_ERROR("Block size must be >= FFT size\n");
		return false;
	}

	/* NOTE: Must use float version of FFTW because we expect
	 * fftw_complex to be float[2] not double[2] to match our
	 * sample type.  This code should work for both real and
	 * complex DFTs (_channels = 1 or 2) as long as the plan is
	 * configured correctly first.
	 *
	 * Since this is asynchronous with the browser we just
	 * FFT the first FFTSIZE samples of each block
	 */

	/* Copy input buffer and apply window */
	for (unsigned int n = 0; n < _fftSize; n++) {
		inbuf[n][0] = *inptr++ * window[n];
		inbuf[n][1] = *inptr++ * window[n];
	}

	pthread_mutex_lock(&mutex);
	fftwf_execute(p);
	pthread_mutex_unlock(&mutex);

	return true;
}

void SpectrumSink::getSpectrum(float *magnitudes)
{
	float scaledb = 20 * log10f((float)_fftSize);

	/* Re-order bins into ascending order of frequency:
	 * IFFT output in standard order:
	 * bin:  0  1 ... N/2  N/2+1 ... N-1
	 * freq: DC 1 ... N/2 -N/2-1 ... -1
	 * For an even-length DFT the N/2 bin is both +ve and -ve
	 * Nyquist (N/2) frequency.
	 */
	pthread_mutex_lock(&mutex);
	for (unsigned int n = 0; n < _fftSize; n++) {
		float db = 10 * log10f(outbuf[n][0] * outbuf[n][0] + outbuf[n][1] * outbuf[n][1]);
		magnitudes[(n < _fftSize / 2) ? (n + _fftSize / 2) : (n - _fftSize / 2)] = db - scaledb;
	}
	pthread_mutex_unlock(&mutex);
}
