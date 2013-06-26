/*
 * playbackdevice.cxx
 *
 *  Created on: 24 Jun 2013
 *      Author: mike
 */

#include "samplesink.h"

/* Default push function converts short to float */
void SampleSink::push(float *samples, unsigned int nframes)
{
	short *sbuf = new short[nframes * _channels];

	for (unsigned int n = 0; n < nframes * _channels; n++)
		sbuf[n] = (short)(32767.0 * samples[n]);
	push(sbuf, nframes);
	delete[] sbuf;
}

/* Default encode function converts float to short */
void SampleSink::push(short *samples, unsigned int nframes)
{
	float *fbuf = new float[nframes * _channels];

	for (unsigned int n = 0; n < nframes * _channels; n++)
		fbuf[n] = (float)samples[n] / 32768.0;
	push(fbuf, nframes);
	delete[] fbuf;
}
