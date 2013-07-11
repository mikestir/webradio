/*
 * audioencoder.cxx
 *
 *  Created on: 21 Jun 2013
 *      Author: mike
 */

#include <vector>
#include <lame/lame.h>

#include "debug.h"
#include "audioencoder.h"

MP3Encoder::MP3Encoder(unsigned int samplerate, unsigned int channels) :
	AudioEncoder(samplerate, channels)
{
	LOG_DEBUG("Configuring for %u Hz %u channels\n", samplerate, channels);

	state = lame_init();
	lame_set_in_samplerate(state, samplerate);
	lame_set_num_channels(state, channels);
	// FIXME: Not declaring MONO and passing the buffer twice seems
	// to be necessary here
	//lame_set_mode(state, channels > 1 ? JOINT_STEREO : MONO);
	lame_set_mode(state, JOINT_STEREO);
	lame_set_VBR(state, vbr_default);
	lame_set_VBR_quality(state,2);
//	lame_set_brate(state,128);
	lame_init_params(state);
}

MP3Encoder::~MP3Encoder()
{
	lame_close(state);
}

const vector<char> MP3Encoder::encode(const vector<float> &interleaved)
{
	unsigned int inframes = interleaved.size() / _channels;
	vector<float> left(inframes), right(inframes);
	vector<char> buffer(inframes * 5 / 4 + 7200); // worst case size

	if (_channels > 2) {
		LOG_ERROR("Unsupported number of channels %u\n", _channels);
		return vector<char>();
	}

	/* Format conversion (LAME wants +/-32768) */
	const float *ptr = interleaved.data();
	for (unsigned int n = 0; n < inframes; n++) {
		left[n] = (*ptr++) * 32768.0;
		if (_channels > 1)
			right[n] = (*ptr++) * 32768.0;
		else
			right[n] = left[n];
	}

	int size = lame_encode_buffer_float(state, left.data(), right.data(), inframes,
			(unsigned char*)buffer.data(), buffer.size());
	if (size < 0) {
		LOG_ERROR("LAME error %d\n", size);
		return vector<char>();
	}

	buffer.resize(size);
	return buffer;
}
