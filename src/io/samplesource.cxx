/*
 * capturedevice.cxx
 *
 *  Created on: 24 Jun 2013
 *      Author: mike
 */

#include <vector>

#include "samplesource.h"

/* Default pull function converts short to float */
unsigned int SampleSource::pull(float *samples, unsigned int maxframes)
{
	vector<short> sbuf(maxframes * _channels);
	unsigned int nframes = pull(sbuf.data(), maxframes);
	for (unsigned int n = 0; n < nframes * _channels; n++)
		samples[n] = (float)sbuf[n] / 32768.0;
	return nframes;
}

/* Default encode function converts float to short */
unsigned int SampleSource::pull(short *samples, unsigned int maxframes)
{
	vector<float> fbuf(maxframes * _channels);
	unsigned int nframes = pull(fbuf.data(), maxframes);
	for (unsigned int n = 0; n < nframes * _channels; n++)
		samples[n] = (short)(fbuf[n] * 32767.0);
	return nframes;
}
