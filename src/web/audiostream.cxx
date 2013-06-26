/*
 * audiostream.cxx
 *
 *  Created on: 20 Jun 2013
 *      Author: mike
 */

#include <string>
#include <vector>
#include <algorithm>

#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "audiostream.h"
#include "audioencoder.h"
#include "debug.h"

/*! Global mountpoint map */
static map<string,AudioStreamManager*> streams;

AudioStreamManager::AudioStreamManager(
		unsigned int samplerate,
		unsigned int channels,
		const string &subdevice) : SampleSink(samplerate, channels, subdevice)
{
	pthread_mutex_init(&mutex, NULL);
	encoder = NULL;
}

AudioStreamManager::~AudioStreamManager()
{
	if (encoder)
		close();
	pthread_mutex_destroy(&mutex);
}

bool AudioStreamManager::open()
{
	if (encoder != NULL)
		close();

	encoder = new MP3Encoder(_samplerate, _channels);
	streams[_subdevice] = this;
	return true;
}

void AudioStreamManager::close()
{
	if (encoder == NULL)
		return;

	streams.erase(_subdevice);
	delete encoder;
	encoder = NULL;
}

void AudioStreamManager::setSamplerate(unsigned int samplerate)
{
	if (encoder != NULL)
		return;

	_samplerate = samplerate;
}

void AudioStreamManager::setChannels(unsigned int channels)
{
	if (encoder != NULL)
		return;

	_channels = channels;
}

void AudioStreamManager::setSubdevice(const string &subdevice)
{
	if (encoder != NULL)
		return;

	_subdevice = subdevice;
}

void AudioStreamManager::push(short *samples, unsigned int nframes)
{
	if (encoder == NULL || _consumers.size() == 0)
		return;

	/* Encode to MP3 and push to all registered consumers */
	vector<char> frame = encoder->encode(samples, nframes);
	produce(frame);
}

void AudioStreamManager::push(float *samples, unsigned int nframes)
{
	if (encoder == NULL || _consumers.size() == 0)
		return;

	/* Encode to MP3 and push to all registered consumers */
	vector<char> frame = encoder->encode(samples, nframes);
	produce(frame);
}

void AudioStreamManager::produce(const vector<char> &stream)
{
	vector<AudioStreamHandler*>::iterator iter;

	if (stream.size() == 0)
		return;

	//LOG_DEBUG("encoded %lu bytes\n", stream.size());

	pthread_mutex_lock(&mutex);
	for (iter = _consumers.begin();
			iter != _consumers.end(); iter++) {
		AudioStreamHandler *handler = *iter;
		handler->push(stream);
	}
	pthread_mutex_unlock(&mutex);
}

void AudioStreamManager::registerConsumer(AudioStreamHandler *consumer)
{
	LOG_DEBUG("registered audio consumer %p for mountpoint %s\n", consumer, _subdevice.c_str());
	pthread_mutex_lock(&mutex);
	_consumers.push_back(consumer);
	pthread_mutex_unlock(&mutex);
}

void AudioStreamManager::deregisterConsumer(AudioStreamHandler *consumer)
{
	pthread_mutex_lock(&mutex);
	_consumers.erase(std::remove(
			_consumers.begin(),
			_consumers.end(),
			consumer),
			_consumers.end());
	pthread_mutex_unlock(&mutex);
	LOG_DEBUG("deregistered audio consumer %p from mountpoint %s\n", consumer, _subdevice.c_str());
}

/*****************/

AudioStreamHandler::AudioStreamHandler() : HttpRequestHandler()
{

}

AudioStreamHandler::~AudioStreamHandler()
{
	/* De-register */
	if (streams[mountpoint])
		streams[mountpoint]->deregisterConsumer(this);
}

HttpRequestHandler* AudioStreamHandler::factory()
{
	return new AudioStreamHandler();
}

void AudioStreamHandler::push(const vector<char> &data)
{
	write(pipefd[1], data.data(), data.size());
}

unsigned short AudioStreamHandler::handleRequest(const string &method, const string &path,
		const vector<char> &requestData, unsigned short status)
{
	//AudioStreamManager::format format;

	if (method != "GET")
		return MHD_HTTP_METHOD_NOT_ALLOWED;

#if 0
	/* Path determines format */
	if (path == "/mp3") {
		_contentType = "audio/mpeg";
		format = AudioStreamManager::MP3;
	} else if (path == "/ogg") {
		_contentType = "audio/ogg";
		format = AudioStreamManager::Vorbis;
	} else {
		return MHD_HTTP_NOT_FOUND;
	}
#else
	/* Extract mountpoint from path */
	mountpoint = path;
	if (streams[mountpoint] == NULL) {
		LOG_ERROR("Request for non-existent audio stream: %s\n", mountpoint.c_str());
		return MHD_HTTP_NOT_FOUND;
	}
	_contentType = "audio/mpeg";
#endif

	if (pipe(pipefd) < 0) {
		LOG_ERROR("pipe error\n");
		return MHD_HTTP_INTERNAL_SERVER_ERROR;
	}

	streams[mountpoint]->registerConsumer(this);
	_isPersistent = true;
	return MHD_HTTP_OK;
}

ssize_t AudioStreamHandler::contentReader(uint64_t pos, char *buf, size_t max)
{
	/* Blocks until data available, as required by the HTTPD library */
	return read(pipefd[0], buf, max);
}
