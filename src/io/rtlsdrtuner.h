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
	bool process(const DspData &in, DspData &out);

	void dataReady(unsigned char *buf, unsigned int len);

	static void* thread_func(void *arg);
	static void callback(unsigned char *buf, unsigned int len, void *arg);

	rtlsdr_dev_t*	dev;

	vector<vector<float> >	ringBuffer;
	unsigned int	head;
	unsigned int	tail;
	pthread_t		thread;
	pthread_mutex_t	mutex;
	pthread_cond_t	readyCondition;
};



#endif /* RTLSDR_H_ */
