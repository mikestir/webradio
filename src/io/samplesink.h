/*
 * playbackdevice.h
 *
 *  Created on: 24 Jun 2013
 *      Author: mike
 */

#ifndef SAMPLESINK_H_
#define SAMPLESINK_H_

#include <vector>
#include <string>

#include "dspblock.h"

using namespace std;

class SampleSink : public DspBlock
{
public:
	SampleSink(
			const string &name = "<undefined>",
			const string &type = "SampleSink") :
			DspBlock(name, type),
			_subdevices(),
			_subdevice("") {}
	virtual ~SampleSink() {}

	const string& subdevice() const { return _subdevice; }
	const vector<string>& subdevices() const { return _subdevices; }

	void setSubdevice(const string &subdevice) {
		if (isRunning())
			return;
		_subdevice = subdevice;
	}
protected:
	vector<string> _subdevices;
private:
	string _subdevice;
};

#endif /* SAMPLESINK_H_ */
