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

#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdint.h>

#include "debug.h"
#include "dspblock.h"

DspData::DspData(const DspData &other) :
			_data(NULL), _capacity(0)
{
	/* Copy constructor */
	_type = other.type();
	resize(other.size());

	switch (_type) {
	case Float:
		copy((const float*)other.data(), (const float*)other.data() + other.size(), (float*)data());
		break;
	case Int8:
		copy((const int8_t*)other.data(), (const int8_t*)other.data() + other.size(), (int8_t*)data());
		break;
	case Int16:
		copy((const int16_t*)other.data(), (const int16_t*)other.data() + other.size(), (int16_t*)data());
		break;
	case Int32:
		copy((const int32_t*)other.data(), (const int32_t*)other.data() + other.size(), (int32_t*)data());
		break;
	default:
		LOG_ERROR("Unsupported data type %d\n", (int)_type);
		throw;
	}
}

DspData::DspData(Type t) :
			_data(NULL),
			_size(0),
			_capacity(0),
			_type(t)
{

}

DspData::DspData(Type t, unsigned int nelements) :
			_data(NULL),
			_size(0),
			_capacity(0),
			_type(t)
{
	resize(nelements);
}

DspData::~DspData()
{
	if (_data)
		free(_data);
}

void DspData::resize(unsigned int nelements)
{
	if (nelements > _capacity) {
		unsigned int size;

		switch (_type) {
		case None: size = 0; break;
		case Float: size = sizeof(float); break;
		case Int8: size = sizeof(int8_t); break;
		case Int16: size = sizeof(int16_t); break;
		case Int32: size = sizeof(int32_t); break;
		default:
			LOG_ERROR("Unsupported data type %d\n", (int)_type);
			size = 0;
		}

		if (size) {
			void *temp = realloc(_data, nelements * size);
			if (temp == NULL) {
				LOG_ERROR("Out of memory when allocating %u bytes (%u of %u)\n", nelements * size, nelements, size);
				throw;
			}
			_data = temp;
			_capacity = nelements;
			LOG_DEBUG("(re)allocated type %d data store for %u elements\n",
				(int)_type, nelements);
		}
	}
	_size = nelements;
}

void DspData::swap(DspData &other)
{
	if (other.type() != type()) {
		LOG_ERROR("Can't swap buffers of differing type\n");
		throw;
	}

	void *otherdata = other._data;
	unsigned int othercapacity = other._capacity;
	unsigned int othersize = other._size;

	other._data = _data;
	other._capacity = _capacity;
	other._size = _size;
	_data = otherdata;
	_capacity = othercapacity;
	_size = othersize;
}

/*********************************/

DspBlock::DspBlock(DspData::Type intype, DspData::Type outtype, 
	const string &name, const string &type) :
			_outputSampleRate(0),
			_outputChannels(0),
			_inputType(intype),
			_outputType(outtype),
			_name(name),
			_blockType(type),
			_inputSampleRate(0),
			_inputChannels(0),
			_decimation(1), _interpolation(1),
#ifdef DSPBLOCK_PROFILE
			_totalNanoseconds(0),
			_totalIn(0), _totalOut(0),
#endif
			_isRunning(false),
			data(outtype)
{

}

DspBlock::~DspBlock()
{
	if (_isRunning)
		stop();
}

bool DspBlock::connect(DspBlock *block)
{
	/* Type check */
	if (block->inputType() != this->outputType()) {
		LOG_ERROR("Type mismatch block %s:%s to %s:%s\n",
			block->blockType().c_str(), block->name().c_str(),
			this->blockType().c_str(), this->name().c_str());
		return false;
	}
	if (_isRunning) {
		/* Start downstream */
		block->start();
	}

	/* Hook up */
	if (find(consumers.begin(), consumers.end(), block) != consumers.end()) {
		LOG_ERROR("Block %s:%s already connected to %s:%s\n",
				block->blockType().c_str(), block->name().c_str(),
				this->blockType().c_str(), this->name().c_str());
		return false;
	}

	consumers.push_back(block);
	LOG_DEBUG("Added block %s:%s as consumer of %s:%s\n",
			block->blockType().c_str(), block->name().c_str(),
			this->blockType().c_str(), this->name().c_str());
	return true;
}

