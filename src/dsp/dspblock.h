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

#define DEFAULT_SAMPLE_RATE			48000
#define DEFAULT_CHANNELS			2
#define DEFAULT_BLOCK_SIZE			16384

using namespace std;

typedef float sample_t;

class DspBlock;
class DspSource;

class DspBlock
{
public:
	friend class DspSource;

	DspBlock(
			const string &name = "<undefined>",
			const string &type = "DspBlock");
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
	uint64_t totalNanoseconds() const { return _totalNanoseconds; }
	unsigned int totalIn() const { return _totalIn; }
	unsigned int totalOut() const { return _totalOut; }
#endif
	bool isRunning() const { return _isRunning; }

	const string& name() const { return _name; }
	const string& type() const { return _type; }
protected:
	/* Subclasses must implement these to do the actual work */
	virtual bool init() { return false; } /*=0; FIXME: gets called under some circumstances! */
	virtual void deinit() {} /*=0; FIXME: gets called under some circumstances! */
	virtual bool process(const vector<sample_t> &inBuffer, vector<sample_t> &outBuffer) { return false; } /*=0; FIXME: gets called under some circumstances! */

	/* Blocks may adjust output sample rate and channel count during
	 * startup */
	unsigned int 		_outputSampleRate;
	unsigned int 		_outputChannels;

private:
	/* Setters, start, stop and run methods are not part of the public
	 * API except where exported by a source */
	bool start();
	void stop();
	bool run(const vector<sample_t> &inBuffer);

	/* Producer blocks call these in their attached consumers to propagate
	 * output sample rate/channel count.  They are simply setters - the
	 * downstream block may still fail to start if the settings are
	 * unacceptable */
	void setSampleRate(unsigned int rate);
	void setChannels(unsigned int channels);

	const string		_name;
	const string		_type;
	unsigned int 		_inputSampleRate;
	unsigned int 		_inputChannels;
	unsigned int		_decimation; // times
	unsigned int		_interpolation; // times
#ifdef DSPBLOCK_PROFILE
	uint64_t			_totalNanoseconds;
	unsigned int		_totalIn; // frames
	unsigned int		_totalOut; // frames
#endif
	bool				_isRunning;

	vector<sample_t>	buffer;
	vector<DspBlock*>	consumers;
};

class DspSource : public DspBlock
{
public:
	DspSource(
			const string &name = "<undefined>",
			const string &type = "DspSource");
	virtual ~DspSource();

	unsigned int blockSize() const { return _blockSize; }

	bool start() { return DspBlock::start(); }
	void stop() { DspBlock::stop(); }
	bool run() { return DspBlock::run(vector<sample_t>(_blockSize)); }
	void setSampleRate(unsigned int rate) { DspBlock::setSampleRate(rate); }
	void setChannels(unsigned int channels) { DspBlock::setChannels(channels); }
	void setBlockSize(unsigned int bytes);
private:
	unsigned int		_blockSize;
};


#endif /* DSPBLOCK_H */
