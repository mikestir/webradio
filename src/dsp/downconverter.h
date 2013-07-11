/*
 * downconverter.h
 *
 *  Created on: 28 Jun 2013
 *      Author: mike
 */

#ifndef DOWNCONVERTER_H_
#define DOWNCONVERTER_H_

#include <vector>
#include <string>

#include "dspblock.h"
#include "lowpass.h"

using namespace std;

class DownConverter : public DspBlock
{
public:
	DownConverter(const string &name = "<undefined>");
	virtual ~DownConverter();

	unsigned int bandwidth() const { return filter->passband(); }
	void setBandwidth(unsigned int hz) { filter->setPassband(hz); }
	unsigned int decimation() const { return filter->decimation(); }
	void setDecimation(unsigned int n) { filter->setDecimation(n); }
	int IF() const { return _if; }
	void setIF(int hz);

private:
	bool init();
	void deinit();
	bool process(const vector<sample_t> &inBuffer, vector<sample_t> &outBuffer);

	LowPass*	filter;
	int			_if;

	// NCO
	vector<float>	sinTable;
	unsigned int	phase;
	int				phaseStep;
};
#endif /* FILTER_H_ */
