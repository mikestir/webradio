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

#include <vector>
#include <string>

#include "debug.h"
#include "pulseaudio.h"

/* FIXME: Should be passed from app */
#define APP_NAME				"WebRadio"
#define PLAYBACK_STREAM_NAME	"Playback"
#define CAPTURE_STREAM_NAME		"Capture"

PulseAudioSource::PulseAudioSource(const string &name) :
	SourceBlock(DspData::Float, name, "PulseAudioSource")
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

bool PulseAudioSource::process(const DspData &in, DspData &out)
{
	int error;

	out.resize(in.size());
	if (pa_simple_read(pa, (float*)out.data(), out.size() * sizeof(float), &error) < 0) {
		LOG_ERROR("Capture read failed: %s\n", pa_strerror(error));
		return false;
	}
	return true;
}

/*******************************/


PulseAudioSink::PulseAudioSink(const string &name) :
	IOBlock(DspData::Float, DspData::None, name, "PulseAudioSink")
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

bool PulseAudioSink::process(const DspData &in, DspData &out)
{
	int error;

	if (pa_simple_write(pa, (const float*)in.data(), in.size() * sizeof(float), &error) < 0) {
		LOG_ERROR("Playback write failed: %s\n", pa_strerror(error));
		return false;
	}
	return true;
}
