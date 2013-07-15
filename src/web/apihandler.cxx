
#include <string>
#include <sstream>
#include <vector>
#include <cmath>

// FIXME: all C standard headers should be called as <cname> in C++
#include <stdlib.h>
#include "apihandler.h"
#include "debug.h"

#include "spectrumsink.h"
#include "tuner.h"
#include "lowpass.h"
#include "downconverter.h"

#define DEFAULT_DIR		"html"

extern SpectrumSink *spectrum;
extern Tuner* tuner;
extern DownConverter *dc1;

ApiHandler::ApiHandler()
{

}

HttpRequestHandler* ApiHandler::factory()
{
	return new ApiHandler();
}

unsigned short ApiHandler::handleRequest(const string &method, const vector<string> &wildcards,
	const vector<char> &requestData, unsigned short status)
{
	vector<float> magn;
	string path = wildcards[0];

	LOG_DEBUG("path = %s\n", path.c_str());

	if (method == "PUT") {
		string dat(requestData.begin(), requestData.end());
		if (path == "tuner") {
			tuner->setCentreFrequency(atoi(dat.c_str()));
			return MHD_HTTP_OK;
		}
		else if (path == "dc") {
			dc1->setIF(atoi(dat.c_str()));
			return MHD_HTTP_OK;
		}
#if 0
		else if (path == "filter") {
			filter->setPassband(atoi(dat.c_str()));
			return MHD_HTTP_OK;
		}
#endif

		return MHD_HTTP_NOT_FOUND;
	}

	if (method != "GET")
		return MHD_HTTP_METHOD_NOT_ALLOWED;

	magn.resize(spectrum->fftSize());
	spectrum->getSpectrum(magn.data());
	
	string out;
	if (path == "flot") {
		stringstream s;
		vector<float>::iterator it;
		s << "[";
		float step = (float)spectrum->inputSampleRate() / (float)spectrum->fftSize();
		float f = (float)tuner->centreFrequency() - (float)(spectrum->inputSampleRate() / 2);
		for (it = magn.begin(); it != magn.end(); it++) {
			s << "[" << f/1000000.0 << "," << *it << "]";
			if (it+1 != magn.end())
				s << ", ";
			f += step;
		}
		s << "]";
		out = s.str();
	} else if (path == "array") {
		stringstream s;
		vector<float>::iterator it;
		s << "[";
		for (it = magn.begin(); it != magn.end(); it++) {
			// inf and nan are not allowed in JSON so serialise as a big negative number
			if (isfinite(*it))
				s << *it;
			else
				s << "-999.9";
			if (it+1 != magn.end())
				s << ", ";
		}
		s << "]";
		out = s.str();
	}
		
	_contentType = "application/json";
	_data.insert(_data.end(), out.begin(), out.end());
	return MHD_HTTP_OK;
}
