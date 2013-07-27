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
