/*
 * ioblock.h
 *
 *  Created on: 13 Aug 2013
 *      Author: mike
 */

#ifndef IOBLOCK_H_
#define IOBLOCK_H_

#include "dspblock.h"

// Extended DspBlock for source/sink devices with common elements
// for selection of sub-device
class IOBlock : public DspBlock
{
public:
	IOBlock(
			DspData::Type intype, DspData::Type outtype,
			const string &name = "<undefined>",
			const string &blockType = "IOBlock");
	virtual ~IOBlock();

	const string& subdevice() const { return _subdevice; }
	const vector<string>& subdevices() const { return _subdevices; }

	void setSubdevice(const string &subdevice);
protected:
	vector<string> _subdevices; // should be filled by constructor
private:
	string _subdevice;
};

// Extended IOBlock exposes start, stop and run methods, and
// provides a common means for setting input block size
class SourceBlock : public IOBlock
{
public:
	SourceBlock(
			DspData::Type outtype,
			const string &name = "<undefined>",
			const string &blockType = "SourceBlock");
	virtual ~SourceBlock();

	bool start();
	void stop();
	bool run();

	unsigned int blockSize() { return _blockSize; }

	void setSampleRate(unsigned int rate);
	void setChannels(unsigned int channels);
	void setBlockSize(unsigned int size); // elements
private:
	unsigned int _blockSize;
	DspData	dummy;
};


#endif /* IOBLOCK_H_ */
