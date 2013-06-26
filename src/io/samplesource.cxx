/*
 * capturedevice.cxx
 *
 *  Created on: 24 Jun 2013
 *      Author: mike
 */

#include "samplesource.h"

/* Default pull function converts short to float */
unsigned int SampleSource::pull(float *samples, unsigned int maxframes)
{
	short *sbuf = new short[maxframes * _channels];
	unsigned int nframes = pull(sbuf, maxframes);
	for (unsigned int n = 0; n < nframes * _channels; n++)
		samples[n] = (float)sbuf[n] / 32768.0;
	delete[] sbuf;
	return nframes;
}

/* Default encode function converts float to short */
unsigned int SampleSource::pull(short *samples, unsigned int maxframes)
{
	float *fbuf = new float[maxframes * _channels];
	unsigned int nframes = pull(fbuf, maxframes);
	for (unsigned int n = 0; n < nframes * _channels; n++)
		samples[n] = (short)(fbuf[n] * 32767.0);
	delete[] fbuf;
	return nframes;
}
