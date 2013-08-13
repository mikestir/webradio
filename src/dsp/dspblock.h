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

class DspBlock
{
public:
	DspBlock(
			const string &name = "<undefined>",
			const string &blockType = "DspBlock");
	virtual ~DspBlock();

	void connect(DspBlock *block);
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

	const string& name() const { return _name; }
	const string& blockType() const { return _blockType; }
	
protected:
	enum Type {
		None = 0, // defines source or sink
		Float,
		Int8,
		Int16,
		Int32,
	};

	/*!
	 * \brief Algorithm input type
	 */
	virtual Type inputType() =0;
	
	/*!
	 * \brief Algorithm output type
	 */
	virtual Type outputType() =0;
	
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
	 * The output buffer will be sized automatically prior to calling, based
	 * on the _outputSampleRate set during startup.
	 *
	 * \param inbuffer		Pointer to input buffer (algorithm must cast to required type)
	 * \param inframes		Number of frames in input buffer
	 * \param outbuffer		Pointer to output buffer (algorithm must cast to required type)
	 * \param outframes		Maximum number of frames to return in output buffer
	 * \return				Number of frames actually returned or negative error code
	 */
	virtual int process(const void *inbuffer, unsigned int inframes, void *outbuffer, unsigned int outframes) =0;

	/* Setters, start, stop and run methods are not part of the public
	 * API except where exported by a source */
	bool start();
	void stop();
	bool run(const void *inbuffer, unsigned int inframes);

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

	void				*outbuffer;
	unsigned int		nelements;
	
	vector<DspBlock*>	consumers;
};

#endif /* DSPBLOCK_H */
