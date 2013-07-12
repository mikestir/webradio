/*
 * lowpass.h
 *
 *  Created on: 28 Jun 2013
 *      Author: mike
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
