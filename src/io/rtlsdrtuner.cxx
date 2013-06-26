/*
 * rtlsdr.cxx
 *
 *  Created on: 24 Jun 2013
 *      Author: mike
 */

#include "debug.h"
#include "rtlsdrtuner.h"

RtlSdrTuner::RtlSdrTuner(unsigned int samplerate,
		unsigned int channels,
		const string &subdevice) :
		Tuner(samplerate, channels, subdevice),
		dev(NULL)
{
	deviceCount = rtlsdr_get_device_count();

	LOG_DEBUG("Found %d devices\n", deviceCount);
	for (int n = 0; n < deviceCount; n++) {
		/* FIXME: Incorporate this into the _subdevices vector in some way */
		deviceNames.push_back(rtlsdr_get_device_name(n));
		LOG_DEBUG("Device %d: %s\n", n, deviceNames[n].c_str());
	}

}

RtlSdrTuner::~RtlSdrTuner()
{
	if (dev)
		close();
}

bool RtlSdrTuner::open()
{
	if (dev)
		close();

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

	/* Push current settings */
	setSamplerate(_samplerate);
	setCentreFrequency(_centreFrequency);
	setOffsetPPM(_offsetPPM);
	setAGC(_AGC);
	setGainDB(_gainDB);

	return true;
}

void RtlSdrTuner::close()
{
	if (!dev)
		return;

	rtlsdr_close(dev);
	dev = NULL;
}

void RtlSdrTuner::setSamplerate(unsigned int samplerate)
{

}

void RtlSdrTuner::setChannels(unsigned int channels)
{
	/* FIXME: Does it make sense for this to be anything other than 2?
	 * Maybe some tuners will only return real samples at a low IF? */
	LOG_ERROR("setChannels not supported\n");
}

void RtlSdrTuner::setSubdevice(const string &subdevice)
{
	/* FIXME: This we could support */
	LOG_ERROR("setSubdevice not supported\n");
}

void RtlSdrTuner::setCentreFrequency(unsigned int hz)
{

}

void RtlSdrTuner::setOffsetPPM(int ppm)
{

}

void RtlSdrTuner::setAGC(bool agc)
{

}

void RtlSdrTuner::setGainDB(int gain)
{

}

unsigned int RtlSdrTuner::pull(float *samples, unsigned int maxframes)
{

}
