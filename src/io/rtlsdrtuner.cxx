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
#include <algorithm>

#include <unistd.h>
#include <pthread.h>
#include <rtl-sdr.h>

#include "debug.h"
#include "rtlsdrtuner.h"

#define N_BUFFERS_LOG2		2
#define N_BUFFERS			(1 << (N_BUFFERS_LOG2))

#define TYPE				DspData::Int16
#define TYPE_CONVERT(a)		(((short)(a) - 128) << 8)

RtlSdrTuner::RtlSdrTuner(const string &name) :
	Tuner(TYPE, name, "RtlSdrTuner"),
	dev(NULL), head(0), tail(0)
{
	unsigned int deviceCount;

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&readyCondition, NULL);
	
	setChannels(2); // only support I/Q sampling
	deviceCount = rtlsdr_get_device_count();

	LOG_DEBUG("Found %d devices\n", deviceCount);
	for (unsigned int n = 0; n < deviceCount; n++) {
		char mfg[256];
		char product[256];
		char serial[256];
		rtlsdr_get_device_usb_strings(n, mfg, product, serial);
		_subdevices.push_back(string(serial));
		LOG_DEBUG("Device %d: %s\n", n, _subdevices[n].c_str());
	}

}

RtlSdrTuner::~RtlSdrTuner()
{
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&readyCondition);
}

void* RtlSdrTuner::thread_func(void* arg)
{
	RtlSdrTuner *self = (RtlSdrTuner*)arg;
	
	rtlsdr_reset_buffer(self->dev);
	sleep(1);
	rtlsdr_read_sync(self->dev, NULL, 4096, NULL);

	LOG_DEBUG("starting async capture\n");
	rtlsdr_read_async(self->dev, &RtlSdrTuner::callback, (void*)self,
		0, 0 /* accept default buffer size/count */);
	LOG_DEBUG("finished async capture\n");
	pthread_exit(0);
}

void RtlSdrTuner::callback(unsigned char *buf, unsigned int len, void *arg)
{
	RtlSdrTuner *self = (RtlSdrTuner*)arg;
	self->dataReady(buf, len);
}

void RtlSdrTuner::dataReady(unsigned char *buf, unsigned int len)
{
	/* Blocks of samples from the async USB handler come here and are
	 * pushed into a ring of blockSize()ed buffers.  These can be popped
	 * asynchronously by the pipeline thread. */

	while (len) {

		/* If tail is full then we have a buffer overflow */
		DspData *buffer = &ringBuffer[tail];
#if 0
		LOG_DEBUG("len = %u tail %u (%u avail)\n", len, tail, blockSize() - (unsigned int)buffer->size());
#endif
		if (buffer->size() == blockSize()) {
			LOG_ERROR("Lost %u bytes\n", len);
			break;
		}

		pthread_mutex_lock(&mutex);
#if 1
		unsigned int start = buffer->size();
		unsigned int add = blockSize() - start;

		if (add > len)
			add = len;
		len -= add;

		buffer->resize(start + add);
		short *ptr = (short*)buffer->data() + start;
		while (add--)
			*ptr++ = TYPE_CONVERT(*buf++);

#else // original vector version
		while (len && buffer->size() < blockSize()) {
			buffer->push_back(TYPE_CONVERT(*buf++));
			len--;
		}
#endif

		if (buffer->size() == blockSize()) {
			/* Give new buffer to app */
			tail = (tail + 1) & (N_BUFFERS - 1);
			pthread_cond_signal(&readyCondition);
		}
		pthread_mutex_unlock(&mutex);
	}
}

void RtlSdrTuner::setCentreFrequency(unsigned int hz)
{
	if (dev == NULL) {
		/* No device yet - update initial */
		_centreFrequency = hz;
		return;
	}

	rtlsdr_set_center_freq(dev, hz);
	_centreFrequency = rtlsdr_get_center_freq(dev);
	LOG_DEBUG("centre frequency set to %u Hz\n", _centreFrequency);
}

void RtlSdrTuner::setOffsetPPM(int ppm)
{
	if (dev == NULL) {
		/* No device yet - update initial */
		_offsetPPM = ppm;
		return;
	}

	rtlsdr_set_freq_correction(dev, ppm);
	_offsetPPM = rtlsdr_get_freq_correction(dev);
	LOG_DEBUG("frequency correction set to %d ppm\n", _offsetPPM);
}

