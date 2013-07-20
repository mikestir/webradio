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

using namespace std;

class RtlSdrTuner : public Tuner
{
public:
	RtlSdrTuner(const string &name = "<undefined>");
	virtual ~RtlSdrTuner();
	static Tuner* factory(const string &name = "<undefined>") {
		return new RtlSdrTuner(name);
	}

	float gainDB();

	void setCentreFrequency(unsigned int hz);
	void setOffsetPPM(int ppm);
	void setAGC(bool agc);
	void setGainDB(float gain);

private:
	bool init();
	void deinit();
	bool process(const vector<sample_t> &inBuffer, vector<sample_t> &outBuffer);

	void dataReady(unsigned char *buf, unsigned int len);

	static void* thread_func(void *arg);
	static void callback(unsigned char *buf, unsigned int len, void *arg);

	rtlsdr_dev_t*	dev;

	vector<vector<sample_t> >	ringBuffer;
	unsigned int	head;
	unsigned int	tail;
	pthread_t		thread;
	pthread_mutex_t	mutex;
	pthread_cond_t	readyCondition;
};



#endif /* RTLSDR_H_ */
