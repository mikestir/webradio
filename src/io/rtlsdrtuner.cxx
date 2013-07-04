/*
 * rtlsdr.cxx
 *
 *  Created on: 24 Jun 2013
 *      Author: mike
 */

#include <vector>

#include <unistd.h>
#include <pthread.h>

#include "debug.h"
#include "rtlsdrtuner.h"

#ifdef RTLSDR_ASYNC_READ
#define MAX_BACKLOG					524288
#endif

RtlSdrTuner::RtlSdrTuner() : Tuner(), dev(NULL)
{
#ifdef RTLSDR_ASYNC_READ
	pthread_mutex_init(&mutex, NULL);
#endif
	
	_channels = 2; // only supports I/Q sampling
	deviceCount = rtlsdr_get_device_count();

	LOG_DEBUG("Found %d devices\n", deviceCount);
	for (unsigned int n = 0; n < deviceCount; n++) {
		/* FIXME: Incorporate this into the _subdevices vector in some way */
		deviceNames.push_back(rtlsdr_get_device_name(n));
		LOG_DEBUG("Device %d: %s\n", n, deviceNames[n].c_str());
	}

}

RtlSdrTuner::~RtlSdrTuner()
{
	if (dev != NULL)
		stop();
	
#ifdef RTLSDR_ASYNC_READ
	pthread_mutex_destroy(&mutex);
#endif
}

#ifdef RTLSDR_ASYNC_READ
void* RtlSdrTuner::thread_func(void* arg)
{
	RtlSdrTuner *self = (RtlSdrTuner*)arg;
	
	rtlsdr_reset_buffer(self->dev);
	sleep(1);
	rtlsdr_read_sync(self->dev, NULL, 4096, NULL);
	LOG_DEBUG("starting async capture\n");
	rtlsdr_read_async(self->dev, &RtlSdrTuner::callback, (void*)self,
		0, 0 /* accept defaults */);
	LOG_DEBUG("finished async capture\n");
	pthread_exit(0);
}

void RtlSdrTuner::callback(unsigned char *buf, unsigned int len, void *arg)
{
	RtlSdrTuner *self = (RtlSdrTuner*)arg;

	/* Blocks of samples from the async USB handler come here and are
	 * pushed into a ring buffer, in turn read by the "pull" method.
	 * This ensures that USB flow continues even if the consumer stops reading */

	// FIXME: We must treat I/Q pairs atomically - must not push or pop half
	// a sample or we will get out of sync
	while (len--) {
		unsigned int nexttail = (self->tail + 1) & (MAX_BACKLOG - 1);
		if (nexttail == self->head) {
			LOG_ERROR("lost %u samples\n", len);
			return;
		}
		self->buffer[self->tail] = ((float)(*buf++) - 128.0) / 128.0;
		self->tail = nexttail;
	}
}
#endif

bool RtlSdrTuner::start()
{
	if (dev != NULL)
		stop();

	/* FIXME: We don't currently support subdevices, just open the first one */
	if (deviceCount < 1) {
		LOG_ERROR("No RTL-SDR devices found\n");
		return false;
	}

	if (rtlsdr_open(&dev, 0) < 0) {
		LOG_ERROR("Error opening device 0\n");
		dev = NULL;
		return false;
	}

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

	/* We adjust actual device sample rate here.  No support for
	 * changing while device is running */
	rtlsdr_set_sample_rate(dev, _samplerate);
	_samplerate = rtlsdr_get_sample_rate(dev);
	LOG_DEBUG("samplerate set to %u Hz\n", _samplerate);
	rtlsdr_set_agc_mode(dev, 1); // RTL2832 internal AGC
	
	/* Push current settings */
	setCentreFrequency(_centreFrequency);
	setOffsetPPM(_offsetPPM);
	setAGC(_AGC);
	setGainDB(_gainDB);
	
#ifdef RTLSDR_ASYNC_READ
	/* Set up async rx */
	buffer.resize(MAX_BACKLOG);
	head = tail = 0;
	if (pthread_create(&thread, NULL, &RtlSdrTuner::thread_func, (void*)this)) {
		LOG_ERROR("Failed starting capture thread\n");
		return false;
	}
#else
	/* Set up sync rx */
	rtlsdr_reset_buffer(dev);
	sleep(1);
	rtlsdr_read_sync(dev, NULL, 4096, NULL);
#endif

	return true;
}

void RtlSdrTuner::stop()
{
	if (dev == NULL)
		return;

#ifdef RTLSDR_ASYNC_READ
	rtlsdr_cancel_async(dev);
	pthread_join(thread, NULL);
	vector<float>().swap(buffer); // release memory
#endif
	rtlsdr_close(dev);
	dev = NULL;
}

void RtlSdrTuner::setSamplerate(unsigned int samplerate)
{
	if (dev != NULL)
		return;
	
	_samplerate = samplerate;
}

void RtlSdrTuner::setChannels(unsigned int channels)
{
	if (dev != NULL)
		return;

	/* Some tuners might support real sampling, but we don't */
	LOG_ERROR("setChannels not supported\n");
}

void RtlSdrTuner::setSubdevice(const string &subdevice)
{
	if (dev != NULL)
		return;
	
	/* FIXME: We should support this */
	LOG_ERROR("setSubdevice not supported\n");
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

unsigned int RtlSdrTuner::pull(float *samples, unsigned int maxframes)
{
	if (dev == NULL)
		return 0;
		
#ifdef RTLSDR_ASYNC_READ
	/* Read data from the ring buffer */
	unsigned int nread;
	for (nread = 0; nread < maxframes * _channels; nread++) {
		if (head == tail)
			break;

		*samples++ = buffer[head];
		head = (head + 1) & (MAX_BACKLOG - 1);
	}
	return nread / _channels;
#else
	/* Read data synchronously */
	vector<unsigned char> cbuf(maxframes * _channels);
	int nread;
	
	rtlsdr_read_sync(dev, (void*)cbuf.data(), maxframes * _channels, &nread);
	
	for (unsigned int n = 0; n < nread; n++)
		*samples++ = (((float)cbuf[n]) - 128.0) / 128.0;

	return nread / _channels;
#endif
}
