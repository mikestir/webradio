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
	static HttpRequestHandler *factory();

	void push(const vector<char> &data);
	unsigned short handleRequest(const string &method, const string &path,
			const vector<char> &requestData, unsigned short status);
protected:
	ssize_t contentReader(uint64_t pos, char *buf, size_t max);

	string	mountpoint;
	int 	pipefd[2];
};

class AudioStreamManager;
class AudioStreamManager : public SampleSink
{
public:
	AudioStreamManager(
			unsigned int samplerate = DEFAULT_SINK_SAMPLE_RATE,
			unsigned int channels = DEFAULT_SINK_CHANNELS,
			const string &subdevice = "");
	virtual ~AudioStreamManager();
	static SampleSink* factory() {
		return new AudioStreamManager();
	}

	bool open();
	void close();

	void setSamplerate(unsigned int samplerate);
	void setChannels(unsigned int channels);
	void setSubdevice(const string &subdevice);

	void push(float *samples, unsigned int nframes);
	void push(short *samples, unsigned int nframes);

	void registerConsumer(AudioStreamHandler *consumer);
	void deregisterConsumer(AudioStreamHandler *consumer);
private:
	void produce(const vector<char> &stream);
	vector<AudioStreamHandler*> _consumers;

	AudioEncoder *encoder;
	pthread_mutex_t mutex;
};

#endif /* AUDIOSTREAM_H_ */