void DspBlock::disconnect(DspBlock *block)
{
	if (_isRunning) {
		/* Stop downstream */
		block->stop();
	}

	/* Un-hook */
	consumers.erase(remove(consumers.begin(), consumers.end(), block),
			consumers.end());
	LOG_DEBUG("Removed block %s:%s as consumer of %s:%s\n",
			block->blockType().c_str(), block->name().c_str(),
			this->blockType().c_str(), this->name().c_str());
}

#ifdef DSPBLOCK_PROFILE
uint64_t DspBlock::nsPerSecond() const
{
	uint64_t nsperframe = _totalNanoseconds / _totalIn;
	uint64_t nspersecond = nsperframe * _inputSampleRate;

	LOG_DEBUG("%s:%s %" PRIu64 " ns/frame (%u Hz) %f ms/s\n",
			blockType().c_str(), name().c_str(),
			nsperframe,
			_inputSampleRate,
			(float)nspersecond / 1000000.0);

	/* Cascade to connected blocks */
	for (vector<DspBlock*>::const_iterator it = consumers.begin(); it != consumers.end(); ++it)
		nspersecond += (*it)->nsPerSecond();
	return nspersecond;
}
#endif

bool DspBlock::start()
{
	/* Start this block - may update its output sample rate */
	LOG_DEBUG("Starting block %s:%s\n", blockType().c_str(), name().c_str());
	if (!init()) {
		LOG_ERROR("Block %s:%s failed to initialise\n", blockType().c_str(), name().c_str());
		return false;
	}
	/* Validate new sample rate and calculate decimation/interpolation rate */
	if (_outputSampleRate && _inputSampleRate >= _outputSampleRate) {
		_decimation = _inputSampleRate / _outputSampleRate;
		_interpolation = 1;
	} else {
		_decimation = 1;
		_interpolation = _outputSampleRate / _inputSampleRate;
	}
	if (_inputSampleRate * _interpolation / _decimation != _outputSampleRate) {
		LOG_ERROR("Sample rates must be integer related\n");
		deinit();
		return false;
	}
#ifdef DSPBLOCK_PROFILE
	_totalIn = _totalOut = 0;
	_totalNanoseconds = 0;
#endif
	_isRunning = true;

	/* Configure and start downstream */
	vector<DspBlock*>::iterator it;
	for (it = consumers.begin(); it != consumers.end(); ++it) {
		/* Cascade this block's output configuration to all connected sinks */
		(*it)->setSampleRate(_outputSampleRate);
		(*it)->setChannels(_outputChannels);
		if (!(*it)->start()) {
			LOG_ERROR("Downstream failed to start - aborting pipeline\n");
			stop();
			return false;
		}
	}

	return true;
}

void DspBlock::stop()
{
	/* Stop downstream first */
	for (vector<DspBlock*>::iterator it = consumers.begin(); it != consumers.end(); ++it)
		(*it)->stop();

	if (_isRunning) {
		LOG_DEBUG("Stopping block %s:%s\n", blockType().c_str(), name().c_str());
		_isRunning = false;
		deinit();
	}
}

bool DspBlock::run(const DspData &in)
{
	if (!_isRunning) {
		LOG_ERROR("Pipeline not started\n");
		return false;
	}

#ifdef DSPBLOCK_PROFILE
	timespec start, end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
#endif

	/* Process */
	if (!process(in, data)) {
		LOG_ERROR("Pipeline failed at block %s:%s\n", blockType().c_str(), name().c_str());
		return false;
	}

#ifdef DSPBLOCK_PROFILE
	/* Profile counters */
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
	_totalNanoseconds += (uint64_t)(end.tv_nsec - start.tv_nsec) +
			(uint64_t)(end.tv_sec - start.tv_sec) * 1000000000ULL;
	_totalIn += in.size() / _inputChannels;
	if (_outputChannels)
		_totalOut += data.size() / _outputChannels;
#endif

	/* Propagate result to all registered consumers */
	for (vector<DspBlock*>::iterator it = consumers.begin(); it != consumers.end(); ++it)
		if (!(*it)->run(data))
			return false;

	return true;
}

void DspBlock::setSampleRate(unsigned int rate)
{
	if (_isRunning)
		return;

	LOG_DEBUG("Setting %s:%s input sample rate to %u\n", blockType().c_str(), name().c_str(), rate);
	_inputSampleRate = rate;
}

void DspBlock::setChannels(unsigned int channels)
{
	if (_isRunning)
		return;

	LOG_DEBUG("Setting %s:%s input channel count to %u\n", blockType().c_str(), name().c_str(), channels);
	_inputChannels = channels;
}
