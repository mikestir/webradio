/*
 * audioencoder.cxx
 *
 *  Created on: 21 Jun 2013
 *      Author: mike
 */

#include <vector>
#include <lame/lame.h>

#include "audioencoder.h"

#define ENCODER_BUF_SIZE		8192

MP3Encoder::MP3Encoder(unsigned int samplerate, unsigned int channels) :
	AudioEncoder(samplerate, channels)
{
	state = lame_init();
	lame_set_in_samplerate(state, samplerate);
	lame_set_num_channels(state, channels);
	// FIXME: Not declaring MONO and passing the buffer twice seems
	// to be necessary here
	//lame_set_mode(state, channels > 1 ? JOINT_STEREO : MONO);
	lame_set_mode(state, JOINT_STEREO);
	lame_set_VBR(state, vbr_default);
	lame_init_params(state);
}

MP3Encoder::~MP3Encoder()
{
	lame_close(state);
}

const vector<char> MP3Encoder::encode(short *samples, unsigned int nframes)
{
	unsigned char buffer[ENCODER_BUF_SIZE];
	int size;

	if (_channels > 1) {
		size = lame_encode_buffer_interleaved(state, samples, nframes,
				buffer, ENCODER_BUF_SIZE);
	} else {
		size = lame_encode_buffer(state, samples, samples, nframes,
				buffer, ENCODER_BUF_SIZE);
	}

	return vector<char>(buffer, buffer + size);
}
