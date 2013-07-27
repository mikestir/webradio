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

#ifndef FILTER_H_
#define FILTER_H_

#include <vector>
#include <string>
#include <fftw3.h>

#include "dspblock.h"

using namespace std;

class LowPass : public DspBlock
{
public:
	LowPass(const string &name = "<undefined>");
	virtual ~LowPass();

	unsigned int passband() const { return _passband; }
	void setPassband(unsigned int hz);
	void setDecimation(unsigned int n);
	void setOutputSampleRate(unsigned int hz);

private:
	bool init();
	void deinit();
	bool process(const vector<sample_t> &inBuffer, vector<sample_t> &outBuffer);

	void recalculate();

	unsigned int	_firLength;

	/* Filter design */
	fftwf_complex 	*spec;
	fftwf_complex 	*impulse;
	fftwf_plan 		p;
	vector<float>	window;
	vector<float>	coeff;
	unsigned int	_passband;

	/* Filter operation */
	vector<sample_t>	block;
	unsigned int	_reqDecimation;
	unsigned int	_reqOutputRate;
};
#endif /* FILTER_H_ */
