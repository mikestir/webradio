/*
 * capturedevice.h
 *
 *  Created on: 24 Jun 2013
 *      Author: mike
 */

#ifndef SAMPLESOURCE_H_
#define SAMPLESOURCE_H_

#include <vector>
#include <string>

#include "dspblock.h"

using namespace std;

class SampleSource : public DspSource
{
public:
	SampleSource(
			const string &name = "<undefined>",
			const string &type = "SampleSource") :
			DspSource(name, type),
			_subdevices(),
			_subdevice("") {}
	virtual ~SampleSource() {}

	const string& subdevice() const { return _subdevice; }
	const vector<string>& subdevices() const { return _subdevices; }

	void setSubdevice(const string &subdevice) {
		if (isRunning())
			return;
		_subdevice = subdevice;
	}

protected:
	vector<string> _subdevices; // should be filled by constructor
private:
	string _subdevice;
};

#endif /* SAMPLESOURCE_H_ */
