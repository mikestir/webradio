/*
 * randsource.h
 *
 *  Created on: 29 Jun 2013
 *      Author: mike
 */

#ifndef RANDSOURCE_H_
#define RANDSOURCE_H_

#include <vector>
#include <string>

#include "samplesource.h"

class RandSource : public SampleSource
{
public:
	RandSource(const string &name = "<undefined>");
	~RandSource();

protected:
	bool init();
	void deinit();
	bool process(const vector<sample_t> &inBuffer, vector<sample_t> &outBuffer);
};

#endif /* RANDSOURCE_H_ */
