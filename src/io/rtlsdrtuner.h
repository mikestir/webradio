/*
 * rtlsdr.h
 *
 *  Created on: 24 Jun 2013
 *      Author: mike
 */

#ifndef RTLSDR_H_
#define RTLSDR_H_

#include <vector>
#include <string>

#include <rtl-sdr.h>

#include "tuner.h"

using namespace std;

class RtlSdrTuner : public Tuner
{
public:
	RtlSdrTuner(
			unsigned int samplerate = DEFAULT_TUNER_SAMPLE_RATE,
			unsigned int channels = DEFAULT_TUNER_CHANNELS,
			const string &subdevice = "rtl=0");
	virtual ~RtlSdrTuner();
	static Tuner* factory() {
		return new RtlSdrTuner();
	}

	bool open();
	void close();

	void setSamplerate(unsigned int samplerate);
	void setChannels(unsigned int channels);
	void setSubdevice(const string &subdevice);
	void setCentreFrequency(unsigned int hz);
	void setOffsetPPM(int ppm);
	void setAGC(bool agc);
	void setGainDB(int gain);

	unsigned int pull(float *samples, unsigned int maxframes);
private:
	unsigned int deviceCount;
	vector<string> deviceNames;
	rtlsdr_dev_t *dev;
};



#endif /* RTLSDR_H_ */
