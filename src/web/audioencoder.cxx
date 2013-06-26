/*
 * audioencoder.c
 *
 *  Created on: 21 Jun 2013
 *      Author: mike
 */

#include <vector>

#include "audioencoder.h"

/* Default encode function converts short to float for encoders that
 * don't implement encoding from short */
const vector<char> AudioEncoder::encode(float *samples, unsigned int nframes)
{
	short *sbuf = new short[nframes * _channels];
	unsigned int n;
	vector<char> out;

	for (n = 0; n < nframes * _channels; n++)
		sbuf[n] = (short)(32767.0 * samples[n]);
	out = encode(sbuf, nframes);
	delete[] sbuf;
	return out;
}

/* Default encode function converts float to short for encoders that
 * don't implement encoding from float */
const vector<char> AudioEncoder::encode(short *samples, unsigned int nframes)
{
	float *fbuf = new float[nframes * _channels];
	unsigned int n;
	vector<char> out;

	for (n = 0; n < nframes * _channels; n++)
		fbuf[n] = (float)samples[n] / 32768.0;
	out = encode(fbuf, nframes);
	delete[] fbuf;
	return out;
}
