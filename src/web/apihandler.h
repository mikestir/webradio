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
	~ApiHandler() {}
	static HttpRequestHandler *factory();

	unsigned short handleRequest(const string &method, const string &path,
			const vector<char> &requestData, unsigned short status);
private:

};

#endif /* APIHANDLER_H_ */
