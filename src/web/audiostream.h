/*
 * audiostream.h
 *
 *  Created on: 20 Jun 2013
 *      Author: mike
 */

#ifndef AUDIOSTREAM_H_
#define AUDIOSTREAM_H_

#include <string>
#include <vector>
#include <pthread.h>

#include "httpserver.h"
#include "audioencoder.h"
#include "samplesink.h"

class AudioStreamHandler : public HttpRequestHandler
{
public:
	AudioStreamHandler();
	~AudioStreamHandler();
	static HttpRequestHandler *factory() {
		return new AudioStreamHandler();
	}

	const string allows() { return "GET"; }

	void push(const vector<char> &data);
	unsigned short doGet(const vector<string> &wildcards, const vector<char> &requestData);
protected:
	ssize_t contentReader(uint64_t pos, char *buf, size_t max);

	string	mountpoint;
	int 	pipefd[2];
};

class AudioStreamManager;
class AudioStreamManager : public SampleSink
{
public:
	AudioStreamManager(const string &name = "<undefined>");
	virtual ~AudioStreamManager();

	void registerConsumer(AudioStreamHandler *consumer);
	void deregisterConsumer(AudioStreamHandler *consumer);
private:
	bool init();
	void deinit();
	bool process(const vector<sample_t> &inBuffer, vector<sample_t> &outBuffer);

	void produce(const vector<char> &stream);
	vector<AudioStreamHandler*> _consumers;

	AudioEncoder *encoder;
	pthread_mutex_t mutex;
};

#endif /* AUDIOSTREAM_H_ */
