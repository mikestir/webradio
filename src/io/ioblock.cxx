/*
 * ioblock.cxx
 *
 *  Created on: 13 Aug 2013
 *      Author: mike
 */

#include <vector>
#include <string>

#include "debug.h"
#include "ioblock.h"

#define DEFAULT_BLOCK_SIZE		16384

IOBlock::IOBlock(const string &name, const string &blockType) :
	DspBlock(name, blockType),
	_subdevices(),
	_subdevice("")
{

}

IOBlock::~IOBlock()
{

}

void IOBlock::setSubdevice(const string &subdevice)
{
	if (isRunning())
		return;

	LOG_DEBUG("Setting IO block %s:%s subdevice to %s\n",
		this->blockType().c_str(), this->name().c_str(), _subdevice.c_str());
	_subdevice = subdevice;
}

SourceBlock::SourceBlock(const string &name, const string &blockType) :
	IOBlock(name, blockType),
	_blockSize(DEFAULT_BLOCK_SIZE)
{

}

SourceBlock::~SourceBlock()
{

}

bool SourceBlock::start()
{
	return DspBlock::start();
}

void SourceBlock::stop()
{
	DspBlock::stop();
}

bool SourceBlock::run()
{
	return DspBlock::run(NULL, _blockSize);
}

void SourceBlock::setSampleRate(unsigned int rate)
{
	DspBlock::setSampleRate(rate);
}

void SourceBlock::setChannels(unsigned int channels)
{
	DspBlock::setChannels(channels);
}

void SourceBlock::setBlockSize(unsigned int size)
{
	if (isRunning())
		return;

	LOG_DEBUG("Setting source block %s:%s block size to %u frames\n",
		this->blockType().c_str(), this->name().c_str(), size);
	_blockSize = size;
}
