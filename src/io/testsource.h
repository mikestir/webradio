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

#ifndef TESTSOURCE_H_
#define TESTSOURCE_H_

#include <vector>
#include <string>

#include "tuner.h"

class TestSource : public Tuner
{
public:
	TestSource(const string &name = "<undefined>");
	~TestSource();
	static Tuner* factory(const string &name = "<undefined>") {
		return new TestSource(name);
	}

protected:
	bool init();
	void deinit();
	bool process(const DspData &in, DspData &out);
	
	// NCO
	vector<int16_t>	sinTable;
	unsigned int	phase[2];
	int				phaseStep[2];
};

#endif /* TESTSOURCE_H_ */
