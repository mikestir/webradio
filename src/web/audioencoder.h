/*
 * audioencoder.h
 *
 *  Created on: 21 Jun 2013
 *      Author: mike
 */

#ifndef AUDIOENCODER_H_
#define AUDIOENCODER_H_

#include <vector>

#include <lame/lame.h>
#include <vorbis/vorbisenc.h>

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

#endif /* AUDIOENCODER_H_ */
