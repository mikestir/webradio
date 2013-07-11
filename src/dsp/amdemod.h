/*
 * am.h
 *
 *  Created on: 29 Jun 2013
 *      Author: mike
 */

#ifndef AMDEMOD_H_
#define AMDEMOD_H_

#include <string>

#include "dspblock.h"

using namespace std;

class AMDemod : public DspBlock
{
public:
	AMDemod(const string &name = "<undefined>");
	virtual ~AMDemod();
private:
	bool init();
	void deinit();
	bool process(const vector<sample_t> &inBuffer, vector<sample_t> &outBuffer);
};

#endif /* AMDEMOD_H_ */
