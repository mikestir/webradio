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
#include <pthread.h>

#include "tuner.h"

#define RTLSDR_ASYNC_READ

using namespace std;

class RtlSdrTuner : public Tuner
{
public:
	RtlSdrTuner();
	virtual ~RtlSdrTuner();

	bool start();
	void stop();

	void setSamplerate(unsigned int samplerate);
	void setChannels(unsigned int channels);
	void setSubdevice(const string &subdevice);
	void setCentreFrequency(unsigned int hz);
	void setOffsetPPM(int ppm);
	void setAGC(bool agc);
	float gainDB();
	void setGainDB(float gain);

	unsigned int pull(float *samples, unsigned int maxframes);

private:
	unsigned int	deviceCount;
	vector<string>	deviceNames;
	rtlsdr_dev_t*	dev;

#ifdef RTLSDR_ASYNC_READ
	static void* thread_func(void *arg);
	static void callback(unsigned char *buf, unsigned int len, void *arg);

	vector<float>	buffer; // FIXME: Make a fast FIFO template class
	unsigned int	head;
	unsigned int	tail;

	pthread_t		thread;
	pthread_mutex_t	mutex;
#endif
};



#endif /* RTLSDR_H_ */
