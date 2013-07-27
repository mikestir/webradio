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

#ifndef DEMODULATOR_H_
#define DEMODULATOR_H_

#include <vector>
#include <string>

#include "dspblock.h"

using namespace std;

class Demodulator : public DspBlock
{
public:
	Demodulator(const string &name = "<undefined>");
	virtual ~Demodulator();

	enum Mode {
		AM,
		FM,
		USB,
		LSB,
		MAX_MODE
	};

	const Mode mode() const { return _mode; }
	void setMode(const Mode mode) { _mode = mode; }
	const string& modeString() const { return _modeStrings[_mode]; }
	bool setModeString(const string &mode);

private:
	bool init();
	void deinit();
	bool process(const vector<sample_t> &inBuffer, vector<sample_t> &outBuffer);

	Mode _mode;
	vector<string> _modeStrings;
	float prev_i;
	float prev_q;
};

#endif /* DEMODULATOR_H_ */
