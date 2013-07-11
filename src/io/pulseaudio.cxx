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

PulseAudioSource::PulseAudioSource(const string &name) :
	SampleSource(name, "PulseAudioSource")
{
	/* Default sample spec */
	ss.format = PA_SAMPLE_FLOAT32LE;
	ss.rate = inputSampleRate();
	ss.channels = inputChannels();
	pa = NULL;
}

PulseAudioSource::~PulseAudioSource()
{

}

bool PulseAudioSource::init()
{
	const char *source_name = NULL;
	int error;

	if (subdevice().length() > 0)
		source_name = subdevice().c_str();

	/* Update output sample rate and number of channels - always returns
	 * whatever we were asked for */
	ss.rate = inputSampleRate();
	ss.channels = inputChannels();
	_outputSampleRate = ss.rate;
	_outputChannels = ss.channels;

	LOG_INFO("Opening device %s for capture (%u Hz, %u channels)\n", source_name, ss.rate, ss.channels);
	pa = pa_simple_new(NULL, APP_NAME,
		PA_STREAM_RECORD, source_name,
		CAPTURE_STREAM_NAME, &ss, NULL, NULL, &error);
	if (pa == NULL) {
		LOG_ERROR("Capture device open failed: %s\n", pa_strerror(error));
		return false;
	}

	return true;
}

void PulseAudioSource::deinit()
{
	pa_simple_free(pa);
	pa = NULL;
}

bool PulseAudioSource::process(const vector<sample_t> &inBuffer, vector<sample_t> &outBuffer)
{
	int error;

	if (pa_simple_read(pa, (float*)outBuffer.data(), outBuffer.size() * sizeof(sample_t), &error) < 0) {
		LOG_ERROR("Capture read failed: %s\n", pa_strerror(error));
		return false;
	}
	return true;
}


/*******************************/


PulseAudioSink::PulseAudioSink(const string &name) :
	SampleSink(name, "PulseAudioSink")
{
	/* Default sample spec */
	ss.format = PA_SAMPLE_FLOAT32LE;
	ss.rate = inputSampleRate();
	ss.channels = inputChannels();
	pa = NULL;
}

PulseAudioSink::~PulseAudioSink()
{

}

bool PulseAudioSink::init()
{
	const char *sink_name = NULL;
	int error;

	if (subdevice().length() > 0)
		sink_name = subdevice().c_str();

	/* This is a sink - no need to set output rate */
	ss.rate = inputSampleRate();
	ss.channels = inputChannels();

	LOG_INFO("Opening device %s for playback (%u Hz, %u channels)\n", sink_name, ss.rate, ss.channels);
	pa = pa_simple_new(NULL, APP_NAME,
		PA_STREAM_PLAYBACK, sink_name,
		PLAYBACK_STREAM_NAME, &ss, NULL, NULL, &error);
	if (pa == NULL) {
		LOG_ERROR("Playback device open failed: %s\n", pa_strerror(error));
		return false;
	}

	return true;
}

void PulseAudioSink::deinit()
{
	pa_simple_free(pa);
	pa = NULL;
}

bool PulseAudioSink::process(const vector<sample_t> &inBuffer, vector<sample_t> &outBuffer)
{
	int error;

	if (pa_simple_write(pa, (const float*)inBuffer.data(), inBuffer.size() * sizeof(sample_t), &error) < 0) {
		LOG_ERROR("Playback write failed: %s\n", pa_strerror(error));
		return false;
	}
	return true;
}
