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

#include <cmath>

#include <stdint.h>

#include "debug.h"
#include "downconverter.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* NCO */
#define PHASE_BITS		31
#define LOOKUP_BITS		16

#define PHASE_MASK		((1UL << PHASE_BITS) - 1)
#define LOOKUP_MASK		((1UL << LOOKUP_BITS) - 1)
#define LOOKUP_SHIFT	(PHASE_BITS - LOOKUP_BITS)

/* CIC */

#define CIC_ORDER		4
#define DEFAULT_SAMPLE_RATE		48000

DownConverter::DownConverter(const string &name) :
	DspBlock(
			DspData::Int16, DspData::Float,
			name, "DownConverter"),
	_if(0),
	phase(0), phaseStep(0),
	integrator(CIC_ORDER * 2), comb(CIC_ORDER * 2), combdelay(CIC_ORDER * 2),
	_reqDecimation(0),
	_reqOutputRate(DEFAULT_SAMPLE_RATE)
{
	/* Calculate sine table for fast NCO */
	sinTable.resize(1UL << LOOKUP_BITS);
	for(unsigned int n = 0; n < sinTable.size(); n++)
		//sinTable[n] = sinf((float)n * 2 * M_PI / (float)(1UL << LOOKUP_BITS));
		sinTable[n] = (int32_t)(sinf((float)n * 2 * M_PI / (float)(1UL << LOOKUP_BITS)) * 32767.0);
}

DownConverter::~DownConverter()
{

}

void DownConverter::setIF(int hz)
{
	_if = hz;

	if (isRunning()) {
		/* Update phase accumulator step */
		phaseStep = (int)((int64_t)hz * (int64_t)(1UL << PHASE_BITS) / (int64_t)inputSampleRate());
	}
}

void DownConverter::setDecimation(unsigned int n)
{
	if (isRunning())
		return;

	_reqDecimation = n;
	_reqOutputRate = 0;
}

void DownConverter::setOutputSampleRate(unsigned int hz)
{
	if (isRunning())
		return;

	_reqOutputRate = hz;
	_reqDecimation = 0;
}

bool DownConverter::init()
{
	if (inputChannels() != 2) {
		LOG_ERROR("Expected IQ input\n");
		return false;
	}

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

	/* Calculate initial phase step */
	phaseStep = (int)((int64_t)_if * (int64_t)(1UL << PHASE_BITS) / (int64_t)inputSampleRate());
	LOG_DEBUG("phaseStep = %d for %d Hz\n", phaseStep, _if);

	return true;
}

void DownConverter::deinit()
{

}

/* CIC bit growth:
 *
 * The bit growth in the integrator is given by:
 *
 * N log2 (RM)
 *
 * where:
 * N = number of stages in the filter (CIC_ORDER)
 * R = the decimation ratio
 * M = differential delay (taps), 1 in this case
 *
 * For a typical decimation of 2.4 MHz down to 32 kHz (75 times)
 * with 16-bit input width and a 4th order filter, the accumulators
 * need length:
 *
 * n = 16 + 4 log2 (75) = 41 bits
 */

bool DownConverter::process(const DspData &in, DspData &out)
{
	// FIXME: This needs to persist across blocks so that decimation
	// ratio doesn't need to have an integer relationship to the
	// input block size, but that may mean the output size is dynamic
	unsigned int dec = 0;

	out.resize(in.size() / decimation());
	
	const short *inptr = (const short*)in.data();
	float *outptr = (float*)out.data();
	int64_t scale = (int64_t)pow(decimation(), CIC_ORDER);

	for (unsigned int n = 0; n < in.size() / 2; n++) {
		/* NCO */
		unsigned int sinidx, cosidx;
		sinidx = phase >> LOOKUP_SHIFT;
		cosidx = (sinidx + (1UL << LOOKUP_BITS) / 4) & LOOKUP_MASK;
		phase = (phase + phaseStep) & PHASE_MASK;

		/* Complex mixer - signal is multiplied by the complex conjugate of the
		 * local oscillator */
		int32_t i = (int32_t)(*inptr++);
		int32_t q = (int32_t)(*inptr++);
		integrator[0] += (int64_t)((i * sinTable[cosidx] + q * sinTable[sinidx]) >> 15); // I
		integrator[1] += (int64_t)((q * sinTable[cosidx] - i * sinTable[sinidx]) >> 15); // Q

		/* CIC decimator */
		for (unsigned int m = 2; m < 2 * CIC_ORDER; m++)
			integrator[m] += integrator[m - 2];

		if (++dec == decimation()) {
			dec = 0;

			/* Comb */
			comb[0] = integrator[2 * CIC_ORDER - 2] - combdelay[0];
			comb[1] = integrator[2 * CIC_ORDER - 1] - combdelay[1];
			combdelay[0] = integrator[2 * CIC_ORDER - 2];
			combdelay[1] = integrator[2 * CIC_ORDER - 1];
			for (unsigned int m = 2; m < 2 * CIC_ORDER; m++) {
				comb[m] = comb[m - 2] - combdelay[m];
				combdelay[m] = comb[m - 2];
			}
			/* Output converted to float */
			*outptr++ = (float)(comb[2 * CIC_ORDER - 2] / scale) / 32768.0;
			*outptr++ = (float)(comb[2 * CIC_ORDER - 1] / scale) / 32768.0;
		}
	}

	return true;
}

