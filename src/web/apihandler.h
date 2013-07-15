#ifndef APIHANDLER_H_
#define APIHANDLER_H_

#include <string>
#include <vector>
#include <map>

#include "httpserver.h"

class ApiHandler : public HttpRequestHandler
{
public:
	ApiHandler();
	~ApiHandler();
	static HttpRequestHandler *factory() {
		return new ApiHandler();
	}

	const string allows() { return "GET, PUT"; }

	unsigned short doGet(const vector<string> &wildcards, const vector<char> &requestData);
	unsigned short doPut(const vector<string> &wildcards, const vector<char> &requestData);
private:

};

#endif /* APIHANDLER_H_ */
