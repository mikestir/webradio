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

#ifndef AUDIOENCODER_H_
#define AUDIOENCODER_H_

#include <vector>

// FIXME: Use config.h for feature selection

#include <lame/lame.h>
#ifdef WITH_VORBIS
#include <vorbis/vorbisenc.h>
#endif

using namespace std;

class AudioEncoder
{
public:
	AudioEncoder(unsigned int samplerate, unsigned int channels) :
		_samplerate(samplerate), _channels(channels) {}
	virtual ~AudioEncoder() {}

	virtual const vector<char> header() {
		return vector<char>();
	}
	virtual const vector<char> encode(const vector<float> &samples) =0;

	const unsigned int channels() const { return _channels; }
	const unsigned int samplerate() const { return _samplerate; }

protected:
	unsigned int _samplerate;
	unsigned int _channels;

};

class MP3Encoder : public AudioEncoder
{
public:
	MP3Encoder(unsigned int samplerate, unsigned int channels);
	~MP3Encoder();

	const vector<char> encode(const vector<float> &samples);
private:
	lame_t			state;
};

#ifdef WITH_VORBIS
class VorbisEncoder : public AudioEncoder
{
public:
	VorbisEncoder(unsigned int samplerate, unsigned int channels);
	~VorbisEncoder();

	const vector<char> header();
	const vector<char> encode(const vector<float> &samples);
private:
	vorbis_info			vi;
	vorbis_comment		vc;
	vorbis_dsp_state	vd;
	vorbis_block		vb;


	ogg_stream_state	os;
	ogg_page			og;

};
#endif

#endif /* AUDIOENCODER_H_ */
