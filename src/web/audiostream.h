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

#ifndef AUDIOSTREAM_H_
#define AUDIOSTREAM_H_

#include <string>
#include <vector>
#include <pthread.h>

#include "httpserver.h"
#include "audioencoder.h"
#include "ioblock.h"

class AudioStreamHandler : public HttpRequestHandler
{
public:
	AudioStreamHandler();
	~AudioStreamHandler();
	static HttpRequestHandler *factory() {
		return new AudioStreamHandler();
	}

	const string allows(const vector<string> &wildcards) { return "GET"; }

	void push(const vector<char> &data);
	unsigned short doGet(const vector<string> &wildcards, const vector<char> &requestData);
protected:
	ssize_t contentReader(uint64_t pos, char *buf, size_t max);

	string	mountpoint;
	int 	pipefd[2];
};

class AudioStreamManager;
class AudioStreamManager : public IOBlock
{
public:
	AudioStreamManager(const string &name = "<undefined>");
	virtual ~AudioStreamManager();

	void registerConsumer(AudioStreamHandler *consumer);
	void deregisterConsumer(AudioStreamHandler *consumer);
private:
	bool init();
	void deinit();
	bool process(const DspData &in, DspData &out);

	void produce(const vector<char> &stream);
	vector<AudioStreamHandler*> _consumers;

	AudioEncoder *encoder;
	pthread_mutex_t mutex;
};

#endif /* AUDIOSTREAM_H_ */
