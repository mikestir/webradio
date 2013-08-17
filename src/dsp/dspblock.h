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

#ifndef DSPBLOCK_H_
#define DSPBLOCK_H_


// FIXME: Use config.h
#define DSPBLOCK_PROFILE

#ifdef DSPBLOCK_PROFILE
#include <time.h>
#include <stdint.h>
#endif

#include <vector>
#include <string>

using namespace std;

class DspData
{
public:
	DspData(const DspData &other);

	enum Type {
		None = 0, // defines source or sink
		Float,
		Int8,
		Int16,
		Int32,
	};

	DspData(Type t);
	DspData(Type t, unsigned int nelements);
	virtual ~DspData();

	/*! \brief Returns pointer to data */
	void* data() { return _data; }
	/*! \brief Returns const pointer to immutable data */
	const void* data() const { return (const void*)_data; }
	/*! \brief Returns size of data (number of elements) */
	unsigned int size() const { return _size; }
	/*! \brief Returns data type */
	Type type() const { return _type; }

	/*!
	 * \brief Set size of data area (number of elements)
	 *
	 * This will increase the capacity of the buffer using realloc
	 * if necessary.  Resizing down does not result in reallocation.
	 */
	void resize(unsigned int nelements);

	/*! \brief Copy-free buffer swap */
	void swap(DspData &other);
private:
	void* _data;
	unsigned int _size;
	unsigned int _capacity;
	Type _type;
};

class DspBlock
{
public:
	DspBlock(
			DspData::Type intype,
			DspData::Type outtype,
			const string &name = "<undefined>",
			const string &blockType = "DspBlock");
	virtual ~DspBlock();

	bool connect(DspBlock *block);
	void disconnect(DspBlock *block);

	unsigned int inputSampleRate() const { return _inputSampleRate; }
	unsigned int outputSampleRate() const { return _outputSampleRate; }
	unsigned int inputChannels() const { return _inputChannels; }
	unsigned int outputChannels() const { return _outputChannels; }
	unsigned int decimation() const { return _decimation; }
	unsigned int interpolation() const { return _interpolation; }
#ifdef DSPBLOCK_PROFILE
	uint64_t nsPerFrameAll() const;
	uint64_t nsPerFrameOne() const { return _totalNanoseconds / _totalIn; }
	uint64_t totalNanoseconds() const { return _totalNanoseconds; }
	unsigned int totalIn() const { return _totalIn; }
	unsigned int totalOut() const { return _totalOut; }
#endif
	bool isRunning() const { return _isRunning; }

	DspData::Type inputType() const { return _inputType; }
	DspData::Type outputType() const { return _outputType; }
	const string& name() const { return _name; }
	const string& blockType() const { return _blockType; }
	
protected:
	/*!
	 * \brief Perform one-time initialisation when pipeline is starting
	 * \return Success or failure
	 */
	virtual bool init() =0;
	
	/*!
	 * \brief Perform algorithm specific clean-up when pipeline is stopping
	 */
	virtual void deinit() =0;
	
	/*!
	 * \brief Execute algorithm into output buffer
	 *
	 * The algorithm must resize the output buffer as required
	 *
	 */
	virtual bool process(const DspData &in, DspData &out) =0;

	/* Setters, start, stop and run methods are not part of the public
	 * API except where exported by a source */
	bool start();
	void stop();
	bool run(const DspData &in);

	/* Producer blocks call these in their attached consumers to propagate
	 * output sample rate/channel count.  They are simply setters - the
	 * downstream block may still fail to start if the settings are
	 * unacceptable */
	void setSampleRate(unsigned int rate);
	void setChannels(unsigned int channels);

#if 0 // For dynamic sample rate support
	/* Algorithms may change sample rate and output channels as required
	 * (although this is an expensive operation).  The change will propagated
	 * to the rest of the pipeline. */
	void setOutputSampleRate(unsigned int rate);
	void setOutputChannels(unsigned int channels);
#endif

	/* Algorithm may update these during startup */
	unsigned int 		_outputSampleRate;
	unsigned int 		_outputChannels;
private:
	DspData::Type		_inputType;
	DspData::Type		_outputType;
	const string		_name;
	const string		_blockType;
	unsigned int 		_inputSampleRate;
	unsigned int 		_inputChannels;
	unsigned int		_decimation; // times
	unsigned int		_interpolation; // times
#ifdef DSPBLOCK_PROFILE
	uint64_t			_totalNanoseconds;
	uint64_t			_totalIn; // frames
	uint64_t			_totalOut; // frames
#endif
	bool				_isRunning;
	vector<DspBlock*>	consumers;
	DspData				data;
};

#endif /* DSPBLOCK_H */