void RtlSdrTuner::setAGC(bool agc)
{
	if (dev == NULL) {
		/* No device yet - update initial */
		_AGC = agc;
		return;
	}

	if (rtlsdr_set_tuner_gain_mode(dev, agc ? 0 : 1) == 0)
		_AGC = agc;
	LOG_DEBUG("AGC mode set to %d\n", _AGC);
}

float RtlSdrTuner::gainDB()
{
	if (dev != NULL && _AGC)
		_gainDB = (float)rtlsdr_get_tuner_gain(dev) / 10.0;

	/* FIXME: Deal with IF gain? */

	return _gainDB;
}

void RtlSdrTuner::setGainDB(float gain)
{
	if (dev == NULL) {
		/* No device yet - update initial */
		_gainDB = gain;
		return;
	}

	if (_AGC)
		return;

	/* FIXME: Deal with IF gain? */

	rtlsdr_set_tuner_gain(dev, (int)(gain * 10.0));
	_gainDB = (float)rtlsdr_get_tuner_gain(dev) / 10.0;
}

bool RtlSdrTuner::init()
{
	if (_subdevices.size() < 1) {
		LOG_ERROR("No RTL-SDR devices found\n");
		return false;
	}

	if (inputChannels() != 2) {
		LOG_ERROR("Only IQ capture supported\n");
		return false;
	}
	_outputChannels = inputChannels();

	/* Subdevice refers to device serial number */
	int index = rtlsdr_get_index_by_serial(subdevice().c_str());
	if (index < 0) {
		LOG_ERROR("Subdevice %s not found\n", subdevice().c_str());
		return false;
	}

	if (rtlsdr_open(&dev, index) < 0) {
		LOG_ERROR("Error opening device %d\n", index);
		dev = NULL;
		return false;
	}
	_name = rtlsdr_get_device_name(index);

	/* Read back some info - might use this in the future */
	unsigned int rtl_freq, tuner_freq;
	char mfg[256];
	char product[256];
	char serial[256];

	rtlsdr_get_xtal_freq(dev, &rtl_freq, &tuner_freq);
	LOG_DEBUG("RTL2832 xtal frequency = %u Hz\n", rtl_freq);
	LOG_DEBUG("Tuner xtal frequency = %u Hz\n", tuner_freq);

	rtlsdr_get_usb_strings(dev, mfg, product, serial);
	LOG_DEBUG("Manufacturer: %s\n", mfg);
	LOG_DEBUG("Product: %s\n", product);
	LOG_DEBUG("Serial: %s\n", serial);
	_manufacturer = mfg;
	_product = product;
	_serial = serial;

	/* We adjust actual device sample rate here.  No support for
	 * changing while device is running */
	rtlsdr_set_sample_rate(dev, inputSampleRate());
	_outputSampleRate = rtlsdr_get_sample_rate(dev);
	LOG_DEBUG("samplerate set to %u Hz\n", _outputSampleRate);
	rtlsdr_set_agc_mode(dev, 1); // RTL2832 internal AGC
	
	/* Push current settings */
	setCentreFrequency(_centreFrequency);
	setOffsetPPM(_offsetPPM);
	setAGC(_AGC);
	setGainDB(_gainDB);
	
	/* Reserve capture buffer pool */
	ringBuffer.resize(N_BUFFERS, DspData(TYPE));

	/* Set up async rx */
	if (pthread_create(&thread, NULL, &RtlSdrTuner::thread_func, (void*)this)) {
		LOG_ERROR("Failed starting capture thread\n");
		return false;
	}

	return true;
}

void RtlSdrTuner::deinit()
{
	rtlsdr_cancel_async(dev);
	pthread_join(thread, NULL);
	rtlsdr_close(dev);
	dev = NULL;
}

bool RtlSdrTuner::process(const DspData &in, DspData &out)
{
	pthread_mutex_lock(&mutex);

	/* Pop ringbuffer, block if empty */
	DspData *nextbuffer = &ringBuffer[head];
	while (nextbuffer->size() != blockSize()) {
#if 0
		LOG_DEBUG("wait blockSize = %u, outBuffer.size = %u, head = %u (avail %u)\n",
				blockSize(), (unsigned int)outBuffer.size(), head, (unsigned int)nextbuffer->size());
#endif
		pthread_cond_wait(&readyCondition, &mutex);
	}
#if 0
	LOG_DEBUG("ready %u (avail %u)\n", head, (unsigned int)nextbuffer->size());
#endif

	/* Fast swap with output buffer */
	out.resize(0);
	out.swap(*nextbuffer);
	head = (head + 1) & (N_BUFFERS - 1);

	pthread_mutex_unlock(&mutex);
	return true;
}

