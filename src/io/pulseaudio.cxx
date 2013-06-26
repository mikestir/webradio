/*
 * pulseaudio.cxx
 *
 *  Created on: 24 Jun 2013
 *      Author: mike
 */

#include <vector>
#include <string>

#include "debug.h"
#include "pulseaudio.h"

/* FIXME: Should be passed from app */
#define APP_NAME				"WebRadio"
#define PLAYBACK_STREAM_NAME	"Playback"
#define CAPTURE_STREAM_NAME		"Capture"

PulseAudioSink::PulseAudioSink(unsigned int samplerate,
		unsigned int channels, const string &subdevice) :
		SampleSink(samplerate, channels, subdevice)
{
	/* Default sample spec */
	ss.format = PA_SAMPLE_S16LE;
	ss.rate = samplerate;
	ss.channels = channels;
	pa = NULL;
}

PulseAudioSink::~PulseAudioSink()
{
	close();
}

bool PulseAudioSink::open()
{
	const char *sink_name = NULL;
	int error;

	/* Calling again re-opens */
	if (pa != NULL)
		close();

	if (_subdevice.length() > 0)
		sink_name = _subdevice.c_str();

	LOG_INFO("Opening device %s for playback\n", sink_name);
	pa = pa_simple_new(NULL, APP_NAME,
		PA_STREAM_PLAYBACK, sink_name,
		PLAYBACK_STREAM_NAME, &ss, NULL, NULL, &error);
	if (pa == NULL) {
		LOG_ERROR("Playback device open failed: %s\n", pa_strerror(error));
		return false;
	}
	return true;
}

void PulseAudioSink::close()
{
	if (pa == NULL)
		return;

	pa_simple_free(pa);
	pa = NULL;
}

void PulseAudioSink::setSamplerate(unsigned int samplerate)
{
	if (pa != NULL)
		return;

	_samplerate = ss.rate = samplerate;
}

void PulseAudioSink::setChannels(unsigned int channels)
{
	if (pa != NULL)
		return;

	_channels = ss.channels = channels;
}

void PulseAudioSink::setSubdevice(const string &subdevice)
{
	if (pa != NULL)
		return;

	LOG_ERROR("Not implemented yet\n");
}

void PulseAudioSink::push(short *samples, unsigned int nframes)
{
	int error;

	if (!pa)
		return;

	if (pa_simple_write(pa, samples, nframes * sizeof(short) * _channels, &error) < 0) {
		LOG_ERROR("Playback write failed: %s\n", pa_strerror(error));
	}
}

/*******************/

PulseAudioSource::PulseAudioSource(unsigned int samplerate,
		unsigned int channels, const string &subdevice) :
		SampleSource(samplerate, channels, subdevice)
{
	/* Default sample spec */
	ss.format = PA_SAMPLE_S16LE;
	ss.rate = samplerate;
	ss.channels = channels;
	pa = NULL;
}

PulseAudioSource::~PulseAudioSource()
{
	close();
}

bool PulseAudioSource::open()
{
	const char *source_name = NULL;
	int error;

	/* Calling again re-opens */
	if (pa != NULL)
		close();

	if (_subdevice.length() > 0)
		source_name = _subdevice.c_str();

	LOG_INFO("Opening device %s for capture\n", source_name);
	pa = pa_simple_new(NULL, APP_NAME,
		PA_STREAM_RECORD, source_name,
		CAPTURE_STREAM_NAME, &ss, NULL, NULL, &error);
	if (pa == NULL) {
		LOG_ERROR("Capture device open failed: %s\n", pa_strerror(error));
		return false;
	}
	return true;
}

void PulseAudioSource::close()
{
	if (pa == NULL)
		return;

	pa_simple_free(pa);
	pa = NULL;
}

void PulseAudioSource::setSamplerate(unsigned int samplerate)
{
	if (pa != NULL)
		return;

	_samplerate = ss.rate = samplerate;
}

void PulseAudioSource::setChannels(unsigned int channels)
{
	if (pa != NULL)
		return;

	_channels = ss.channels = channels;
}

void PulseAudioSource::setSubdevice(const string &subdevice)
{
	if (pa != NULL)
		return;

	LOG_ERROR("Not implemented yet\n");
}

unsigned int PulseAudioSource::pull(short *samples, unsigned int maxframes)
{
	int error;

	if (!pa)
		return 0;

	if (pa_simple_read(pa, samples, maxframes * sizeof(short) * _channels, &error) < 0) {
		LOG_ERROR("Capture read failed: %s\n", pa_strerror(error));
	}
	return maxframes;
}
