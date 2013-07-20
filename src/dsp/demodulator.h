/*
 * demodulator.h
 *
 *  Created on: 29 Jun 2013
 *      Author: mike
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
